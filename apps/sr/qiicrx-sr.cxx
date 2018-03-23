// DCMTK includes
#include <dcmtk/config/osconfig.h>   // make sure OS specific configuration is included first
#include <dcmtk/ofstd/ofstream.h>
#include <dcmtk/ofstd/oftest.h>
#include <dcmtk/ofstd/ofstd.h>

#include <dcmtk/dcmsr/dsrdoc.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmiod/modhelp.h>
#include <dcmtk/dcmsr/dsrtypes.h>
#include <dcmtk/dcmsr/codes/dcm.h>

// STD include
#include <iostream>
#include <exception>

// DCMQI includes
#include "dcmqi/Exceptions.h"
#include "dcmqi/QIICRConstants.h"
#include "dcmqi/QIICRUIDs.h"
#include "dcmqi/internal/VersionConfigure.h"
#include "dcmqi/Helper.h"
#include "dcmqi/TIDQIICRXReader.h"

#include <json/json.h>

#include <dcmqi/tidqiicrx.h>

#include "qiicrx-srCLP.h"

int main(int argc, char** argv){
  PARSE_ARGS;

  if(inputDICOM.size()) {
    // read DICOM, write JSON

    if(!metaDataFileName.size()){
      std::cerr << "Error: output metadata file is required when input is a DICOM file!" << std::endl;
      return -1;
    }

    DcmFileFormat srFF;
    CHECK_COND(srFF.loadFile(inputDICOM.c_str()));
    DcmDataset& dataset = *srFF.getDataset();

    DSRDocument doc;
    if(doc.read(dataset).good()){
      Json::Value metaRoot;
      TIDQIICRXReader reader(doc.getTree());
      metaRoot["imageLibrary"] = reader.getImageLibrary();

      ofstream outputFile;
      outputFile.open(metaDataFileName.c_str());
      outputFile << metaRoot;
    } else {
      std::cerr << "Error: Failed to read input DICOM document" << std::endl;
    }

    return 0;
  } else {
    if(!compositeContextDataDir.size() ||
     !metaDataFileName.size() ||
     !outputFileName.size() ||
     !imageLibraryDataDir.size()){
       std::cerr << "Error: required arguments are missing!" << std::endl;
       return -1;
     }
  }

  // read metaData
  Json::Value metaRoot;
  {
    try {
      std::ifstream metainfoStream(metaDataFileName.c_str(), std::ifstream::binary);
      metainfoStream >> metaRoot;
    } catch (std::exception& e) {
      std::cout << e.what() << '\n';
      return -1;
    }
  }

  TIDQIICRX_PIRADS report(DSRCodedEntryValue("009001","99QIICR","Prostate mpMRI PI-RADS report"));
  report.setLanguage(DSRCodedEntryValue("eng", "RFC5646", "English"));
  report.getObservationContext().addPersonObserver("Foo^Bar");

  // Initialize Image library
  CHECK_COND(report.getImageLibrary().createNewImageLibrary());

  if(metaRoot.isMember("imageLibrary")){
    // each entry in imageLibrary corresponds to an individual series used in PI-RADS
    //  interpretation. For each entry, it is expected that there are "piradsSeriesType" and
    //  "inputDICOMDirectory" attributes defined.
    for(Json::ArrayIndex i=0;i<metaRoot["imageLibrary"].size();i++){

      DcmFileFormat ff;
      OFString dicomFilePath;
      OFList<OFString> fileList;
      OFString seriesDir;

      Json::Value libraryItem = metaRoot["imageLibrary"][i];

      OFStandard::combineDirAndFilename(seriesDir,imageLibraryDataDir.c_str(),libraryItem["inputDICOMDirectory"].asCString());

      OFStandard::searchDirectoryRecursively(seriesDir,fileList,"","",OFFalse);
      // for each item identified, add it to a separate imageLibrary group
      CHECK_COND(report.getImageLibrary().addImageGroup());
      while(!fileList.empty()){
        OFString imageLibraryItemFile = fileList.front();
        fileList.pop_front();
        CHECK_COND(ff.loadFile(imageLibraryItemFile));
        CHECK_COND(report.getImageLibrary().addImageEntry(*ff.getDataset(),
          TID1600_ImageLibrary::withAllDescriptors));
      }
      // factor out common attributes of the individual entries up to the group level
      CHECK_COND(report.getImageLibrary().moveCommonImageDescriptorsToImageGroups());
    }
  }

  // This call will factor out all of the common entries at the group level
  CHECK_COND(report.getImageLibrary().moveCommonImageDescriptorsToImageGroups());

  DcmFileFormat ff;
  DcmDataset *dataset = ff.getDataset();

  OFString contentDate, contentTime;
  DcmDate::getCurrentDate(contentDate);
  DcmTime::getCurrentTime(contentTime);

  DSRDocument doc;
  doc.setTreeFromRootTemplate(report, OFTrue /*expandTree*/);

  // iterate over annotated nodes of TID1602 and add codes defining series typed
  {
    DSRDocumentTree &st = doc.getTree();
    size_t nnid = st.gotoAnnotatedNode("TID 1602 - Row 1");
    unsigned cnt = 0;
    while(nnid){
      Json::Value libraryItem = metaRoot["imageLibrary"][cnt];
      CHECK_COND(st.addContentItem(DSRTypes::RT_hasAcqContext, DSRTypes::VT_Code, DSRCodedEntryValue("991000","99QIICR","Prostate mpMRI acquisition type"), OFTrue));
      DSRCodedEntryValue seriesType (libraryItem["piradsSeriesType"]["CodeValue"].asCString(),
        libraryItem["piradsSeriesType"]["CodingSchemeDesignator"].asCString(),
        libraryItem["piradsSeriesType"]["CodeMeaning"].asCString());
      CHECK_COND(st.getCurrentContentItem().setCodeValue(seriesType, OFTrue));
      nnid = st.gotoNextAnnotatedNode("TID 1602 - Row 1");
      cnt++;
    }
  }

  doc.setSeriesDate(contentDate.c_str());
  doc.setSeriesTime(contentTime.c_str());

  doc.write(*dataset);

  // initialize composite context
  {
    OFString ccFullPath;

    OFStandard::combineDirAndFilename(ccFullPath,compositeContextDataDir.c_str(),metaRoot["compositeContext"].asCString());

    DcmFileFormat ccFileFormat;
    ccFileFormat.loadFile(ccFullPath);
    DcmModuleHelpers::copyPatientModule(*ccFileFormat.getDataset(),*dataset);
    DcmModuleHelpers::copyPatientStudyModule(*ccFileFormat.getDataset(),*dataset);
    DcmModuleHelpers::copyGeneralStudyModule(*ccFileFormat.getDataset(),*dataset);
  }

  ff.saveFile(outputFileName.c_str(), EXS_LittleEndianExplicit);

  return 0;
}

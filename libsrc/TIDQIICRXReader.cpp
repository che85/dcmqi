#include "dcmqi/TIDQIICRXReader.h"


Json::Value DSRCodedEntryValue2CodeSequence(const DSRCodedEntryValue &value) {
  Json::Value codeSequence;
  codeSequence["CodeValue"] = value.getCodeValue().c_str();
  codeSequence["CodeMeaning"] = value.getCodeMeaning().c_str();
  codeSequence["CodingSchemeDesignator"] = value.getCodingSchemeDesignator().c_str();
  return codeSequence;
}

TIDQIICRXReader::TIDQIICRXReader(const DSRDocumentTree &tree)
  : DSRDocumentTree(tree) {
    // check for expected template identification
    if (!compareTemplateIdentification("QIICRX", "99QIICR"))
      std::cerr << "warning: template identification \"TID QIICRX (99QIICR)\" not found" << OFendl;

}

/*
This function initializes imageLibrary item of the JSON metadata, as defined by the dcmqi
converter. It will not extract all of the attributes, but only the acquisition type, and the list
of all instances for the images that belong to each of the image library groups.
*/
Json::Value TIDQIICRXReader::getImageLibrary(){
  Json::Value imageLibrary(Json::arrayValue);

  //DSRDocumentTreeNodeCursor cursor(getCursor());

  // iterate over the document tree and read the measurements
  if (gotoNamedNode(CODE_DCM_ImageLibrary)) {
    if(gotoNamedChildNode(CODE_DCM_ImageLibraryGroup)) {
      do {
        Json::Value imageLibraryGroup;
        // remember cursor to current content item (as a starting point)
        //if(gotoNamedChildNode(DSRCodedEntryValue("991000","99QICR","Prostate mpMRI acquisition type")))
        DSRDocumentTreeNodeCursor groupCursor(getCursor());
        Json::Value groupItem;
        groupItem["piradsSeriesType"]= getContentItem(DSRCodedEntryValue("991000","99QIICR","Prostate mpMRI acquisition type"), groupCursor);
        // find all image items that are included
        Json::Value groupInstances(Json::arrayValue);
        if(groupCursor.gotoChild()){
          const DSRDocumentTreeNode *node;
          do{
            node = groupCursor.getNode();
            if(node!=NULL && node->getValueType() == VT_Image){
              groupInstances.append(OFstatic_cast(
              const DSRImageTreeNode *, node)->getValue().getSOPInstanceUID().c_str());
            }
          } while(groupCursor.gotoNext());
        }
        groupItem["instanceUIDs"] = groupInstances;
        imageLibrary.append(groupItem);

      } while (gotoNextNamedNode(CODE_DCM_ImageLibraryGroup, OFFalse /*searchIntoSub*/));
    }
  }
  return imageLibrary;
}

Json::Value TIDQIICRXReader::getContentItem(const DSRCodedEntryValue &conceptName,
                                          DSRDocumentTreeNodeCursor cursor)
{
  Json::Value contentValue;
  // try to go to the given content item
  if (gotoNamedChildNode(conceptName, cursor)) {
    const DSRDocumentTreeNode *node = cursor.getNode();
    if (node != NULL) {
      //COUT << "- " << conceptName.getCodeMeaning() << ": ";
      // use appropriate value for output
      switch (node->getValueType()) {
        case VT_Text:
          contentValue = OFstatic_cast(
          const DSRTextTreeNode *, node)->getValue().c_str();
          break;
        case VT_UIDRef:
          contentValue = OFstatic_cast(
          const DSRUIDRefTreeNode *, node)->getValue().c_str();
          break;
        case VT_Code:
          contentValue = DSRCodedEntryValue2CodeSequence(OFstatic_cast(
          const DSRCodeTreeNode *, node)->getValue());
          break;
        case VT_Image:
          contentValue = OFstatic_cast(
          const DSRImageTreeNode *, node)->getValue().getSOPInstanceUID().c_str();
          break;
        default:
          std::cout << "Error: failed to find content item" << OFendl;
      }
    }
  }
  return contentValue;
}

size_t TIDQIICRXReader::gotoNamedChildNode(const DSRCodedEntryValue &conceptName,
                          DSRDocumentTreeNodeCursor &cursor)
{
  size_t nodeID = 0;
  // goto the first child node
  if (conceptName.isValid() && cursor.gotoChild()) {
    const DSRDocumentTreeNode *node;
    // iterate over all nodes on this level
    do {
      node = cursor.getNode();
      // and check for the desired concept name
      if ((node != NULL) && (node->getConceptName() == conceptName))
        nodeID = node->getNodeID();
    } while ((nodeID == 0) && cursor.gotoNext());
  }
  return nodeID;
}

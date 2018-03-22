#ifndef DCMQI_TIDQIICRXREADER_H
#define DCMQI_TIDQIICRXREADER_H

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcmtk/dcmsr/dsrdoc.h"
#include "dcmtk/dcmsr/dsrcodtn.h"
#include "dcmtk/dcmsr/dsrimgtn.h"
#include "dcmtk/dcmsr/dsrnumtn.h"
#include "dcmtk/dcmsr/dsrtextn.h"
#include "dcmtk/dcmsr/dsruidtn.h"

#include "dcmtk/dcmsr/codes/dcm.h"
#include "dcmtk/dcmsr/codes/ncit.h"
#include "dcmtk/dcmsr/codes/srt.h"
#include "dcmtk/dcmsr/codes/umls.h"
#include <dcmtk/dcmsr/dsruidtn.h>

#include "dcmtk/dcmdata/dcfilefo.h"

#include <json/json.h>


// Based on the code provided by @jriesmeier, see
//  https://gist.github.com/fedorov/41e42c1e701d74b2391792241809fe62

class TIDQIICRXReader
  : DSRDocumentTree
{
  public:
    TIDQIICRXReader(const DSRDocumentTree &tree);

    Json::Value getContentItem(const DSRCodedEntryValue &conceptName,
                               DSRDocumentTreeNodeCursor cursor);

    Json::Value getImageLibrary();

    using DSRDocumentTree::gotoNamedChildNode;

  protected:

    size_t gotoNamedChildNode(const DSRCodedEntryValue &conceptName,
                              DSRDocumentTreeNodeCursor &cursor);
};

#endif // DCMQI_TID1500READER_H

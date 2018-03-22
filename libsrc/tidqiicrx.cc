/*
 *
 *  Copyright (C) 2015-2017, J. Riesmeier, Oldenburg, Germany
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  Source file for class TID1500_MeasurementReport
 *
 *  Author: Joerg Riesmeier
 *
 */


#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcmtk/dcmsr/cmr/tid15def.h"
#include "dcmtk/dcmsr/cmr/logger.h"
#include "dcmtk/dcmsr/codes/dcm.h"
#include "dcmtk/dcmsr/codes/umls.h"
#include "dcmtk/dcmsr/dsrtpltn.h"

#include "dcmtk/dcmdata/dcuid.h"

#include <dcmqi/QIICRUIDs.h>
#include <dcmqi/tidqiicrx.h>

// helper macros for checking the return value of API calls
#define CHECK_RESULT(call) if (result.good()) result = call
#define STORE_RESULT(call) result = call
#define GOOD_RESULT(call) if (result.good()) call
#define BAD_RESULT(call) if (result.bad()) call

// index positions in node list (makes source code more readable)
#define MEASUREMENT_REPORT               0
#define OBSERVATION_CONTEXT              1
#define LAST_PROCEDURE_REPORTED          2
#define IMAGING_MEASUREMENTS             3
#define LAST_VOLUMETRIC_ROI_MEASUREMENTS 4
#define LAST_MEASUREMENT_GROUP           5
#define QUALITATIVE_EVALUATIONS          6
#define NUMBER_OF_LIST_ENTRIES           7

// general information on TID 1500 (Measurement Report)
// TODO: replace those with the valid temporary ones!
#define TEMPLATE_NUMBER      "QIICRX"
#define MAPPING_RESOURCE     "99QIICR"
#define MAPPING_RESOURCE_UID QIICR_CODING_SCHEME_UID_ROOT
#define TEMPLATE_TYPE        OFTrue   /* extensible */
#define TEMPLATE_ORDER       OFFalse  /* non-significant */


TIDQIICRX_PIRADS::TIDQIICRX_PIRADS(const CID7021_MeasurementReportDocumentTitles &title,
                                                     const OFBool check)
  : DSRRootTemplate(DT_EnhancedSR, TEMPLATE_NUMBER, MAPPING_RESOURCE, MAPPING_RESOURCE_UID),
    Language(new TID1204_LanguageOfContentItemAndDescendants()),
    ObservationContext(new TID1001_ObservationContext()),
    ImageLibrary(new TID1600_ImageLibrary())
{
    setExtensible(TEMPLATE_TYPE);
    setOrderSignificant(TEMPLATE_ORDER);
    /* need to store position of various content items */
    reserveEntriesInNodeList(NUMBER_OF_LIST_ENTRIES, OFTrue /*initialize*/);
    /* if specified, create an initial report */
    if (title.hasSelectedValue())
        createMeasurementReport(title, check);
}


void TIDQIICRX_PIRADS::clear()
{
    DSRRootTemplate::clear();
    Language->clear();
    ObservationContext->clear();
    ImageLibrary->clear();
}


OFBool TIDQIICRX_PIRADS::isValid() const
{
    /* check whether base class is valid and all required content items are present */
    return DSRRootTemplate::isValid() &&
        Language->isValid() && ObservationContext->isValid() && ImageLibrary->isValid() &&
        hasProcedureReported();
}


OFBool TIDQIICRX_PIRADS::hasProcedureReported() const
{
    /* check for content item at TID 1500 (Measurement Report) Row 4 */
    return (getEntryFromNodeList(LAST_PROCEDURE_REPORTED) > 0);
}


OFCondition TIDQIICRX_PIRADS::getDocumentTitle(DSRCodedEntryValue &titleCode)
{
    OFCondition result = EC_Normal;
    /* go to content item at TID 1500 (Measurement Report) Row 1 */
    if (gotoEntryFromNodeList(this, MEASUREMENT_REPORT) > 0)
    {
        titleCode = getCurrentContentItem().getConceptName();
    } else {
        /* in case of error, clear the result variable */
        titleCode.clear();
        result = SR_EC_ContentItemNotFound;
    }
    return result;
}


OFCondition TIDQIICRX_PIRADS::setLanguage(const CID5000_Languages &language,
                                                   const CID5001_Countries &country,
                                                   const OFBool check)
{
    /* TID 1500 (Measurement Report) Row 2 */
    return getLanguage().setLanguage(language, country, check);
}


OFCondition TIDQIICRX_PIRADS::addProcedureReported(const CID100_QuantitativeDiagnosticImagingProcedures &procedure,
                                                            const OFBool check)
{
    OFCondition result = EC_IllegalParameter;
    /* make sure that there is a coded entry */
    if (procedure.hasSelectedValue())
    {
        /* go to last content item at TID 1500 (Measurement Report) Row 3 or 4 */
        if (gotoLastEntryFromNodeList(this, LAST_PROCEDURE_REPORTED, OBSERVATION_CONTEXT) > 0)
        {
            /* TID 1500 (Measurement Report) Row 4 */
            STORE_RESULT(addContentItem(RT_hasConceptMod, VT_Code, CODE_DCM_ProcedureReported, check));
            CHECK_RESULT(getCurrentContentItem().setCodeValue(procedure, check));
            CHECK_RESULT(getCurrentContentItem().setAnnotationText("TID 1500 - Row 4"));
            /* store ID of recently added node for later use */
            GOOD_RESULT(storeEntryInNodeList(LAST_PROCEDURE_REPORTED, getNodeID()));
        } else
            result = CMR_EC_NoMeasurementReport;
    }
    return result;
}


// protected methods

OFCondition TIDQIICRX_PIRADS::createMeasurementReport(const CID7021_MeasurementReportDocumentTitles &title,
                                                               const OFBool check)
{
    OFCondition result = EC_IllegalParameter;
    /* make sure that there is a coded entry */
    if (title.hasSelectedValue())
    {
        /* reassure that the report is definitely empty */
        if (isEmpty())
        {
            /* TID 1500 (Measurement Report) Row 1 */
            STORE_RESULT(addContentItem(RT_isRoot, VT_Container, title, check));
            CHECK_RESULT(getCurrentContentItem().setAnnotationText("TID QIICRX - Row 1"));
            /* store ID of root node for later use */
            GOOD_RESULT(storeEntryInNodeList(MEASUREMENT_REPORT, getNodeID()));
            /* TID 1500 (Measurement Report) Row 2 */
            CHECK_RESULT(includeTemplate(Language, AM_belowCurrent, RT_hasConceptMod));
            CHECK_RESULT(getCurrentContentItem().setAnnotationText("TID QIICRX - Row 2"));
            /* TID 1500 (Measurement Report) Row 3 */
            CHECK_RESULT(includeTemplate(ObservationContext, AM_afterCurrent, RT_hasObsContext));
            CHECK_RESULT(getCurrentContentItem().setAnnotationText("TID QIICRX - Row 3"));
            GOOD_RESULT(storeEntryInNodeList(OBSERVATION_CONTEXT, getNodeID()));
            /* TID 1500 (Measurement Report) Row 5 */
            CHECK_RESULT(includeTemplate(ImageLibrary, AM_afterCurrent, RT_contains));
            CHECK_RESULT(getCurrentContentItem().setAnnotationText("TID QIICRX - Row 5"));
            /* if anything went wrong, clear the report */
            BAD_RESULT(clear());
        } else
            result = SR_EC_InvalidTemplateStructure;
    }
    return result;
}

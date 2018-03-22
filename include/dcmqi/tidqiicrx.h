/*
 *
 *  Copyright (C) 2015-2017, J. Riesmeier, Oldenburg, Germany
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  Header file for class TID1500_MeasurementReport
 *
 *  Author: Joerg Riesmeier
 *
 */


#ifndef CMR_TIDQIICRX_PIRADS_H
#define CMR_TIDQIICRX_PIRADS_H

#include "dcmtk/config/osconfig.h"   /* make sure OS specific configuration is included first */

#include "dcmtk/dcmsr/dsrrtpl.h"
#include "dcmtk/dcmsr/dsrstpl.h"

#include "dcmtk/dcmsr/cmr/define.h"
#include "dcmtk/dcmsr/cmr/tid1001.h"
#include "dcmtk/dcmsr/cmr/tid1204.h"
#include "dcmtk/dcmsr/cmr/tid1411.h"
#include "dcmtk/dcmsr/cmr/tid1501.h"
#include "dcmtk/dcmsr/cmr/tid1600.h"
#include "dcmtk/dcmsr/cmr/cid100.h"
#include "dcmtk/dcmsr/cmr/cid6147.h"
#include "dcmtk/dcmsr/cmr/cid7021.h"
#include "dcmtk/dcmsr/cmr/cid7181.h"
#include "dcmtk/dcmsr/cmr/cid7464.h"
#include "dcmtk/dcmsr/cmr/cid7469.h"


// include this file in doxygen documentation

/** @file tid1500.h
 *  @brief Interface class for TID 1500 in module dcmsr/cmr
 */


/*------------------------*
 *  constant definitions  *
 *------------------------*/

/** @name specific error conditions for TID 1500 (and included templates) in module dcmsr/cmr
 */
//@{

/// error: there is no measurement report to add content items to
extern DCMTK_CMR_EXPORT const OFConditionConst CMR_EC_NoMeasurementReport;
/// error: there is no measurement group to add entries to
extern DCMTK_CMR_EXPORT const OFConditionConst CMR_EC_NoMeasurementGroup;
/// error: the given segmentation object does not conform to the template constraints
extern DCMTK_CMR_EXPORT const OFConditionConst CMR_EC_InvalidSegmentationObject;
/// error: the given DICOM object is not a real world value mapping object
extern DCMTK_CMR_EXPORT const OFConditionConst CMR_EC_InvalidRealWorldValueMappingObject;

//@}


/*---------------------*
 *  class declaration  *
 *---------------------*/

/** Implementation of DCMR Template:
 *  TID 1500 - Measurement Report (and included templates 1204, 1001, 1600, 1411, 1501).
 *  All added content items are annotated with a text in the format "TID 1500 - Row [n]".
 ** @note Please note that currently only the mandatory and some optional/conditional
 *        content items and included templates are supported.
 */
class DCMTK_CMR_EXPORT TIDQIICRX_PIRADS
  : public DSRRootTemplate
{

  public:

    /** (default) constructor.
     *  Also creates an initial, almost empty measurement report by calling
     *  createNewMeasurementReport(), but only if a non-empty 'title' is passed.
     ** @param  title  optional document title to be set (from CID 7021 - Measurement
     *                 Report Document Titles), i.e.\ the concept name of the root node
     *  @param  check  if enabled, check value for validity before setting it
     */
    TIDQIICRX_PIRADS(const CID7021_MeasurementReportDocumentTitles &title = CID7021_MeasurementReportDocumentTitles(),
                              const OFBool check = OFTrue);

    /** clear internal member variables.
     *  Also see notes on the clear() method of the base class.
     */
    virtual void clear();

    /** check whether the current internal state is valid.
     *  That means, check whether the base class is valid, the mandatory included
     *  templates TID 1204, 1001 and 1600 are valid, and whether hasProcedureReported()
     *  as well as hasImagingMeasurements() or hasQualitativeEvaluations() return true.
     *  In addition, each of the included templates TID 1411 and 1501 should either be
     *  empty or valid.
     ** @return OFTrue if valid, OFFalse otherwise
     */
    virtual OFBool isValid() const;

    /** check whether there are any 'Procedure reported' content items (TID 1500 - Row 4)
     *  in this measurement report.  This content item is mandatory, i.e. one or more
     *  instances of the associated content item should be present.
     ** @return OFTrue if at least one procedure reported is present, OFFalse otherwise
     */
    OFBool hasProcedureReported() const;

    /** get language of this report as defined by TID 1204 (Language of Content Item and
     *  Descendants).  This included template is mandatory, i.e. should not be empty.
     ** @return reference to internally managed SR template
     */
    inline TID1204_LanguageOfContentItemAndDescendants &getLanguage() const
    {
        return *OFstatic_cast(TID1204_LanguageOfContentItemAndDescendants *, Language.get());
    }

    /** get observation context of this report as defined by TID 1001 (Observation
     *  Context).  This included template is mandatory, i.e. should not be empty.
     ** @return reference to internally managed SR template
     */
    inline TID1001_ObservationContext &getObservationContext() const
    {
        return *OFstatic_cast(TID1001_ObservationContext *, ObservationContext.get());
    }

    /** get image library of this report as defined by TID 1600 (Image Library).
     *  This included template is mandatory, i.e. should not be empty.
     ** @return reference to internally managed SR template
     */
    inline TID1600_ImageLibrary &getImageLibrary() const
    {
        return *OFstatic_cast(TID1600_ImageLibrary *, ImageLibrary.get());
    }

    /** get document title of this report, i.e.\ the concept name of the root node
     ** @param  titleCode  coded entry that specifies the document title of this report
     ** @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition getDocumentTitle(DSRCodedEntryValue &titleCode);

    /** set language of this report as defined by TID 1204 (Language of Content Item and
     *  Descendants)
     ** @param  language  language of the report, being a language that is primarily
     *                    used for human communication (from CID 5000 - Languages)
     *  @param  country   coded entry that describes the country-specific variant of
     *                    'language' (optional, from CID 5001 - Countries)
     *  @param  check     if enabled, check values for validity before setting them
     ** @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition setLanguage(const CID5000_Languages &language,
                            const CID5001_Countries &country = CID5001_Countries(),
                            const OFBool check = OFTrue);

    /** add the imaging procedure whose results are reported (TID 1500 - Row 4).
     *  There should be at least a single instance of the associated content item.
     ** @param  procedure  coded entry that describes the imaging procedure to be added
     *                     (from CID 100 - Quantitative Diagnostic Imaging Procedures)
     *  @param  check      if enabled, check value for validity before setting it
     ** @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition addProcedureReported(const CID100_QuantitativeDiagnosticImagingProcedures &procedure,
                                     const OFBool check = OFTrue);

  protected:

    /** create the mandatory (and other supported) content items of this template,
     *  i.e.\ TID 1500 - Row 1 to 6 and 8.  It is expected that the tree is currently
     *  empty.
     ** @param  title  coded entry that specifies the document title to be set
     *  @param  check  if enabled, check value for validity before setting it
     ** @return status, EC_Normal if successful, an error code otherwise
     */
    OFCondition createMeasurementReport(const CID7021_MeasurementReportDocumentTitles &title,
                                        const OFBool check);

  private:

    // shared pointer to included template "Language of Content Item and Descendants" (TID 1204)
    DSRSharedSubTemplate Language;
    // shared pointer to included template "Observation Context" (TID 1001)
    DSRSharedSubTemplate ObservationContext;
    // shared pointer to included template "Image Library" (TID 1600)
    DSRSharedSubTemplate ImageLibrary;
};

#endif

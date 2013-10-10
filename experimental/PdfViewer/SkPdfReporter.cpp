/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPdfReporter.h"

#include "SkTypes.h"

const char* severityName[] = {
"Info",
"CodeWarning",
"Warning",
"IgnoreError",
"Error",
"FatalError",
};

const char* getSeverityName(SkPdfIssueSeverity sev) {
    if (0 <= sev && sev < _kCount__SkPdfIssueSeverity) {
        return severityName[sev];
    }
    SkASSERT(false);
    return "UNKOWN SEVERITY";
}

// TODO(edisonn): add a flag to set the minimum warning level

#ifdef PDF_REPORT
void SkPdfReport(SkPdfIssueSeverity sev, SkPdfIssue issue,
                 const char* context,
                 const SkPdfNativeObject* obj,
                 SkPdfContext* pdfContext) {
    if (sev >= kIgnoreError_SkPdfIssueSeverity) {
        printf("%s: %s\n", getSeverityName(sev), context);
    }
}

void SkPdfReportIf(bool report,
                   SkPdfIssueSeverity sev, SkPdfIssue issue,
                   const char* context,
                   const SkPdfNativeObject* obj,
SkPdfContext* pdfContext) {
    if (!report) {
        return;
    }
    SkPdfReport(sev, issue, context, obj, pdfContext);
}

void SkPdfReportUnexpectedType(SkPdfIssueSeverity sev,
                               const char* context,
                               const SkPdfNativeObject* obj,
                               int anyOfTypes, SkPdfContext* pdfContext) {
    if (sev >= kIgnoreError_SkPdfIssueSeverity) {
        printf("%s: %s\n", getSeverityName(sev), context);
    }
}

#endif  // PDF_REPORT

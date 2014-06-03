#ifndef DMPDFTask_DEFINED
#define DMPDFTask_DEFINED

#include "DMPDFRasterizeTask.h"
#include "DMExpectations.h"
#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

namespace DM {

// This task renders a GM using Skia's PDF backend.
// If rasterizePdfProc is non-NULL, it will spawn a PDFRasterizeTask.
class PDFTask : public CpuTask {
public:
    PDFTask(const char* suffix,
            Reporter*,
            TaskRunner*,
            const Expectations&,
            skiagm::GMRegistry::Factory,
            RasterizePdfProc);

    virtual void draw() SK_OVERRIDE;

    virtual bool shouldSkip() const SK_OVERRIDE;

    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    SkAutoTDelete<skiagm::GM> fGM;
    const SkString fName;
    const Expectations& fExpectations;
    RasterizePdfProc fRasterize;
};

}  // namespace DM

#endif  // DMPDFTask_DEFINED

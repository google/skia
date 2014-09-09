#ifndef DMPDFTask_DEFINED
#define DMPDFTask_DEFINED

#include "DMPDFRasterizeTask.h"
#include "DMTask.h"
#include "SkBitmap.h"
#include "SkPicture.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

namespace DM {

// This task renders a GM or SKP using Skia's PDF backend.
// If rasterizePdfProc is non-NULL, it will spawn a PDFRasterizeTask.
class PDFTask : public CpuTask {
public:
    PDFTask(const char*,
            Reporter*,
            TaskRunner*,
            skiagm::GMRegistry::Factory,
            RasterizePdfProc);

    PDFTask(Reporter*,
            TaskRunner*,
            const SkPicture*,
            SkString name,
            RasterizePdfProc);

    virtual void draw() SK_OVERRIDE;

    virtual bool shouldSkip() const SK_OVERRIDE;

    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    // One of these two will be set.
    SkAutoTDelete<skiagm::GM> fGM;
    SkAutoTUnref<const SkPicture> fPicture;

    const SkString fName;
    RasterizePdfProc fRasterize;
};

}  // namespace DM

#endif  // DMPDFTask_DEFINED

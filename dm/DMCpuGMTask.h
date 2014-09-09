#ifndef DMCpuGMTask_DEFINED
#define DMCpuGMTask_DEFINED

#include "DMReporter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

// This is the main entry point for drawing GMs with the CPU.  Commandline
// flags control whether this kicks off various comparison tasks when done.

namespace DM {

class CpuGMTask : public CpuTask {
public:
    CpuGMTask(const char* config,
              Reporter*,
              TaskRunner*,
              skiagm::GMRegistry::Factory,
              SkColorType);

    virtual void draw() SK_OVERRIDE;
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    skiagm::GMRegistry::Factory fGMFactory;
    SkAutoTDelete<skiagm::GM> fGM;
    const SkString fName;
    const SkColorType fColorType;
};

}  // namespace DM

#endif // DMCpuGMTask_DEFINED

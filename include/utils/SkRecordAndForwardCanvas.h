
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"

/** \class SkRecordAndForwardCanvas
  A Canvas which records all public API calls made to SkCanvas
  compare to SkRecorder, which records the virtual method calls
 */
class SkRecordAndForwardCanvas : public SkCanvas{
public:
  void flush() override;
  void drawColor(SkColor color, SkBlendMode mode = SkBlendMode::kSrcOver) override;
};

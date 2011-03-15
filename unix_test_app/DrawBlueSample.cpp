#include "SkCanvas.h"
#include "SkColor.h"
#include "SampleCode.h"
#include "SkView.h"

class DrawBlue : public SkView {

public:
     DrawBlue() {}
protected:
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorBLUE);
    }
};

static SkView* MyFactory() { return new DrawBlue; }
static SkViewRegister reg(MyFactory);

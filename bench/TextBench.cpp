#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkFontHost.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkSfntUtils.h"
#include "SkString.h"
#include "SkTemplates.h"

/*  Some considerations for performance:
        short -vs- long strings (measuring overhead)
        tiny -vs- large pointsize (measure blit -vs- overhead)
        1 -vs- many point sizes (measure cache lookup)
        normal -vs- subpixel -vs- lineartext (minor)
        force purge after each draw to measure scaler
        textencoding?
        text -vs- postext - pathtext
 */
class TextBench : public SkBenchmark {
    SkPaint     fPaint;
    int         fCount;
    SkPoint*    fPos;
    SkString    fText;
    SkString    fName;
    enum { N = 600 };
public:
    TextBench(void* param, const char text[], int ps, bool linearText,
              bool posText, SkColor color = SK_ColorBLACK) : INHERITED(param) {
        fText.set(text);

        fPaint.setAntiAlias(true);
        fPaint.setTextSize(SkIntToScalar(ps));
        fPaint.setLinearText(linearText);
        fPaint.setColor(color);

        if (posText) {
            SkAutoTArray<SkScalar> storage(fText.size());
            SkScalar* widths = storage.get();
            fCount = fPaint.getTextWidths(fText.c_str(), fText.size(), widths);
            fPos = new SkPoint[fCount];
            SkScalar x = 0;
            for (int i = 0; i < fCount; i++) {
                fPos[i].set(x, 0);
                x += widths[i];
            }
        } else {
            fCount = 0;
            fPos = NULL;
        }
    }

    virtual ~TextBench() {
        delete[] fPos;
    }

protected:
    virtual const char* onGetName() {
        fName.printf("text_%g", SkScalarToFloat(fPaint.getTextSize()));
        if (fPaint.isLinearText()) {
            fName.append("_linear");
        }
        if (fPos) {
            fName.append("_pos");
        }

        if (SK_ColorBLACK != fPaint.getColor()) {
            fName.appendf("_%02X", fPaint.getAlpha());
        }
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        const SkIPoint dim = this->getSize();
        SkRandom rand;

        SkPaint paint(fPaint);
        this->setupPaint(&paint);
        paint.setColor(fPaint.getColor());  // need our specified color

        const SkScalar x0 = SkIntToScalar(-10);
        const SkScalar y0 = SkIntToScalar(-10);

        for (int i = 0; i < N; i++) {
            SkScalar x = x0 + rand.nextUScalar1() * dim.fX;
            SkScalar y = y0 + rand.nextUScalar1() * dim.fY;
            if (fPos) {
                canvas->save(SkCanvas::kMatrix_SaveFlag);
                canvas->translate(x, y);
                canvas->drawPosText(fText.c_str(), fText.size(), fPos, paint);
                canvas->restore();
            } else {
                canvas->drawText(fText.c_str(), fText.size(), x, y, paint);
            }
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

#define STR     "Hamburgefons"
#define SMALL   9
#define BIG     48

static SkBenchmark* Fact0(void* p) { return new TextBench(p, STR, SMALL, false, false); }
static SkBenchmark* Fact01(void* p) { return new TextBench(p, STR, SMALL, false, false, 0xFFFF0000); }
static SkBenchmark* Fact02(void* p) { return new TextBench(p, STR, SMALL, false, false, 0x88FF0000); }

static SkBenchmark* Fact1(void* p) { return new TextBench(p, STR, SMALL, false, true); }
static SkBenchmark* Fact2(void* p) { return new TextBench(p, STR, SMALL, true, false); }
static SkBenchmark* Fact3(void* p) { return new TextBench(p, STR, SMALL, true, true); }

static SkBenchmark* Fact4(void* p) { return new TextBench(p, STR, BIG, false, false); }
static SkBenchmark* Fact41(void* p) { return new TextBench(p, STR, BIG, false, false, 0xFFFF0000); }
static SkBenchmark* Fact42(void* p) { return new TextBench(p, STR, BIG, false, false, 0x88FF0000); }

static SkBenchmark* Fact5(void* p) { return new TextBench(p, STR, BIG, false, true); }
static SkBenchmark* Fact6(void* p) { return new TextBench(p, STR, BIG, true, false); }
static SkBenchmark* Fact7(void* p) { return new TextBench(p, STR, BIG, true, true); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg01(Fact01);
static BenchRegistry gReg02(Fact02);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);
static BenchRegistry gReg4(Fact4);
static BenchRegistry gReg41(Fact41);
static BenchRegistry gReg42(Fact42);
static BenchRegistry gReg5(Fact5);
static BenchRegistry gReg6(Fact6);
static BenchRegistry gReg7(Fact7);


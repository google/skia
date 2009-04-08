#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkFontHost.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkString.h"
#include "SkTemplates.h"

static void dump_font(const char name[], SkFontID fontID) {
    SkDebugf("Font \"%s\" %x\n", name, fontID);
    int count = SkFontHost::CountTables(fontID);
    SkAutoTArray<SkFontTableTag> storage(count);
    SkFontTableTag* tags = storage.get();
    SkFontHost::GetTableTags(fontID, tags);
    for (int i = 0; i < count; i++) {
        uint32_t tag = tags[i];
        uint8_t data[4];
        size_t size = SkFontHost::GetTableSize(fontID, tag);
        size_t bytes = SkFontHost::GetTableData(fontID, tag,
                                                0, sizeof(data), data);
        SkDebugf("   tag=%c%c%c%c size=%d bytes=%d %x %x %x %x\n",
                 uint8_t(tag>>24), uint8_t(tag>>16), uint8_t(tag>>8), uint8_t(tag),
                 size, bytes, data[0], data[1], data[2], data[3]);
    }
}

static void test_tables() {
    static bool gOnce;
    if (gOnce) {
        return;
    }
    gOnce = true;
    
    static const char* gNames[] = {
        "Arial", "Times", "Courier"
    };
    
    for (size_t i = 0; i < SK_ARRAY_COUNT(gNames); i++) {
        SkTypeface* tf = SkTypeface::CreateFromName(gNames[i], SkTypeface::kNormal);
        if (tf) {
            SkFontID fontID = tf->uniqueID();
            dump_font(gNames[i], fontID);
            tf->unref();
        }
    }
}

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
    enum { N = 300 };
public:
    TextBench(const char text[], int ps, bool linearText, bool posText) {
        test_tables();
        
        fText.set(text);

        fPaint.setAntiAlias(true);
        fPaint.setTextSize(SkIntToScalar(ps));
        fPaint.setLinearText(linearText);

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
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        const SkIPoint dim = this->getSize();
        SkRandom rand;

        SkPaint paint(fPaint);
        this->setupPaint(&paint);

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

static SkBenchmark* Fact0(void*) { return new TextBench(STR, SMALL, false, false); }
static SkBenchmark* Fact1(void*) { return new TextBench(STR, SMALL, false, true); }
static SkBenchmark* Fact2(void*) { return new TextBench(STR, SMALL, true, false); }
static SkBenchmark* Fact3(void*) { return new TextBench(STR, SMALL, true, true); }
static SkBenchmark* Fact4(void*) { return new TextBench(STR, BIG, false, false); }
static SkBenchmark* Fact5(void*) { return new TextBench(STR, BIG, false, true); }
static SkBenchmark* Fact6(void*) { return new TextBench(STR, BIG, true, false); }
static SkBenchmark* Fact7(void*) { return new TextBench(STR, BIG, true, true); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);
static BenchRegistry gReg4(Fact4);
static BenchRegistry gReg5(Fact5);
static BenchRegistry gReg6(Fact6);
static BenchRegistry gReg7(Fact7);

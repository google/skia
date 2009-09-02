#include "SampleCode.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkUtils.h"
#include "Forth.h"

class MyOutput : public ForthOutput {
public:
    SkString fStr;

    virtual void show(const char text[]) {
        fStr.set(text);
    }
};

class SkForthCtx {
public:
    SkCanvas    fCanvas;
    SkPaint     fPaint;

    void init(const SkBitmap& bm) {
        fCanvas.setBitmapDevice(bm);
        fPaint.setAntiAlias(true);
    }
};

class SkForthCtx_FW : public ForthWord {
public:
    SkForthCtx_FW() : fCtx(NULL) {}
    
    void setCtx(SkForthCtx* ctx) { fCtx = ctx; }
    
    SkCanvas* canvas() const { return &fCtx->fCanvas; }
    SkPaint* paint() const { return &fCtx->fPaint; }

private:
    SkForthCtx* fCtx;
};

class setColor_FW : public SkForthCtx_FW {
public:
    virtual void exec(ForthEngine* fe) {
        paint()->setColor(fe->pop());
    }
    
    static SkForthCtx_FW* New() { return new setColor_FW; }
};

class setStyle_FW : public SkForthCtx_FW {
public:
    virtual void exec(ForthEngine* fe) {
        paint()->setStyle((SkPaint::Style)fe->pop());
    }
    
    static SkForthCtx_FW* New() { return new setStyle_FW; }
};

class setStrokeWidth_FW : public SkForthCtx_FW {
public:
    virtual void exec(ForthEngine* fe) {
        paint()->setStrokeWidth(fe->fpop());
    }
    
    static SkForthCtx_FW* New() { return new setStrokeWidth_FW; }
};

class translate_FW : public SkForthCtx_FW {
public:
    virtual void exec(ForthEngine* fe) {
        SkScalar dy = fe->fpop();
        SkScalar dx = fe->fpop();
        canvas()->translate(dx, dy);
    }
    
    static SkForthCtx_FW* New() { return new translate_FW; }
};

class drawColor_FW : public SkForthCtx_FW {
public:
    virtual void exec(ForthEngine* fe) {
        canvas()->drawColor(fe->pop());
    }
    
    static SkForthCtx_FW* New() { return new drawColor_FW; }
};

class drawRect_FW : public SkForthCtx_FW {
public:
    virtual void exec(ForthEngine* fe) {
        SkRect r;
        r.fBottom = fe->fpop();
        r.fRight = fe->fpop();
        r.fTop = fe->fpop();
        r.fLeft = fe->fpop();
        canvas()->drawRect(r, *paint());
    }
    
    static SkForthCtx_FW* New() { return new drawRect_FW; }
};

class drawCircle_FW : public SkForthCtx_FW {
public:
    virtual void exec(ForthEngine* fe) {
        SkScalar radius = fe->fpop();
        SkScalar y = fe->fpop();
        SkScalar x = fe->fpop();
        canvas()->drawCircle(x, y, radius, *paint());
    }
    
    static SkForthCtx_FW* New() { return new drawCircle_FW; }
};

class drawLine_FW : public SkForthCtx_FW {
public:
    virtual void exec(ForthEngine* fe) {
        SkScalar x0, y0, x1, y1;
        y1 = fe->fpop();
        x1 = fe->fpop();
        y0 = fe->fpop();
        x0 = fe->fpop();
        canvas()->drawLine(x0, y0, x1, y1, *paint());
    }
    
    static SkForthCtx_FW* New() { return new drawLine_FW; }
};

static const struct {
    const char* fName;
    SkForthCtx_FW* (*fProc)();
} gWordRecs[] = {
    { "setColor",       setColor_FW::New },
    { "setStyle",       setStyle_FW::New },
    { "setStrokeWidth", setStrokeWidth_FW::New },
    { "translate",      translate_FW::New },
    { "drawColor",      drawColor_FW::New },
    { "drawRect",       drawRect_FW::New },
    { "drawCircle",     drawCircle_FW::New },
    { "drawLine",       drawLine_FW::New },
};

static void load_words(ForthEnv* env, SkForthCtx* ctx) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gWordRecs); i++) {
        SkForthCtx_FW* word = gWordRecs[i].fProc();
        word->setCtx(ctx);
        env->addWord(gWordRecs[i].fName, word);
    }
}

///////////////////////////////////////////////////////////////////////////////

void Forth_test_stdwords(bool verbose);

class ForthView : public SkView {
    ForthEnv    fEnv;
    ForthWord*  fOnClickWord;

    SkBitmap    fBM;
    SkForthCtx  fContext;
public:
	ForthView() {
        Forth_test_stdwords(false);

        load_words(&fEnv, &fContext);

        fBM.setConfig(SkBitmap::kARGB_8888_Config, 640, 480);
        fBM.allocPixels();
        fBM.eraseColor(0);
        fContext.init(fBM);

        fEnv.parse(": mycolor ( x. y. -- x. y. ) OVER OVER f< IF #FFFF0000 ELSE #FF0000FF THEN setColor ;");
        fEnv.parse(": view.onClick ( x. y. -- ) mycolor 10. drawCircle ;");
        fOnClickWord = fEnv.findWord("view.onClick");
#if 0
        env.parse(
                  ": draw1 "
                  "10. setStrokeWidth 1 setStyle #FF000000 setColor "
                  "10. 20. 200. 100. drawLine "
                  "0 setStyle #80FF0000 setColor "
                  "50. 50. 250. 150. drawRect "
                  ";");
        env.parse("#FF0000FF drawColor "
                  "draw1 "
                  "150. 120. translate "
                  "draw1 "
                  );
#endif
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Forth");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    void test_onClick(ForthEnv* env) {
        ForthWord* word = env->findWord("view.onClick");
        if (word) {
            const intptr_t idata[2] = { 40, 2 };
            intptr_t odata[1] = { -1 };
            ForthCallBlock block;
            block.in_data = idata;
            block.in_count = 2;
            block.out_data = odata;
            block.out_count = 1;
            word->call(&block);
            SkDebugf("after view.onClick depth %d count %d top %d\n",
                     block.out_depth, block.out_count, odata[0]);
        } else {
            SkDebugf("------ view.onClick not found\n");
        }
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        drawBG(canvas);
        canvas->drawBitmap(fBM, 0, 0, NULL);
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return fOnClickWord ? new Click(this) : NULL;
    }
    
    virtual bool onClick(Click* click) {
        intptr_t idata[2] = {
            f2i_bits(click->fCurr.fX), f2i_bits(click->fCurr.fY)
        };
        ForthCallBlock block;
        block.in_data = idata;
        block.in_count = 2;
        block.out_count = 0;
        fOnClickWord->call(&block);
        this->inval(NULL);
        return true;
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ForthView; }
static SkViewRegister reg(MyFactory);


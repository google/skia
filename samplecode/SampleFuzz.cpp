
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkBlurMaskFilter.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkXfermode.h"
#include "SkMatrix.h"
#include "SkColor.h"
#include "SkRandom.h"

static void set2x3(SkMatrix* m, float a, float b, float c, float d, float e, float f) {
    m->reset();
    m->set(0, a);
    m->set(1, b);
    m->set(2, c);
    m->set(3, d);
    m->set(4, e);
    m->set(5, f);
}

static SkRandom gRand;
static bool return_large;
static bool return_undef;
static bool quick;
static bool scale_large;
static int scval = 1;
static float transval = 0;

static int R(float x) {
  return (int)floor(SkScalarToFloat(gRand.nextUScalar1()) * x);
}

#if defined _WIN32
#pragma warning ( push )
// we are intentionally causing an overflow here
//      (warning C4756: overflow in constant arithmetic)
#pragma warning ( disable : 4756 )
#endif

static float huge() {
    double d = 1e100;
    float f = (float)d;
    return f;
}

#if defined _WIN32
#pragma warning ( pop )
#endif

static float make_number() {
  float v = 0;
  int sel;

  if (return_large == true && R(3) == 1) {
      sel = R(6);
  } else {
      sel = R(4);
  }

  if (return_undef == false && sel == 0) {
      sel = 1;
  }

  if (R(2) == 1) {
      v = (float)R(100);
  } else {

      switch (sel) {
        case 0: break;
        case 1: v = 0; break;
        case 2: v = 0.000001f; break;
        case 3: v = 10000; break;
        case 4: v = 2000000000; break;
        case 5: v = huge(); break;
      }

  }

  if (R(4) == 1) {
      v = -v;
  }

  return v;
}

static SkColor make_color() {
  if (R(2) == 1) return 0xFFC0F0A0; else return 0xFF000090;
}


static SkColor make_fill() {
#if 0
  int sel;

  if (quick == true) sel = 0; else sel = R(6);

  switch (sel) {

    case 0:
    case 1:
    case 2:
      return make_color();
      break;

    case 3:
      var r = ctx.createLinearGradient(make_number(),make_number(),make_number(),make_number());
      for (i=0;i<4;i++)
        r.addColorStop(make_number(),make_color());
      return r;
      break;

    case 4:
      var r = ctx.createRadialGradient(make_number(),make_number(),make_number(),make_number(),make_number(),make_number());
      for (i=0;i<4;i++)
        r.addColorStop(make_number(),make_color());
      return r;
      break;

    case 5:
      var r = ctx.createPattern(imgObj,"repeat");
      if (R(6) == 0)
        r.addColorStop(make_number(),make_color());
      return r;
      break;
  }
#else
    return make_color();
#endif
}


static void do_fuzz(SkCanvas* canvas) {
    SkPath path;
    SkPaint paint;
    paint.setAntiAlias(true);

  for (int i=0;i<100;i++) {
  switch (R(33)) {

    case 0:
          paint.setColor(make_fill());
      break;

    case 1:
      paint.setAlpha(gRand.nextU() & 0xFF);
      break;

      case 2: {
          SkXfermode::Mode mode;
          switch (R(3)) {
            case 0: mode = SkXfermode::kSrc_Mode; break;
            case 1: mode = SkXfermode::kXor_Mode; break;
            case 2:
            default:  // silence warning
              mode = SkXfermode::kSrcOver_Mode; break;
          }
          paint.setXfermodeMode(mode);
      }
      break;

    case 3:
      switch (R(2)) {
          case 0: paint.setStrokeCap(SkPaint::kRound_Cap); break;
        case 1: paint.setStrokeCap(SkPaint::kButt_Cap); break;
      }
      break;

    case 4:
      switch (R(2)) {
          case 0: paint.setStrokeJoin(SkPaint::kRound_Join); break;
        case 1: paint.setStrokeJoin(SkPaint::kMiter_Join); break;
      }
      break;

    case 5:
      paint.setStrokeWidth(make_number());
      break;

    case 6:
      paint.setStrokeMiter(make_number());
      break;

    case 7:
      if (quick == true) break;
          SkSafeUnref(paint.setMaskFilter(SkBlurMaskFilter::Create(kNormal_SkBlurStyle,
                                                                   make_number())));
      break;

    case 8:
      if (quick == true) break;
      //ctx.shadowColor = make_fill();
      break;

    case 9:
      if (quick == true) break;
      //ctx.shadowOffsetX = make_number();
      //ctx.shadowOffsetY = make_number();
      break;

    case 10:
      canvas->restore();
      break;

    case 11:
      canvas->rotate(make_number());
      break;

    case 12:
      canvas->save();
      break;

    case 13:
      canvas->scale(-1,-1);
      break;

    case 14:

      if (quick == true) break;

      if (transval == 0) {
        transval = make_number();
        canvas->translate(transval,0);
      } else {
        canvas->translate(-transval,0);
        transval = 0;
      }

      break;

          case 15: {
              SkRect r;
              r.set(make_number(),make_number(),make_number(),make_number());
              SkPaint::Style s = paint.getStyle();
              paint.setStyle(SkPaint::kFill_Style);
              canvas->drawRect(r, paint);
              paint.setStyle(s);
              // clearrect
          } break;

    case 16:
      if (quick == true) break;
//      ctx.drawImage(imgObj,make_number(),make_number(),make_number(),make_number(),make_number(),make_number(),make_number(),make_number());
      break;

          case 17: {
          SkRect r;
          r.set(make_number(),make_number(),make_number(),make_number());
              SkPaint::Style s = paint.getStyle();
              paint.setStyle(SkPaint::kFill_Style);
          canvas->drawRect(r, paint);
              paint.setStyle(s);
          } break;

    case 18:
          path.reset();
      break;

    case 19:
      // ctx.clip() is evil.
      break;

    case 20:
          path.close();
      break;

          case 21: {
          SkPaint::Style s = paint.getStyle();
          paint.setStyle(SkPaint::kFill_Style);
          canvas->drawPath(path, paint);
          paint.setStyle(s);
          } break;

          case 22: {
              SkPaint::Style s = paint.getStyle();
              paint.setStyle(SkPaint::kFill_Style);
              canvas->drawPath(path, paint);
              paint.setStyle(s);
          } break;

          case 23: {
              SkRect r;
              r.set(make_number(),make_number(),make_number(),make_number());
              SkPaint::Style s = paint.getStyle();
              paint.setStyle(SkPaint::kStroke_Style);
              canvas->drawRect(r, paint);
              paint.setStyle(s);
          } break;

    case 24:
      if (quick == true) break;
      //ctx.arc(make_number(),make_number(),make_number(),make_number(),make_number(),true);
      break;

    case 25:
      if (quick == true) break;
      //ctx.arcTo(make_number(),make_number(),make_number(),make_number(),make_number());
      break;

    case 26:
      if (quick == true) break;
      //ctx.bezierCurveTo(make_number(),make_number(),make_number(),make_number(),make_number(),make_number());
      break;

    case 27:
      path.lineTo(make_number(),make_number());
      break;

    case 28:
      path.moveTo(make_number(),make_number());
      break;

    case 29:
      if (quick == true) break;
      path.quadTo(make_number(),make_number(),make_number(),make_number());
      break;

          case 30: {
      if (quick == true) break;
              SkMatrix matrix;
      set2x3(&matrix, make_number(),make_number(),make_number(),make_number(),make_number(),make_number());
              canvas->concat(matrix);
          } break;

          case 31: {
      if (quick == true) break;
          SkMatrix matrix;
          set2x3(&matrix, make_number(),make_number(),make_number(),make_number(),make_number(),make_number());
          canvas->setMatrix(matrix);
          } break;

    case 32:

      if (scale_large == true) {

        switch (scval) {
          case 0: canvas->scale(-1000000000,1);
                  canvas->scale(-1000000000,1);
                  scval = 1; break;
          case 1: canvas->scale(-.000000001f,1); scval = 2; break;
          case 2: canvas->scale(-.000000001f,1); scval = 0; break;
        }

      }

      break;



  }
  }

}

//////////////////////////////////////////////////////////////////////////////

class FuzzView : public SampleView {
public:
    FuzzView() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Fuzzer");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        do_fuzz(canvas);
        this->inval(nullptr);
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new FuzzView; }
static SkViewRegister reg(MyFactory);

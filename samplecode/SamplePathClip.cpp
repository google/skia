#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkPorterDuff.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#if 0
static void CFString2SkString(CFStringRef ref, SkString* str) {
    str->reset();
    int count = CFStringGetLength(ref);
    for (int i = 0; i < count; i++) {
        SkString tmp;
        UniChar c = CFStringGetCharacterAtIndex(ref, i);
        tmp.setUTF16(&c, 1);
        str->append(tmp);
    }
}

static size_t get_table_size(ATSFontRef font, uint32_t tableTag) {
    ByteCount size;
    OSStatus status = ATSFontGetTable(font, tableTag, 0, 0, NULL, &size);
    if (status) {
        SkDebugf("*** ATSFontGetTable returned %d\n", status);
        size = -1;
    }
    return size;
}

static void test_ats() {
    OSStatus status;
    ATSFontIterator iter;
    status = ATSFontIteratorCreate(kATSFontContextLocal, NULL, NULL,
                                   kATSOptionFlagsUnRestrictedScope, &iter);

    for (int index = 0;; index++) {
        ATSFontRef fontRef;
        status = ATSFontIteratorNext(iter, &fontRef);
        if (status) {
            break;
        }
        
        CFStringRef name;
        SkString str;
        ATSFontGetName(fontRef, kATSOptionFlagsDefault, &name);
        CFString2SkString(name, &str);
        if (str.size() > 0 && str.c_str()[0] != '.') {
            SkDebugf("[%3d] font %x cmap %d 'name' %d <%s>\n", index, fontRef,
                     get_table_size(fontRef, 'cmap'),
                     get_table_size(fontRef, 'name'),
                     str.c_str());
        }
        CFRelease(name);
    }
    ATSFontIteratorRelease(&iter);
}
#endif

class PathClipView : public SkView {
public:
    SkRect fOval;
    SkPoint fCenter;

	PathClipView() {
        fOval.set(0, 0, SkIntToScalar(200), SkIntToScalar(50));
        fCenter.set(SkIntToScalar(250), SkIntToScalar(250));
        
//        test_ats();
    }
    
    virtual ~PathClipView() {}
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "PathClip");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkRect oval = fOval;
        oval.offset(fCenter.fX - oval.centerX(), fCenter.fY - oval.centerY());
        
        SkPaint p;
        p.setAntiAlias(true);
        
        p.setStyle(SkPaint::kStroke_Style);
        canvas->drawOval(oval, p);

        SkRect r;
        r.set(SkIntToScalar(200), SkIntToScalar(200),
              SkIntToScalar(300), SkIntToScalar(300));
        canvas->clipRect(r);
        
        p.setStyle(SkPaint::kFill_Style);
        p.setColor(SK_ColorRED);
        canvas->drawRect(r, p);
     
        p.setColor(0x800000FF);
        r.set(SkIntToScalar(150), SkIntToScalar(10),
              SkIntToScalar(250), SkIntToScalar(400));
        canvas->drawOval(oval, p);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fCenter.set(x, y);
        this->inval(NULL);
        return NULL;
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PathClipView; }
static SkViewRegister reg(MyFactory);


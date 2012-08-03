
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkDumpCanvas.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

#include "SkStream.h"
#include "SkXMLParser.h"

class PictFileView : public SampleView {
    SkString    fFilename;
    SkPicture*  fPicture;
    
    static SkPicture* LoadPicture(const char path[]) {
        SkPicture* pic = NULL;

        SkBitmap bm;
        if (SkImageDecoder::DecodeFile(path, &bm)) {
            bm.setImmutable();
            pic = SkNEW(SkPicture);
            SkCanvas* can = pic->beginRecording(bm.width(), bm.height());
            can->drawBitmap(bm, 0, 0, NULL);
            pic->endRecording();
        } else {
            SkFILEStream stream(path);
            if (stream.isValid()) {
                pic = SkNEW_ARGS(SkPicture, (&stream));
            }
        }
        return pic;
    }

public:
    PictFileView(const char name[] = NULL) : fFilename(name) {
        fPicture = NULL;
    }
    
    virtual ~PictFileView() {
        SkSafeUnref(fPicture);
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString name("P:");
            name.append(fFilename);
            SampleCode::TitleR(evt, name.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {
        if (!fPicture) {
            fPicture = LoadPicture(fFilename.c_str());
        }
        if (fPicture) {
            canvas->drawPicture(*fPicture);
        }
    }
    
private:
    typedef SampleView INHERITED;
};

SampleView* CreateSamplePictFileView(const char filename[]);
SampleView* CreateSamplePictFileView(const char filename[]) {
    return new PictFileView(filename);
}

//////////////////////////////////////////////////////////////////////////////

#if 0
static SkView* MyFactory() { return new PictFileView; }
static SkViewRegister reg(MyFactory);
#endif


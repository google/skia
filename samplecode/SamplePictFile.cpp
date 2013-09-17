
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
#include "SkOSFile.h"
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
#include "SkSurface.h"
#include "SkXMLParser.h"

class PictFileView : public SampleView {
    SkString    fFilename;
    SkPicture*  fPicture;
    SkPicture*  fBBoxPicture;
    bool        fUseBBox;

    static SkPicture* LoadPicture(const char path[], bool useBBox) {
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
                pic = SkPicture::CreateFromStream(&stream);
            } else {
                SkDebugf("coun't load picture at \"path\"\n", path);
            }

            if (false) {
                SkSurface* surf = SkSurface::NewRasterPMColor(pic->width(), pic->height());
                surf->getCanvas()->drawPicture(*pic);
                surf->unref();
            }
            if (false) { // re-record
                SkPicture p2;
                pic->draw(p2.beginRecording(pic->width(), pic->height()));
                p2.endRecording();

                SkString path2(path);
                path2.append(".new.skp");
                SkFILEWStream writer(path2.c_str());
                p2.serialize(&writer);
            }
        }

        if (useBBox) {
            SkPicture* bboxPicture = SkNEW(SkPicture);
            pic->draw(bboxPicture->beginRecording(pic->width(), pic->height(),
                    SkPicture::kOptimizeForClippedPlayback_RecordingFlag));
            bboxPicture->endRecording();
            SkDELETE(pic);
            return bboxPicture;

        } else {
            return pic;
        }
    }

public:
    PictFileView(const char name[] = NULL) : fFilename(name) {
        fPicture = NULL;
        fBBoxPicture = NULL;
        fUseBBox = false;
    }

    virtual ~PictFileView() {
        SkSafeUnref(fPicture);
        SkSafeUnref(fBBoxPicture);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString name("P:");
            const char* basename = strrchr(fFilename.c_str(), SkPATH_SEPARATOR);
            name.append(basename ? basename+1: fFilename.c_str());
            if (fUseBBox) {
                name.append(" <bbox>");
            }
            SampleCode::TitleR(evt, name.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual bool onEvent(const SkEvent& evt) {
        if (evt.isType("PictFileView::toggleBBox")) {
            fUseBBox = !fUseBBox;
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPicture** picture = fUseBBox ? &fBBoxPicture : &fPicture;

        if (!*picture) {
            *picture = LoadPicture(fFilename.c_str(), fUseBBox);
        }
        if (*picture) {
            canvas->drawPicture(**picture);
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

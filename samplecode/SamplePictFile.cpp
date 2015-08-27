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
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
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

#include "SkGlyphCache.h"

#include "SkDrawFilter.h"
class SkCounterDrawFilter : public SkDrawFilter {
public:
    SkCounterDrawFilter(int count) : fCount(count) {}

    bool filter(SkPaint*, Type t) override {
        return --fCount >= 0;
    }

    int fCount;
};

class PictFileView : public SampleView {
public:
    PictFileView(const char name[] = nullptr)
        : fFilename(name)
        , fBBox(kNo_BBoxType)
        , fTileSize(SkSize::Make(0, 0)) {
        for (int i = 0; i < kBBoxTypeCount; ++i) {
            fPictures[i] = nullptr;
        }
        fCount = 0;
    }

    virtual ~PictFileView() {
        for (int i = 0; i < kBBoxTypeCount; ++i) {
            SkSafeUnref(fPictures[i]);
        }
    }

    void onTileSizeChanged(const SkSize &tileSize) override {
        if (tileSize != fTileSize) {
            fTileSize = tileSize;
        }
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SkString name("P:");
            const char* basename = strrchr(fFilename.c_str(), SkPATH_SEPARATOR);
            name.append(basename ? basename+1: fFilename.c_str());
            switch (fBBox) {
            case kNo_BBoxType:
                // No name appended
                break;
            case kRTree_BBoxType:
                name.append(" <bbox: R>");
                break;
            default:
                SkASSERT(false);
                break;
            }
            SampleCode::TitleR(evt, name.c_str());
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case 'n': fCount += 1; this->inval(nullptr); return true;
                case 'p': fCount -= 1; this->inval(nullptr); return true;
                case 's': fCount =  0; this->inval(nullptr); return true;
                default: break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    bool onEvent(const SkEvent& evt) override {
        if (evt.isType("PictFileView::toggleBBox")) {
            fBBox = (BBoxType)((fBBox + 1) % kBBoxTypeCount);
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkASSERT(static_cast<int>(fBBox) < kBBoxTypeCount);
        SkPicture** picture = fPictures + fBBox;

#ifdef SK_GLYPHCACHE_TRACK_HASH_STATS
        SkGraphics::PurgeFontCache();
#endif

        if (!*picture) {
            *picture = LoadPicture(fFilename.c_str(), fBBox);
        }
        if (*picture) {
            SkCounterDrawFilter filter(fCount);
            if (fCount > 0) {
                canvas->setDrawFilter(&filter);
            }
            canvas->drawPicture(*picture);
            canvas->setDrawFilter(nullptr);
        }

#ifdef SK_GLYPHCACHE_TRACK_HASH_STATS
        SkGlyphCache::Dump();
        SkDebugf("\n");
#endif
    }

private:
    enum BBoxType {
        kNo_BBoxType,
        kRTree_BBoxType,

        kLast_BBoxType = kRTree_BBoxType,
    };
    static const int kBBoxTypeCount = kLast_BBoxType + 1;

    SkString    fFilename;
    SkPicture*  fPictures[kBBoxTypeCount];
    BBoxType    fBBox;
    SkSize      fTileSize;
    int         fCount;

    SkPicture* LoadPicture(const char path[], BBoxType bbox) {
        SkAutoTUnref<SkPicture> pic;

        SkBitmap bm;
        if (SkImageDecoder::DecodeFile(path, &bm)) {
            bm.setImmutable();
            SkPictureRecorder recorder;
            SkCanvas* can = recorder.beginRecording(SkIntToScalar(bm.width()),
                                                    SkIntToScalar(bm.height()),
                                                    nullptr, 0);
            can->drawBitmap(bm, 0, 0, nullptr);
            pic.reset(recorder.endRecording());
        } else {
            SkFILEStream stream(path);
            if (stream.isValid()) {
                pic.reset(SkPicture::CreateFromStream(&stream));
            } else {
                SkDebugf("coun't load picture at \"path\"\n", path);
            }

            if (false) { // re-record
                SkPictureRecorder recorder;
                pic->playback(recorder.beginRecording(pic->cullRect().width(),
                                                      pic->cullRect().height(),
                                                      nullptr, 0));
                SkAutoTUnref<SkPicture> p2(recorder.endRecording());

                SkString path2(path);
                path2.append(".new.skp");
                SkFILEWStream writer(path2.c_str());
                p2->serialize(&writer);
            }
        }

        if (nullptr == pic) {
            return nullptr;
        }

        SkAutoTDelete<SkBBHFactory> factory;
        switch (bbox) {
        case kNo_BBoxType:
            // no bbox playback necessary
            return pic.detach();
        case kRTree_BBoxType:
            factory.reset(new SkRTreeFactory);
            break;
        default:
            SkASSERT(false);
        }

        SkPictureRecorder recorder;
        pic->playback(recorder.beginRecording(pic->cullRect().width(),
                                              pic->cullRect().height(),
                                              factory.get(), 0));
        return recorder.endRecording();
    }

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

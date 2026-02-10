/*
 * Copyright 2026 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/RecorderPriv.h"
#endif

#include <string.h>

using namespace skia_private;

class CrBug478659067GM : public skiagm::GM {
public:
    CrBug478659067GM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString getName() const override { return SkString("crbug_478659067"); }

    SkISize getISize() override { return SkISize::Make(1024, 1280); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkISize size = this->getISize();
        if (!canvas->getBaseLayerSize().isEmpty()) {
            size = canvas->getBaseLayerSize();
        }
        SkImageInfo info = SkImageInfo::MakeN32(size.width(), size.height(), kPremul_SkAlphaType,
                                                canvas->imageInfo().refColorSpace());
        SkSurfaceProps inputProps;
        canvas->getProps(&inputProps);
        SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag | inputProps.flags(),
                             inputProps.pixelGeometry());
        sk_sp<SkSurface> surface;

        SkScalar dfSize = 162;
#if defined(SK_GRAPHITE)
        if (auto recorder = canvas->recorder()) {
            surface = SkSurfaces::RenderTarget(recorder, info, skgpu::Mipmapped::kNo, &props);
            dfSize = recorder->priv().caps()->glyphsAsPathsFontSize();
        }
#endif
        // Effectively make this test graphite only
        if (!surface) {
            *errorMsg = "Graphite only";
            return DrawResult::kSkip;
        }

        // Create a new canvas with the DeviceIndepdentFonts flag enabled
        SkCanvas* graphiteCanvas = surface->getCanvas();
        graphiteCanvas->clear(0xffffffff);

        const char* dfText = "TheQuickBrownFoxJumpsOverTheLazyDog_0123456789";
        const size_t dfLen = strlen(dfText);

        SkScalar scale = graphiteCanvas->getLocalToDeviceAs3x3().getMaxScale();
        if (scale <= 0.0f) {
            scale = 1.0f;
        }

        SkFont font(ToolUtils::CreatePortableTypeface("serif", SkFontStyle()));
        font.setSize((dfSize - 64) / scale);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);

        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        SkScalar lineSpacing = font.getSize();
        for (int i = 1; i <= 6; ++i) {
            SkAutoCanvasRestore acr(graphiteCanvas, true);
            graphiteCanvas->translate(SkIntToScalar(10), (i * lineSpacing) - 10);
            // We skew the font in order to prevent glyph reuse. This is needed to overflow the
            // cache page
            font.setSkewX(i * 0.05f);
            graphiteCanvas->drawSimpleText(dfText, dfLen, SkTextEncoding::kUTF8, 0, 0, font, paint);
        }

        graphiteCanvas->getSurface()->draw(canvas, 0, 0);
        return DrawResult::kOk;
    }
};

DEF_GM(return new CrBug478659067GM;)

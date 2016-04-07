/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "VulkanViewer.h"

#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkCommonFlags.h"

DEFINE_string(key, "",
              "Space-separated key/value pairs to add to JSON identifying this builder.");

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new VulkanViewer(argc, argv, platformData);
}

static bool on_key_handler(Window::Key key, Window::InputState state, uint32_t modifiers,
                           void* userData) {
    VulkanViewer* vv = reinterpret_cast<VulkanViewer*>(userData);

    return vv->onKey(key, state, modifiers);
}

static void on_paint_handler(SkCanvas* canvas, void* userData) {
    VulkanViewer* vv = reinterpret_cast<VulkanViewer*>(userData);

    return vv->onPaint(canvas);
}

VulkanViewer::VulkanViewer(int argc, char** argv, void* platformData)
    : fGMs(skiagm::GMRegistry::Head())
    , fCurrentMeasurement(0) {
    memset(fMeasurements, 0, sizeof(fMeasurements));

    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->attach(Window::kVulkan_BackendType, 0, nullptr);

    // register callbacks
    fWindow->registerKeyFunc(on_key_handler, this);
    fWindow->registerPaintFunc(on_paint_handler, this);

    SkAutoTDelete<skiagm::GM> gm(fGMs->factory()(nullptr));
    SkString title("VulkanViewer: ");
    title.append(gm->getName());
    fWindow->setTitle(title.c_str());
    fWindow->show();
}

VulkanViewer::~VulkanViewer() {
    fWindow->detach();
    delete fWindow;
}

bool VulkanViewer::onKey(Window::Key key, Window::InputState state, uint32_t modifiers) {
    if (Window::kDown_InputState == state && (modifiers & Window::kFirstPress_ModifierKey) &&
        key == Window::kRight_Key) {
        fGMs = fGMs->next();
        SkAutoTDelete<skiagm::GM> gm(fGMs->factory()(nullptr));
        SkString title("VulkanViewer: ");
        title.append(gm->getName());
        fWindow->setTitle(title.c_str());
    }

    return true;
}

void VulkanViewer::onPaint(SkCanvas* canvas) {
    SkAutoTDelete<skiagm::GM> gm(fGMs->factory()(nullptr));

    canvas->save();
    gm->draw(canvas);
    canvas->restore();

    drawStats(canvas);
}

void VulkanViewer::drawStats(SkCanvas* canvas) {
    static const float kPixelPerMS = 2.0f;
    static const int kDisplayWidth = 130;
    static const int kDisplayHeight = 100;
    static const int kDisplayPadding = 10;
    static const int kGraphPadding = 3;
    static const SkScalar kBaseMS = 1000.f / 60.f;  // ms/frame to hit 60 fps

    SkISize canvasSize = canvas->getDeviceSize();
    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(canvasSize.fWidth-kDisplayWidth-kDisplayPadding),
                                   SkIntToScalar(kDisplayPadding),
                                   SkIntToScalar(kDisplayWidth), SkIntToScalar(kDisplayHeight));
    SkPaint paint;
    canvas->save();

    canvas->clipRect(rect);
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(rect, paint);
    // draw the 16ms line
    paint.setColor(SK_ColorLTGRAY);
    canvas->drawLine(rect.fLeft, rect.fBottom - kBaseMS*kPixelPerMS,
                     rect.fRight, rect.fBottom - kBaseMS*kPixelPerMS, paint);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(rect, paint);

    int x = SkScalarTruncToInt(rect.fLeft) + kGraphPadding;
    const int xStep = 2;
    const int startY = SkScalarTruncToInt(rect.fBottom);
    int i = fCurrentMeasurement;
    do {
        int endY = startY - (int)(fMeasurements[i] * kPixelPerMS + 0.5);  // round to nearest value
        canvas->drawLine(SkIntToScalar(x), SkIntToScalar(startY),
                         SkIntToScalar(x), SkIntToScalar(endY), paint);
        i++;
        i &= (kMeasurementCount - 1);  // fast mod
        x += xStep;
    } while (i != fCurrentMeasurement);

    canvas->restore();
}

void VulkanViewer::onIdle(double ms) {
    // Record measurements
    fMeasurements[fCurrentMeasurement++] = ms;
    fCurrentMeasurement &= (kMeasurementCount - 1);  // fast mod
    SkASSERT(fCurrentMeasurement < kMeasurementCount);

    fWindow->onPaint();
}

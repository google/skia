/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "VulkanViewer.h"

#include "GMSlide.h"
#include "SKPSlide.h"

#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkOSFile.h"
#include "SkRandom.h"
#include "SkStream.h"

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

DEFINE_bool2(fullscreen, f, true, "Run fullscreen.");
DEFINE_string(key, "", "Space-separated key/value pairs to add to JSON identifying this builder.");
DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of bench name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching bench to always be skipped\n"
               "^ requires the start of the bench to match\n"
               "$ requires the end of the bench to match\n"
               "^ and $ requires an exact match\n"
               "If a bench does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");
DEFINE_string(skps, "skps", "Directory to read skps from.");






VulkanViewer::VulkanViewer(int argc, char** argv, void* platformData) : fCurrentMeasurement(0) {
    memset(fMeasurements, 0, sizeof(fMeasurements));

    SkDebugf("Command line arguments: ");
    for (int i = 1; i < argc; ++i) {
        SkDebugf("%s ", argv[i]);
    }
    SkDebugf("\n");

    SkCommandLineFlags::Parse(argc, argv);

    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->attach(Window::kVulkan_BackendType, 0, nullptr);

    // register callbacks
    fWindow->registerKeyFunc(on_key_handler, this);
    fWindow->registerPaintFunc(on_paint_handler, this);

    // set up slides
    this->initSlides();

    // set up first frame
    SkString title("VulkanViewer: ");
    title.append(fSlides[0]->getName());
    fCurrentSlide = 0;
    fWindow->setTitle(title.c_str());
    fWindow->show();
}

static sk_sp<SkPicture> read_picture(const char path[]) {
    if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path)) {
        return nullptr;
    }

    SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
    if (stream.get() == nullptr) {
        SkDebugf("Could not read %s.\n", path);
        return nullptr;
    }

    auto pic = SkPicture::MakeFromStream(stream.get());
    if (!pic) {
        SkDebugf("Could not read %s as an SkPicture.\n", path);
    }
    return pic;
}


static sk_sp<SKPSlide> loadSKP(const SkString& path) {
    sk_sp<SkPicture> pic = read_picture(path.c_str());
    if (!pic) {
        return nullptr;
    }

    SkString name = SkOSPath::Basename(path.c_str());
    return sk_sp<SKPSlide>(new SKPSlide(name.c_str(), pic));
}

void VulkanViewer::initSlides() {
    const skiagm::GMRegistry* gms(skiagm::GMRegistry::Head());
    while (gms) {
        SkAutoTDelete<skiagm::GM> gm(gms->factory()(nullptr));

        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, gm->getName())) {
            sk_sp<Slide> slide(new GMSlide(gm.release()));
            fSlides.push_back(slide);
        }

        gms = gms->next();
    }

    // reverse array
    for (int i = 0; i < fSlides.count()/2; ++i) {
        sk_sp<Slide> temp = fSlides[i];
        fSlides[i] = fSlides[fSlides.count() - i - 1];
        fSlides[fSlides.count() - i - 1] = temp;
    }

    // SKPs
    for (int i = 0; i < FLAGS_skps.count(); i++) {
        if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
            SkString path(FLAGS_skps[i]);
            sk_sp<SKPSlide> slide = loadSKP(path);
            if (slide) {
                fSlides.push_back(slide);
            }
        } else {
            SkOSFile::Iter it(FLAGS_skps[i], ".skp");
            SkString path;
            while (it.next(&path)) {
                SkString skpName = SkOSPath::Join(FLAGS_skps[i], path.c_str());
                sk_sp<SKPSlide> slide = loadSKP(skpName);
                if (slide) {
                    fSlides.push_back(slide);
                }
            }
        }
    }
}


VulkanViewer::~VulkanViewer() {
    fWindow->detach();
    delete fWindow;
}

bool VulkanViewer::onKey(Window::Key key, Window::InputState state, uint32_t modifiers) {
    if (Window::kDown_InputState == state && (modifiers & Window::kFirstPress_ModifierKey)) {
        if (key == Window::kRight_Key) {
            fCurrentSlide++;
            if (fCurrentSlide >= fSlides.count()) {
                fCurrentSlide = 0;
            }
            SkString title("VulkanViewer: ");
            title.append(fSlides[fCurrentSlide]->getName());
            fWindow->setTitle(title.c_str());
        } else if (key == Window::kLeft_Key) {
            fCurrentSlide--;
            if (fCurrentSlide < 0) {
                fCurrentSlide = fSlides.count()-1;
            }
            SkString title("VulkanViewer: ");
            title.append(fSlides[fCurrentSlide]->getName());
            fWindow->setTitle(title.c_str());
        }
    }

    return true;
}

void VulkanViewer::onPaint(SkCanvas* canvas) {

    canvas->clear(SK_ColorWHITE);

    canvas->save();
    fSlides[fCurrentSlide]->draw(canvas);
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

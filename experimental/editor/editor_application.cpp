// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// [Work In Progress] Proof of principle of a text editor written with Skia & SkShaper.
// https://bugs.skia.org/9020

#include "include/core/SkExecutor.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/effects/SkGradientShader.h"
#include "modules/skshaper/include/SkShaper.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/CommandSet.h"
#include "tools/sk_app/Window.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

struct Timer {
    double fTime;
    const char* fDesc;
    Timer(const char* desc = "") : fTime(SkTime::GetNSecs()), fDesc(desc) {}
    ~Timer() { SkDebugf("%s: %d ms\n", fDesc, (int)((SkTime::GetNSecs() - fTime) * 1e-6)); }
};

struct TextLine {
    TextLine(std::string s) : fText(std::move(s)) {}
    std::string fText;
    float fHeight = 0;
    sk_sp<const SkTextBlob> fBlob;
    bool fSelected = false;  // Will allow selection of subset of text later.
    // Also will track presence of cursor.

    void shape(const SkFont& font, const SkShaper* shaper, float width) {
        SkTextBlobBuilderRunHandler textBlobBuilder(fText.c_str(), {0, 0});
        shaper->shape(fText.c_str(), fText.size(), font, true, width, &textBlobBuilder);
        fHeight = std::max(textBlobBuilder.endPoint().y(), font.getSpacing());
        fBlob = textBlobBuilder.makeBlob();
    }
};

std::vector<TextLine> read_file(const char* path) {
    std::vector<TextLine> ret;
    if (path) {
        std::ifstream stream(path);
        for (std::string line; std::getline(stream, line);) {
            ret.push_back(TextLine(line));
        }
    }
    return ret;
}

struct EditorLayer : public sk_app::Window::Layer {
    sk_app::Window* fParent = nullptr;
    std::vector<TextLine> fLines;
    const SkShaper* fShaper = nullptr;
    int fMargin = 10;
    int fPos = 10;
    int fWidth = 0;
    int fHeight = 0;
    SkFont fFont{nullptr, 24};

    EditorLayer(std::vector<TextLine> lines, SkShaper* shaper)
        : fLines(std::move(lines)), fShaper(shaper) {}

    void onPaint(SkSurface* surface) override {
        Timer timer("painting");
        SkColor background = SkColorSetARGB(0xFF, 0xCC, 0xCC, 0xCC);
        SkColor foreground = SkColorSetARGB(0xFF, 0x00, 0x00, 0x00);
        SkCanvas* c = surface->getCanvas();
        c->clipRect({0, 0, (float)fWidth, (float)fHeight});
        c->clear(background);
        float y = fPos;
        SkPaint p;
        p.setColor(foreground);
        SkPaint diff;
        diff.setColor(SK_ColorWHITE);
        diff.setBlendMode(SkBlendMode::kDifference);
        float width = (float)(fWidth - 2 * fMargin);
        for (const TextLine& line : fLines) {
            if (line.fBlob) {
                c->drawTextBlob(line.fBlob.get(), fMargin, y, p);
            }
            if (line.fSelected) {
                c->drawRect(SkRect{(float)fMargin, y, width, y + line.fHeight}, diff);
            }
            y += line.fHeight;
        }
    }

    void reshape() {
        Timer timer("shaping");
        float width = (float)(fWidth - 2 * fMargin);
        #ifdef SK_EDITOR_GO_FAST
        SkSemaphore semaphore;
        std::unique_ptr<SkExecutor> executor = SkExecutor::MakeFIFOThreadPool(100);
        for (TextLine& line : fLines) {
            executor->add([&]() {
                auto shaper = SkShaper::Make();
                line.shape(fFont, shaper.get(), width);
                semaphore.signal();
            });
        }
        for (const TextLine& l : fLines) { semaphore.wait(); }
        #else
        for (TextLine& line : fLines) {
            line.shape(fFont, fShaper, width);
        }
        #endif
    }

    void onResize(int width, int height) override {
        if (SkISize{fWidth, fHeight} != SkISize{width, height}) {
            fHeight = height;
            if (width != fWidth) {
                fWidth = width;
                this->reshape();
            }
            if (fParent) {
                fParent->inval();
            }
        }
    }

    void onAttach(sk_app::Window* w) override { fParent = w; }

    bool onMouseWheel(float delta, uint32_t modifiers) override {
        int newpos = std::min(fPos + (int)(delta * fFont.getSpacing()), fMargin);
        if (newpos != fPos) {
            fPos = newpos;
            if (fParent) { fParent->inval(); }
            return true;
        }
        return false;
    }

    bool onMouse(int x, int y, sk_app::Window::InputState state, uint32_t modifiers) override {
        if (sk_app::Window::kDown_InputState == state) {
            y -= fPos;
            if (y >= 0) {
                for (TextLine& line : fLines) {
                    if (y < line.fHeight) {
                        line.fSelected = !line.fSelected;
                        SkDebugf("  %d %d\n", x - (int)fMargin, y);
                        fParent->inval();
                        break;
                    }
                    y -= line.fHeight;
                }
            }
            return true;
        }
        return false;
    }
};

struct EditorApplication : public sk_app::Application {
    std::unique_ptr<SkShaper> fShaper;
    std::unique_ptr<sk_app::Window> fWindow;
    std::unique_ptr<EditorLayer> fLayer;
    sk_app::CommandSet fCommandSet;

    EditorApplication(const char* path, void* platformData)
        : fShaper(SkShaper::Make())
        , fWindow(sk_app::Window::CreateNativeWindow(platformData))
    {
        fWindow->attach(sk_app::Window::kRaster_BackendType);
        fLayer.reset(new EditorLayer(read_file(path), fShaper.get()));
        fWindow->pushLayer(fLayer.get());
        fCommandSet.attach(fWindow.get());
        fWindow->show();
        fLayer->onResize(fWindow->width(), fWindow->height());
    }
    ~EditorApplication() override { fWindow->detach(); }

    void onIdle() override {}
};

}  // namespace

sk_app::Application* sk_app::Application::Create(int argc, char** argv, void* dat) {
    if (argc > 1) {
        return new EditorApplication(argv[1], dat);
    }
    return new EditorApplication(nullptr, dat);
}

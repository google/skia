/*
 * Copyright 2018 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ImageViewer_DEFINED
#define ImageViewer_DEFINED

#include "sk_app/Application.h"
#include "sk_app/CommandSet.h"
#include "sk_app/Window.h"
#include "ImGuiLayer.h"
#include "SkTHash.h"
#include "SkTouchGesture.h"

class SkCanvas;
class SkImage;

struct ImGui_FilteredListBox {
    bool draw(const char* label, int* index, int numItems, std::function<const char*(int)> items);

    ImGuiTextFilter       fFilter;
    ImVector<const char*> fFilteredItems;
    ImVector<int>         fFilteredIndices;
};

class ImageViewer : public sk_app::Application, sk_app::Window::Layer {
public:
    ImageViewer(int argc, char** argv, void* platformData);
    ~ImageViewer() override;

    void onIdle() override;

    void onBackendCreated() override;
    void onPaint(SkCanvas* canvas) override;
    bool onMouse(int x, int y, sk_app::Window::InputState state, uint32_t modifiers) override;
    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers) override;
    bool onChar(SkUnichar c, uint32_t modifiers) override;

private:
    void initImages();

    void drawImagePicker();
    void drawImages(SkCanvas* canvs);
    void drawMenuBar();

    SkTArray<SkString> fSrcDirs;

    sk_app::Window* fWindow;
    sk_app::CommandSet fCommands;
    SkTouchGesture fGesture;
    ImGuiLayer fImGuiLayer;

    SkTArray<SkString> fImageNames;
    SkTHashMap<SkString, SkTArray<SkString>*> fImagesByName;

    int fCurrentImage;
    ImVector<bool> fVisibleDirs;
    bool fGridMode = false;

    bool fLimitScale = true;
    float fScale = 1.0f;

    struct LabeledImage {
        SkString fLabel;
        sk_sp<SkImage> fImage;
    };
    SkTArray<LabeledImage> fImages;

    ImGui_FilteredListBox fImageListBox;
};

#endif

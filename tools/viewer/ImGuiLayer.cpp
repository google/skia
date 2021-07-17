/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/ImGuiLayer.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSwizzle.h"
#include "include/core/SkTime.h"
#include "include/core/SkVertices.h"

#include "imgui.h"

#include <stdlib.h>
#include <map>

using namespace sk_app;

static void build_ImFontAtlas(ImFontAtlas& atlas, SkPaint& fontPaint) {
    int w, h;
    unsigned char* pixels;
    atlas.GetTexDataAsAlpha8(&pixels, &w, &h);
    SkImageInfo info = SkImageInfo::MakeA8(w, h);
    SkPixmap pmap(info, pixels, info.minRowBytes());
    SkMatrix localMatrix = SkMatrix::Scale(1.0f / w, 1.0f / h);
    auto fontImage = SkImage::MakeFromRaster(pmap, nullptr, nullptr);
    auto fontShader = fontImage->makeShader(SkSamplingOptions(SkFilterMode::kLinear), localMatrix);
    fontPaint.setShader(fontShader);
    fontPaint.setColor(SK_ColorWHITE);
    atlas.TexID = &fontPaint;
}

ImGuiLayer::ImGuiLayer() {
    // ImGui initialization:
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Keymap...
    io.KeyMap[ImGuiKey_Tab]        = (int)skui::Key::kTab;
    io.KeyMap[ImGuiKey_LeftArrow]  = (int)skui::Key::kLeft;
    io.KeyMap[ImGuiKey_RightArrow] = (int)skui::Key::kRight;
    io.KeyMap[ImGuiKey_UpArrow]    = (int)skui::Key::kUp;
    io.KeyMap[ImGuiKey_DownArrow]  = (int)skui::Key::kDown;
    io.KeyMap[ImGuiKey_PageUp]     = (int)skui::Key::kPageUp;
    io.KeyMap[ImGuiKey_PageDown]   = (int)skui::Key::kPageDown;
    io.KeyMap[ImGuiKey_Home]       = (int)skui::Key::kHome;
    io.KeyMap[ImGuiKey_End]        = (int)skui::Key::kEnd;
    io.KeyMap[ImGuiKey_Delete]     = (int)skui::Key::kDelete;
    io.KeyMap[ImGuiKey_Backspace]  = (int)skui::Key::kBack;
    io.KeyMap[ImGuiKey_Enter]      = (int)skui::Key::kOK;
    io.KeyMap[ImGuiKey_Escape]     = (int)skui::Key::kEscape;
    io.KeyMap[ImGuiKey_A]          = (int)skui::Key::kA;
    io.KeyMap[ImGuiKey_C]          = (int)skui::Key::kC;
    io.KeyMap[ImGuiKey_V]          = (int)skui::Key::kV;
    io.KeyMap[ImGuiKey_X]          = (int)skui::Key::kX;
    io.KeyMap[ImGuiKey_Y]          = (int)skui::Key::kY;
    io.KeyMap[ImGuiKey_Z]          = (int)skui::Key::kZ;

    build_ImFontAtlas(*io.Fonts, fFontPaint);
}

ImGuiLayer::~ImGuiLayer() {
    ImGui::DestroyContext();
}

void ImGuiLayer::setScaleFactor(float scaleFactor) {
    ImGui::GetStyle().ScaleAllSizes(scaleFactor);

    ImFontAtlas& atlas = *ImGui::GetIO().Fonts;
    atlas.Clear();
    ImFontConfig cfg;
    cfg.SizePixels = 13 * scaleFactor;
    atlas.AddFontDefault(&cfg)->DisplayOffset.y = scaleFactor;
    build_ImFontAtlas(atlas, fFontPaint);
}

#if defined(SK_BUILD_FOR_UNIX)
static const char* get_clipboard_text(void* user_data) {
    Window* w = (Window*)user_data;
    return w->getClipboardText();
}

static void set_clipboard_text(void* user_data, const char* text) {
    Window* w = (Window*)user_data;
    w->setClipboardText(text);
}
#endif

void ImGuiLayer::onAttach(Window* window) {
    fWindow = window;

#if defined(SK_BUILD_FOR_UNIX)
    ImGuiIO& io = ImGui::GetIO();
    io.ClipboardUserData = fWindow;
    io.GetClipboardTextFn = get_clipboard_text;
    io.SetClipboardTextFn = set_clipboard_text;
#endif
}

bool ImGuiLayer::onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) {
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = static_cast<float>(x);
    io.MousePos.y = static_cast<float>(y);
    if (skui::InputState::kDown == state) {
        io.MouseDown[0] = true;
    } else if (skui::InputState::kUp == state) {
        io.MouseDown[0] = false;
    }
    return io.WantCaptureMouse;
}

bool ImGuiLayer::onMouseWheel(float delta, skui::ModifierKey modifiers) {
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel += delta;
    return true;
}

void ImGuiLayer::skiaWidget(const ImVec2& size, SkiaWidgetFunc func) {
    intptr_t funcIndex = fSkiaWidgetFuncs.count();
    fSkiaWidgetFuncs.push_back(func);
    ImGui::Image((ImTextureID)funcIndex, size);
}

void ImGuiLayer::onPrePaint() {
    // Update ImGui input
    ImGuiIO& io = ImGui::GetIO();

    static double previousTime = 0.0;
    double currentTime = SkTime::GetSecs();
    io.DeltaTime = static_cast<float>(currentTime - previousTime);
    previousTime = currentTime;

    io.DisplaySize.x = static_cast<float>(fWindow->width());
    io.DisplaySize.y = static_cast<float>(fWindow->height());

    io.KeyAlt   = io.KeysDown[static_cast<int>(skui::Key::kOption)];
    io.KeyCtrl  = io.KeysDown[static_cast<int>(skui::Key::kCtrl)];
    io.KeyShift = io.KeysDown[static_cast<int>(skui::Key::kShift)];
    io.KeySuper = io.KeysDown[static_cast<int>(skui::Key::kSuper)];

    ImGui::NewFrame();
}

void ImGuiLayer::onPaint(SkSurface* surface) {
    // This causes ImGui to rebuild vertex/index data based on all immediate-mode commands
    // (widgets, etc...) that have been issued
    ImGui::Render();

    // Then we fetch the most recent data, and convert it so we can render with Skia
    const ImDrawData* drawData = ImGui::GetDrawData();
    SkTDArray<SkPoint> pos;
    SkTDArray<SkPoint> uv;
    SkTDArray<SkColor> color;

    auto canvas = surface->getCanvas();

    for (int i = 0; i < drawData->CmdListsCount; ++i) {
        const ImDrawList* drawList = drawData->CmdLists[i];

        // De-interleave all vertex data (sigh), convert to Skia types
        pos.rewind(); uv.rewind(); color.rewind();
        for (int j = 0; j < drawList->VtxBuffer.size(); ++j) {
            const ImDrawVert& vert = drawList->VtxBuffer[j];
            pos.push_back(SkPoint::Make(vert.pos.x, vert.pos.y));
            uv.push_back(SkPoint::Make(vert.uv.x, vert.uv.y));
            color.push_back(vert.col);
        }
        // ImGui colors are RGBA
        SkSwapRB(color.begin(), color.begin(), color.count());

        int indexOffset = 0;

        // Draw everything with canvas.drawVertices...
        for (int j = 0; j < drawList->CmdBuffer.size(); ++j) {
            const ImDrawCmd* drawCmd = &drawList->CmdBuffer[j];

            SkAutoCanvasRestore acr(canvas, true);

            // TODO: Find min/max index for each draw, so we know how many vertices (sigh)
            if (drawCmd->UserCallback) {
                drawCmd->UserCallback(drawList, drawCmd);
            } else {
                intptr_t idIndex = (intptr_t)drawCmd->TextureId;
                if (idIndex < fSkiaWidgetFuncs.count()) {
                    // Small image IDs are actually indices into a list of callbacks. We directly
                    // examing the vertex data to deduce the image rectangle, then reconfigure the
                    // canvas to be clipped and translated so that the callback code gets to use
                    // Skia to render a widget in the middle of an ImGui panel.
                    ImDrawIdx rectIndex = drawList->IdxBuffer[indexOffset];
                    SkPoint tl = pos[rectIndex], br = pos[rectIndex + 2];
                    canvas->clipRect(SkRect::MakeLTRB(tl.fX, tl.fY, br.fX, br.fY));
                    canvas->translate(tl.fX, tl.fY);
                    fSkiaWidgetFuncs[idIndex](canvas);
                } else {
                    SkPaint* paint = static_cast<SkPaint*>(drawCmd->TextureId);
                    SkASSERT(paint);

                    canvas->clipRect(SkRect::MakeLTRB(drawCmd->ClipRect.x, drawCmd->ClipRect.y,
                                                      drawCmd->ClipRect.z, drawCmd->ClipRect.w));
                    auto vertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                                         drawList->VtxBuffer.size(),
                                                         pos.begin(), uv.begin(), color.begin(),
                                                         drawCmd->ElemCount,
                                                         drawList->IdxBuffer.begin() + indexOffset);
                    canvas->drawVertices(vertices, SkBlendMode::kModulate, *paint);
                }
                indexOffset += drawCmd->ElemCount;
            }
        }
    }

    fSkiaWidgetFuncs.reset();
}

bool ImGuiLayer::onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifiers) {
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[static_cast<int>(key)] = (skui::InputState::kDown == state);
    return io.WantCaptureKeyboard;
}

bool ImGuiLayer::onChar(SkUnichar c, skui::ModifierKey modifiers) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput) {
        if (c > 0 && c < 0x10000) {
            io.AddInputCharacter(c);
        }
        return true;
    }
    return false;
}

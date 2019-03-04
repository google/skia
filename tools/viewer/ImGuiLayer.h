/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef ImGuiLayer_DEFINED
#define ImGuiLayer_DEFINED

#include "SkPaint.h"
#include "SkTArray.h"
#include "sk_app/Window.h"

#include "imgui.h"

namespace ImGui {

// Helper object for drawing in a widget region, with draggable points
struct DragCanvas {
    DragCanvas(const void* id, SkPoint tl = { 0.0f, 0.0f }, SkPoint br = { 1.0f, 1.0f },
               float aspect = -1.0f)
            : fID(0), fDragging(false) {
        ImGui::PushID(id);
        fDrawList = ImGui::GetWindowDrawList();

        // Logical size
        SkScalar w = SkTAbs(br.fX - tl.fX),
                 h = SkTAbs(br.fY - tl.fY);

        // Determine aspect ratio automatically by default
        if (aspect < 0) {
            aspect = h / w;
        }

        float availWidth = SkTMax(ImGui::GetContentRegionAvailWidth(), 1.0f);
        fPos = ImGui::GetCursorScreenPos();
        fSize = ImVec2(availWidth, availWidth * aspect);

        SkPoint local[4] = {
            { tl.fX, tl.fY },
            { br.fX, tl.fY },
            { tl.fX, br.fY },
            { br.fX, br.fY },
        };
        SkPoint screen[4] = {
            { fPos.x          , fPos.y           },
            { fPos.x + fSize.x, fPos.y           },
            { fPos.x          , fPos.y + fSize.y },
            { fPos.x + fSize.x, fPos.y + fSize.y },
        };
        fLocalToScreen.setPolyToPoly(local, screen, 4);
        fScreenToLocal.setPolyToPoly(screen, local, 4);
    }

    ~DragCanvas() {
        ImGui::SetCursorScreenPos(ImVec2(fPos.x, fPos.y + fSize.y));
        ImGui::Spacing();
        ImGui::PopID();
    }

    void fillColor(ImU32 color) {
        fDrawList->AddRectFilled(fPos, ImVec2(fPos.x + fSize.x, fPos.y + fSize.y), color);
    }

    void dragPoint(SkPoint* p, bool tooltip = false, ImU32 color = 0xFFFFFFFF) {
        // Transform points from logical coordinates to screen coordinates
        SkPoint center = fLocalToScreen.mapXY(p->fX, p->fY);

        // Invisible 10x10 button
        ImGui::PushID(fID++);
        ImGui::SetCursorScreenPos(ImVec2(center.fX - 5, center.fY - 5));
        ImGui::InvisibleButton("", ImVec2(10, 10));

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging()) {
            // Update screen position to track mouse, clamped to our area
            ImGuiIO& io = ImGui::GetIO();
            center.set(SkTPin(io.MousePos.x, fPos.x, fPos.x + fSize.x),
                       SkTPin(io.MousePos.y, fPos.y, fPos.y + fSize.y));

            // Update local coordinates for the caller
            *p = fScreenToLocal.mapXY(center.fX, center.fY);
            fDragging = true;
        }

        if (tooltip && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("x: %.3f\ny: %.3f", p->fX, p->fY);
        }

        ImGui::PopID();

        fScreenPoints.push_back(ImVec2(center.fX, center.fY));
        fDrawList->AddCircle(fScreenPoints.back(), 5.0f, color);
    }

    ImDrawList* fDrawList;

    // Location and dimensions (in screen coordinates)
    ImVec2 fPos;
    ImVec2 fSize;

    // Screen coordinates of points (for additional user drawing)
    SkSTArray<4, ImVec2, true> fScreenPoints;

    // To simplify dragPoint
    SkMatrix fLocalToScreen;
    SkMatrix fScreenToLocal;

    int fID;
    bool fDragging;
};

}

class ImGuiLayer : public sk_app::Window::Layer {
public:
    ImGuiLayer();
    ~ImGuiLayer() override;

    typedef std::function<void(SkCanvas*)> SkiaWidgetFunc;
    void skiaWidget(const ImVec2& size, SkiaWidgetFunc func);

    void onAttach(sk_app::Window* window) override;
    void onPrePaint() override;
    void onPaint(SkSurface*) override;
    bool onMouse(int x, int y, sk_app::Window::InputState state, uint32_t modifiers) override;
    bool onMouseWheel(float delta, uint32_t modifiers) override;
    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers) override;
    bool onChar(SkUnichar c, uint32_t modifiers) override;

private:
    sk_app::Window* fWindow;
    SkPaint fFontPaint;
    SkTArray<SkiaWidgetFunc> fSkiaWidgetFuncs;
};

#endif

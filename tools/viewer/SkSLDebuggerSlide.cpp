/*
* Copyright 2021 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SkSLDebuggerSlide.h"

#include "include/core/SkCanvas.h"
#include "tools/viewer/Viewer.h"

#include <algorithm>
#include <cstdio>
#include "imgui.h"

using namespace sk_app;

///////////////////////////////////////////////////////////////////////////////

SkSLDebuggerSlide::SkSLDebuggerSlide() {
    fName = "Debugger";
    fTrace = sk_make_sp<SkSL::SkVMDebugTrace>();
}

void SkSLDebuggerSlide::load(SkScalar winWidth, SkScalar winHeight) {}

void SkSLDebuggerSlide::unload() {
    fTrace = sk_make_sp<SkSL::SkVMDebugTrace>();
    fPlayer.reset(nullptr);
}

void SkSLDebuggerSlide::showLoadTraceGUI() {
    ImGui::InputText("Trace Path", fTraceFile, SK_ARRAY_COUNT(fTraceFile));
    bool load = ImGui::Button("Load Debug Trace");

    if (load) {
        SkFILEStream file(fTraceFile);
        if (!file.isValid()) {
            ImGui::OpenPopup("Can't Open Trace");
        } else if (!fTrace->readTrace(&file)) {
            ImGui::OpenPopup("Invalid Trace");
        } else {
            // Trace loaded successfully. On the next refresh, the user will see the debug UI.
            fPlayer.reset(fTrace);
            fPlayer.step();
            return;
        }
    }

    if (ImGui::BeginPopupModal("Can't Open Trace", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("The trace file doesn't exist.");
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Invalid Trace", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("The trace data could not be parsed.");
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SkSLDebuggerSlide::showDebuggerGUI() {
    bool updateScroll = false;

    if (ImGui::Button("Step")) {
        fPlayer.step();
        updateScroll = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step Over")) {
        fPlayer.stepOver();
        updateScroll = true;
    }

    constexpr ImGuiTableFlags kTableFlags =
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
            ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
            ImGuiTableFlags_Hideable;
    constexpr ImGuiTableColumnFlags kColumnFlags =
            ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder |
            ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoSort |
            ImGuiTableColumnFlags_NoHeaderLabel;

    ImVec2 contentRect = ImGui::GetContentRegionAvail();
    ImVec2 codeViewSize = ImVec2(0.0f, contentRect.y);
    if (ImGui::BeginTable("Code View", /*column=*/2, kTableFlags, codeViewSize)) {
        ImGui::TableSetupColumn("", kColumnFlags | ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Code", kColumnFlags | ImGuiTableColumnFlags_WidthStretch);

        ImGuiListClipper clipper;
        clipper.Begin(fTrace->fSource.size());
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                size_t humanReadableLine = row + 1;

                ImGui::TableNextRow();
                if (fPlayer.getCurrentLine() == (int)humanReadableLine) {
                    ImGui::TableSetBgColor(
                            ImGuiTableBgTarget_RowBg1,
                            ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg)));
                }
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%03zu ", humanReadableLine);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", fTrace->fSource[row].c_str());
            }
        }

        if (updateScroll) {
            int linesVisible = contentRect.y / ImGui::GetTextLineHeightWithSpacing();
            int centerLine = (fPlayer.getCurrentLine() - 1) - (linesVisible / 2);
            centerLine = std::max(0, centerLine);
            ImGui::SetScrollY(clipper.ItemsHeight * centerLine);
        }

        ImGui::EndTable();
    }
}

void SkSLDebuggerSlide::showRootGUI() {
    if (fTrace->fSource.empty()) {
        this->showLoadTraceGUI();
        return;
    }

    this->showDebuggerGUI();
}

void SkSLDebuggerSlide::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    ImGui::Begin("Debugger", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    this->showRootGUI();
    ImGui::End();
}

bool SkSLDebuggerSlide::animate(double nanos) {
    return true;
}

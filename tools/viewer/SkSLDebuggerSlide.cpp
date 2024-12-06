/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SkSLDebuggerSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "tools/sk_app/Application.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "imgui.h"

using namespace sk_app;
using LineNumberMap = SkSL::SkSLDebugTracePlayer::LineNumberMap;

SkSLDebuggerSlide::SkSLDebuggerSlide() {
    fName = "Debugger";
    fTrace = sk_make_sp<SkSL::DebugTracePriv>();
}

void SkSLDebuggerSlide::load(SkScalar winWidth, SkScalar winHeight) {}

void SkSLDebuggerSlide::unload() {
    fTrace = sk_make_sp<SkSL::DebugTracePriv>();
    fPlayer.reset(nullptr);
    fPlayer.setBreakpoints(std::unordered_set<int>{});
}

void SkSLDebuggerSlide::showLoadTraceGUI() {
    ImGui::InputText("Trace Path", fTraceFile, std::size(fTraceFile));
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
            fRefresh = true;
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
    if (ImGui::Button("Reset")) {
        fPlayer.reset(fTrace);
        fRefresh = true;
    }
    ImGui::SameLine(/*offset_from_start_x=*/0, /*spacing=*/100);
    if (ImGui::Button("Step")) {
        fPlayer.step();
        fRefresh = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step Over")) {
        fPlayer.stepOver();
        fRefresh = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step Out")) {
        fPlayer.stepOut();
        fRefresh = true;
    }
    ImGui::SameLine(/*offset_from_start_x=*/0, /*spacing=*/100);
    if (ImGui::Button(fPlayer.getBreakpoints().empty() ? "Run" : "Run to Breakpoint")) {
        fPlayer.run();
        fRefresh = true;
    }

    this->showStackTraceTable();
    this->showVariableTable();
    this->showCodeTable();
}

void SkSLDebuggerSlide::showCodeTable() {
    constexpr ImGuiTableFlags kTableFlags =
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
            ImGuiTableFlags_BordersV;
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

                // Show line numbers and breakpoints.
                ImGui::TableSetColumnIndex(0);
                const LineNumberMap& lineNumberMap = fPlayer.getLineNumbersReached();
                LineNumberMap::const_iterator iter = lineNumberMap.find(humanReadableLine);
                bool reachable = iter != lineNumberMap.end() && iter->second > 0;
                bool breakpointOn = fPlayer.getBreakpoints().count(humanReadableLine);
                if (breakpointOn) {
                    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(1.0f, 0.0f, 0.0f, 0.70f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.85f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                } else if (reachable) {
                    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 0.75f));
                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 0.25f));
                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
                }
                if (ImGui::SmallButton(SkStringPrintf("%03zu ", humanReadableLine).c_str())) {
                    if (breakpointOn) {
                        fPlayer.removeBreakpoint(humanReadableLine);
                    } else if (reachable) {
                        fPlayer.addBreakpoint(humanReadableLine);
                    }
                }
                ImGui::PopStyleColor(4);

                // Show lines of code.
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", fTrace->fSource[row].c_str());
            }
        }

        if (fRefresh) {
            int linesVisible = contentRect.y / ImGui::GetTextLineHeightWithSpacing();
            int centerLine = (fPlayer.getCurrentLine() - 1) - (linesVisible / 2);
            centerLine = std::max(0, centerLine);
            ImGui::SetScrollY(clipper.ItemsHeight * centerLine);
            fRefresh = false;
        }

        ImGui::EndTable();
    }
}

void SkSLDebuggerSlide::showStackTraceTable() {
    constexpr ImGuiTableFlags kTableFlags =
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
            ImGuiTableFlags_BordersV | ImGuiTableFlags_NoHostExtendX;
    constexpr ImGuiTableColumnFlags kColumnFlags =
            ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide |
            ImGuiTableColumnFlags_NoSort;

    std::vector<int> callStack = fPlayer.getCallStack();

    ImVec2 contentRect = ImGui::GetContentRegionAvail();
    ImVec2 stackViewSize = ImVec2(contentRect.x / 3.0f,
                                  ImGui::GetTextLineHeightWithSpacing() * kNumTopRows);
    if (ImGui::BeginTable("Call Stack", /*column=*/1, kTableFlags, stackViewSize)) {
        ImGui::TableSetupColumn("Stack", kColumnFlags);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(callStack.size());
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                int funcIdx = callStack.rbegin()[row];
                SkASSERT(funcIdx >= 0 && (size_t)funcIdx < fTrace->fFuncInfo.size());
                const SkSL::FunctionDebugInfo& funcInfo = fTrace->fFuncInfo[funcIdx];

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", funcInfo.name.c_str());
            }
        }
        ImGui::EndTable();
    }

    ImGui::SameLine();
}

void SkSLDebuggerSlide::showVariableTable() {
    constexpr ImGuiTableFlags kTableFlags =
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
            ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
    constexpr ImGuiTableColumnFlags kColumnFlags =
            ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide |
            ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthStretch;

    int frame = fPlayer.getStackDepth() - 1;
    std::vector<SkSL::SkSLDebugTracePlayer::VariableData> vars;
    if (frame >= 0) {
        vars = fPlayer.getLocalVariables(frame);
    } else {
        vars = fPlayer.getGlobalVariables();
    }
    ImVec2 varViewSize = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * kNumTopRows);
    if (ImGui::BeginTable("Variables", /*column=*/2, kTableFlags, varViewSize)) {
        ImGui::TableSetupColumn("Variable", kColumnFlags);
        ImGui::TableSetupColumn("Value", kColumnFlags);
        ImGui::TableHeadersRow();
        if (!vars.empty()) {
            ImGuiListClipper clipper;
            clipper.Begin(vars.size());
            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                    const SkSL::SkSLDebugTracePlayer::VariableData& var = vars.at(row);
                    SkASSERT(var.fSlotIndex >= 0);
                    SkASSERT((size_t)var.fSlotIndex < fTrace->fSlotInfo.size());
                    const SkSL::SlotDebugInfo& slotInfo = fTrace->fSlotInfo[var.fSlotIndex];

                    ImGui::TableNextRow();
                    if (var.fDirty) {
                        // Highlight recently-changed variables.
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1,
                                               ImGui::GetColorU32(ImVec4{0.0f, 1.0f, 0.0f, 0.20f}));
                    }
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s%s", slotInfo.name.c_str(),
                                        fTrace->getSlotComponentSuffix(var.fSlotIndex).c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s",
                                fTrace->slotValueToString(var.fSlotIndex, var.fValue).c_str());
                }
            }
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

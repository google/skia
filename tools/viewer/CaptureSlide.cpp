/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/CaptureSlide.h"

#include "imgui.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkDebug.h"
#include "include/private/SkLog.h"
#include "src/capture/SkCapture.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkRecord.h"
#include "src/utils/SkJSONWriter.h"
#include "tools/ProcsUtils.h"
#include "tools/UrlDataManager.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/debugger/DrawCommand.h"

#include <string>

const char* RecordTypeNames[] = {
#define CASE(T) #T,
        SK_RECORD_TYPES(CASE)
#undef CASE
};

const int JSON_TEXT_BOX_HEIGHT = 200;
// A width of -1 means use all available width
const int JSON_TEXT_BOX_WIDTH = -1;

const float EXPLORER_DEFAULT_WIDTH = 450.0f;
const float EXPLORER_DEFAULT_HEIGHT = 500.0f;
const float EXPLORER_DEFAULT_POS_X = 20.0f;
const float EXPLORER_DEFAULT_POS_Y = 20.0f;

CaptureSlide::CaptureSlide(const SkString& name, const SkString& path) {
    auto data = SkData::MakeFromFileName(path.c_str());
    fCapture = SkCapture::MakeFromData(data);
    if (fCapture) {
        fMetadata = fCapture->getMetadata();
        this->updateActiveAssetIndex();
    } else {
        SKIA_LOG_E("Couldn't load capture %s", path.c_str());
    }
}

CaptureSlide::~CaptureSlide() {}

void drawCommandHistory(const DebugCanvas* debugCanvas) {
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Appearing);
    if (!ImGui::Begin("Command History")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Total Commands: %d", debugCanvas->getSize());
    ImGui::Separator();

    ImGui::BeginChild("CommandList", ImVec2(0, 0), true);
    for (int i = 0; i < debugCanvas->getSize(); ++i) {
        DrawCommand* cmd = debugCanvas->getDrawCommandAt(i);
        const char* commandName = DrawCommand::GetCommandString(cmd->getOpType());

        ImGui::PushID(i);
        if (ImGui::TreeNode(commandName, "%d: %s", i, commandName)) {
            SkDynamicMemoryWStream stream;
            SkJSONWriter writer(
                    &stream, ToolUtils::default_serial_procs(), SkJSONWriter::Mode::kPretty);
            UrlDataManager urlDataManager(SkString("data"));
            writer.beginObject();
            cmd->toJSON(writer, urlDataManager);
            writer.endObject();
            writer.flush();

            stream.write8(0);
            sk_sp<SkData> data = stream.detachAsData();
            char* dataString = const_cast<char*>(static_cast<const char*>(data->data()));

            ImGui::InputTextMultiline("###json_output",
                                      dataString,
                                      data->size(),
                                      ImVec2(JSON_TEXT_BOX_WIDTH, JSON_TEXT_BOX_HEIGHT),
                                      ImGuiInputTextFlags_ReadOnly);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::End();
}

void drawRecordingTimelineTab(SkCapture* capture,
                              int& currentAssetIndex,
                              int& currentRecordingCaptureIndex,
                              int& currentDrawTaskIndex,
                              bool& invalidate) {
    ImGui::Text("Total Recordings: %u", capture->getMetadata().numRecordingCaptures);
    ImGui::Separator();

    ImGui::Columns(2, "TimelineSplit", true);

    // Column 0: Recordings List
    ImGui::Text("Recordings:");
    ImGui::BeginChild("RecordingList", ImVec2(0, 300), true);
    for (int i = 0; i < (int)capture->getMetadata().numRecordingCaptures; ++i) {
        const auto* rec = capture->getRecordingCapture(i);
        if (!rec) continue;
        SkString label = SkStringPrintf("Recording #%d", i);
        const bool isSelected = (currentRecordingCaptureIndex == i);
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            if (currentRecordingCaptureIndex != i) {
                currentRecordingCaptureIndex = i;
                currentDrawTaskIndex = 0; // reset draw task selection
                invalidate = true;
            }
        }
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    // Column 1: DrawTasks List
    const auto* activeRec = capture->getRecordingCapture(currentRecordingCaptureIndex);
    if (activeRec) {
        ImGui::Text("DrawTasks (%d):", (int)activeRec->fDrawTasks.size());
        ImGui::BeginChild("DrawTaskList", ImVec2(0, 300), true);
        for (int j = 0; j < (int)activeRec->fDrawTasks.size(); ++j) {
            const auto& task = activeRec->fDrawTasks[j];
            uint32_t assetIdx = task.fAssetIndex;
            auto pic = capture->getAsset(assetIdx);
            SkString label;
            if (pic) {
                label = SkStringPrintf("Task #%d: Asset %u (%dx%d)",
                                       j, assetIdx, (int)pic->cullRect().width(), (int)pic->cullRect().height());
            } else {
                label = SkStringPrintf("Task #%d: Asset %u (Null)", j, assetIdx);
            }
            const bool isSelected = (currentDrawTaskIndex == j);
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                if (currentDrawTaskIndex != j) {
                    currentDrawTaskIndex = j;
                    invalidate = true;
                }
            }
        }
        ImGui::EndChild();

        // Update currentAssetIndex to point to the selected sub-picture's asset index
        if (currentDrawTaskIndex >= 0 && currentDrawTaskIndex < (int)activeRec->fDrawTasks.size()) {
            currentAssetIndex = activeRec->fDrawTasks[currentDrawTaskIndex].fAssetIndex;
        } else {
            currentAssetIndex = -1;
        }
    } else {
        ImGui::Text("No active recording selected.");
    }

    ImGui::Columns(1); // Reset back to single column
}

void drawFlatAssetCatalogTab(SkCapture* capture,
                             int& currentAssetIndex,
                             bool& invalidate) {
    ImGui::Text("Total Picture Assets: %u", capture->getMetadata().numAssets);
    ImGui::Separator();
    ImGui::BeginChild("FlatCatalogList", ImVec2(0, 0), true);
    for (int i = 0; i < (int)capture->getMetadata().numAssets; ++i) {
        auto pic = capture->getAsset(i);
        SkString label;
        if (pic) {
            label = SkStringPrintf("Asset Index %d (%dx%d)",
                                   i, (int)pic->cullRect().width(), (int)pic->cullRect().height());
        } else {
            label = SkStringPrintf("Asset Index %d (Null)", i);
        }
        const bool isSelected = (currentAssetIndex == i);
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            if (currentAssetIndex != i) {
                currentAssetIndex = i;
                invalidate = true;
            }
        }
    }
    ImGui::EndChild();
}

void drawCaptureExplorer(SkCapture* capture,
                         int& currentAssetIndex,
                         int& currentRecordingCaptureIndex,
                         int& currentDrawTaskIndex,
                         bool& invalidate) {
    ImGui::SetNextWindowSize(ImVec2(EXPLORER_DEFAULT_WIDTH, EXPLORER_DEFAULT_HEIGHT),
                             ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(EXPLORER_DEFAULT_POS_X, EXPLORER_DEFAULT_POS_Y),
                            ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("SkCapture Explorer")) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("ExplorerTabBar")) {
        if (ImGui::BeginTabItem("Recording Timeline")) {
            drawRecordingTimelineTab(capture,
                                     currentAssetIndex,
                                     currentRecordingCaptureIndex,
                                     currentDrawTaskIndex,
                                     invalidate);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Flat Asset Catalog")) {
            drawFlatAssetCatalogTab(capture, currentAssetIndex, invalidate);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void CaptureSlide::draw(SkCanvas* canvas) {
    if (!fCapture) {
        return;
    }

    drawCaptureExplorer(fCapture.get(),
                        fCurrentAssetIndex,
                        fCurrentRecordingCaptureIndex,
                        fCurrentDrawTaskIndex,
                        fInvalidate);

    auto focusPicture = fCapture->getAsset(fCurrentAssetIndex);
    if (!focusPicture) {
        // TODO handle this case with an elegant UI popup.
        return;
    }

    auto bounds = focusPicture->cullRect().roundOut();

    if (!fDebugCanvas) {
        fDebugCanvas = std::make_unique<DebugCanvas>(bounds.width(), bounds.height());
        focusPicture->playback(fDebugCanvas.get());
    }

    canvas->save();
    canvas->clipIRect(bounds, SkClipOp::kIntersect);
    canvas->drawPicture(focusPicture);
    canvas->restore();

    drawCommandHistory(fDebugCanvas.get());
}

bool CaptureSlide::animate(double) {
    if (fInvalidate) {
        fInvalidate = false;
        return true;
    }
    return fInvalidate;
}

void CaptureSlide::load(SkScalar, SkScalar) {}

void CaptureSlide::unload() { fCapture.reset(nullptr); }

SkISize CaptureSlide::getDimensions() const { return {0, 0}; }

bool CaptureSlide::onChar(SkUnichar c) {
    switch (c) {
        case 'N': {
            int numSubs = fMetadata.numRecordingCaptures;
            if (numSubs > 0) {
                fCurrentRecordingCaptureIndex = (fCurrentRecordingCaptureIndex + 1) % numSubs;
                fCurrentDrawTaskIndex = 0;
                this->updateActiveAssetIndex();
                fInvalidate = true;
                fDebugCanvas.reset();
            }
            return true;
        }
        case 'P': {
            int numSubs = fMetadata.numRecordingCaptures;
            if (numSubs > 0) {
                fCurrentRecordingCaptureIndex = (fCurrentRecordingCaptureIndex + numSubs - 1)
                    % numSubs;
                fCurrentDrawTaskIndex = 0;
                this->updateActiveAssetIndex();
                fInvalidate = true;
                fDebugCanvas.reset();
            }
            return true;
        }
        case 'n': {
            const auto* activeRec = fCapture->getRecordingCapture(fCurrentRecordingCaptureIndex);
            if (activeRec && activeRec->fDrawTasks.size() > 0) {
                int numPics = activeRec->fDrawTasks.size();
                fCurrentDrawTaskIndex = (fCurrentDrawTaskIndex + 1) % numPics;
                this->updateActiveAssetIndex();
                fInvalidate = true;
                fDebugCanvas.reset();
            }
            return true;
        }
        case 'p': {
            const auto* activeRec = fCapture->getRecordingCapture(fCurrentRecordingCaptureIndex);
            if (activeRec && activeRec->fDrawTasks.size() > 0) {
                int numPics = activeRec->fDrawTasks.size();
                fCurrentDrawTaskIndex = (fCurrentDrawTaskIndex + numPics - 1) % numPics;
                this->updateActiveAssetIndex();
                fInvalidate = true;
                fDebugCanvas.reset();
            }
            return true;
        }
        case 'F':
            ImGui::GetIO().FontGlobalScale = ImGui::GetIO().FontGlobalScale + 0.1f;
            return true;
        case 'S':
            ImGui::GetIO().FontGlobalScale = ImGui::GetIO().FontGlobalScale - 0.1f;
            return true;
    }

    return Slide::onChar(c);
}

void CaptureSlide::updateActiveAssetIndex() {
    if (!fCapture) {
        return;
    }
    const auto* activeRec = fCapture->getRecordingCapture(fCurrentRecordingCaptureIndex);
    if (activeRec && !activeRec->fDrawTasks.empty()) {
        fCurrentDrawTaskIndex = SkTPin(fCurrentDrawTaskIndex, 0, activeRec->fDrawTasks.size() - 1);
        fCurrentAssetIndex = activeRec->fDrawTasks[fCurrentDrawTaskIndex].fAssetIndex;
    } else {
        fCurrentAssetIndex = -1;
    }
}

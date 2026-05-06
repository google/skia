/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/CaptureSlide.h"

#include <string>

#include "imgui.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkDebug.h"
#include "src/capture/SkCapture.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkRecord.h"
#include "src/utils/SkJSONWriter.h"
#include "tools/UrlDataManager.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/debugger/DrawCommand.h"

const char* RecordTypeNames[] = {
#define CASE(T) #T,
        SK_RECORD_TYPES(CASE)
#undef CASE
};

CaptureSlide::CaptureSlide(const SkString& name, const SkString& path) {
    auto data = SkData::MakeFromFileName(path.c_str());
    fCapture = SkCapture::MakeFromData(data);
    if (fCapture) {
        fMetadata = fCapture->getMetadata();
    } else {
        SkDebugf("Couldn't load capture %s", path.c_str());
    }
}

CaptureSlide::~CaptureSlide() {}

void drawCommandHistory(const DebugCanvas* debugCanvas) {
    static int currentCommandIdx = 0;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Appearing);
    if (!ImGui::Begin("Command History")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Total Commands: %d", debugCanvas->getSize());
    ImGui::Separator();

    const char* previewValue = "Select a command...";
    if (currentCommandIdx >= 0 && currentCommandIdx < debugCanvas->getSize()) {
        DrawCommand* cmd = debugCanvas->getDrawCommandAt(currentCommandIdx);
        previewValue = DrawCommand::GetCommandString(cmd->getOpType());
    }

    if (ImGui::BeginCombo("Select Command", previewValue)) {
        for (int i = 0; i < debugCanvas->getSize(); ++i) {
            const bool isSelected = (currentCommandIdx == i);

            DrawCommand* cmd = debugCanvas->getDrawCommandAt(i);
            const char* commandName = DrawCommand::GetCommandString(cmd->getOpType());

            if (ImGui::Selectable(commandName, isSelected)) {
                currentCommandIdx = i;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (currentCommandIdx >= 0 && currentCommandIdx < debugCanvas->getSize()) {
        ImGui::Separator();
        ImGui::Text("JSON for Command #%d:", currentCommandIdx);

        DrawCommand* cmd = debugCanvas->getDrawCommandAt(currentCommandIdx);

        SkDynamicMemoryWStream stream;
        SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
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
                                  ImVec2(-1, -1),
                                  ImGuiInputTextFlags_ReadOnly);
    }
    ImGui::End();
}

void drawMetadata(int currentPictureIndex) {
    ImGui::Begin("Capture Info");
    ImGui::Text("Current Index: %d", currentPictureIndex);
    ImGui::End();
}

void CaptureSlide::draw(SkCanvas* canvas) {
    if (!fCapture) {
        return;
    }

    auto focusPicture = fCapture->getPicture(fCurrentPictureIndex);

    auto bounds = focusPicture->cullRect().roundOut();
    DebugCanvas debugCanvas(bounds.width(), bounds.height());
    focusPicture->playback(&debugCanvas);

    canvas->clipIRect(bounds, SkClipOp::kIntersect);
    canvas->drawPicture(fCapture->getPicture(fCurrentPictureIndex));

    drawMetadata(fCurrentPictureIndex);

    drawCommandHistory(&debugCanvas);
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
        case 'N':
            fCurrentPictureIndex = (fCurrentPictureIndex + 1) % fMetadata.numPictures;
            fInvalidate = true;
            return true;
        case 'P':
            fCurrentPictureIndex =
                    (fCurrentPictureIndex + fMetadata.numPictures - 1) % fMetadata.numPictures;
            fInvalidate = true;
            return true;
        case 'F':
            ImGui::GetIO().FontGlobalScale = ImGui::GetIO().FontGlobalScale + 0.1f;
            return true;
        case 'S':
            ImGui::GetIO().FontGlobalScale = ImGui::GetIO().FontGlobalScale - 0.1f;
            return true;
    }

    return Slide::onChar(c);
}

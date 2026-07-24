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
#include "include/private/SkDebug.h"
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

    if (!fDebugCanvas) {
        fDebugCanvas = std::make_unique<DebugCanvas>(bounds.width(), bounds.height());
        focusPicture->playback(fDebugCanvas.get());
    }

    canvas->clipIRect(bounds, SkClipOp::kIntersect);
    canvas->drawPicture(focusPicture);

    drawMetadata(fCurrentPictureIndex);

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
        case 'N':
            fCurrentPictureIndex = (fCurrentPictureIndex + 1) % fMetadata.numPictures;
            fInvalidate = true;
            fDebugCanvas.reset();
            return true;
        case 'P':
            fCurrentPictureIndex =
                    (fCurrentPictureIndex + fMetadata.numPictures - 1) % fMetadata.numPictures;
            fInvalidate = true;
            fDebugCanvas.reset();
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

/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkTestCanvas.h"

#include "include/codec/SkCodec.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"  // IWYU pragma: keep
#include "include/core/SkData.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/base/SkDebug.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/text/GlyphRun.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>

class SkPaint;

SkTestCanvas<SkSlugTestKey>::SkTestCanvas(SkCanvas* canvas)
        : SkCanvas(sk_ref_sp(canvas->rootDevice())) {}

void SkTestCanvas<SkSlugTestKey>::onDrawGlyphRunList(
        const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) {
    SkRect bounds = glyphRunList.sourceBoundsWithOrigin();
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }
    auto layer = this->aboutToDraw(this, paint, &bounds);
    if (layer) {
        if (glyphRunList.hasRSXForm()) {
            this->SkCanvas::onDrawGlyphRunList(glyphRunList, layer->paint());
        } else {
            auto slug = this->onConvertGlyphRunListToSlug(glyphRunList, layer->paint());
            this->drawSlug(slug.get());
        }
    }
}

SkTestCanvas<SkSerializeSlugTestKey>::SkTestCanvas(SkCanvas* canvas)
        : SkCanvas(sk_ref_sp(canvas->rootDevice())) {}

void SkTestCanvas<SkSerializeSlugTestKey>::onDrawGlyphRunList(
        const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) {
    SkRect bounds = glyphRunList.sourceBoundsWithOrigin();
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }
    auto layer = this->aboutToDraw(this, paint, &bounds);
    if (layer) {
        if (glyphRunList.hasRSXForm()) {
            this->SkCanvas::onDrawGlyphRunList(glyphRunList, layer->paint());
        } else {
            sk_sp<SkData> bytes;
            {
                auto slug = this->onConvertGlyphRunListToSlug(glyphRunList, layer->paint());
                if (slug != nullptr) {
                    SkSerialProcs procs;
                    procs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
                        return SkPngEncoder::Encode(nullptr, img, SkPngEncoder::Options{});
                    };
                    bytes = slug->serialize(procs);
                }
            }
            {
                if (bytes != nullptr) {
                    SkDeserialProcs procs;
                    procs.fImageProc =
                            [](const void* bytes, size_t length, void*) -> sk_sp<SkImage> {
                        auto data = SkData::MakeWithoutCopy(bytes, length);
                        auto codec = SkPngDecoder::Decode(data, nullptr);
                        SkASSERT(codec);
                        return std::get<0>(codec->getImage());
                    };
                    auto slug = sktext::gpu::Slug::Deserialize(
                            bytes->data(), bytes->size(), nullptr, procs);
                    this->drawSlug(slug.get());
                }
            }
        }
    }
}


// A do nothing handle manager for the remote strike server.
class ServerHandleManager : public SkStrikeServer::DiscardableHandleManager {
public:
    SkDiscardableHandleId createHandle() override {
        return 0;
    }

    bool lockHandle(SkDiscardableHandleId id) override {
        return true;
    }

    bool isHandleDeleted(SkDiscardableHandleId id) override {
        return false;
    }
};

// Lock the strikes into the cache for the length of the test. This handler is tied to the lifetime
// of the canvas used to render the entire test.
class ClientHandleManager : public SkStrikeClient::DiscardableHandleManager {
public:
    bool deleteHandle(SkDiscardableHandleId id) override {
        return fIsLocked;
    }

    void assertHandleValid(SkDiscardableHandleId id) override {
        DiscardableHandleManager::assertHandleValid(id);
    }

    void notifyCacheMiss(SkStrikeClient::CacheMissType type, int fontSize) override {

    }

    void notifyReadFailure(const ReadFailureData& data) override {
        DiscardableHandleManager::notifyReadFailure(data);
    }

    void unlock() {
        fIsLocked = true;
    }

private:
    bool fIsLocked{false};
};

SkTestCanvas<SkRemoteSlugTestKey>::SkTestCanvas(SkCanvas* canvas)
        : SkCanvas(sk_ref_sp(canvas->rootDevice()))
        , fServerHandleManager(new ServerHandleManager{})
        , fClientHandleManager(new ClientHandleManager{})
        , fStrikeServer(fServerHandleManager.get())
        , fStrikeClient(fClientHandleManager) {}

// Allow the strikes to be freed from the strike cache after the test has been drawn.
SkTestCanvas<SkRemoteSlugTestKey>::~SkTestCanvas() {
    static_cast<ClientHandleManager*>(fClientHandleManager.get())->unlock();
}

void SkTestCanvas<SkRemoteSlugTestKey>::onDrawGlyphRunList(
        const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) {
    SkRect bounds = glyphRunList.sourceBoundsWithOrigin();
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }
    auto layer = this->aboutToDraw(this, paint, &bounds);
    if (layer) {
        if (glyphRunList.hasRSXForm()) {
            this->SkCanvas::onDrawGlyphRunList(glyphRunList, layer->paint());
        } else {
            sk_sp<SkData> slugBytes;
            std::vector<uint8_t> glyphBytes;
            {
                auto analysisCanvas = fStrikeServer.makeAnalysisCanvas(
                        this->topDevice()->width(),
                        this->topDevice()->height(),
                        this->fProps,
                        this->topDevice()->imageInfo().refColorSpace(),
                        // TODO: Where should we get this value from?
                        /*DFTSupport=*/ true);

                // TODO: Move the analysis canvas processing up to the via to handle a whole
                //  document at a time. This is not the correct way to handle the CTM; it doesn't
                //  work for layers.
                analysisCanvas->setMatrix(this->getLocalToDevice());
                auto slug = analysisCanvas->onConvertGlyphRunListToSlug(glyphRunList,
                                                                        layer->paint());
                if (slug != nullptr) {
                    SkSerialProcs procs;
                    procs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
                        return SkPngEncoder::Encode(nullptr, img, SkPngEncoder::Options{});
                    };
                    slugBytes = slug->serialize(procs);
                }
                fStrikeServer.writeStrikeData(&glyphBytes);
            }
            {
                if (!glyphBytes.empty()) {
                    fStrikeClient.readStrikeData(glyphBytes.data(), glyphBytes.size());
                }
                if (slugBytes != nullptr) {
                    SkDeserialProcs procs;
                    procs.fImageProc =
                            [](const void* bytes, size_t length, void*) -> sk_sp<SkImage> {
                        auto data = SkData::MakeWithoutCopy(bytes, length);
                        auto codec = SkPngDecoder::Decode(data, nullptr);
                        SkASSERT(codec);
                        return std::get<0>(codec->getImage());
                    };
                    auto slug = sktext::gpu::Slug::Deserialize(
                            slugBytes->data(), slugBytes->size(), &fStrikeClient, procs);
                    this->drawSlug(slug.get());
                }
            }
        }
    }
}

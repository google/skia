/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// TestCanvas is a simple way to make a testing canvas which is allowed to use private
// facilities of SkCanvas without having to add a friend to SkCanvas.h.
//
// You create a Key (a simple empty struct) to make a template specialization class. You need to
// make a key for each of the different Canvases you need. The implementations of the canvases
// are in SkCanvas.cpp, which allows the use of helper classes.

#ifndef TestCanvas_DEFINED
#define TestCanvas_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"

#include <memory>

class SkPaint;

namespace sktext { class GlyphRunList; }

namespace skiatest {

// You can only make template specializations of TestCanvas.
template <typename Key> class TestCanvas;

// A test canvas to test using slug rendering instead of text blob rendering.
struct SkSlugTestKey {};
template <>
class TestCanvas<SkSlugTestKey> : public SkCanvas {
public:
    TestCanvas(SkCanvas* canvas);
    void onDrawGlyphRunList(
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) override;
};

struct SkSerializeSlugTestKey {};
template <>
class TestCanvas<SkSerializeSlugTestKey> : public SkCanvas {
public:
    TestCanvas(SkCanvas* canvas);
    void onDrawGlyphRunList(
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) override;
};

struct SkRemoteSlugTestKey {};
template <>
class TestCanvas<SkRemoteSlugTestKey> : public SkCanvas {
public:
    TestCanvas(SkCanvas* canvas);
    ~TestCanvas() override;
    void onDrawGlyphRunList(
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) override;

private:
    std::unique_ptr<SkStrikeServer::DiscardableHandleManager> fServerHandleManager;
    sk_sp<SkStrikeClient::DiscardableHandleManager> fClientHandleManager;
    SkStrikeServer fStrikeServer;
    SkStrikeClient fStrikeClient;
};

}  // namespace skiatest

#endif  // TestCanvas_DEFINED

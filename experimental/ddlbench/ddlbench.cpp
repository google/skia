// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/ddlbench/Cmds.h"
#include "experimental/ddlbench/Fake.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkOSFile.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/utils/SkOSPath.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"

#include <algorithm>

using sk_gpu_test::GrContextFactory;

static DEFINE_string(png, "", "if set, save a .png proof to disk at this file location");
static DEFINE_string(src, "", "input .skp file");

static void exitf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(1);
}

static void check_params(GrDirectContext* dContext,
                         int width, int height, SkColorType ct, SkAlphaType at, int numSamples) {

    if (dContext->maxRenderTargetSize() < std::max(width, height)) {
        exitf("render target size %ix%i not supported by platform (max: %i)",
              width, height, dContext->maxRenderTargetSize());
    }

    GrBackendFormat format = dContext->defaultBackendFormat(ct, GrRenderable::kYes);
    if (!format.isValid()) {
        exitf("failed to get GrBackendFormat from SkColorType: %d", ct);
    }

    int supportedSampleCount = dContext->priv().caps()->getRenderTargetSampleCount(numSamples,
                                                                                   format);
    if (supportedSampleCount != numSamples) {
        exitf("sample count %i not supported by platform", numSamples);
    }
}

static bool mkdir_p(const SkString& dirname) {
    if (dirname.isEmpty() || dirname == SkString("/")) {
        return true;
    }
    return mkdir_p(SkOSPath::Dirname(dirname.c_str())) && sk_mkdir(dirname.c_str());
}

static void save_files(int testID, const SkBitmap& expected, const SkBitmap& actual) {
    char name[64];

    _snprintf(name, 64, "c://src//bugs//expected%d.png", testID);
    name[63] = '\0';

    if (!mkdir_p(SkOSPath::Dirname(name))) {
        exitf("failed to create directory for png \"%s\"", name);
    }
    if (!ToolUtils::EncodeImageToFile(name, expected, SkEncodedImageFormat::kPNG, 100)) {
        exitf("failed to save png to \"%s\"", name);
    }

    _snprintf(name, 64, "c://src//bugs//actual%d.png", testID);
    name[63] = '\0';

    if (!ToolUtils::EncodeImageToFile(name, actual, SkEncodedImageFormat::kPNG, 100)) {
        exitf("failed to save png to \"%s\"", name);
    }
}

// unit test - exercise sorting behavior
static void key_test() {
    SortKey k;
    SkASSERT(!k.transparent());
    int tmp = k.depth();
    SkASSERT(k.depth() == 0);
    int i = k.material();
    SkASSERT(k.material() == 0);
//    k.dump();

    SortKey k1(false, 1, 3);
    SkASSERT(!k1.transparent());
    tmp = k1.depth();
    SkASSERT(k1.depth() == 1);
    SkASSERT(k1.material() == 3);
//    k1.dump();

    SortKey k2(true, 2, 1);
    SkASSERT(k2.transparent());
    tmp = k2.depth();
    SkASSERT(k2.depth() == 2);
    SkASSERT(k2.material() == 1);
//    k2.dump();
}

static void check_order(const std::vector<int>& actualOrder,
                        const std::vector<int>& expectedOrder) {
    if (expectedOrder.size() != actualOrder.size()) {
        exitf("Op count mismatch. Expected %d - got %d\n",
              expectedOrder.size(),
              actualOrder.size());
    }

    if (expectedOrder != actualOrder) {
        SkDebugf("order mismatch:\n");
        SkDebugf("E %d: ", expectedOrder.size());
        for (auto t : expectedOrder) {
            SkDebugf("%d", t);
        }
        SkDebugf("\n");

        SkDebugf("A %d: ", actualOrder.size());
        for (auto t : actualOrder) {
            SkDebugf("%d", t);
        }
        SkDebugf("\n");
    }
}

typedef int (*PFTest)(std::vector<const Cmd*>* test, std::vector<int>* expectedOrder);

static void sort_test(PFTest testcase) {
    std::vector<const Cmd*> test;
    std::vector<int> expectedOrder;
    int testID = testcase(&test, &expectedOrder);


    SkBitmap expectedBM;
    expectedBM.allocPixels(SkImageInfo::MakeN32Premul(256, 256));
    expectedBM.eraseColor(SK_ColorBLACK);
    SkCanvas real(expectedBM);

    SkBitmap actualBM;
    actualBM.allocPixels(SkImageInfo::MakeN32Premul(256, 256));
    actualBM.eraseColor(SK_ColorBLACK);

    FakeCanvas fake(actualBM);
    for (auto c : test) {
        c->execute(&fake);
        c->execute(&real);
    }

    fake.finalize();

    std::vector<int> actualOrder = fake.getOrder();
    check_order(actualOrder, expectedOrder);

    save_files(testID, expectedBM, actualBM);
}

// Simple test - green rect should appear atop the red rect
static int test1(std::vector<const Cmd*>* test, std::vector<int>* expectedOrder) {
    expectedOrder->push_back(1);
    expectedOrder->push_back(0);

    test->push_back(new RectCmd(0, 1, SkIRect::MakeXYWH(10, 10 , 100, 100), SK_ColorRED,  SK_ColorBLACK));
    test->push_back(new RectCmd(1, 1, SkIRect::MakeXYWH(50, 50 , 100, 100), SK_ColorGREEN, SK_ColorBLACK));
    return 1;
}

// Simple test - blue rect atop green rect atop red rect
static int test2(std::vector<const Cmd*>* test, std::vector<int>* expectedOrder) {
    expectedOrder->push_back(2);
    expectedOrder->push_back(1);
    expectedOrder->push_back(0);

    test->push_back(new RectCmd(0, 1, SkIRect::MakeXYWH(10,   10 , 100, 100), SK_ColorRED,   SK_ColorBLACK));
    test->push_back(new RectCmd(1, 1, SkIRect::MakeXYWH(50,   50 , 100, 100), SK_ColorGREEN, SK_ColorBLACK));
    test->push_back(new RectCmd(2, 1, SkIRect::MakeXYWH(100, 100 , 100, 100), SK_ColorBLUE,  SK_ColorBLACK));
    return 2;
}

// Transparency test - opaque blue rect atop transparent green rect atop opaque red rect
static int test3(std::vector<const Cmd*>* test, std::vector<int>* expectedOrder) {
    expectedOrder->push_back(2);
    expectedOrder->push_back(0);
    expectedOrder->push_back(1);

    test->push_back(new RectCmd(0, 1, SkIRect::MakeXYWH(10,   10 , 100, 100), SK_ColorRED,  SK_ColorBLACK));
    test->push_back(new RectCmd(1, 1, SkIRect::MakeXYWH(50,   50 , 100, 100), 0x8000FF00,   SK_ColorBLACK));
    test->push_back(new RectCmd(2, 1, SkIRect::MakeXYWH(100, 100 , 100, 100), SK_ColorBLUE, SK_ColorBLACK));
    return 3;
}

// Multi-transparency test - transparent blue rect atop transparent green rect atop
// transparent red rect
static int test4(std::vector<const Cmd*>* test, std::vector<int>* expectedOrder) {
    expectedOrder->push_back(0);
    expectedOrder->push_back(1);
    expectedOrder->push_back(2);

    test->push_back(new RectCmd(0, 1, SkIRect::MakeXYWH(10,   10 , 100, 100), 0x80FF0000, SK_ColorBLACK));
    test->push_back(new RectCmd(1, 1, SkIRect::MakeXYWH(50,   50 , 100, 100), 0x8000FF00, SK_ColorBLACK));
    test->push_back(new RectCmd(2, 1, SkIRect::MakeXYWH(100, 100 , 100, 100), 0x800000FF, SK_ColorBLACK));
    return 4;
}

// All opaque:
//   normal1, linear1, radial1, normal2, linear2, radial2
// Which gets sorted to:
//   normal2, normal1, linear2, linear1, radial2, radial1
// So, front to back w/in each material type.
static int test5(std::vector<const Cmd*>* test, std::vector<int>* expectedOrder) {

    // This pushes sorting by material above sorting by Z. Thus we'll get less front to
    // back benefit.
    expectedOrder->push_back(3);
    expectedOrder->push_back(0);
    expectedOrder->push_back(4);
    expectedOrder->push_back(1);
    expectedOrder->push_back(5);
    expectedOrder->push_back(2);

    SkIRect r{0, 0, 100, 100};
    test->push_back(new RectCmd(0, 1, r.makeOffset(8, 8), SK_ColorRED, SK_ColorBLACK));
    test->push_back(new RectCmd(1, 2, r.makeOffset(48, 48), SK_ColorGREEN, SK_ColorWHITE));
    test->push_back(new RectCmd(2, 3, r.makeOffset(98, 98), SK_ColorBLUE, SK_ColorBLACK));
    test->push_back(new RectCmd(3, 1, r.makeOffset(148, 148), SK_ColorCYAN, SK_ColorBLACK));
    test->push_back(new RectCmd(4, 2, r.makeOffset(148, 8), SK_ColorMAGENTA, SK_ColorWHITE));
    test->push_back(new RectCmd(5, 3, r.makeOffset(8, 148), SK_ColorYELLOW, SK_ColorBLACK));
    return 5;
}

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);

    const GrContextFactory::ContextType kContextType = GrContextFactory::kGL_ContextType;
    const GrContextOptions kContextOptions;
    const GrContextFactory::ContextOverrides kOverrides = GrContextFactory::ContextOverrides::kNone;
    SkColorType ct = kRGBA_8888_SkColorType;
    SkAlphaType at = kPremul_SkAlphaType;

    SkGraphics::Init();

    GrContextFactory factory(kContextOptions);

    key_test();
    sort_test(test1);
    sort_test(test2);
    sort_test(test3);
    sort_test(test4);
    sort_test(test5);

    SkDebugf("done\n");
    return 0;
}

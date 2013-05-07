#include "SkBitmap.h"
#include "SkDevice.h"
#include "SkCanvas.h"
#include "SkImageEncoder.h"
#include "SkStream.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkString.h"
#include "Test.h"

static void make_filepath(SkString* path, const char* dir, const SkString& name) {
    size_t len = strlen(dir);
    path->set(dir);
    if (len > 0 && dir[len - 1] != '/') {
        path->append("\\");
    }
    path->append(name);
}

static void PathOpsSkpClipTest(skiatest::Reporter* reporter) {
const char pictDir[] = "C:\\Users\\caryclark\\skp";
    const char outSkpClipDir[] = "C:\\Users\\caryclark\\skpClip";
    const char outOldClipDir[] = "C:\\Users\\caryclark\\oldClip";
    SkOSFile::Iter iter(pictDir, "skp");
    SkString filename;
    while (iter.next(&filename)) {
#if 0
        if (strcmp(filename.c_str(), "tabl_androidpolice.skp")) {
            continue;
        }
#endif
        SkString path;
        make_filepath(&path, pictDir, filename);
        SkFILEStream stream(path.c_str());
        if (!stream.isValid()) {
            continue;
        }
        SkPicture* pic = SkNEW_ARGS(SkPicture, (&stream));
        int width = pic->width();
        int height = pic->height();
        SkBitmap bitmap;
        bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        bitmap.allocPixels();
        SkCanvas canvas(bitmap);
        filename.remove(filename.size() - 3, 3);
        filename.append("png");
        for (int i = 0; i < 2; ++i) {
            bool useOp = i ? true : false;
            canvas.setAllowSimplifyClip(useOp);
            pic->draw(&canvas);
            SkString outFile;
            make_filepath(&outFile, useOp ? outSkpClipDir : outOldClipDir, filename);
            SkImageEncoder::EncodeFile(outFile.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
        }
        SkDELETE(pic);
        reporter->bumpTestCount();
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsSkpClipTest)

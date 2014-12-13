#include "DMImageTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkImageDecoder.h"
#include "SkRandom.h"

#include <string.h>

namespace DM {

// This converts file names like "mandrill_128.r11.ktx" into
// "mandrill-128-r11_ktx" or "mandrill-128-r11-5-subsets_ktx".
static SkString task_name(SkString filename, int subsets) {
    const char* ext = strrchr(filename.c_str(), '.');
    SkString name(filename.c_str(), ext - filename.c_str());
    if (subsets > 0) {
        name.appendf("_%d_subsets", subsets);
    }
    name = FileToTaskName(name);  // Promote any stray '.' in the filename to '_'.
    name.append(ext);             // Tack on the extension, including the '.'.
    return FileToTaskName(name);  // Promote that last '.' to '_', other '_' to '-'.
}

ImageTask::ImageTask(Reporter* r, TaskRunner* t, const SkData* encoded, SkString name, int subsets)
    : CpuTask(r, t)
    , fEncoded(SkRef(encoded))
    , fName(task_name(name, subsets))
    , fSubsets(subsets) {}

void ImageTask::draw() {
    if (fSubsets == 0) {
        // Decoding the whole image is considerably simpler than decoding subsets!
        SkBitmap bitmap;
        if (!SkImageDecoder::DecodeMemory(fEncoded->data(), fEncoded->size(), &bitmap)) {
            return this->fail("Can't DecodeMemory");
        }
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, "image", bitmap)));
        return;
    }

    SkMemoryStream stream(fEncoded->data(), fEncoded->size());
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(&stream));
    if (!decoder) {
        return this->fail("Can't find good decoder.");
    }

    int w,h;
    if (!decoder->buildTileIndex(&stream, &w, &h) || w*h == 1) {
        return;  // Subset decoding is not always supported.
    }

    SkBitmap composite;
    composite.allocN32Pixels(w,h);  // We're lazy here and just always use native 8888.
    composite.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(composite);

    SkRandom rand;
    for (int i = 0; i < fSubsets; i++) {
        SkIRect rect;
        do {
            rect.fLeft   = rand.nextULessThan(w);
            rect.fTop    = rand.nextULessThan(h);
            rect.fRight  = rand.nextULessThan(w);
            rect.fBottom = rand.nextULessThan(h);
            rect.sort();
        } while (rect.isEmpty());

        SkBitmap subset;
        if (!decoder->decodeSubset(&subset, rect, kN32_SkColorType)) {
            return this->fail("Could not decode subset.");
        }
        canvas.drawBitmap(subset, SkIntToScalar(rect.fLeft), SkIntToScalar(rect.fTop));
    }
    canvas.flush();
    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, "image", composite)));
}

}  // namespace DM

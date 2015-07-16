/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <VisualBench/VisualBenchmarkStream.h>
#include "GMBench.h"
#include "SkOSFile.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "VisualSKPBench.h"

DEFINE_string2(match, m, NULL,
               "[~][^]substring[$] [...] of bench name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching bench to always be skipped\n"
               "^ requires the start of the bench to match\n"
               "$ requires the end of the bench to match\n"
               "^ and $ requires an exact match\n"
               "If a bench does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");
DEFINE_string(skps, "skps", "Directory to read skps from.");

VisualBenchmarkStream::VisualBenchmarkStream()
    : fBenches(BenchRegistry::Head())
    , fGMs(skiagm::GMRegistry::Head())
    , fSourceType(NULL)
    , fBenchType(NULL)
    , fCurrentSKP(0) {
    for (int i = 0; i < FLAGS_skps.count(); i++) {
        if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
            fSKPs.push_back() = FLAGS_skps[i];
        } else {
            SkOSFile::Iter it(FLAGS_skps[i], ".skp");
            SkString path;
            while (it.next(&path)) {
                fSKPs.push_back() = SkOSPath::Join(FLAGS_skps[0], path.c_str());
            }
        }
    }
}

bool VisualBenchmarkStream::ReadPicture(const char* path, SkAutoTUnref<SkPicture>* pic) {
    // Not strictly necessary, as it will be checked again later,
    // but helps to avoid a lot of pointless work if we're going to skip it.
    if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path)) {
        return false;
    }

    SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
    if (stream.get() == NULL) {
        SkDebugf("Could not read %s.\n", path);
        return false;
    }

    pic->reset(SkPicture::CreateFromStream(stream.get()));
    if (pic->get() == NULL) {
        SkDebugf("Could not read %s as an SkPicture.\n", path);
        return false;
    }
    return true;
}

Benchmark* VisualBenchmarkStream::next() {
    Benchmark* bench;
    // skips non matching benches
    while ((bench = this->innerNext()) &&
           (SkCommandLineFlags::ShouldSkip(FLAGS_match, bench->getUniqueName()) ||
            !bench->isSuitableFor(Benchmark::kGPU_Backend))) {
        bench->unref();
    }
    return bench;
}

Benchmark* VisualBenchmarkStream::innerNext() {
    while (fBenches) {
        Benchmark* bench = fBenches->factory()(NULL);
        fBenches = fBenches->next();
        if (bench->isVisual()) {
            fSourceType = "bench";
            fBenchType  = "micro";
            return bench;
        }
        bench->unref();
    }

    while (fGMs) {
        SkAutoTDelete<skiagm::GM> gm(fGMs->factory()(NULL));
        fGMs = fGMs->next();
        if (gm->runAsBench()) {
            fSourceType = "gm";
            fBenchType  = "micro";
            return SkNEW_ARGS(GMBench, (gm.detach()));
        }
    }

    // Render skps
    while (fCurrentSKP < fSKPs.count()) {
        const SkString& path = fSKPs[fCurrentSKP++];
        SkAutoTUnref<SkPicture> pic;
        if (!ReadPicture(path.c_str(), &pic)) {
            continue;
        }

        SkString name = SkOSPath::Basename(path.c_str());
        fSourceType = "skp";
        fBenchType = "playback";
        return SkNEW_ARGS(VisualSKPBench, (name.c_str(), pic.get()));
    }

    return NULL;
}

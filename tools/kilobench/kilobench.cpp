/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCommandLineFlags.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "VisualSKPBench.h"

/*
 * This is an experimental GPU only benchmarking program.  The initial implementation will only
 * support SKPs.
 */

// To get image decoders linked in we have to do the below magic
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string(skps, "skps", "Directory to read skps from.");

DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of GM name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching bench to always be skipped\n"
               "^ requires the start of the bench to match\n"
               "$ requires the end of the bench to match\n"
               "^ and $ requires an exact match\n"
               "If a bench does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

namespace kilobench {
class BenchmarkStream {
public:
    BenchmarkStream() : fCurrentSKP(0) {
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

    Benchmark* next() {
        Benchmark* bench = nullptr;
        // skips non matching benches
        while ((bench = this->innerNext()) &&
               (SkCommandLineFlags::ShouldSkip(FLAGS_match, bench->getUniqueName()) ||
                !bench->isSuitableFor(Benchmark::kGPU_Backend))) {
            delete bench;
        }
        return bench;
    }

private:
    static bool ReadPicture(const char* path, SkAutoTUnref<SkPicture>* pic) {
        // Not strictly necessary, as it will be checked again later,
        // but helps to avoid a lot of pointless work if we're going to skip it.
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path)) {
            return false;
        }

        SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
        if (stream.get() == nullptr) {
            SkDebugf("Could not read %s.\n", path);
            return false;
        }

        pic->reset(SkPicture::CreateFromStream(stream.get()));
        if (pic->get() == nullptr) {
            SkDebugf("Could not read %s as an SkPicture.\n", path);
            return false;
        }
        return true;
    }

    Benchmark* innerNext() {
        // Render skps
        while (fCurrentSKP < fSKPs.count()) {
            const SkString& path = fSKPs[fCurrentSKP++];
            SkAutoTUnref<SkPicture> pic;
            if (!ReadPicture(path.c_str(), &pic)) {
                continue;
            }

            SkString name = SkOSPath::Basename(path.c_str());
            return new VisualSKPBench(name.c_str(), pic.get());
        }

        return nullptr;
    }

    SkTArray<SkString> fSKPs;
    int fCurrentSKP;
};

} // namespace kilobench

int kilobench_main() {
    kilobench::BenchmarkStream benchStream;
    while (Benchmark* b = benchStream.next()) {
        SkAutoTDelete<Benchmark> bench(b);
        // TODO actual stuff
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return kilobench_main();
}
#endif

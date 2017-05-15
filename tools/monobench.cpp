/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "OverwriteLine.h"
#include "SkGraphics.h"
#include "SkSurface.h"
#include "SkTaskGroup.h"
#include <algorithm>
#include <chrono>
#include <limits>
#include <regex>
#include <stdlib.h>
#include <string>
#include <vector>


#if defined(SK_BUILD_FOR_WIN32)
static const char* kEllipsis = "...";
#else
static const char* kEllipsis = "…";
#endif

int main(int argc, char** argv) {
    SkGraphics::Init();
    SkTaskGroup::Enabler enabled;

    using clock = std::chrono::high_resolution_clock;
    using ns = std::chrono::duration<double, std::nano>;

    std::regex pattern;
    int limit = 2147483647;
    if (argc > 1) { pattern = argv[1]; }
    if (argc > 2) { limit = atoi(argv[2]); }

    struct Bench {
        std::unique_ptr<Benchmark> b;
        std::string                name;
        ns                         best;
    };
    std::vector<Bench> benches;

    for (auto r = BenchRegistry::Head(); r; r = r->next()) {
        std::unique_ptr<Benchmark> bench{ r->factory()(nullptr) };

        std::string name = bench->getName();
        if (std::regex_search(name, pattern) &&
                (bench->isSuitableFor(Benchmark::kNonRendering_Backend) ||
                 bench->isSuitableFor(Benchmark::      kRaster_Backend))) {
            bench->delayedSetup();
            benches.emplace_back(Bench{std::move(bench), name,
                                       ns{std::numeric_limits<double>::infinity()}});
        }
    }

    if (benches.size() == 0) {
        SkDebugf("No bench matched.\n");
        return 1;
    }

    if (benches.size() > 1) {
        int common_prefix = benches[0].name.size();
        for (size_t i = 1; i < benches.size(); i++) {
            int len = std::mismatch(benches[i-1].name.begin(), benches[i-1].name.end(),
                                    benches[i-0].name.begin())
                .first - benches[i-1].name.begin();
            common_prefix = std::min(common_prefix, len);
        }
        std::string prefix = benches[0].name.substr(0, common_prefix);
        if (common_prefix) {
            for (auto& bench : benches) {
                bench.name.replace(0, common_prefix, kEllipsis);
            }
        }

        int common_suffix = benches[0].name.size();
        for (size_t i = 1; i < benches.size(); i++) {
            int len = std::mismatch(benches[i-1].name.rbegin(), benches[i-1].name.rend(),
                                    benches[i-0].name.rbegin())
                .first - benches[i-1].name.rbegin();
            common_suffix = std::min(common_suffix, len);
        }
        std::string suffix = benches[0].name.substr(benches[0].name.size() - common_suffix);
        if (common_suffix) {
            for (auto& bench : benches) {
                bench.name.replace(bench.name.size() - common_suffix, common_suffix, kEllipsis);
            }
        }

        SkDebugf("%s%s%s\n", prefix.c_str(), kEllipsis, suffix.c_str());
    }

    int samples = 0;
    while (samples < limit) {
        std::random_shuffle(benches.begin(), benches.end());
        for (auto& bench : benches) {
            sk_sp<SkSurface> dst;
            SkCanvas* canvas = nullptr;
            if (!bench.b->isSuitableFor(Benchmark::kNonRendering_Backend)) {
                dst = SkSurface::MakeRaster(SkImageInfo::MakeS32(bench.b->getSize().x(),
                                                                 bench.b->getSize().y(),
                                                                 kPremul_SkAlphaType));
                canvas = dst->getCanvas();
                bench.b->perCanvasPreDraw(canvas);
            }
            for (int loops = 1; loops < 1000000000;) {

                bench.b->preDraw(canvas);
                auto start = clock::now();
                    bench.b->draw(loops, canvas);
                ns elapsed = clock::now() - start;
                bench.b->postDraw(canvas);

                if (elapsed < std::chrono::milliseconds{10}) {
                    loops *= 2;
                    continue;
                }

                bench.best = std::min(bench.best, elapsed / loops);
                samples++;

                struct Result { const char* name; ns best; };
                std::vector<Result> sorted(benches.size());
                for (size_t i = 0; i < benches.size(); i++) {
                    sorted[i].name = benches[i].name.c_str();
                    sorted[i].best = benches[i].best;
                }
                std::sort(sorted.begin(), sorted.end(), [](const Result& a, const Result& b) {
                    return a.best < b.best;
                });

                SkDebugf("%s%d", kSkOverwriteLine, samples);
                for (auto& result : sorted) {
                    if (sorted.size() == 1) {
                        SkDebugf("  %s %gns" , result.name, result.best.count());
                    } else {
                        SkDebugf("  %s %.3gx", result.name, result.best / sorted[0].best);
                    }
                }
                break;
            }
            if (canvas) {
                bench.b->perCanvasPostDraw(canvas);
            }
        }
    }

    return 0;
}

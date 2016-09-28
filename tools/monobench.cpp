/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkGraphics.h"
#include "SkTaskGroup.h"
#include <algorithm>
#include <chrono>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

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
                bench->isSuitableFor(Benchmark::kNonRendering_Backend)) {
            bench->delayedSetup();
            benches.emplace_back(Bench{std::move(bench), name, ns{1.0/0.0}});
        }
    }

    if (benches.size() == 0) {
        printf("No bench matched.\n");
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
                bench.name.replace(0, common_prefix, "…");
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
                bench.name.replace(bench.name.size() - common_suffix, common_suffix, "…");
            }
        }

        printf("%s…%s\n", prefix.c_str(), suffix.c_str());
    }

    int samples = 0;
    while (samples < limit) {
        for (auto& bench : benches) {
            for (int loops = 1; loops < 1000000000;) {
                bench.b->preDraw(nullptr);
                auto start = clock::now();
                    bench.b->draw(loops, nullptr);
                ns elapsed = clock::now() - start;
                bench.b->postDraw(nullptr);

                if (elapsed < std::chrono::milliseconds{10}) {
                    loops *= 2;
                    continue;
                }

                bench.best = std::min(bench.best, elapsed / loops);
                samples++;

                std::sort(benches.begin(), benches.end(), [](const Bench& a, const Bench& b) {
                    return a.best < b.best;
                });
                printf("\r\033[K%d", samples);
                for (auto& bench : benches) {
                    if (benches.size() == 1) {
                        printf("  %s %gns" , bench.name.c_str(), bench.best.count());
                    } else {
                        printf("  %s %.3gx", bench.name.c_str(), bench.best / benches[0].best);
                    }
                }
                fflush(stdout);
                break;
            }
        }
    }

    return 0;
}

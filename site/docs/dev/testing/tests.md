---
title: 'Writing Skia Tests'
linkTitle: 'Writing Skia Tests'
---

We assume you have already synced Skia's dependencies and set up Skia's build
system.

<!--?prettify lang=sh?-->

    python2 tools/git-sync-deps
    bin/gn gen out/Debug
    bin/gn gen out/Release --args='is_debug=false'

## Writing a Unit Test

1.  Add a file `tests/NewUnitTest.cpp`:

    <!--?prettify lang=cc?-->

        /*
         * Copyright ........
         *
         * Use of this source code is governed by a BSD-style license
         * that can be found in the LICENSE file.
         */
        #include "Test.h"
        DEF_TEST(NewUnitTest, reporter) {
            if (1 + 1 != 2) {
                ERRORF(reporter, "%d + %d != %d", 1, 1, 2);
            }
            bool lifeIsGood = true;
            REPORTER_ASSERT(reporter, lifeIsGood);
        }

2.  Add `NewUnitTest.cpp` to `gn/tests.gni`.

3.  Recompile and run test:

    <!--?prettify lang=sh?-->

        ninja -C out/Debug dm
        out/Debug/dm --match NewUnitTest

## Writing a Rendering Test

1.  Add a file `gm/newgmtest.cpp`:

    <!--?prettify lang=cc?-->

        /*
         * Copyright ........
         *
         * Use of this source code is governed by a BSD-style license
         * that can be found in the LICENSE file.
         */
        #include "gm.h"
        DEF_SIMPLE_GM(newgmtest, canvas, 128, 128) {
            canvas->clear(SK_ColorWHITE);
            SkPaint p;
            p.setStrokeWidth(2);
            canvas->drawLine(16, 16, 112, 112, p);
        }

2.  Add `newgmtest.cpp` to `gn/gm.gni`.

3.  Recompile and run test:

    <!--?prettify lang=sh?-->

        ninja -C out/Debug dm
        out/Debug/dm --match newgmtest

4.  Run the GM inside Viewer:

    <!--?prettify lang=sh?-->

        ninja -C out/Debug viewer
        out/Debug/viewer --slide GM_newgmtest

## Writing a Benchmark Test

1.  Add a file `bench/FooBench.cpp`:

    <!--?prettify lang=cc?-->

        /*
         * Copyright ........
         *
         * Use of this source code is governed by a BSD-style license
         * that can be found in the LICENSE file.
         */
        #include "Benchmark.h"
        #include "SkCanvas.h"
        namespace {
        class FooBench : public Benchmark {
        public:
            FooBench() {}
            virtual ~FooBench() {}
        protected:
            const char* onGetName() override { return "Foo"; }
            SkIPoint onGetSize() override { return SkIPoint{100, 100}; }
            void onDraw(int loops, SkCanvas* canvas) override {
                while (loops-- > 0) {
                    canvas->drawLine(0.0f, 0.0f, 100.0f, 100.0f, SkPaint());
                }
            }
        };
        }  // namespace
        DEF_BENCH(return new FooBench;)

2.  Add `FooBench.cpp` to `gn/bench.gni`.

3.  Recompile and run nanobench:

    <!--?prettify lang=sh?-->

        ninja -C out/Release nanobench
        out/Release/nanobench --match Foo

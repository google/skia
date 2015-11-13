Writing Unit and Rendering Tests
================================

Writing a Unit Test
-------------------

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

2.  Recompile and run test:

        python bin/sync-and-gyp
        ninja -C out/Debug dm
        out/Debug/dm --match NewUnitTest

Writing a Rendering Test
------------------------

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

2.  Recompile and run test:

        python bin/sync-and-gyp
        ninja -C out/Debug dm
        out/Debug/dm --match newgmtest

3.  Run the GM inside SampleApp:

        python bin/sync-and-gyp
        ninja -C out/Debug SampleApp
        out/Debug/SampleApp --slide GM:newgmtest

    On MacOS, try this:

        out/Debug/SampleApp.app/Contents/MacOS/SampleApp --slide GM:newgmtest

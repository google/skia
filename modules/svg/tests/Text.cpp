/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>

#include "include/utils/SkNoDrawCanvas.h"
#include "modules/svg/src/SkSVGTextPriv.h"
#include "tests/Test.h"

DEF_TEST(Svg_Text_PosProvider, r) {
    const auto L = [](float x) { return SkSVGLength(x); };
    const float N = SkSVGTextContext::PosAttrs()[SkSVGTextContext::PosAttrs::kX];

    static const struct PosTestDesc {
        size_t                   offseta;
        std::vector<SkSVGLength> xa, ya;

        size_t                   offsetb;
        std::vector<SkSVGLength> xb, yb;

        std::vector<SkPoint>     expected;
    } gTests[] = {
        {
            0, {}, {},
            0, {}, {},

            { {N,N} }
        },

        {
            0, { L(1) }, {},
            0, {      }, {},

            { {1,N}, {N,N} }
        },
        {
            0, {       }, {},
            0, { L(10) }, {},

            { {10,N}, {N,N} }
        },
        {
            0, { L( 1) }, {},
            0, { L(10) }, {},

            { {10,N}, {N,N} }
        },
        {
            0, { L( 1), L(2) }, {},
            0, { L(10)       }, {},

            { {10,N}, {2,N}, {N,N} }
        },
        {
            0, { L(1), L( 2) }, {},
            1, {       L(20) }, {},

            { {1,N}, {20,N}, {N,N} }
        },
        {
            0, { L(1), L( 2), L(3) }, {},
            1, {       L(20)       }, {},

            { {1,N}, {20,N}, {3,N}, {N,N} }
        },
        {
            0, { L(1), L(2), L( 3) }, {},
            2, {             L(30) }, {},

            { {1,N}, {2,N}, {30,N}, {N,N} }
        },
        {
            0, { L(1)              }, {},
            2, {             L(30) }, {},

            { {1,N}, {N,N}, {30,N}, {N,N} }
        },


        {
            0, {}, { L(4) },
            0, {}, {      },

            { {N,4}, {N,N} }
        },
        {
            0, {}, {       },
            0, {}, { L(40) },

            { {N,40}, {N,N} }
        },
        {
            0, {}, { L( 4) },
            0, {}, { L(40) },

            { {N,40}, {N,N} }
        },
        {
            0, {}, { L( 4), L(5) },
            0, {}, { L(40)       },

            { {N,40}, {N,5}, {N,N} }
        },
        {
            0, {}, { L(4), L( 5) },
            1, {}, {       L(50) },

            { {N,4}, {N,50}, {N,N} }
        },
        {
            0, {}, { L(4), L( 5), L(6) },
            1, {}, {       L(50)       },

            { {N,4}, {N,50}, {N,6}, {N,N} }
        },
        {
            0, {}, { L(4), L(5), L( 6) },
            2, {}, {             L(60) },

            { {N,4}, {N,5}, {N,60}, {N,N} }
        },
        {
            0, {}, { L(4)              },
            2, {}, {             L(60) },

            { {N,4}, {N,N}, {N,60}, {N,N} }
        },

        {
            0, { L( 1), L(2)}, { L( 4)        },
            0, { L(10)      }, { L(40), L(50) },

            { {10,40}, {2,50}, {N,N} }
        },
        {
            0, { L(1), L( 2), L(3) }, { L(4), L( 5)        },
            1, {       L(20)       }, {       L(50), L(60) },

            { {1,4}, {20,50}, {3,60}, {N,N} }
        },
    };

    const SkSVGTextContext::ShapedTextCallback mock_cb =
        [](const SkSVGRenderContext&, const sk_sp<SkTextBlob>&, const SkPaint*, const SkPaint*) {};

    auto test = [&](const PosTestDesc& tst) {
        auto a = SkSVGText::Make();
        auto b = SkSVGTSpan::Make();
        a->appendChild(b);

        a->setX(tst.xa);
        a->setY(tst.ya);
        b->setX(tst.xb);
        b->setY(tst.yb);

        const SkSVGIDMapper mapper;
        const SkSVGLengthContext lctx({0,0});
        const SkSVGPresentationContext pctx;
        SkNoDrawCanvas canvas(0, 0);
        sk_sp<SkFontMgr> fmgr;
        sk_sp<skresources::ResourceProvider> rp;
        const SkSVGRenderContext ctx(&canvas, fmgr, rp, mapper, lctx, pctx, nullptr);

        SkSVGTextContext tctx(ctx, mock_cb);
        SkSVGTextContext::ScopedPosResolver pa(*a, lctx, &tctx, tst.offseta);
        SkSVGTextContext::ScopedPosResolver pb(*b, lctx, &tctx, tst.offsetb);

        for (size_t i = 0; i < tst.expected.size(); ++i) {
            const auto& exp = tst.expected[i];
            auto pos = i >= tst.offsetb ? pb.resolve(i) : pa.resolve(i);

            REPORTER_ASSERT(r, pos[SkSVGTextContext::PosAttrs::kX] == exp.fX);
            REPORTER_ASSERT(r, pos[SkSVGTextContext::PosAttrs::kY] == exp.fY);
        }
    };

    for (const auto& tst : gTests) {
        test(tst);
    }
}

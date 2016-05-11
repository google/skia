/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlend_opts_DEFINED
#define SkBlend_opts_DEFINED

namespace SK_OPTS_NS {

#if 0

#else

    static inline void srcover_srgb_srgb_1(uint32_t* dst, uint32_t src) {
        switch (src >> 24) {
            case 0x00:             return; 
            case 0xff: *dst = src; return;
        }

        Sk4f d = SkNx_cast<float>(Sk4b::Load( dst)),
             s = SkNx_cast<float>(Sk4b::Load(&src));

        // Approximate sRGB gamma as 2.0.
        Sk4f d_sq = d*d,
             s_sq = s*s;
        d = Sk4f{d_sq[0], d_sq[1], d_sq[2], d[3]};
        s = Sk4f{s_sq[0], s_sq[1], s_sq[2], s[3]};

        // SrcOver.
        Sk4f invA = 1.0f - s[3]*(1/255.0f);
        d = s + d * invA;

        // Re-apply approximate sRGB gamma.
        Sk4f d_sqrt = d.sqrt();
        d = Sk4f{d_sqrt[0], d_sqrt[1], d_sqrt[2], d[3]};

        SkNx_cast<uint8_t>(d).store(dst);
    }

    static inline void srcover_srgb_srgb(uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
        while (ndst > 0) {
            int n = SkTMin(ndst, nsrc);

            for (int i = 0; i < n; i++) {
                srcover_srgb_srgb_1(dst++, src[i]);
            }
            ndst -= n;
        }
    }
    
#endif

}  // namespace SK_OPTS_NS

#endif//SkBlend_opts_DEFINED

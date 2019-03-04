/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkColorFilterPriv_DEFINED
#define SkColorFilterPriv_DEFINED

#ifdef SK_SUPPORT_GPU

#include "SkColorFilter.h"
#include "SkString.h"

using SkRuntimeColorFilterFn = void(*)(float[4], const void*);

class SK_API SkRuntimeColorFilterFactory {
public:
    /**
     * Creates a factory which creates runtime color filters. The SkSL must define a 'main' function
     * with the signature 'void main(inout half4 color)'. The SkSL will be used when rendering in
     * GPU mode, with the 'color' parameter providing the current value on input and receiving the
     * new color value on exit. In software mode, the cpuFunc will be called with the current color
     * and a pointer to the 'inputs' bytes. cpuFunc may be left null, in which case only GPU
     * rendering is supported.
     */
    SkRuntimeColorFilterFactory(SkString sksl, SkRuntimeColorFilterFn cpuFunc = nullptr);

    /**
     * Creates a color filter instance with the specified inputs. In GPU rendering, the inputs are
     * used to populate the values of 'in' variables. For instance, given the color filter:
     *    in uniform float x;
     *    in uniform float y;
     *    void main(inout half4 color) {
     *        ...
     *    }
     * The values of the x and y inputs come from the 'inputs' SkData, which are laid out as a
     * struct with two float elements. If there are no inputs, the 'inputs' parameter may be null.
     *
     * In CPU rendering, a pointer to the input bytes is passed as the second parameter to
     * 'cpuFunc'.
     */
    sk_sp<SkColorFilter> make(sk_sp<SkData> inputs);

private:
    int fIndex;
    SkString fSkSL;
    SkRuntimeColorFilterFn fCpuFunc;
};

#endif // SK_SUPPORT_GPU

#endif  // SkColorFilterPriv_DEFINED

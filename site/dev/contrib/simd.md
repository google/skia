Skia's New Approach to SIMD
===========================

Most hot software paths in Skia are implemented with processor-specific SIMD instructions.  For graphics performance, the parallelism from SIMD is essential: there is simply no realistic way to eek the same performance out of portable C++ code as we can from the SSE family of instruction sets on x86 or from NEON on ARM or from MIPS32's DSP instructions.  Depending on the particular code path and math involved, we see 2, 4, 8, or even ~16x performance increases over portable code when really exploiting the processor-specific SIMD instructions.

But the SIMD code we've piled up over the years has some serious problems.  It's often quite low-level, with poor factoring leading to verbose, bug prone, and difficult to read code.  SIMD instrinsic types and functions take a good long while to get used to reading, let alone writing, and assembly is generally just a complete non-starter.  SIMD coverage of Skia methods is not dense: a particular drawing routine might be specialized for NEON but not for SSE, or might have a MIPS DSP implementation but no NEON.  Even when we have full instruction set coverage, the implementations of these specialized routines may not produce identical results, either when compared with each other or with our portable fallback code.  The SIMD implementations are often simply incorrect, but the code is so fragile and difficult to understand, we can't fix it.  There are long lived bugs in our tracker involving crashes and buffer under- and overflows that we simply cannot fix because no one on the team understands the code involved.  And finally, to top it all off, the code isn't always even really that fast.

This all needs to change.  I want Skia developers to be able to write correct, clear, and fast code, and in software rendering, SIMD is the only way to get "fast".  This document outlines a new vision for how Skia will use SIMD instructions with no compromises, writing clear code _once_ that runs quickly on all platforms we support.

The Plan
--------

We're going to wrap low-level platform-specific instrinsics with zero-cost abstractions with interfaces matching Skia's higher-level-but-still-quite-low-level use cases.  Skia code will write to this interface _once_, which then compiles to efficient SSE, NEON, or portable code (MIPS is quite TBD, for now group it conceptually under portable code) via platform-specific backends.  The key here is to find the right sweet spot of abstraction that allows us to express the graphical concepts we want in Skia while allowing each of those platform-specific backends flexibility to implement those concepts as efficiently as possible.

While Skia uses a mix of float, 32-bit, 16-bit, and 8-bit integer SIMD instructions, 32-bit integers fall quite behind the rest in usage.  Since we tend to operate on 8888 ARGB values, 8-bit SIMD tends to be the most natural and fastest approach, but when multiplication gets involved (essentially all the time), 16-bit SIMD inevitably gets tangled in there.  For some operations like division, square roots, or math with high range or precision requirements, we expand our 8-bit pixel components up to floats, and working with a single pixel as a 4-float vector becomes most natural.  This plan focuses on how we'll deal with these majority cases: floats, and 8- and 16-bit integers.

`SkNf` for floats
---------------

Wrapping floats with an API that allows efficient implementation on SSE and NEON is by far the easiest task involved here.  Both SSE and NEON naturally work with 128-bit vectors of 4 floats, and they have a near 1-to-1 correspondence between operations.  Indeed, the correspondence is so close that it's tempting to solve this problem by picking one set of intrinsics, e.g. NEON, and just `#define`ing portable and SSE implementations of NEON:

    #define float32x4_t __m128
    #define vmulq_f32 _mm_mul_ps
    #define vaddq_f32 _mm_add_ps
    #define vld1q_f32 _mm_loadu_ps
    #define vst1q_f32 _mm_storeu_ps
    ...

This temptation starts to break down when you notice:

-   there are operations that don't quite correspond, e.g. `_mm_movemask_ps`; and
-   math written with either SSE or NEON instrinsics is still very hard to read; and
-   sometimes we want to work with 4 floats, but sometimes 2, maybe even 8, etc.

So we use a wrapper class `SkNf<N>`, parameterized on N, how many floats the vector contains, constrained at compile time to be a power of 2.  `SkNf` provides all the methods you'd expect on vector of N floats: loading and storing from float arrays, all the usual arithmetic operators, min and max, low and high precision reciprocal and sqrt, all the usual comparison operators, and a `.thenElse()` method acting as a non-branching ternary `?:` operator.  To support Skia's main graphic needs, `SkNf` can also load and store from a vector of N _bytes_, converting up to a float when loading and rounding down to [0,255] when storing.

As a convenience, `SkNf<N>` has two default implementations: `SkNf<1>` performs all these operations on a single float, and the generic `SkNf<N>` simply recurses onto two `SkNf<N/2>`.  This allows our different backends to inject specialiations where most natural: the portable backend does nothing, so all `SkNf<N>` recurse down to the default `SkNf<1>`;  the NEON backend specializes `SkNf<2>` with `float32x2_t` and 64-bit SIMD methods, and `SkNf<4>` with `float32x4_t` and 128-bit SIMD methods; the SSE backend specializes both `SkNf<4>` and `SkNf<2>` to use the full or lower half of an `__m128` vector, respectively.  A future AVX backend could simply drop in an `SkNf<8>` specialization.

Our most common float use cases are working with 2D coordinates and with 4-float-component pixels.  Since these are so common, we've made simple typedefs for these two use cases, `Sk2f` and `Sk4f`, and also versions reminding you that it can work with vectors of `SkScalar` (a Skia-specific float typedef) too: `Sk2s`, `Sk4s`.

`SkNf` in practice
----------------

To date we have implemented several parts of Skia using Sk4f:

  1. `SkColorMatrixFilter`
  2. `SkRadialGradient`
  3. `SkColorCubeFilter`
  4. Three complicated `SkXfermode` subclasses: `ColorBurn`, `ColorDodge`, and `SoftLight`.

In all these cases, we have been able to write a single implementation, producing the same results cross-platform.  The first three of those sites using Sk4f are entirely newly vectorized, and run much faster than the previous portable implementations.  The 3 Sk4f transfermodes replaced portable, SSE, and NEON implementations which all produced different results, and the Sk4f versions are all faster than their predecessors.

`SkColorCubeFilter` stands out as a particularly good example of how and why to use Sk4f over custom platform-specific intrinsics.  Starting from some portable code and a rather slow SSE-only sketch, a Google Chromium dev, an Intel contributor, and I worked together to write an Sk4f version that's more than twice as fast as the original, and runs fast on _both_ x86 and ARM.

`SkPx` for 8- and 16-bit fixed point math
----------------------------------------

Building an abstraction layer over 8- and 16-bit fixed point math has proven to be quite a challenge.  In fixed point, NEON and SSE again have some overlap, and they could probably be implemented in terms of each other if you were willing to sacrifice performance on SSE in favor of NEON or vice versa.  But unlike with floats, where `SkNf` is really a pretty thin veneer over very similar operations, to really get the best performance out of each fixed point instruction set you need to work in rather different idioms.

`SkPx`, our latest approach (there have been alpha `Sk16b` and beta `Sk4px` predecessors) to 8- and 16-bit SIMD  tries to abstract over those idioms to again allow Skia developers to write one piece of clear graphics code that different backends can translate into their native intrinsics idiomatically.

`SkPx` is really a family of three related types:

  1. `SkPx` itself represents between 1 and `SkPx::N` 8888 ARGB pixels, where `SkPx::N` is a backend-specific compile-time power of 2.
  2. `SkPx::Wide` represents those same pixels, but with 16-bits of space per component.
  3.  `SkPx::Alpha` represents the alpha channels of those same pixels.

`SkPx`, `Wide` and `Alpha` create a somewhat complicated algebra of operations entirely motivated by the graphical operations we need to perform.  Here are some examples:

    SkPx::LoadN(const uint32_t*)   -> SkPx  // Load full cruising-speed SkPx.
    SkPx::Load(n, const uint32_t*) -> SkPx  // For the 0<n<N ragged tail.
    
    SkPx.storeN(uint32_t*)   // Store a full SkPx.
    SkPx.store(n, uint32_t*) // For the ragged 0<n<N tail.

    SkPx + SkPx -> SkPx
    SkPx - SkPx -> SkPx
    SkPx.saturatedAdd(SkPx) -> SkPx

    SkPx.alpha() -> Alpha   // Extract alpha channels.
    Alpha::LoadN(const uint8_t*)   -> Alpha  // Like SkPx loads, in 8-bit steps.
    Alpha::Load(n, const uint8_t*) -> Alpha

    SkPx.widenLo()   -> Wide  // argb -> 0a0r0g0b
    SkPx.widenHi()   -> Wide  // argb -> a0r0g0b0
    SkPx.widenLoHi() -> Wide  // argb -> aarrggbb

    Wide + Wide -> Wide
    Wide - Wide -> Wide
    Wide << bits -> Wide
    Wide >> bits -> Wide

    SkPx * Alpha -> Wide    // 8 x 8 -> 16 bit
    Wide.div255() -> SkPx   // 16-bit -> 8 bit

    // A faster approximation of (SkPx * Alpha).div255().
    SkPx.approxMulDiv255(Alpha) -> SkPx

We allow each `SkPx` backend to choose how it physically represents `SkPx`, `SkPx::Wide`, and `SkPx::Alpha` and to choose any power of two as its `SkPx::N` sweet spot.  Code working with SkPx typically runs a loop like this:

    while (n >= SkPx::N) {
    	// Apply some_function() to SkPx::N pixels.
    	some_function(SkPx::LoadN(src), SkPx::LoadN(dst)).storeN(dst);
    	src += SkPx::N; dst += SkPx::N; n -= SkPx::N;
    }
    if (n > 0) {
    	// Finish up the tail of 0<n<N pixels.
    	some_function(SkPx::Load(n, src), SkPx::Load(n, dst)).store(n, dst);
    }

The portable code is of course the simplest place to start looking at implementation details: its `SkPx` is just `uint8_t[4]`, its `SkPx::Wide` `uint16_t[4]`, and its `SkPx::Alpha` just `uint8_t`.  Its preferred number of pixels to work with is `SkPx::N = 1`.  (Amusingly, GCC and Clang seem pretty good about autovectorizing this backend using 32-bit math, which typically ends up within ~2x of the best we can do ourselves.)

The most important difference between SSE and NEON when working in fixed point is that SSE works most naturally with 4 interlaced pixels at a time (argbargbargbargb), while NEON works most naturally with 8 planar pixels at a time (aaaaaaaa, rrrrrrrr, gggggggg, bbbbbbbb).  Trying to jam one of these instruction sets into the other's idiom ends up somewhere between not quite optimal (working with interlaced pixels in NEON) and ridiculously inefficient (trying to work with planar pixels in SSE).

So `SkPx`'s SSE backend sets N to 4 pixels, stores them interlaced in an `__m128i`, representing `Wide` as two `__m128i` and `Alpha` as an `__m128i` with each pixel's alpha component replicated four times.  SkPx's NEON backend works with 8 planar pixels, loading them with `vld4_u8` into an `uint8x8x4_t` struct of 4 8-component `uint8x8_t` planes.  `Alpha` is just a single `uint8x8_t` 8-component plane, and `Wide` is NEON's natural choice, `uint16x8x4_t`.

(It's fun to speculate what an AVX2 backend might look like.  Do we make `SkPx` declare it wants to work with 8 pixels at a time, or leave it at 4?  Does `SkPx` become `__m256i`, or maybe only `SkPx::Wide` does?  What's the best way to represent `Alpha`?  And of course, what about AVX-512?)

Keeping `Alpha` as a single dense `uint8x8_t` plane allows the NEON backend to be much more efficient with operations involving `Alpha`.  We'd love to do this in SSE too, where we store `Alpha` somewhat inefficiently with each alpha component replicated 4 times, but SSE simply doesn't expose efficient ways to transpose interlaced pixels into planar pixels and vice versa.  We could write them ourselves, but only as rather complex compound operations that slow things down more than they help.

These details will inevitably change over time.  The important takeaway here is, to really work at peak throughput in SIMD fixed point, you need to work with the idiom of the instruction set, and `SkPx` is a design that can present a consistent interface to abstract away backend details for you.

`SkPx` in practice
----------------

I am in the process of rolling out `SkPx`.  Some Skia code is already using its precursor, `Sk4px`, which is a bit like `SkPx` that forces `N=4` and restricts the layout to always use interlaced pixels: i.e. fine for SSE, not great for NEON.

  1. All ~20 other `SkXfermode` subclasses that are not implemented with `SkNf`.
  2. SkBlitRow::Color32
  3. SkBlitMask::BlitColor

I can certainly say that the `Sk4px` and `SkPx` implementations of these methods are clearer, less buggy, and that all the `SkXfermode` implementations sped up at least 2x when porting from custom per-platform intrinsics.  `Sk4px` has lead to some pretty bad performance regressions that `SkPx` is designed to avoid.  This is an area of active experiementation and iteration.

In Summary
----------

I am confident that Skia developers soon will be able to write single, clear, maintainable, and of course _fast_,  graphical algorithms using `SkNf` and `SkPx`.  As I have been porting our algorithms, I have perversely enjoyed replacing thousands of lines of unmaintainable code with usually mere dozens of readable code.

I'm also confident that if you're looking to use floats, `SkNf` is ready.  Do not write NEON or SSE SIMD code if you're looking to use floats, and do not accept external contributions that do so.  Use `SkNf` instead.

`SkPx` is less proven, and while its design and early tests look promising, it's still at the stage where we should try it aware that we might need to fall back on hand-written SSE or NEON.
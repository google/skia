Skia can now be built for WebAssembly SIMD128 by setting the GN arg `skia_wasm_simd = true`.
The CPU raster pipeline then runs four pixels per stage and enables its lowp stages, instead of
running one pixel at a time. Such builds need WebAssembly SIMD in the browser (Chrome 91,
Firefox 89, Safari 16.4).

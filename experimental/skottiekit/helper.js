var nullptr = 0; // emscripten doesn't like to take null as uintptr_t

// arr can be a normal JS array or a TypedArray
// dest is something like SkottieKit.HEAPF32
// ptr can be optionally provided if the memory was already allocated.
function copy1dArray(arr, dest, ptr) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  // This was created with SkottieKit.Malloc, so it's already been copied.
  if (arr['_ck']) {
    return arr.byteOffset;
  }
  if (!ptr) {
    ptr = SkottieKit._malloc(arr.length * dest.BYTES_PER_ELEMENT);
  }
  // In c++ terms, the WASM heap is a uint8_t*, a long buffer/array of single
  // byte elements. When we run _malloc, we always get an offset/pointer into
  // that block of memory.
  // SkottieKit exposes some different views to make it easier to work with
  // different types. HEAPF32 for example, exposes it as a float*
  // However, to make the ptr line up, we have to do some pointer arithmetic.
  // Concretely, we need to convert ptr to go from an index into a 1-byte-wide
  // buffer to an index into a 4-byte-wide buffer (in the case of HEAPF32)
  // and thus we divide ptr by 4.
  dest.set(arr, ptr / dest.BYTES_PER_ELEMENT);
  return ptr;
}

/// <reference path="embind.d.ts" />
/// <reference path="public_api.d.ts" />
declare var Module: embind.EmbindModule;
namespace memory {
    export const nullptr = 0;

    export const copy1dArray = (arr: any[] | public_api.TypedArray, dest: Heaps, ptr?: number) => {
      if (!arr || !arr.length) {
        return nullptr;
      }
      const bytesPerElement = Module[dest].BYTES_PER_ELEMENT;
      if (!ptr) {
        ptr = Module._malloc(arr.length * bytesPerElement);
      }
      Module[dest].set(arr, ptr / bytesPerElement);
      return ptr;
    }

    export const freeIfNecessary = (ptr: number, arr: any[] | public_api.TypedArray) => {
        if (arr && !(arr as any)["_ck"]) {
        Module._free(ptr);
      }
    }

    // Recommended over enums by Effective Typescript pg 197.
    export type Heaps = "HEAPF32" | "HEAPU8" | "HEAPU16" | "HEAPU32" | "HEAP8" | "HEAP16" | "HEAP32";
}
declare namespace embind {
  export interface EmbindModule {
    // The following are provided by emscripten
    onRuntimeInitialized(): void;
    _malloc(bytes: number): number;
    _free(ptr: number): void;

    HEAPF32: Float32Array;
    HEAPU8: Uint8Array;
    HEAPU16: Uint16Array;
    HEAPU32: Uint32Array;
    HEAP8: Int8Array;
    HEAP16: Int16Array;
    HEAP32: Int32Array;
  }

  export interface EmbindObject<T extends EmbindObject<T>> {
    clone(): T;
    delete(): void;
    deleteAfter(): void;
    isAliasOf(other: any): boolean;
    isDeleted(): boolean;
  }
}

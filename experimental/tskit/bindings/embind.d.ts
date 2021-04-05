declare namespace embind {
    export interface EmbindModule {
        // The following are provided by emscripten
        onRuntimeInitialized(): void;
        _malloc(bytes: number): number;
        _free(ptr: number): void;
    }

    export interface EmbindObject<T extends EmbindObject<T>> {
        clone(): T;
        delete(): void;
        deleteAfter(): void;
        isAliasOf(other: any): boolean;
        isDeleted(): boolean;
    }
}

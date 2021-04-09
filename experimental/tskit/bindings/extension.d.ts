/// <reference path="embind.d.ts" />
declare namespace extension {
  export interface Bindings {
    _privateExtension(rPtr: number, len: number): number;
    _withObject(obj: CompoundObj): void
    readonly Extension: ExtensionConstructor;
  }

  export interface ExtensionConstructor {
    new(name?: string): Extension;
  }

  export interface Extension extends embind.EmbindObject<Extension> {
    getName(): string;
    _setName(name: string): void;
  }

  export interface CompoundObj {
    alpha: number;
    beta: string;
    gamma?: number;
  }
}

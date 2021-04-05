/// <reference path="embind.d.ts" />
declare namespace core {
  export interface Bindings {
    _privateFunction(x: number, y: number): number;

    publicFunction(input: string): void;

    readonly Something: SomethingConstructor;
  }

  export interface SomethingConstructor {
    new(name: string): Something;
  }

  export interface Something extends embind.EmbindObject<Something> {
    getName(): string;

    _setName(name: string): void;
  }
}

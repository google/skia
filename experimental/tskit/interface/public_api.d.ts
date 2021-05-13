declare namespace public_api {
  export interface CanvasKit {
    publicExtension(myRects: InputFlattenedRectArray): number;
    sayHello(x: number, y: number): void;
    publicFunction(input: string): void;
    withObject(obj: CompoundObj): void;

    readonly Extension: ExtensionConstructor;
    readonly Something: SomethingConstructor;
  }

  export interface ExtensionConstructor {
    new(): Extension;
    new(name: string): Extension;
  }

  export interface SomethingConstructor {
    new(name: string): Something;
  }

  export interface Extension extends EmbindObject<Extension> {
    getProp(): string;
    setProp(p: string): void;
  }

  export interface Something extends EmbindObject<Something> {
    getName(): string;
    setName(name: string): void;
  }

  export interface CompoundObj {
    alpha: number;
    beta: string;
    gamma?: number;
  }

  export type InputFlattenedRectArray = Float32Array | number[];
  export type TypedArray = Float32Array | Int32Array;

  export interface EmbindObject<T extends EmbindObject<T>> {
    clone(): T;
    delete(): void;
    deleteAfter(): void;
    isAliasOf(other: any): boolean;
    isDeleted(): boolean;
  }
}

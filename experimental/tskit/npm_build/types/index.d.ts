export interface CanvasKit {
    /**
     * This function says hello
     *
     * @param x some number
     * @param y some other number
     */
    sayHello(x: number, y: number): void;

    /**
     * publicExtension takes the number of rects and returns how
     * many of them have the point (5, 5) in them.
     * @param myRects
     * @ts publicExtension(myRects: InputFlattenedRectArray): void;
     */
    publicExtension(myRects: InputFlattenedRectArray): number;

    /**
     * This function does a public thing.
     * @param input an ice cream flavor
     */
    publicFunction(input: string): void;

    withObject(obj: CompoundObj): void;

    readonly Extension: ExtensionConstructor;
    readonly Something: SomethingConstructor;
}

export interface ExtensionConstructor {
    /**
     * Returns an extension with the provided property.
     * @param name - if not provided, use a default value
     */
    new(name?: string): Extension;
}

export interface SomethingConstructor {
    /**
     * Returns a Something with the provided name.
     * @param name
     */
    new(name: string): Something;
}

export interface Extension extends EmbindObject<Extension> {
    /**
     * Returns the associated property.
     */
    getProp(): string;
    /**
     * This sets the property with a prefix.
     * @param p
     */
    setProp(p: string): void;
}

/**
 * The Something class is quite something. See SkSomething.h for more.
 */
export interface Something extends EmbindObject<Something> {
    /**
     * Returns the associated name.
     */
    getName(): string;
    /**
     * This sets the name twice for good measure.
     * @param name
     */
    setName(name: string): void;
}

export interface CompoundObj {
    alpha: number;
    beta: string;
    /**
     * This field (gamma) should be documented.
     * @optional - default value is 1.0
     */
    gamma?: number;
}

export type InputFlattenedRectArray = Float32Array | number[];

/**
 * CanvasKit is built with Emscripten and Embind. Embind adds the following methods to all objects
 * that are exposed with it.
 */
export interface EmbindObject<T extends EmbindObject<T>> {
    clone(): T;
    delete(): void;
    deleteAfter(): void;
    isAliasOf(other: any): boolean;
    isDeleted(): boolean;
}
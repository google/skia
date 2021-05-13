/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <string>

#include "experimental/tskit/bindings/bindings.h"

// SkRect.h tossed in here to avoid having to include/link it in POC
typedef float SkScalar;
struct SkRect {
    SkScalar fLeft;    //!< smaller x-axis bounds
    SkScalar fTop;     //!< smaller y-axis bounds
    SkScalar fRight;   //!< larger x-axis bounds
    SkScalar fBottom;  //!< larger y-axis bounds

    bool contains(SkScalar x, SkScalar y) const {
        return x >= fLeft && x < fRight && y >= fTop && y < fBottom;
    }
};

class Extension {
public:
    Extension(): fProp("foo") {}
    Extension(std::string n): fProp(n) {}

    const std::string getProp() {
        return fProp;
    }

    void setProp(std::string p) {
        fProp = p;
    }

private:
    std::string fProp;
};

struct CompoundObj {
    int alpha;
    std::string beta;
    float gamma;
};


EMSCRIPTEN_BINDINGS(Extension) {
    TS_PRIVATE_EXPORT("_privateExtension(rPtr: number, len: number): number")
    function("_privateExtension", optional_override([](uintptr_t rPtr, size_t len)->int {
        int containsPoint = 0;
        SkRect* rects = reinterpret_cast<SkRect*>(rPtr);
        for (int i = 0; i < len; i++) {
            if (rects[i].contains(5, 5)) {
                containsPoint++;
            }
        }
        return containsPoint;
    }));

    TS_PRIVATE_EXPORT("_withObject(obj: CompoundObj): void")
    function("_withObject", optional_override([](CompoundObj o)->void {
        printf("Object %d %s %f\n", o.alpha, o.beta.c_str(), o.gamma);
    }));

    /**
     * The Extension class extends the core components.
     */
    class_<Extension>("Extension")
        .constructor<>()
        /**
         * Returns an extension with the provided property.
         * @param name - if not provided, use a default value
         */
        TS_EXPORT("new(name?: string): Extension")
        .constructor<std::string>()
        /**
         * Returns the associated property.
         */
        TS_EXPORT("getProp(): string")
        .function("getProp", &Extension::getProp)
        TS_PRIVATE_EXPORT("setProp(p: string): void")
        .function("_setProp", &Extension::setProp);

    value_object<CompoundObj>("CompoundObj")
        /** @type number */
        .field("alpha", &CompoundObj::alpha)
         /** @type string */
        .field("beta", &CompoundObj::beta)
        /**
         * This field (gamma) should be documented.
         * The default value is 1.0 if not set.
         * @type @optional number
         */
        .field("gamma", &CompoundObj::gamma);
}

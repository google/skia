---
title: 'Coding Style Guidelines'
linkTitle: 'Coding Style Guidelines'
---

These conventions have evolved over time. Some of the earlier code in both
projects doesn't strictly adhere to the guidelines. However, as the code evolves
we hope to make the existing code conform to the guildelines.

## Files

We use `.cpp` and `.h` as extensions for C++ source and header files.

Headers that aren't meant for public consumption should be placed in `src/`
directories so that they aren't in a client's search path, or in
`include/private/` (such as `include/private/base/`) if they need to be used by
public headers.

`#include` directives use full paths relative to the repository root (e.g.
`#include "include/core/SkTypes.h"` or `#include "src/core/SkPriv.h"`). We
follow "include what you use" (do not rely on transitive includes), except in
headers where forward declaring a name is preferred over an `#include` when
sufficient.

Forward declarations and file includes should be in alphabetical order. In both
`.cpp` and `.h` files, Skia headers (`"..."`) come first, followed by system and
C/C++ standard library headers (`<...>`). In `.cpp` files, the corresponding
header for that implementation file comes first before all other includes. You
can run `python3 tools/rewrite_includes.py` to automatically sort Skia includes
and forward declarations (which is also checked on presubmit).

### No define before sktypes

Do not use #if/#ifdef before including "SkTypes.h" (directly or indirectly).
Most things you'd #if on tend to not yet be decided until SkTypes.h.

We use 4 spaces, not tabs.

We use Unix style endlines (LF).

We prefer no trailing whitespace but aren't very strict about it.

We wrap lines at 100 columns unless it is excessively ugly (use your judgement).

### Formatting and `git clang-format`

Skia provides a [`.clang-format`](https://skia.googlesource.com/skia/+/main/.clang-format)
configuration so you can format your modified lines before uploading:

```
git clang-format
```

`git clang-format` is a helpful guide and should be used often, but it is not
the end-all be-all. Automated formatting does not always produce the most
readable result—for example, when formatting tabular data, matrix/geometry grids,
aligned comments or assignments, or hand-wrapped expressions. Use `git clang-format`
as a starting point, and use your judgment when manual formatting is clearer.

## Naming

Externally visible types and functions follow these prefix and namespace
conventions:

* **Core Skia**: Uses an `Sk`- prefix to designate they're part of Skia (e.g.
  `SkCanvas`).
* **Ganesh**: Uses a `Gr`- prefix (e.g. `GrRecordingContext`).
* **Namespaced subsystems**: Newer subsystems use namespaces and omit `Sk`/`Gr`
  prefixes on types and filenames inside those namespaces (e.g.
  `skgpu::graphite::Recorder` in `Recorder.h`):
  * `skgpu::graphite` (Graphite)
  * `skgpu` (shared GPU utilities)
  * `skcpu` (CPU backend utilities)
  * `sktext` and `SkSL`
  * `skia_private` (internal containers and utilities)
* **Nested types**: Need not be prefixed.

<!--?prettify?-->

```
class SkClass {
public:
    class HelperClass {
        ...
    };
};

namespace skgpu::graphite {
class Recorder {
    ...
};
}  // namespace skgpu::graphite
```

Data fields in structs, classes, and unions that have methods begin with
lower-case f and are then camel-capped, to distinguish those fields from other
variables. Types that are predominantly meant for direct field access don't need
f-decoration.

<!--?prettify?-->

```
struct GrCar {
    float milesDriven;
    Color color;
};

class GrMotorcyle {
public:
    float getMilesDriven() const { return fMilesDriven; }
    void  setMilesDriven(float milesDriven) { fMilesDriven = milesDriven; }

    Color getColor() const { return fColor; }
private:
    float fMilesDriven;
    Color fColor;
};
```

Global variables are similar but prefixed with g and camel-capped.

<!--?prettify?-->

```
bool gLoggingEnabled;
```

Local variables and arguments are camel-capped with no initial cap.

<!--?prettify?-->

```
int herdCats(const Array& cats) {
    int numCats = cats.count();
}
```

Variables declared `constexpr` or `const`, and whose value is fixed for the
duration of the program, are named with a leading "k" and then camel-capped.

<!--?prettify?-->

```
int drawPicture() {
    constexpr SkISize kPictureSize = {100, 100};
    constexpr float kZoom = 1.0f;
}
```

Enum values are also prefixed with `k`. Scoped enums (`enum class`) are
preferred in new code and do not need suffixes on their enumerators. Unscoped
enum values are postfixed with an underscore and singular name of the enum name.
The enum itself should be singular for exclusive values or plural for a
bitfield (often paired with `SK_MAKE_BITMASK_OPS` and `SkEnumBitMask`). If a
count is needed it is `k<singular enum name>Count`, not a member of the enum,
and is derived from using a `kLast` member of the enum (see example).

<!--?prettify?-->

```
// Scoped enum class (preferred for new code) does not need suffixes.
enum class SkPancakeType {
     kBlueberry,
     kPlain,
     kChocolateChip,

     kLast = kChocolateChip
};
static constexpr int kPancakeTypeCount = static_cast<int>(SkPancakeType::kLast) + 1;
```

<!--?prettify?-->

```
// Unscoped enum should have a suffix after the enum name.
enum SkDonutType {
     kGlazed_DonutType,
     kSprinkles_DonutType,
     kChocolate_DonutType,
     kMaple_DonutType,

     kLast_DonutType = kMaple_DonutType
};

static const int kDonutTypeCount = kLast_DonutType + 1;
```

<!--?prettify?-->

```
enum SkSausageIngredientBits {
    kFennel_SausageIngredientBit = 0x1,
    kBeef_SausageIngredientBit   = 0x2
};
```

<!--?prettify?-->

```
enum SkMatrixFlags {
    kTranslate_MatrixFlag = 0x1,
    kRotate_MatrixFlag    = 0x2
};
```

Macros are all caps with underscores between words. Macros that have greater
than file scope should be prefixed `SK`, `SKGPU`, or `GR`. Header guards in
namespaced directories such as Graphite include the namespace prefix (e.g.
`skgpu_graphite_Recorder_DEFINED`).

File-local helper functions in implementation (`.cpp`) files are lower-case with
underscores separating words, and may be declared `static` or placed in an
anonymous `namespace`:

<!--?prettify?-->

```
static inline bool tastes_like_chicken(Food food) {
    return kIceCream_Food != food;
}
```

Externed functions or static class functions are camel-capped with an initial
cap:

<!--?prettify?-->

```
bool SkIsOdd(int n);

class SkFoo {
public:
    static int FooInstanceCount();

    // Not static.
    int barBaz();
};
```

## Macros

Ganesh macros that are GL-specific should be prefixed GR_GL.

<!--?prettify?-->

```
#define GR_GL_TEXTURE0 0xdeadbeef
```

Ganesh prefers that macros are always defined and the use of `#if MACRO` rather
than `#ifdef MACRO`.

<!--?prettify?-->

```
#define GR_GO_SLOWER 0
...
#if GR_GO_SLOWER
    Sleep(1000);
#endif
```

Graphite and shared GPU macros use the `SKGPU_` prefix (e.g.
`SKGPU_ASSERT_SINGLE_OWNER`). When checking whether a macro is defined, prefer
`#if defined(SK_MACRO)` over `#ifdef SK_MACRO`.

## Braces

Open braces don't get a newline. `else` and `else if` appear on the same line as
opening and closing braces unless preprocessor conditional compilation
interferes. Braces are always used with `if`, `else`, `while`, `for`, and `do`.
Prefer multi-line `if` statements over single-line `if` statements, unless
aligning a series of simple checks improves readability.

<!--?prettify?-->

```
if (...) {
    oneOrManyLines;
}

if (...) {
    oneOrManyLines;
} else if (...) {
    oneOrManyLines;
} else {
    oneOrManyLines;
}

for (...) {
    oneOrManyLines;
}

while (...) {
    oneOrManyLines;
}

void function(...) {
    oneOrManyLines;
}

if (!error) {
    proceed_as_usual();
}
#if HANDLE_ERROR
else {
    freak_out();
}
#endif
```

## Flow Control

There is a space between flow control words and parentheses, and between
parentheses and braces:

<!--?prettify?-->

```
while (...) {
}

do {
} while (...);

switch (...) {
...
}
```

Cases and default in switch statements are indented from the switch. When a
switch maps enum values to simple return values or assignments, compact
single-line `case` statements are also fine when they improve readability.

<!--?prettify?-->

```
switch (color) {
    case kBlue:
        ...
        break;
    case kGreen:
        ...
        break;
    ...
    default:
        ...
        break;
}
```

Fallthrough from one case to the next is annotated with `[[fallthrough]]`.
However, when multiple case statements in a row are used, they do not need the
`[[fallthrough]]` annotation.

<!--?prettify?-->

```
switch (recipe) {
    ...
    case kSmallCheesePizza_Recipe:
    case kLargeCheesePizza_Recipe:
        ingredients |= kCheese_Ingredient | kDough_Ingredient | kSauce_Ingredient;
        break;
    case kCheeseOmelette_Recipe:
        ingredients |= kCheese_Ingredient;
        [[fallthrough]]
    case kPlainOmelette_Recipe:
        ingredients |= (kEgg_Ingredient | kMilk_Ingredient);
        break;
    ...
}
```

When a block is needed to declare variables within a case follow this pattern:

<!--?prettify?-->

```
switch (filter) {
    ...
    case kGaussian_Filter: {
        Bitmap srcCopy = src->makeCopy();
        ...
    } break;
    ...
};
```

## Classes

Unless there is a need for forward declaring something, class declarations
should be ordered `public`, `protected`, `private`. Within each visibility
section (`public`, `private`), fields should not be intermixed with methods.
It's nice to keep all data fields together at the end.

<!--?prettify?-->

```
class SkFoo {
public:
    ...

protected:
    ...

private:
    void barHelper(...);
    ...

    SkBar fBar;
    ...
};
```

Virtual functions that are overridden in derived classes should use override,
and the virtual keyword should be omitted.

<!--?prettify?-->

```
void myVirtual() override {
}
```

If you call a method on a parent type that must stand out as specifically the
parent's version of that method, such as `Parent::method()`. The `this->` that
would normally be required before a method call on the current object is not
necessary when using a scope qualifier.

<!--?prettify?-->

```
class GrDillPickle : public GrPickle {
    ...
    bool onTasty() const override {
        return GrPickle::onTasty() &&
               fFreshDill;
    }
    ...
private:
    bool fFreshDill;
};
```

Constructor initializers should be placed on the same line as the constructor,
if they fit. Otherwise, each initializer should be on its own line, indented,
with punctuation placed before the initializer.

<!--?prettify?-->

```
GrDillPickle::GrDillPickle() : GrPickle(), fSize(kDefaultPickleSize) {}

GrDillPickle::GrDillPickle(float size, float crunchiness, const PickleOptions* options)
        : GrPickle(options)
        , fSize(size)
        , fCrunchiness(crunchiness) {}
```

Constructors that take one argument should almost always be explicit, with
exceptions made only for the (rare) automatic compatibility class.

<!--?prettify?-->

```
class Foo {
    explicit Foo(int x);  // Good.
    Foo(float y);         // Spooky implicit conversion from float to Foo.  No no no!
    ...
};
```

Method calls within method calls should be prefixed with dereference of the
'this' pointer. For example:

<!--?prettify?-->

```
this->method();
```

A common pattern for virtual methods in Skia is to include a public non-virtual
(or final) method, paired with a private virtual method named "onMethodName".
This ensures that the base-class method is always invoked and gives it control
over how the virtual method is used, rather than relying on each subclass to
call `Parent::onMethodName()`. For example:

<!--?prettify?-->

```
class SkSandwich {
public:
    void assemble() {
        // All sandwiches must have bread on the top and bottom.
        this->addIngredient(kBread_Ingredient);
        this->onAssemble();
        this->addIngredient(kBread_Ingredient);
    }
    bool cook() {
        return this->onCook();
    }

private:
    // All sandwiches must implement onAssemble.
    virtual void onAssemble() = 0;
    // Sandwiches can remain uncooked by default.
    virtual bool onCook() { return true; }
};

class SkGrilledCheese : public SkSandwich {
private:
    void onAssemble() override {
        this->addIngredient(kCheese_Ingredient);
    }
    bool onCook() override {
        return this->toastOnGriddle();
    }
};

class SkPeanutButterAndJelly : public SkSandwich {
private:
    void onAssemble() override {
        this->addIngredient(kPeanutButter_Ingredient);
        this->addIngredient(kGrapeJelly_Ingredient);
    }
};
```

## Integer Types

We follow the Google C++ guide for ints and are slowly making older code conform
to this

(https://google.github.io/styleguide/cppguide.html#Integer_Types)

Summary: Use `int` unless you have need a guarantee on the bit count, then use
`stdint.h` types (`int32_t`, etc). Assert that counts, etc are not negative
instead of using unsigned. Bitfields use `uint32_t` unless they have to be made
shorter for packing or performance reasons.

## Function Parameters

Mandatory constant object parameters are passed to functions as const
references. Optional constant object parameters are passed to functions as const
pointers. Mutable object parameters are passed to functions as pointers. We very
rarely pass anything by non-const reference.

<!--?prettify?-->

```
// src and paint are optional
void SkCanvas::drawBitmapRect(const SkBitmap& bitmap, const SkIRect* src,
                              const SkRect& dst, const SkPaint* paint = nullptr);

// metrics is mutable (it is changed by the method)
SkScalar SkPaint::getFontMetrics(FontMetric* metrics, SkScalar scale) const;

```

If function arguments or parameters do not all fit on one line, the overflowing
parameters may be lined up with the first parameter on the next line (either
grouped across lines or placed one per line):

<!--?prettify?-->

```
void drawBitmapRect(const SkBitmap& bitmap, const SkRect& dst,
                    const SkPaint* paint = nullptr) {
    this->drawBitmapRectToRect(bitmap, nullptr, dst, paint,
                               kNone_DrawBitmapRectFlag);
}
```

or all parameters placed on the next line and indented eight spaces:

<!--?prettify?-->

```
void drawBitmapRect(
        const SkBitmap& bitmap, const SkRect& dst, const SkPaint* paint = nullptr) {
    this->drawBitmapRectToRect(
            bitmap, nullptr, dst, paint, kNone_DrawBitmapRectFlag);
}
```

## Python

Python code follows the
[Google Python Style Guide](https://google.github.io/styleguide/pyguide.html).

## Folder Organiziation

Skia's public API should live in the `include` directory. Skia's private headers and implementation
files should live in the `src` directory. The `modules` directory contains extra features that are
built on top of Skia (`modules/skcms` being an exception) and can be used by clients.
Private utilities to test Skia live in `tools` and can be used for reference but should not be used
by clients.

No header in `include` should depend on files in other directories (`modules/skcms` being an exception).
This is to prevent private symbols from leaking into client code via transitive dependencies.

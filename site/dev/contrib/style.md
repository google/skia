Coding Style Guidelines
=======================

These conventions have evolved over time. Some of the earlier code in both
projects doesn’t strictly adhere to the guidelines. However, as the code evolves
we hope to make the existing code conform to the guildelines.

Files
-----

We use .cpp and .h as extensions for c++ source and header files. We use
foo_impl.h for headers with inline definitions for class foo.

Headers that aren’t meant for public consumption should be placed in src
directories so that they aren’t in a client’s search path.

We prefer to minimize includes. If forward declaring a name in a header is
sufficient then that is preferred to an include.

Forward declarations and file includes should be in alphabetical order (but we
aren't very strict about it).

<span id="no-define-before-sktypes"></span>
Do not use #if/#ifdef before including "SkTypes.h" (directly or indirectly).

We use spaces not tabs (4 of them).

We use Unix style endlines (LF).

We prefer no trailing whitespace but aren't very strict about it.

We wrap lines at 100 columns unless it is excessively ugly (use your judgement).
The soft line length limit was changed from 80 to 100 columns in June 2012. Thus,
most files still adhere to the 80 column limit. It is not necessary or worth
significant effort to promote 80 column wrapped files to 100 columns. Please
don't willy-nilly insert longer lines in 80 column wrapped files. Either be
consistent with the surrounding code or, if you really feel the need, promote
the surrounding code to 100 column wrapping.

Naming
------

Both projects use a prefix to designate that they are Skia prefix for classes,
enums, structs, typedefs etc is Sk. Ganesh’s is Gr. Nested types should not be
prefixed.

<!--?prettify?-->
~~~~
class SkClass {
public:
    class HelperClass {
        ...
    };
};
~~~~

Data fields in structs, classes, unions begin with lowercase f and are then 
camel capped.

<!--?prettify?-->
~~~~
struct GrCar {
    ...
    float fMilesDriven;
    ...
};
~~~~

Globals variables are similar but prefixed with g and camel-capped

<!--?prettify?-->
~~~~
bool gLoggingEnabled
Local variables begin lowercases and are camel-capped.

int herdCats(const Array& cats) {
    int numCats = cats.count();
}
~~~~

Enum values are prefixed with k. Unscoped enum values are post fixed with
an underscore and singular name of the enum name. The enum itself should be
singular for exclusive values or plural for a bitfield. If a count is needed it
is  k&lt;singular enum name&gt;Count and not be a member of the enum (see example):

<!--?prettify?-->
~~~~
enum class SkPancakeType {
     kBlueberry,
     kPlain,
     kChocolateChip,
};
~~~~

<!--?prettify?-->
~~~~
enum SkPancakeType {
     kBlueberry_PancakeType,
     kPlain_PancakeType,
     kChocolateChip_PancakeType,
    
     kLast_PancakeType = kChocolateChip_PancakeType
};

static const SkPancakeType kPancakeTypeCount = kLast_PancakeType + 1;
~~~~

A bitfield:

<!--?prettify?-->
~~~~
enum SkSausageIngredientBits {
    kFennel_SuasageIngredientBit = 0x1,
    kBeef_SausageIngredientBit   = 0x2
};
~~~~

or:

<!--?prettify?-->
~~~~
enum SkMatrixFlags {
    kTranslate_MatrixFlag = 0x1,
    kRotate_MatrixFlag    = 0x2
};
~~~~

Exception: anonymous enums can be used to declare integral constants, e.g.:

<!--?prettify?-->
~~~~
enum { kFavoriteNumber = 7 };
~~~~

Macros are all caps with underscores between words. Macros that have greater
than file scope should be prefixed SK or GR.

Static non-class functions in implementation files are lower case with
underscores separating words:

<!--?prettify?-->
~~~~
static inline bool tastes_like_chicken(Food food) {
    return kIceCream_Food != food;
}
~~~~

Externed functions or static class functions are camel-capped with an initial cap:

<!--?prettify?-->
~~~~
bool SkIsOdd(int n);

class SkFoo {
public:
    static int FooInstanceCount();
};
~~~~

Macros
------

Ganesh macros that are GL-specific should be prefixed GR_GL.

<!--?prettify?-->
~~~~
#define GR_GL_TEXTURE0 0xdeadbeef
~~~~

Ganesh prefers that macros are always defined and the use of #if MACRO rather than 
#ifdef MACRO.

<!--?prettify?-->
~~~~
#define GR_GO_SLOWER 0
...
#if GR_GO_SLOWER
    Sleep(1000);
#endif
~~~~

Skia tends to use #ifdef SK_MACRO for boolean flags.

Braces
------

Open braces don’t get a newline. “else” and “else if” appear on same line as
opening and closing braces unless preprocessor conditional compilation
interferes. Braces are always used with if, else, while, for, and do.

<!--?prettify?-->
~~~~
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
~~~~

Flow Control
------------

There is a space between flow control words and parentheses and between 
parentheses and braces:

<!--?prettify?-->
~~~~
while (...) {
}

do {
} while(...);

switch (...) {
...
}
~~~~

Cases and default in switch statements are indented from the switch.

<!--?prettify?-->
~~~~
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
~~~~

Fallthrough from one case to the next is commented unless it is trivial:

<!--?prettify?-->
~~~~
switch (recipe) {
    ...
    case kCheeseOmelette_Recipe:
        ingredients |= kCheese_Ingredient;
        // fallthrough
    case kPlainOmelette_Recipe:
        ingredients |= (kEgg_Ingredient | kMilk_Ingredient);
        break;
    ...
}
~~~~

When a block is needed to declare variables within a case follow this pattern:

<!--?prettify?-->
~~~~
switch (filter) {
    ...
    case kGaussian_Filter: {
        Bitmap srcCopy = src->makeCopy(); 
        ...
        break;
    }
    ...
};
~~~~

Classes
-------

Unless there is a need for forward declaring something, class declarations
should be ordered public, protected, private. Each should be preceded by a
newline. Within each visibility section (public, private), fields should not be
intermixed with methods.

<!--?prettify?-->
~~~~
class SkFoo {

public:
    ...

protected:
    ...        

private:
    SkBar fBar;
    ...

    void barHelper(...);
    ...
};
~~~~

Subclasses should have a private typedef of their super class called INHERITED:

<!--?prettify?-->
~~~~
class GrDillPickle : public GrPickle {
    ...
private:
    typedef GrPickle INHERITED;
};
~~~~

Virtual functions that are overridden in derived classes should use override
(and not the override keyword). The virtual keyword can be omitted.

<!--?prettify?-->
~~~~
void myVirtual() override {
}
~~~~

This should be the last element of their private section, and all references to 
base-class implementations of a virtual function should be explicitly qualified:

<!--?prettify?-->
~~~~
void myVirtual() override {
    ...
    this->INHERITED::myVirtual();
    ...
}
~~~~

As in the above example, derived classes that redefine virtual functions should
use override to note that explicitly.

Constructor initializers should be one per line, indented, with punctuation
placed before the initializer. This is a fairly new rule so much of the existing
code is non-conforming. Please fix as you go!

<!--?prettify?-->
~~~~
GrDillPickle::GrDillPickle()
    : GrPickle()
    , fSize(kDefaultPickleSize) {
    ...
}
~~~~

Constructors that take one argument should almost always be explicit, with 
exceptions made only for the (rare) automatic compatibility class.

<!--?prettify?-->
~~~~
class Foo {
    explicit Foo(int x);  // Good.
    Foo(float y);         // Spooky implicit conversion from float to Foo.  No no no!
    ...
};
~~~~

Method calls within method calls should be prefixed with dereference of the 
'this' pointer. For example:

<!--?prettify?-->
~~~~
this->method();
~~~~

Comparisons
-----------

We prefer that equality operators between lvalues and rvalues place the lvalue 
on the right:

<!--?prettify?-->
~~~~
if (7 == luckyNumber) {
    ...
}
~~~~

However, inequality operators need not follow this rule:

<!--?prettify?-->
~~~~
if (count > 0) {
    ...
}
~~~~

Comments

We use doxygen-style comments.

For grouping or separators in an implementation file we use 80 slashes

<!--?prettify?-->
~~~~
void SkClassA::foo() {
    ...
}

////////////////////////////////////////////////////////////////

void SkClassB::bar() {
    ...
}
~~~~

Integer Types
-------------

We follow the Google C++ guide for ints and are slowly making older code conform to this

(http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Integer_Types)

Summary: Use int unless you have need a guarantee on the bit count, then use
stdint.h types (int32_t, etc). Assert that counts, etc are not negative instead
of using unsigned. Bitfields use uint32_t unless they have to be made shorter
for packing or performance reasons.

nullptr, 0
-------

Use nullptr for pointers, 0 for ints. We prefer explicit nullptr comparisons when
checking for nullptr pointers (as documentation):

<!--?prettify?-->
~~~~
if (nullptr == x) {  // slightly preferred over if (!x)
   ...
}
~~~~

When checking non-nullptr pointers explicit comparisons are not required because it
reads like a double negative:

<!--?prettify?-->
~~~~
if (x) {  // slightly preferred over if (nullptr != x)
   ...
}
~~~~

Returning structs
-----------------

If the desired behavior is for a function to return a struct, we prefer using a
struct as an output parameter

<!--?prettify?-->
~~~~
void modify_foo(SkFoo* foo) {
    // Modify foo
}
~~~~

Then the function can be called as followed:

<!--?prettify?-->
~~~~
SkFoo foo;
modify_foo(&foo);
~~~~

This way, if return value optimization cannot be used there is no performance
hit. It also means that modify_foo can actually return a boolean for whether the
call was successful. In this case, initialization of foo can potentially be
skipped on failure (assuming the constructor for SkFoo does no initialization).

<!--?prettify?-->
~~~~
bool modify_foo(SkFoo* foo) {
    if (some_condition) {
        // Modify foo
        return true;
    }
    // Leave foo unmodified
    return false;
}
~~~~

Function Parameters
-------------------

Mandatory constant object parameters are passed to functions as const references
if they are not retained by the receiving function. Optional constant object
parameters are passed to functions as const pointers. Objects that the called
function will retain, either directly or indirectly, are passed as pointers.
Variable (i.e. mutable) object parameters are passed to functions as pointers.

<!--?prettify?-->
~~~~
// src and paint are optional
void SkCanvas::drawBitmapRect(const SkBitmap& bitmap, const SkIRect* src, 
                             const SkRect& dst, const SkPaint* paint = nullptr);
// metrics is mutable (it is changed by the method)
SkScalar SkPaint::getFontMetrics(FontMetric* metrics, SkScalar scale) const;
// A reference to foo is retained by SkContainer
void SkContainer::insert(const SkFoo* foo);
~~~~

If function arguments or parameters do not all fit on one line, they may be
lined up with the first parameter on the same line

<!--?prettify?-->
~~~~
void drawBitmapRect(const SkBitmap& bitmap, const SkRect& dst,
                    const SkPaint* paint = nullptr) {
    this->drawBitmapRectToRect(bitmap, nullptr, dst, paint,
                               kNone_DrawBitmapRectFlag);
}
~~~~

or placed on the next line indented eight spaces

<!--?prettify?-->
~~~~
void drawBitmapRect(
        const SkBitmap& bitmap, const SkRect& dst,
        const SkPaint* paint = nullptr) {
    this->drawBitmapRectToRect(
            bitmap, nullptr, dst, paint, kNone_DrawBitmapRectFlag);
}
~~~~

Python
------

Python code follows the [Google Python Style Guide](http://google-styleguide.googlecode.com/svn/trunk/pyguide.html).


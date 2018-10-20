Flattenables
============

Many objects in Skia, such as `SkShaders` and other effects on `SkPaint`, need to be 
flattened into a data stream for either transport or as part of the key to the 
font cache. Classes for these objects should derive from `SkFlattenable` or one of 
its subclasses. If you create a new flattenable class, you need to make sure you 
do a few things so that it will work on all platforms:

1: Override the method `flatten` (the default scope is protected):

<!--?prettify?-->
~~~~
virtual void flatten(SkFlattenableWriteBuffer& buffer) const override {
    this->INHERITED::flatten(buffer);
    // Write any private data that needs to be stored to recreate this object
}
~~~~

2: Override the (protected) constructor that creates an object from an 
`SkFlattenableReadBuffer`:

<!--?prettify?-->
~~~~
SkNewClass(SkFlattenableReadBuffer& buffer)
: INHERITED(buffer) {
    // Read the data from the buffer in the same order as it was written to the
    // SkFlattenableWriteBuffer and construct the new object
}
~~~~

3: Declare a set of deserialization procs for your object in the class declaration:
We have a macro for this:

<!--?prettify?-->
~~~~
private:

SK_FLATTENABLE_HOOKS(SkNewClass)
~~~~

4: If your class is declared in a `.cpp` file or in a private header file, create a 
function to register its group:
This occurs in cases where the classes are hidden behind a factory, like many effects 
and shaders are.  Then in the parent class header file (such as `SkGradientShader`) you 
need to add:

<!--?prettify?-->
~~~~
public:

static void RegisterFlattenables();
~~~~

Then in the cpp file you define all the members of the group together:

<!--?prettify?-->
~~~~
void SkGroupClass::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkMemberClass1)

    SK_REGISTER_FLATTENABLE(SkMemberClass2)

    // etc
}
~~~~


5: Register your flattenable with the global registrar:
You need to add one line to `SkFlattenable::InitalizeFlattenables()`. To register the 
flattenable in a Skia build, that function is defined in `SkGlobalInitialization_default.cpp`. 
For Chromium, it is in `SkGlobalInitialization_chromium.cpp`.
For a single flattenable add

<!--?prettify?-->
~~~~
SK_REGISTER_FLATTENABLE(SkNewClass)
~~~~

For a group, add

<!--?prettify?-->
~~~~
SkGroupClass::RegisterFlattenables();
~~~~


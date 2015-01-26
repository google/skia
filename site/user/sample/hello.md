Creating a Skia "Hello World!"
==============================

This tutorial will guide you through the steps to create a Hello World Desktop
application in Skia.

Who this tutorial is for:
-------------------------

This will be useful to you if you want to create a window that can receive
events and to which you can draw with Skia.

Step 1: Check out and build Skia
--------------------------------

Follow the instructions for: Linux, Mac OS X or Windows. The framework that we
will be using does not currently support other platforms.

Once you have a working development environment, we can move on to the next step.

Step 2: Build the included HelloSkia Example
--------------------------------------------

We will be using the "SkiaExamples" framework. You can find it in the
experimental/SkiaExamples directory. There is an included HelloWorld example,
and we will start by building it before we go ahead and create our own.

### On Mac OS X

Run `GYP_GENERATORS="ninja" ./gyp_skia`
This will generate a ninja target, and `ninja -C out/Debug SkiaExamples` will create `SkiaExamples.app`

### On Linux:
Run `GYP_GENERATORS="ninja" ./gyp_skia`

Build the SkiaExamples target:

    ninja -C out/Release SkiaExamples

The SkiaExamples binary should be in `out/Release/SkiaExamples`

### On Windows

Run `./gyp_skia`

There should be a Visual Studio project `out/gyp/SkiaExamples.vcproj`  with
which you can build the SkiaExamples binary.

### Run the SkiaExamples.

You should see a window open displaying rotating text and some geometry.

Step 3: Create your own Sample
------------------------------

Create a file `experimental/SkiaExamples/Tutorial.cpp` within the Skia tree.  Copy the following code:

<!--?prettify lang=cc?-->

~~~~
#include "SkExample.h"
#include "SkDevice.h"

class HelloTutorial : public SkExample {
    public:
        HelloTutorial(SkExampleWindow* window)
            : SkExample(window)
        {
            fName = "Tutorial";  // This is how Skia will find your example.

            fWindow->setupBackend(SkExampleWindow::kGPU_DeviceType);
           // Another option is the CPU backend:  fWindow->setupBackend(kRaster_DeviceType);
        }

    protected:
        void draw(SkCanvas* canvas) SK_OVERRIDE {
            // Clear background
            canvas->drawColor(SK_ColorWHITE);

            SkPaint paint;
            // Draw a message with a nice black paint.
            paint.setFlags(SkPaint::kAntiAlias_Flag);
            paint.setColor(SK_ColorBLACK);
            paint.setTextSize(SkIntToScalar(20));

            static const char message[] = "Hello World!";

            // Translate and draw the text:
            canvas->save();
            canvas->translate(SkIntToScalar(50), SkIntToScalar(100));
            canvas->drawText(message, strlen(message), SkIntToScalar(0), SkIntToScalar(0), paint);
            canvas->restore();

            // If you ever want to do animation. Use the inval method to trigger a redraw.
            this->fWindow->inval(NULL);
        }
};

static SkExample* MyFactory(SkExampleWindow* window) {
    return new HelloTutorial(window);
}
static SkExample::Registry registry(MyFactory);
~~~~


Step 4: Compile and run SkiaExamples with your Sample
-----------------------------------------------------

Here is what you have to do to compile your example. There will be
functionality to make this easier, but for now, this is what you have to do:

*   Open `gyp/experimental.gyp` and look for the `SkiaExamples` target.

*   In the 'sources' section of the SkiaExampels target, add
    `../experimental/SkiaExamples/Tutorial.cpp` to the list of sources.

*   Repeat Step 2 to update our gyp targets and build our example.

*   Run the SkiaExamples, specifying the name of our new example:

        $> out/Release/SkiaExamples --match Tutorial

Step 5: How to iterate through multiple examples
------------------------------------------------

If you did not specify an example with the `--match` flag, or if your match
string matches more than one example, you can use the *n* key to iterate
through all of the examples registered.


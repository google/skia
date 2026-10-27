# Skia GMs

GMs are correctness tests that define a drawing and its expected output. They are used for verifying drawing results across different backends and configurations.

## Defining a GM

Most GMs inherit from `skiagm::GM` and use the `DEF_GM` macro.

```cpp
#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

DEF_GM(return new MyNewGM;)

class MyNewGM : public skiagm::GM {
protected:
    SkString getName() const override { return SkString("mynewgm"); }
    SkISize getISize() override { return SkISize::Make(100, 100); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeWH(50, 50), paint);
    }
};
```

- `getName()`: Returns the unique name of the GM. This is used with the `--match` flag.
- `getISize()`: Returns the dimensions of the GM's canvas.
- `onDraw()`: The core drawing logic for the test.

## Adding to the Build
When you create a new GM file in `gm/`, you **must** add it to the `gm_sources` list in `gn/gm.gni` for it to be included in the build.

## Running GMs

GMs are run using `dm` with the `gm` source.

```bash
out/Debug/dm --src gm --match mynewgm
```

## Visualizing GMs
Use `viewer` for interactive visualization and debugging of GMs.

```bash
out/Debug/viewer --match mynewgm
```

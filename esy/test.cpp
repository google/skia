#include "SkSurface.h"
#include "SkPath.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"

int main (int argc, char * const argv[]) {
  const char * filePath = argv[1];
  int width = 256;
  int height = 256;

  // create canvas to draw on
  sk_sp<SkSurface> rasterSurface = SkSurface::MakeRasterN32Premul(width, height);
  SkCanvas* canvas = rasterSurface->getCanvas();

  // creating a path to be drawn
  SkPath path;
  path.moveTo(10.0f, 10.0f);
  path.lineTo(100.0f, 0.0f);
  path.lineTo(100.0f, 100.0f);
  path.lineTo(0.0f, 100.0f);
  path.lineTo(50.0f, 50.0f);
  path.close();

  // creating a paint to draw with
  SkPaint p;
  p.setAntiAlias(true);

  // clear out which may be was drawn before and draw the path
  canvas->clear(SK_ColorWHITE);
  canvas->drawPath(path, p);

  // make a PNG encoded image using the canvas
  sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
  if (!img) { return 1; }
  sk_sp<SkData> png(img->encodeToData());
  if (!png) { return 1; }

  // write the data to the file specified by filePath
  SkFILEWStream out(filePath);
  (void)out.write(png->data(), png->size());

  return 0;
}

#include "SkDebugCanvas.h"
#include "SkPicture.h"
#include "SkSurface.h"
#include <emscripten.h>
#include <emscripten/bind.h>

using JSColor = int32_t;

struct SimpleImageInfo {
    int width;
    int height;
    SkColorType colorType;
    SkAlphaType alphaType;
    // TODO color spaces?
};

SkImageInfo toSkImageInfo(const SimpleImageInfo& sii) {
    return SkImageInfo::Make(sii.width, sii.height, sii.colorType, sii.alphaType);
}

void loadSkpAndDraw(SkSurface* surface) {
	SkDebugf("loadSkpAndDraw called.\n");
	SkDebugCanvas debugCanvas(100,50);
	SkDebugf("SkDebugCanvas created.\n");


	// load and parse skp file that was included with --preload-file
    sk_sp<SkPicture> picture = SkPicture::MakeFromStream(
    	SkStream::MakeFromFile("experimental/wasm-skp-debugger/sample2.skp").get());
	SkDebugf("parsed skp file.\n");
	// draw commands
    debugCanvas.drawPicture(picture);
    SkDebugf("drew picture.\n");
	debugCanvas.draw(surface->getCanvas());

	SkDebugf("SkDebugCanvas has %d commands.\n", debugCanvas.getSize());
	surface->getCanvas()->flush();
	SkDebugf("After Flush SkDebugCanvas has %d commands.\n", debugCanvas.getSize());

	
}

using namespace emscripten;
EMSCRIPTEN_BINDINGS(my_module) {
    function("loadSkpAndDraw", &loadSkpAndDraw, allow_raw_pointers());

    enum_<SkColorType>("ColorType").value("RGBA_8888", SkColorType::kRGBA_8888_SkColorType);
    enum_<SkAlphaType>("AlphaType").value("Unpremul", SkAlphaType::kUnpremul_SkAlphaType);
    value_object<SimpleImageInfo>("SkImageInfo")
        .field("width",     &SimpleImageInfo::width)
        .field("height",    &SimpleImageInfo::height)
        .field("colorType", &SimpleImageInfo::colorType)
        .field("alphaType", &SimpleImageInfo::alphaType);
    constant("TRANSPARENT", (JSColor) SK_ColorTRANSPARENT);
    function("_getRasterDirectSurface", optional_override([](const SimpleImageInfo ii,
                                                             uintptr_t /* uint8_t*  */ pPtr,
                                                             size_t rowBytes)->sk_sp<SkSurface> {
        uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
        SkImageInfo imageInfo = toSkImageInfo(ii);
        return SkSurface::MakeRasterDirect(imageInfo, pixels, rowBytes, nullptr);
    }), allow_raw_pointers());
    class_<SkSurface>("SkSurface")
        .smart_ptr<sk_sp<SkSurface>>("sk_sp<SkSurface>")
        .function("width", &SkSurface::width)
        .function("height", &SkSurface::height)
        .function("_flush", &SkSurface::flush)
        .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers());
    class_<SkCanvas>("SkCanvas")
        .function("clear", optional_override([](SkCanvas& self, JSColor color)->void {
            // JS side gives us a signed int instead of an unsigned int for color
            // Add a optional_override to change it out.
            self.clear(SkColor(color));
        }));
}
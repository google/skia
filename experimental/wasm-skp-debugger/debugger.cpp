
#include "SkDebugCanvas.h"
#include <emscripten.h>
#include <emscripten/bind.h>

void makeCanvas() {
	SkDebugCanvas fDebugCanvas(100,50);
}

using namespace emscripten;
EMSCRIPTEN_BINDINGS(my_module) {
    function("makeCanvas", &makeCanvas  );
}
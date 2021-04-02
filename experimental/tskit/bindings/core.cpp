
#include <string>

#include <emscripten.h>
#include <emscripten/bind.h>
using namespace emscripten;

EMSCRIPTEN_BINDINGS(Skia) {
    function("globalFunction", optional_override([](size_t length)->size_t {
        return length * 7;
    }));
}
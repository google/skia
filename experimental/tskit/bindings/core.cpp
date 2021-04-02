
#include <string>

#include <emscripten.h>
#include <emscripten/bind.h>
using namespace emscripten;

class Something {
public:
    Something(std::string n): fName;

    const std::string getName() {
        return fName;
    }

private:
    std::string fName;
};

// This macro will help create ... . It regenerated with
// make re-gen
#define TS_EXPORT(foo)

EMSCRIPTEN_BINDINGS(Skia) {
    TS_EXPORT(_privateFunction(length: number): number)
    function("_privateFunction", optional_override([](size_t length)->size_t {
        return length * 7;
    }));

    TS_EXPORT(_myNewThing(input: string): number)
    function("_myNewThing", optional_override([](std::string s)->int {
        return s.length;
    }));


    class_<Something>("Something")
         /*TS */
        .constructor<std::string>()
        /*TS Something::getName()*/
        .function("getName", &Something::getName);
}
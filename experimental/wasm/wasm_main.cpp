#include "SkPath.h"
#include "SkPathOps.h"

#include <emscripten/emscripten.h>
#include <stdio.h>

// the asm renames the mangled C++ name into something more human readable.
const char* EMSCRIPTEN_KEEPALIVE SimplifySVG(const char* s) asm("SimplifySVG");
const char* EMSCRIPTEN_KEEPALIVE SimplifySVG(const char* s) {
    printf("I saw this string %s\n", s);
    return "teh quick brown fox";
}

int main(int argc, char const *argv[])
{
    //Test out the Simplify
    SkPath path;
    path.moveTo(0,0);
    path.lineTo(40,0);
    path.lineTo(20,20);
    path.close();
    path.moveTo(20,0);
    path.lineTo(60,0);
    path.lineTo(40,20);
    path.close();

    SkPath simple;

    Simplify(path, &simple);

    simple.dump();

    return 0;
}

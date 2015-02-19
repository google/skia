
/*
 * Copyright 2013 Google, Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDebugUtils_DEFINED
#define SkDebugUtils_DEFINED

#include "SkTypes.h"

// These functions dump 0, 1, and 2d arrays of data in a format that's
// compatible with Mathematica for quick visualization


template<class T>
inline void SkDebugDumpMathematica( const T val ) {
    SkDEBUGFAIL("Need to specialize SkDebugDumpMathematica for your type, sorry.");
}

template<class T>
inline void SkDebugDumpMathematica(const char *name, const T *array, int size) {
    SkDebugf("%s", name);
    SkDebugf(" = {");
    for (int i=0 ; i < size ; i++) {
        SkDebugDumpMathematica<T>(array[i]);
        if (i != size-1) SkDebugf(", ");
    }
    SkDebugf("};\n");
}

template<class T>
inline void SkDebugDumpMathematica(const char *name, const T *array, int width, int height) {
    SkDebugf("%s", name);
    SkDebugf(" = {\n");
    for (int i=0 ; i < height ; i++) {
        SkDebugf("  {");
        for (int j = 0 ; j < width ; j++) {
            SkDebugDumpMathematica<T>(array[i*width + j]);
            if (j != width-1) {
                SkDebugf(", ");
            }
        }
        SkDebugf("}");
        if (i != height-1) {
            SkDebugf(", \n");
        }
    }
    SkDebugf("\n};\n");
}

template<class T>
inline void SkDebugDumpMathematica( const char *name, const T val ) {
    SkDebugf("%s", name);
    SkDebugf(" = ");
    SkDebugDumpMathematica<T>(val);
    SkDebugf(";\n");
}

template<>
inline void SkDebugDumpMathematica<uint8_t>( const uint8_t val ) {
    SkDebugf("%u", val);
}

template<>
inline void SkDebugDumpMathematica<unsigned int>( const unsigned int val ) {
    SkDebugf("%u", val);
}

template<>
inline void SkDebugDumpMathematica<int>( const int val ) {
    SkDebugf("%d", val);
}

template<>
inline void SkDebugDumpMathematica<size_t>( const size_t val ) {
    SkDebugf("%u", val);
}

template<>
void SkDebugDumpMathematica<const char *>( const char * val ) {
    SkDebugf("%s", val);
}

template<>
inline void SkDebugDumpMathematica<float>( float val ) {
    SkDebugf("%f", val);
}


#endif

/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#ifndef GrGLEffect_DEFINED
#define GrGLEffect_DEFINED

#include "GrGLInterface.h"
#include "GrStringBuilder.h"

class GrEffect;

struct ShaderCodeSegments {
    GrSStringBuilder<256> fVSUnis;
    GrSStringBuilder<256> fVSAttrs;
    GrSStringBuilder<256> fVaryings;
    GrSStringBuilder<256> fFSUnis;
    GrSStringBuilder<512> fVSCode;
    GrSStringBuilder<512> fFSCode;
};

/**
 * This class is currently a stub.  This will be a base class for "effects", 
 * which extend the data model of GrPaint and extend the capability of
 * GrGLProgram in a modular fashion.
 */
class GrGLEffect {
protected:
    GrGLEffect(GrEffect* effect) {}
public:    
    virtual ~GrGLEffect() {}
    static GrGLEffect* Create(GrEffect* effect) { return NULL; }
    void genShaderCode(ShaderCodeSegments* segments) {}
    bool doGLSetup(GrPrimitiveType type, GrGLint program) { return true; }
    bool doGLPost() { return true; }
    void buildKey(GrBinHashKeyBuilder& key) const {}
    GrGLEffect* nextEffect() { return NULL; }
};

#endif

/*
    Copyright 2010 Google Inc.

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


#ifndef GrGpuGLShaders_DEFINED
#define GrGpuGLShaders_DEFINED

#include "GrGpuGL.h"

// Programmable OpenGL or OpenGL ES 2.0
class GrGpuGLShaders : public GrGpuGL {
public:
             GrGpuGLShaders();
    virtual ~GrGpuGLShaders();
    
    virtual void resetContext();

    // type of colors used by a program
    enum ColorType {
        kNone_ColorType,
        kAttrib_ColorType,
        kUniform_ColorType,
    };
protected:
    // overrides from GrGpu
    virtual bool flushGraphicsState(PrimitiveType type);
    virtual void setupGeometry(uint32_t startVertex,
                               uint32_t startIndex,
                               uint32_t vertexCount,
                               uint32_t indexCount);
    
private:
    void resetContextHelper();
    
    // sets the texture matrix uniform for currently bound program
    void flushTexMatrix(GLint location, 
                        GrGLTexture::Orientation orientation);
    // sets the MVP matrix uniform for currently bound program
    void flushMatrix(GLint location);
    
    void flushTwoPointRadial(GLint paramsLocation, const GrSamplerState&);
    
    // reads shader from array and compiles it with GL, returns shader ID or 0 if failed
    GLuint loadShader(GLenum type, const char* src);
    
    struct ProgramData;
    // creates a GL program with two shaders attached. 
    // Gets the relevant uniform locations.
    // Sets the texture sampler if present to texture 0
    // Binds the program
    // returns true if succeeded.
    bool createProgram(GLuint vshader,
                       GLuint fshader,
                       bool hasTexMatrix,
                       bool hasTexCoords,
                       ColorType colorType,
                       bool twoPointRadial,
                       ProgramData* program);

    // called at flush time to setup the appropriate program
    void flushProgram(PrimitiveType type);

    enum Programs {
        // use vertex coordinates         
        kTextureVertCoords_Program = 0,
        kTextureVertCoordsProj_Program,
        
        // use separate tex coords
        kTextureTexCoords_Program,
        kTextureTexCoordsProj_Program,

        // constant color texture, no proj
        // verts as a tex coords
        kTextureVertCoordsNoColor_Program,        

        // constant color texture, no proj
        // separate tex coords
        kTextureTexCoordsNoColor_Program,

        // special program for text glyphs
        kText_Program,

        // programs for radial texture lookup
        kRadialTextureVertCoords_Program,
        kRadialTextureTexCoords_Program,

        // programs for sweep texture lookup
        kSweepTextureVertCoords_Program,
        kSweepTextureTexCoords_Program, 
        
        // programs for two-point radial lookup
        kTwoPointRadialTextureVertCoords_Program,
        kTwoPointRadialTextureTexCoords_Program,
        
        // color only drawing
        kNoTexture_Program,

        kProgramCount
    };

    // Records per-program information
    // we can specify the attribute locations so that they are constant
    // across our shaders. But the driver determines the uniform locations 
    // at link time. We don't need to remember the sampler uniform location
    // because we will bind a texture slot to it and never change it
    // Uniforms are program-local so we can't rely on fHWState to hold the 
    // previous uniform state after a program change.
    struct ProgramData {
        // IDs
        GLuint    fVShaderID;
        GLuint    fFShaderID;
        GLuint    fProgramID;
        
        // shader uniform locations (-1 if shader doesn't use them)
        GLint     fMatrixLocation;
        GLint     fTexMatrixLocation;
        GLint     fColorLocation;
        GLint     fTwoPointParamsLocation;
        
        ColorType fColorType;

        // these reflect the current values of uniforms
        // (GL uniform values travel with program)
        GrMatrix                    fViewMatrix;
        GrMatrix                    fTextureMatrices[kNumStages];
        GrColor                     fColor;
        GrGLTexture::Orientation    fTextureOrientation;
        GrScalar                    fRadial2CenterX1;
        GrScalar                    fRadial2Radius0;
        bool                        fRadial2PosRoot;
    };
    
    ProgramData fPrograms[kProgramCount];
    Programs    fHWProgram;
    
    GrGLTexture::Orientation  fTextureOrientation;

    typedef GrGpuGL INHERITED;
};

#endif

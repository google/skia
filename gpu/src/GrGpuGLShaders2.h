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


#ifndef GrGpuGLShaders2_DEFINED
#define GrGpuGLShaders2_DEFINED

#include "GrGpuGL.h"

// Programmable OpenGL or OpenGL ES 2.0
class GrGpuGLShaders2 : public GrGpuGL {
public:
             GrGpuGLShaders2();
    virtual ~GrGpuGLShaders2();

protected:
    // overrides from GrGpu
    virtual bool flushGraphicsState(GrPrimitiveType type);
    virtual void setupGeometry(int* startVertex,
                               int* startIndex,
                               int vertexCount,
                               int indexCount);

private:

    virtual void resetContext();

    // Helpers to make code more readable
    const GrMatrix& getHWSamplerMatrix(int stage);
    void recordHWSamplerMatrix(int stage, const GrMatrix& matrix);

    // sets the texture matrix uniform for currently bound program
    void flushTextureMatrix(int stage);

    // sets the MVP matrix uniform for currently bound program
    void flushViewMatrix();

    // flushes the parameters to two point radial gradient
    void flushRadial2(int stage);

    // called at flush time to setup the appropriate program
    void flushProgram(GrPrimitiveType type);

    struct Program;

    struct StageDesc;
    struct ProgramDesc;

    struct UniLocations;
    struct StageUniLocations;

    struct ShaderCodeSegments;

    class ProgramCache;

    // gets a description of needed shader
    void getProgramDesc(GrPrimitiveType primType, ProgramDesc* desc);

    // generates and compiles a program from a description and vertex layout
    // will change GL's bound program
    static void GenProgram(const ProgramDesc& desc, Program* program);

    // generates code for a stage of the shader
    static void GenStageCode(int stageNum,
                             const StageDesc& desc,
                             const char* psInColor,
                             const char* psOutColor,
                             const char* vsInCoord,
                             ShaderCodeSegments* segments,
                             StageUniLocations* locations);

    // Compiles a GL shader, returns shader ID or 0 if failed
    // params have same meaning as glShaderSource
    static GrGLuint CompileShader(GrGLenum type, int stringCnt,
                                  const char** strings,
                                  int* stringLengths);
    static void DeleteProgram(Program* program);

    void ProgramUnitTest();

    ProgramCache*   fProgramCache;
    Program*        fProgram;
    GrGLuint          fHWProgramID;

    typedef GrGpuGL INHERITED;
};

#endif


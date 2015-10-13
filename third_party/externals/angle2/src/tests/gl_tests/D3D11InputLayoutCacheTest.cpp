//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// D3D11InputLayoutCacheTest:
//   Stress to to reproduce a bug where we weren't fluing the case correctly.
//

#include <sstream>

#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"

using namespace angle;

namespace
{

class D3D11InputLayoutCacheTest : public ANGLETest
{
  protected:
    D3D11InputLayoutCacheTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigAlphaBits(8);
    }

    GLuint makeProgramWithAttribCount(unsigned int attribCount)
    {
        std::stringstream strstr;

        strstr << "attribute vec2 position;" << std::endl;
        for (unsigned int attribIndex = 0; attribIndex < attribCount; ++attribIndex)
        {
            strstr << "attribute float a" << attribIndex << ";" << std::endl;
        }
        strstr << "varying float v;" << std::endl
               << "void main() {" << std::endl
               << "    v = 0.0;" << std::endl;
        for (unsigned int attribIndex = 0; attribIndex < attribCount; ++attribIndex)
        {
            strstr << "    v += a" << attribIndex << ";" << std::endl;
        }
        strstr << "    gl_Position = vec4(position, 0.0, 1.0);" << std::endl
               << "}" << std::endl;

        const std::string basicFragmentShader =
            "varying highp float v;\n"
            "void main() {"
            "   gl_FragColor = vec4(v / 255.0, 0.0, 0.0, 1.0);\n"
            "}\n";

        return CompileProgram(strstr.str(), basicFragmentShader);
    }
};

// Stress the cache by setting a small cache size and drawing with a bunch of shaders
// with different input signatures.
TEST_P(D3D11InputLayoutCacheTest, StressTest)
{
    // Hack the ANGLE!
    gl::Context *context = reinterpret_cast<gl::Context *>(getEGLWindow()->getContext());
    rx::Renderer11 *renderer11 = rx::GetAs<rx::Renderer11>(context->getRenderer());
    rx::InputLayoutCache *inputLayoutCache = renderer11->getInputLayoutCache();

    // Clamp the cache size to something tiny
    inputLayoutCache->setCacheSize(4);

    GLint maxAttribs = 0;
    context->getIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);

    // Reserve one attrib for position
    unsigned int maxInputs = static_cast<unsigned int>(maxAttribs) - 2;

    std::vector<GLuint> programs;
    for (unsigned int attribCount = 0; attribCount <= maxInputs; ++attribCount)
    {
        GLuint program = makeProgramWithAttribCount(attribCount);
        ASSERT_NE(0u, program);
        programs.push_back(program);
    }

    // Iteratively do a simple drop operation, trying every attribute count from 0..MAX_ATTRIBS.
    // This should thrash the cache.
    for (unsigned int iterationCount = 0; iterationCount < 10; ++iterationCount)
    {
        ASSERT_GL_NO_ERROR();

        for (unsigned int attribCount = 0; attribCount <= maxInputs; ++attribCount)
        {
            GLuint program = programs[attribCount];
            glUseProgram(program);

            for (unsigned int attribIndex = 0; attribIndex < attribCount; ++attribIndex)
            {
                std::stringstream attribNameStr;
                attribNameStr << "a" << attribIndex;
                std::string attribName = attribNameStr.str();

                GLint location = glGetAttribLocation(program, attribName.c_str());
                ASSERT_NE(-1, location);
                glVertexAttrib1f(location, 1.0f);
                glDisableVertexAttribArray(location);
            }

            drawQuad(program, "position", 0.5f);
            EXPECT_PIXEL_EQ(0, 0, attribCount, 0, 0, 255u);
        }
    }

    for (GLuint program : programs)
    {
        glDeleteProgram(program);
    }
}

ANGLE_INSTANTIATE_TEST(D3D11InputLayoutCacheTest, ES2_D3D11());

}  // anonymous namespace

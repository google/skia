//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "system_utils.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

class OcclusionQueriesTest : public ANGLETest
{
  protected:
    OcclusionQueriesTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mProgram = 0;
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string passthroughVS = SHADER_SOURCE
        (
            attribute highp vec4 position;
            void main(void)
            {
                gl_Position = position;
            }
        );

        const std::string passthroughPS = SHADER_SOURCE
        (
            precision highp float;
            void main(void)
            {
               gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
            }
        );

        mProgram = CompileProgram(passthroughVS, passthroughPS);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

TEST_P(OcclusionQueriesTest, IsOccluded)
{
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // draw a quad at depth 0.3
    glEnable(GL_DEPTH_TEST);
    glUseProgram(mProgram);
    drawQuad(mProgram, "position", 0.3f);
    glUseProgram(0);

    EXPECT_GL_NO_ERROR();

    GLuint query = 0;
    glGenQueriesEXT(1, &query);
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    drawQuad(mProgram, "position", 0.8f); // this quad should be occluded by first quad
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
    }

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);

    EXPECT_GL_NO_ERROR();

    glDeleteQueriesEXT(1, &query);

    EXPECT_GLENUM_EQ(GL_FALSE, result);
}

TEST_P(OcclusionQueriesTest, IsNotOccluded)
{
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    GLuint query = 0;
    glGenQueriesEXT(1, &query);
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    drawQuad(mProgram, "position", 0.8f); // this quad should not be occluded
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result); // will block waiting for result

    EXPECT_GL_NO_ERROR();

    glDeleteQueriesEXT(1, &query);

    EXPECT_GLENUM_EQ(GL_TRUE, result);
}

TEST_P(OcclusionQueriesTest, Errors)
{
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    GLuint query = 0;
    GLuint query2 = 0;
    glGenQueriesEXT(1, &query);

    EXPECT_EQ(glIsQueryEXT(query), GL_FALSE);
    EXPECT_EQ(glIsQueryEXT(query2), GL_FALSE);

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, 0); // can't pass 0 as query id
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT, query2); // can't initiate a query while one's already active
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    EXPECT_EQ(glIsQueryEXT(query), GL_TRUE);
    EXPECT_EQ(glIsQueryEXT(query2), GL_FALSE); // have not called begin

    drawQuad(mProgram, "position", 0.8f); // this quad should not be occluded
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT); // no active query for this target
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT, query); // can't begin a query as a different type than previously used
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT, query2); // have to call genqueries first
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glGenQueriesEXT(1, &query2);
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT, query2); // should be ok now
    EXPECT_EQ(glIsQueryEXT(query2), GL_TRUE);

    drawQuad(mProgram, "position", 0.3f); // this should draw in front of other quad
    glDeleteQueriesEXT(1, &query2); // should delete when query becomes inactive
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT); // should not incur error; should delete query + 1 at end of execution.
    EXPECT_GL_NO_ERROR();

    swapBuffers();

    EXPECT_GL_NO_ERROR();

    GLuint ready = GL_FALSE;
    glGetQueryObjectuivEXT(query2, GL_QUERY_RESULT_AVAILABLE_EXT, &ready); // this query is now deleted
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(OcclusionQueriesTest, ES2_D3D9(), ES2_D3D11());

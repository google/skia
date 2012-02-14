
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLColorConversionTest.h"
#include "GrGLContextInfo.h"
#include "GrGLShaderVar.h"
#include "GrGLSL.h"

/**
 * We expect the floating point range [i - .5] / 255 through [i + .5] / 255 to 
 * round to be converted to byte value i when output from a fragment shader into
 * byte-per-channel color format. To test the rounding rule we draws a 256 pixel
 * wide strip. At pixel i FS writes values close to the high and low endpoints
 * of this range are written as color components. After read pixels we check how
 * the values were actually rounded. As a sanity check, a value smack dab in the
 * middle of the range we expect to convert to i is also output.
 */
GrGLByteColorConversion
    GrGLFloatToByteColorConversion(const GrGLContextInfo& ctxInfo) {

    bool fail = true;

    GrGLuint dstRb = 0;
    GrGLuint dstFbo = 0;
    GrGLuint vertBuf = 0;
    GrGLuint vsId = 0;
    GrGLuint fsId = 0;
    GrGLuint progId = 0;
    GrGLShaderVar posIn, tOut, tIn, colorOut;
    GrStringBuilder vs, fs;

    const GrGLInterface* gl = ctxInfo.interface();
    GrGLSLGeneration gen = ctxInfo.glslGeneration();
    GrGLBinding binding = ctxInfo.binding();

    // setup the dst
    GR_GL_CALL(gl, GenFramebuffers(1, &dstFbo));
    GrAssert(0 != dstFbo);
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, dstFbo));
    GR_GL_CALL(gl, GenRenderbuffers(1, &dstRb));
    GrAssert(0 != dstRb);
    GR_GL_CALL(gl, BindRenderbuffer(GR_GL_RENDERBUFFER, dstRb));
    GR_GL_CALL(gl, RenderbufferStorage(GR_GL_RENDERBUFFER,
                                       GR_GL_RGBA8,
                                       256, 1));
    GR_GL_CALL(gl, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, 
                                           GR_GL_COLOR_ATTACHMENT0,
                                           GR_GL_RENDERBUFFER,
                                           dstRb));
    GR_GL_CALL(gl, Viewport(0, 0, 256, 1));
    GrGLenum status;
    GR_GL_CALL_RET(gl, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
    if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
        goto FINISHED;
    }

    GR_GL_CALL_RET(gl, vsId, CreateShader(GR_GL_VERTEX_SHADER));
    GR_GL_CALL_RET(gl, fsId, CreateShader(GR_GL_FRAGMENT_SHADER));
    GR_GL_CALL_RET(gl, progId, CreateProgram());

    GR_GL_CALL(gl, Disable(GR_GL_CULL_FACE));
    GR_GL_CALL(gl, Disable(GR_GL_BLEND));
    GR_GL_CALL(gl, Disable(GR_GL_DEPTH_TEST));
    GR_GL_CALL(gl, Disable(GR_GL_DITHER));
    GR_GL_CALL(gl, Disable(GR_GL_SCISSOR_TEST));
    GR_GL_CALL(gl, Disable(GR_GL_STENCIL_TEST));
    GR_GL_CALL(gl, ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));

    // setup the geometry: a single tri that fills the screen
    static const GrGLfloat verts[] = {-2.f, -1.f,
                                       1.f, -1.f,
                                       1.f,  2.f};
    GR_GL_CALL(gl, GenBuffers(1, &vertBuf));
    GR_GL_CALL(gl, BindBuffer(GR_GL_ARRAY_BUFFER, vertBuf));
    GR_GL_CALL(gl, BufferData(GR_GL_ARRAY_BUFFER,
                              sizeof(verts),
                              verts,
                              GR_GL_STATIC_DRAW));
    GR_GL_CALL(gl, EnableVertexAttribArray(0));
    GR_GL_CALL(gl, VertexAttribPointer(0,      // index
                                       2,      // vector size
                                       GR_GL_FLOAT,
                                       false,
                                       2 * sizeof(GrGLfloat),
                                       0));    // offset


    const char* verStr = GrGetGLSLVersionDecl(binding, gen);

    posIn.set(GrGLShaderVar::kVec2f_Type,
              GrGLShaderVar::kAttribute_TypeModifier,
              "pos");
    tOut.set(GrGLShaderVar::kVec2f_Type,
             GrGLShaderVar::kOut_TypeModifier,
             "t");
    tIn.set(GrGLShaderVar::kVec2f_Type,
            GrGLShaderVar::kIn_TypeModifier,
            "t");
    bool declColor = GrGLSLSetupFSColorOuput(gen, "color", &colorOut);

    vs.append(verStr);
    posIn.appendDecl(ctxInfo, &vs);
    tOut.appendDecl(ctxInfo, &vs);
    vs.appendf("void main() {\n"
                "    gl_Position = vec4(%s.xy, 0.0, 1.0);\n"
                "    %s = (%s + vec2(1.0, 1.0)) / 2.0;\n"
                "}\n",
                posIn.getName().c_str(),
                tOut.getName().c_str(),
                posIn.getName().c_str());

    fs.append(verStr);
    fs.append(GrGetGLSLShaderPrecisionDecl(binding));
    tIn.appendDecl(ctxInfo, &fs);

    if (declColor) {
        colorOut.appendDecl(ctxInfo, &fs);
    }
    fs.append( "void main() {\n");
    fs.appendf("    float intVal = floor(%s.x * 256.0) ;\n", tIn.getName().c_str());
    fs.append( "    float justAboveExpectedLowCutoff = (intVal - 0.49) / 255.0;\n");
    fs.append( "    float justBelowExpectedHighCutoff = (intVal + 0.49) / 255.0;\n");
    fs.append( "    float middleOfExpectedRange = intVal / 255.0;\n");
    fs.appendf("    %s = vec4(justAboveExpectedLowCutoff,justBelowExpectedHighCutoff,middleOfExpectedRange,0.0);\n", colorOut.getName().c_str());
    fs.append( "}\n");

    GrGLint vsCompiled = GR_GL_INIT_ZERO;
    const GrGLchar* vsStr = vs.c_str();
    const GrGLint vsLen = vs.size();
    GrAssert(0 != vsId);
    GR_GL_CALL(gl, ShaderSource(vsId, 1, &vsStr, &vsLen));
    GR_GL_CALL(gl, CompileShader(vsId));
    GR_GL_CALL(gl, GetShaderiv(vsId, GR_GL_COMPILE_STATUS, &vsCompiled));
    if (!vsCompiled) {
        goto FINISHED;
    }

    GrGLint fsCompiled = GR_GL_INIT_ZERO;
    const GrGLchar* fsStr = fs.c_str();
    const GrGLint fsLen = fs.size();
    GrAssert(0 != fsId);
    GR_GL_CALL(gl, ShaderSource(fsId, 1, &fsStr, &fsLen));
    GR_GL_CALL(gl, CompileShader(fsId));
    GR_GL_CALL(gl, GetShaderiv(fsId, GR_GL_COMPILE_STATUS, &fsCompiled));
    if (!fsCompiled) {
        goto FINISHED;
    }

    GrGLint progLinked = GR_GL_INIT_ZERO;
    GrAssert(0 != progId);
    GR_GL_CALL(gl, AttachShader(progId, vsId));
    GR_GL_CALL(gl, AttachShader(progId, fsId));
    if (declColor) {
        GR_GL_CALL(gl, BindFragDataLocation(progId, 0,
                                            colorOut.getName().c_str()));
    }
    GR_GL_CALL(gl, BindAttribLocation(progId, 0, posIn.getName().c_str()));
    GR_GL_CALL(gl, LinkProgram(progId));
    GR_GL_CALL(gl, GetProgramiv(progId, GR_GL_LINK_STATUS, &progLinked));
    if (!progLinked) {
        goto FINISHED;
    }
    GR_GL_CALL(gl, UseProgram(progId));

    GR_GL_CALL(gl, DrawArrays(GR_GL_TRIANGLES, 0, 3));
    uint32_t readData[256];
    // set to junk values just in case readPixels silently fails (as it is
    // known to do on some Intel GMA drivers).
    memset(readData, 0xab, sizeof(readData));
    GR_GL_CALL(gl, PixelStorei(GR_GL_PACK_ALIGNMENT, 4));
    GR_GL_CALL(gl, ReadPixels(0, 0, 256, 1, GR_GL_RGBA,
                                GR_GL_UNSIGNED_BYTE, readData));

    bool foundHighBias = false;
    bool foundLowBias = false;
    for (int i = 0; i < 256 && (!foundHighBias || !foundLowBias); ++i) {
        int nearLowCutoffVal = readData[i] & 0x000000ff;
        int nearHighCutoffVal = (readData[i] & 0x0000ff00) >> 8;
        int middle = (readData[i] & 0x00ff0000) >> 16;

        if (middle != i) {
            goto FINISHED;
        }

        int diff0 = i - nearLowCutoffVal;
        int diff1 = nearHighCutoffVal - i;
        if (diff0 < 0 || diff0 > 1) {
            goto FINISHED;
        }
        if (diff1 < 0 || diff1 > 1) {
            goto FINISHED;
        }

        if (1 == diff0) {
            foundHighBias = true;
        }

        if (1 == diff1) {
            foundLowBias = true;
        }
    }
    fail = (foundHighBias && foundLowBias);

FINISHED:
    if (dstFbo) {
        GR_GL_CALL(gl, DeleteFramebuffers(1, &dstFbo));
    }
    if (dstRb) {
        GR_GL_CALL(gl, DeleteRenderbuffers(1, &dstRb));
    }
    if (vertBuf) {
        GR_GL_CALL(gl, DeleteBuffers(1, &vertBuf));
    }
    if (vsId) {
        GR_GL_CALL(gl, DeleteShader(vsId));
    }
    if (fsId) {
        GR_GL_CALL(gl, DeleteShader(fsId));
    }
    if (progId) {
        GR_GL_CALL(gl, DeleteProgram(progId));
    }

    if (fail) {
        GrPrintf("Could not categorize how float-to-byte conversion is "
                 "performed from frag shader output.\n");
        return kUnknown_GrGLByteColorConversion;
    }

    if (foundHighBias) {
        return kHigh_GrGLByteColorConversion;
    } else if (foundLowBias) {
        return kLow_GrGLByteColorConversion;
    } else {
        return kRound_GrGLByteColorConversion;
    }
}

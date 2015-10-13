//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsGL.cpp: Implements the FuntionsGL class to contain loaded GL functions

#include "libANGLE/renderer/gl/FunctionsGL.h"

#include <algorithm>

#include "common/string_utils.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

namespace rx
{

static void GetGLVersion(PFNGLGETSTRINGPROC getStringFunction, gl::Version *outVersion, StandardGL *outStandard)
{
    const std::string version = reinterpret_cast<const char*>(getStringFunction(GL_VERSION));
    if (version.find("OpenGL ES") == std::string::npos)
    {
        // OpenGL spec states the GL_VERSION string will be in the following format:
        // <version number><space><vendor-specific information>
        // The version number is either of the form major number.minor number or major
        // number.minor number.release number, where the numbers all have one or more
        // digits
        *outStandard = STANDARD_GL_DESKTOP;
        *outVersion = gl::Version(version[0] - '0', version[2] - '0');
    }
    else
    {
        // ES spec states that the GL_VERSION string will be in the following format:
        // "OpenGL ES N.M vendor-specific information"
        *outStandard = STANDARD_GL_ES;
        *outVersion = gl::Version(version[10] - '0', version[12] - '0');
    }
}

static std::vector<std::string> GetIndexedExtensions(PFNGLGETINTEGERVPROC getIntegerFunction, PFNGLGETSTRINGIPROC getStringIFunction)
{
    std::vector<std::string> result;

    GLint numExtensions;
    getIntegerFunction(GL_NUM_EXTENSIONS, &numExtensions);

    result.reserve(numExtensions);

    for (GLint i = 0; i < numExtensions; i++)
    {
        result.push_back(reinterpret_cast<const char*>(getStringIFunction(GL_EXTENSIONS, i)));
    }

    return result;
}

template <typename T>
static void AssignGLEntryPoint(void *function, T *outFunction)
{
    *outFunction = reinterpret_cast<T>(function);
}

template <typename T>
static void AssignGLExtensionEntryPoint(const std::vector<std::string> &extensions, const char *requiredExtensionString,
                                        void *function, T *outFunction)
{
    std::vector<std::string> requiredExtensions;
    angle::SplitStringAlongWhitespace(requiredExtensionString, &requiredExtensions);
    for (const std::string& requiredExtension : requiredExtensions)
    {
        if (std::find(extensions.begin(), extensions.end(), requiredExtension) == extensions.end())
        {
            return;
        }
    }

    *outFunction = reinterpret_cast<T>(function);
}

FunctionsGL::FunctionsGL()
    : version(),
      standard(),
      extensions(),

      blendFunc(nullptr),
      clear(nullptr),
      clearColor(nullptr),
      clearDepth(nullptr),
      clearStencil(nullptr),
      colorMask(nullptr),
      cullFace(nullptr),
      depthFunc(nullptr),
      depthMask(nullptr),
      depthRange(nullptr),
      disable(nullptr),
      drawBuffer(nullptr),
      enable(nullptr),
      finish(nullptr),
      flush(nullptr),
      frontFace(nullptr),
      getBooleanv(nullptr),
      getDoublev(nullptr),
      getError(nullptr),
      getFloatv(nullptr),
      getIntegerv(nullptr),
      getString(nullptr),
      getTexImage(nullptr),
      getTexLevelParameterfv(nullptr),
      getTexLevelParameteriv(nullptr),
      getTexParameterfv(nullptr),
      getTexParameteriv(nullptr),
      hint(nullptr),
      isEnabled(nullptr),
      lineWidth(nullptr),
      logicOp(nullptr),
      pixelStoref(nullptr),
      pixelStorei(nullptr),
      pointSize(nullptr),
      polygonMode(nullptr),
      readBuffer(nullptr),
      readPixels(nullptr),
      scissor(nullptr),
      stencilFunc(nullptr),
      stencilMask(nullptr),
      stencilOp(nullptr),
      texImage1D(nullptr),
      texImage2D(nullptr),
      texParameterf(nullptr),
      texParameterfv(nullptr),
      texParameteri(nullptr),
      texParameteriv(nullptr),
      viewport(nullptr),

      bindTexture(nullptr),
      copyTexImage1D(nullptr),
      copyTexImage2D(nullptr),
      copyTexSubImage1D(nullptr),
      copyTexSubImage2D(nullptr),
      deleteTextures(nullptr),
      drawArrays(nullptr),
      drawElements(nullptr),
      genTextures(nullptr),
      isTexture(nullptr),
      polygonOffset(nullptr),
      texSubImage1D(nullptr),
      texSubImage2D(nullptr),

      blendColor(nullptr),
      blendEquation(nullptr),
      copyTexSubImage3D(nullptr),
      drawRangeElements(nullptr),
      texImage3D(nullptr),
      texSubImage3D(nullptr),

      deleteFencesNV(nullptr),
      genFencesNV(nullptr),
      isFenceNV(nullptr),
      testFenceNV(nullptr),
      getFenceivNV(nullptr),
      finishFenceNV(nullptr),
      setFenceNV(nullptr),

      activeTexture(nullptr),
      compressedTexImage1D(nullptr),
      compressedTexImage2D(nullptr),
      compressedTexImage3D(nullptr),
      compressedTexSubImage1D(nullptr),
      compressedTexSubImage2D(nullptr),
      compressedTexSubImage3D(nullptr),
      getCompressedTexImage(nullptr),
      sampleCoverage(nullptr),

      blendFuncSeparate(nullptr),
      multiDrawArrays(nullptr),
      multiDrawElements(nullptr),
      pointParameterf(nullptr),
      pointParameterfv(nullptr),
      pointParameteri(nullptr),
      pointParameteriv(nullptr),

      beginQuery(nullptr),
      bindBuffer(nullptr),
      bufferData(nullptr),
      bufferSubData(nullptr),
      deleteBuffers(nullptr),
      deleteQueries(nullptr),
      endQuery(nullptr),
      genBuffers(nullptr),
      genQueries(nullptr),
      getBufferParameteriv(nullptr),
      getBufferPointerv(nullptr),
      getBufferSubData(nullptr),
      getQueryObjectiv(nullptr),
      getQueryObjectuiv(nullptr),
      getQueryiv(nullptr),
      isBuffer(nullptr),
      isQuery(nullptr),
      mapBuffer(nullptr),
      unmapBuffer(nullptr),

      attachShader(nullptr),
      bindAttribLocation(nullptr),
      blendEquationSeparate(nullptr),
      compileShader(nullptr),
      createProgram(nullptr),
      createShader(nullptr),
      deleteProgram(nullptr),
      deleteShader(nullptr),
      detachShader(nullptr),
      disableVertexAttribArray(nullptr),
      drawBuffers(nullptr),
      enableVertexAttribArray(nullptr),
      getActiveAttrib(nullptr),
      getActiveUniform(nullptr),
      getAttachedShaders(nullptr),
      getAttribLocation(nullptr),
      getProgramInfoLog(nullptr),
      getProgramiv(nullptr),
      getShaderInfoLog(nullptr),
      getShaderSource(nullptr),
      getShaderiv(nullptr),
      getUniformLocation(nullptr),
      getUniformfv(nullptr),
      getUniformiv(nullptr),
      getVertexAttribPointerv(nullptr),
      getVertexAttribdv(nullptr),
      getVertexAttribfv(nullptr),
      getVertexAttribiv(nullptr),
      isProgram(nullptr),
      isShader(nullptr),
      linkProgram(nullptr),
      shaderSource(nullptr),
      stencilFuncSeparate(nullptr),
      stencilMaskSeparate(nullptr),
      stencilOpSeparate(nullptr),
      uniform1f(nullptr),
      uniform1fv(nullptr),
      uniform1i(nullptr),
      uniform1iv(nullptr),
      uniform2f(nullptr),
      uniform2fv(nullptr),
      uniform2i(nullptr),
      uniform2iv(nullptr),
      uniform3f(nullptr),
      uniform3fv(nullptr),
      uniform3i(nullptr),
      uniform3iv(nullptr),
      uniform4f(nullptr),
      uniform4fv(nullptr),
      uniform4i(nullptr),
      uniform4iv(nullptr),
      uniformMatrix2fv(nullptr),
      uniformMatrix3fv(nullptr),
      uniformMatrix4fv(nullptr),
      useProgram(nullptr),
      validateProgram(nullptr),
      vertexAttrib1d(nullptr),
      vertexAttrib1dv(nullptr),
      vertexAttrib1f(nullptr),
      vertexAttrib1fv(nullptr),
      vertexAttrib1s(nullptr),
      vertexAttrib1sv(nullptr),
      vertexAttrib2d(nullptr),
      vertexAttrib2dv(nullptr),
      vertexAttrib2f(nullptr),
      vertexAttrib2fv(nullptr),
      vertexAttrib2s(nullptr),
      vertexAttrib2sv(nullptr),
      vertexAttrib3d(nullptr),
      vertexAttrib3dv(nullptr),
      vertexAttrib3f(nullptr),
      vertexAttrib3fv(nullptr),
      vertexAttrib3s(nullptr),
      vertexAttrib3sv(nullptr),
      vertexAttrib4Nbv(nullptr),
      vertexAttrib4Niv(nullptr),
      vertexAttrib4Nsv(nullptr),
      vertexAttrib4Nub(nullptr),
      vertexAttrib4Nubv(nullptr),
      vertexAttrib4Nuiv(nullptr),
      vertexAttrib4Nusv(nullptr),
      vertexAttrib4bv(nullptr),
      vertexAttrib4d(nullptr),
      vertexAttrib4dv(nullptr),
      vertexAttrib4f(nullptr),
      vertexAttrib4fv(nullptr),
      vertexAttrib4iv(nullptr),
      vertexAttrib4s(nullptr),
      vertexAttrib4sv(nullptr),
      vertexAttrib4ubv(nullptr),
      vertexAttrib4uiv(nullptr),
      vertexAttrib4usv(nullptr),
      vertexAttribPointer(nullptr),

      uniformMatrix2x3fv(nullptr),
      uniformMatrix2x4fv(nullptr),
      uniformMatrix3x2fv(nullptr),
      uniformMatrix3x4fv(nullptr),
      uniformMatrix4x2fv(nullptr),
      uniformMatrix4x3fv(nullptr),

      beginConditionalRender(nullptr),
      beginTransformFeedback(nullptr),
      bindBufferBase(nullptr),
      bindBufferRange(nullptr),
      bindFragDataLocation(nullptr),
      bindFramebuffer(nullptr),
      bindRenderbuffer(nullptr),
      bindVertexArray(nullptr),
      blitFramebuffer(nullptr),
      checkFramebufferStatus(nullptr),
      clampColor(nullptr),
      clearBufferfi(nullptr),
      clearBufferfv(nullptr),
      clearBufferiv(nullptr),
      clearBufferuiv(nullptr),
      colorMaski(nullptr),
      deleteFramebuffers(nullptr),
      deleteRenderbuffers(nullptr),
      deleteVertexArrays(nullptr),
      disablei(nullptr),
      enablei(nullptr),
      endConditionalRender(nullptr),
      endTransformFeedback(nullptr),
      flushMappedBufferRange(nullptr),
      framebufferRenderbuffer(nullptr),
      framebufferTexture1D(nullptr),
      framebufferTexture2D(nullptr),
      framebufferTexture3D(nullptr),
      framebufferTextureLayer(nullptr),
      genFramebuffers(nullptr),
      genRenderbuffers(nullptr),
      genVertexArrays(nullptr),
      generateMipmap(nullptr),
      getBooleani_v(nullptr),
      getFragDataLocation(nullptr),
      getFramebufferAttachmentParameteriv(nullptr),
      getIntegeri_v(nullptr),
      getRenderbufferParameteriv(nullptr),
      getStringi(nullptr),
      getTexParameterIiv(nullptr),
      getTexParameterIuiv(nullptr),
      getTransformFeedbackVarying(nullptr),
      getUniformuiv(nullptr),
      getVertexAttribIiv(nullptr),
      getVertexAttribIuiv(nullptr),
      isEnabledi(nullptr),
      isFramebuffer(nullptr),
      isRenderbuffer(nullptr),
      isVertexArray(nullptr),
      mapBufferRange(nullptr),
      renderbufferStorage(nullptr),
      renderbufferStorageMultisample(nullptr),
      texParameterIiv(nullptr),
      texParameterIuiv(nullptr),
      transformFeedbackVaryings(nullptr),
      uniform1ui(nullptr),
      uniform1uiv(nullptr),
      uniform2ui(nullptr),
      uniform2uiv(nullptr),
      uniform3ui(nullptr),
      uniform3uiv(nullptr),
      uniform4ui(nullptr),
      uniform4uiv(nullptr),
      vertexAttribI1i(nullptr),
      vertexAttribI1iv(nullptr),
      vertexAttribI1ui(nullptr),
      vertexAttribI1uiv(nullptr),
      vertexAttribI2i(nullptr),
      vertexAttribI2iv(nullptr),
      vertexAttribI2ui(nullptr),
      vertexAttribI2uiv(nullptr),
      vertexAttribI3i(nullptr),
      vertexAttribI3iv(nullptr),
      vertexAttribI3ui(nullptr),
      vertexAttribI3uiv(nullptr),
      vertexAttribI4bv(nullptr),
      vertexAttribI4i(nullptr),
      vertexAttribI4iv(nullptr),
      vertexAttribI4sv(nullptr),
      vertexAttribI4ubv(nullptr),
      vertexAttribI4ui(nullptr),
      vertexAttribI4uiv(nullptr),
      vertexAttribI4usv(nullptr),
      vertexAttribIPointer(nullptr),

      copyBufferSubData(nullptr),
      drawArraysInstanced(nullptr),
      drawElementsInstanced(nullptr),
      getActiveUniformBlockName(nullptr),
      getActiveUniformBlockiv(nullptr),
      getActiveUniformName(nullptr),
      getActiveUniformsiv(nullptr),
      getUniformBlockIndex(nullptr),
      getUniformIndices(nullptr),
      primitiveRestartIndex(nullptr),
      texBuffer(nullptr),
      uniformBlockBinding(nullptr),

      clientWaitSync(nullptr),
      deleteSync(nullptr),
      drawElementsBaseVertex(nullptr),
      drawElementsInstancedBaseVertex(nullptr),
      drawRangeElementsBaseVertex(nullptr),
      fenceSync(nullptr),
      framebufferTexture(nullptr),
      getBufferParameteri64v(nullptr),
      getInteger64i_v(nullptr),
      getInteger64v(nullptr),
      getMultisamplefv(nullptr),
      getSynciv(nullptr),
      isSync(nullptr),
      multiDrawElementsBaseVertex(nullptr),
      provokingVertex(nullptr),
      sampleMaski(nullptr),
      texImage2DMultisample(nullptr),
      texImage3DMultisample(nullptr),
      waitSync(nullptr),

      bindFragDataLocationIndexed(nullptr),
      bindSampler(nullptr),
      deleteSamplers(nullptr),
      genSamplers(nullptr),
      getFragDataIndex(nullptr),
      getQueryObjecti64v(nullptr),
      getQueryObjectui64v(nullptr),
      getSamplerParameterIiv(nullptr),
      getSamplerParameterIuiv(nullptr),
      getSamplerParameterfv(nullptr),
      getSamplerParameteriv(nullptr),
      isSampler(nullptr),
      queryCounter(nullptr),
      samplerParameterIiv(nullptr),
      samplerParameterIuiv(nullptr),
      samplerParameterf(nullptr),
      samplerParameterfv(nullptr),
      samplerParameteri(nullptr),
      samplerParameteriv(nullptr),
      vertexAttribDivisor(nullptr),
      vertexAttribP1ui(nullptr),
      vertexAttribP1uiv(nullptr),
      vertexAttribP2ui(nullptr),
      vertexAttribP2uiv(nullptr),
      vertexAttribP3ui(nullptr),
      vertexAttribP3uiv(nullptr),
      vertexAttribP4ui(nullptr),
      vertexAttribP4uiv(nullptr),

      beginQueryIndexed(nullptr),
      bindTransformFeedback(nullptr),
      blendEquationSeparatei(nullptr),
      blendEquationi(nullptr),
      blendFuncSeparatei(nullptr),
      blendFunci(nullptr),
      deleteTransformFeedbacks(nullptr),
      drawArraysIndirect(nullptr),
      drawElementsIndirect(nullptr),
      drawTransformFeedback(nullptr),
      drawTransformFeedbackStream(nullptr),
      endQueryIndexed(nullptr),
      genTransformFeedbacks(nullptr),
      getActiveSubroutineName(nullptr),
      getActiveSubroutineUniformName(nullptr),
      getActiveSubroutineUniformiv(nullptr),
      getProgramStageiv(nullptr),
      getQueryIndexediv(nullptr),
      getSubroutineIndex(nullptr),
      getSubroutineUniformLocation(nullptr),
      getUniformSubroutineuiv(nullptr),
      getUniformdv(nullptr),
      isTransformFeedback(nullptr),
      minSampleShading(nullptr),
      patchParameterfv(nullptr),
      patchParameteri(nullptr),
      pauseTransformFeedback(nullptr),
      resumeTransformFeedback(nullptr),
      uniform1d(nullptr),
      uniform1dv(nullptr),
      uniform2d(nullptr),
      uniform2dv(nullptr),
      uniform3d(nullptr),
      uniform3dv(nullptr),
      uniform4d(nullptr),
      uniform4dv(nullptr),
      uniformMatrix2dv(nullptr),
      uniformMatrix2x3dv(nullptr),
      uniformMatrix2x4dv(nullptr),
      uniformMatrix3dv(nullptr),
      uniformMatrix3x2dv(nullptr),
      uniformMatrix3x4dv(nullptr),
      uniformMatrix4dv(nullptr),
      uniformMatrix4x2dv(nullptr),
      uniformMatrix4x3dv(nullptr),
      uniformSubroutinesuiv(nullptr),

      activeShaderProgram(nullptr),
      bindProgramPipeline(nullptr),
      clearDepthf(nullptr),
      createShaderProgramv(nullptr),
      deleteProgramPipelines(nullptr),
      depthRangeArrayv(nullptr),
      depthRangeIndexed(nullptr),
      depthRangef(nullptr),
      genProgramPipelines(nullptr),
      getDoublei_v(nullptr),
      getFloati_v(nullptr),
      getProgramBinary(nullptr),
      getProgramPipelineInfoLog(nullptr),
      getProgramPipelineiv(nullptr),
      getShaderPrecisionFormat(nullptr),
      getVertexAttribLdv(nullptr),
      isProgramPipeline(nullptr),
      programBinary(nullptr),
      programParameteri(nullptr),
      programUniform1d(nullptr),
      programUniform1dv(nullptr),
      programUniform1f(nullptr),
      programUniform1fv(nullptr),
      programUniform1i(nullptr),
      programUniform1iv(nullptr),
      programUniform1ui(nullptr),
      programUniform1uiv(nullptr),
      programUniform2d(nullptr),
      programUniform2dv(nullptr),
      programUniform2f(nullptr),
      programUniform2fv(nullptr),
      programUniform2i(nullptr),
      programUniform2iv(nullptr),
      programUniform2ui(nullptr),
      programUniform2uiv(nullptr),
      programUniform3d(nullptr),
      programUniform3dv(nullptr),
      programUniform3f(nullptr),
      programUniform3fv(nullptr),
      programUniform3i(nullptr),
      programUniform3iv(nullptr),
      programUniform3ui(nullptr),
      programUniform3uiv(nullptr),
      programUniform4d(nullptr),
      programUniform4dv(nullptr),
      programUniform4f(nullptr),
      programUniform4fv(nullptr),
      programUniform4i(nullptr),
      programUniform4iv(nullptr),
      programUniform4ui(nullptr),
      programUniform4uiv(nullptr),
      programUniformMatrix2dv(nullptr),
      programUniformMatrix2fv(nullptr),
      programUniformMatrix2x3dv(nullptr),
      programUniformMatrix2x3fv(nullptr),
      programUniformMatrix2x4dv(nullptr),
      programUniformMatrix2x4fv(nullptr),
      programUniformMatrix3dv(nullptr),
      programUniformMatrix3fv(nullptr),
      programUniformMatrix3x2dv(nullptr),
      programUniformMatrix3x2fv(nullptr),
      programUniformMatrix3x4dv(nullptr),
      programUniformMatrix3x4fv(nullptr),
      programUniformMatrix4dv(nullptr),
      programUniformMatrix4fv(nullptr),
      programUniformMatrix4x2dv(nullptr),
      programUniformMatrix4x2fv(nullptr),
      programUniformMatrix4x3dv(nullptr),
      programUniformMatrix4x3fv(nullptr),
      releaseShaderCompiler(nullptr),
      scissorArrayv(nullptr),
      scissorIndexed(nullptr),
      scissorIndexedv(nullptr),
      shaderBinary(nullptr),
      useProgramStages(nullptr),
      validateProgramPipeline(nullptr),
      vertexAttribL1d(nullptr),
      vertexAttribL1dv(nullptr),
      vertexAttribL2d(nullptr),
      vertexAttribL2dv(nullptr),
      vertexAttribL3d(nullptr),
      vertexAttribL3dv(nullptr),
      vertexAttribL4d(nullptr),
      vertexAttribL4dv(nullptr),
      vertexAttribLPointer(nullptr),
      viewportArrayv(nullptr),
      viewportIndexedf(nullptr),
      viewportIndexedfv(nullptr),

      bindImageTexture(nullptr),
      drawArraysInstancedBaseInstance(nullptr),
      drawElementsInstancedBaseInstance(nullptr),
      drawElementsInstancedBaseVertexBaseInstance(nullptr),
      drawTransformFeedbackInstanced(nullptr),
      drawTransformFeedbackStreamInstanced(nullptr),
      getActiveAtomicCounterBufferiv(nullptr),
      getInternalformativ(nullptr),
      memoryBarrier(nullptr),
      texStorage1D(nullptr),
      texStorage2D(nullptr),
      texStorage3D(nullptr),

      bindVertexBuffer(nullptr),
      clearBufferData(nullptr),
      clearBufferSubData(nullptr),
      copyImageSubData(nullptr),
      debugMessageCallback(nullptr),
      debugMessageControl(nullptr),
      debugMessageInsert(nullptr),
      dispatchCompute(nullptr),
      dispatchComputeIndirect(nullptr),
      framebufferParameteri(nullptr),
      getDebugMessageLog(nullptr),
      getFramebufferParameteriv(nullptr),
      getInternalformati64v(nullptr),
      getPointerv(nullptr),
      getObjectLabel(nullptr),
      getObjectPtrLabel(nullptr),
      getProgramInterfaceiv(nullptr),
      getProgramResourceIndex(nullptr),
      getProgramResourceLocation(nullptr),
      getProgramResourceLocationIndex(nullptr),
      getProgramResourceName(nullptr),
      getProgramResourceiv(nullptr),
      invalidateBufferData(nullptr),
      invalidateBufferSubData(nullptr),
      invalidateFramebuffer(nullptr),
      invalidateSubFramebuffer(nullptr),
      invalidateTexImage(nullptr),
      invalidateTexSubImage(nullptr),
      multiDrawArraysIndirect(nullptr),
      multiDrawElementsIndirect(nullptr),
      objectLabel(nullptr),
      objectPtrLabel(nullptr),
      popDebugGroup(nullptr),
      pushDebugGroup(nullptr),
      shaderStorageBlockBinding(nullptr),
      texBufferRange(nullptr),
      texStorage2DMultisample(nullptr),
      texStorage3DMultisample(nullptr),
      textureView(nullptr),
      vertexAttribBinding(nullptr),
      vertexAttribFormat(nullptr),
      vertexAttribIFormat(nullptr),
      vertexAttribLFormat(nullptr),
      vertexBindingDivisor(nullptr),

      bindBuffersBase(nullptr),
      bindBuffersRange(nullptr),
      bindImageTextures(nullptr),
      bindSamplers(nullptr),
      bindTextures(nullptr),
      bindVertexBuffers(nullptr),
      bufferStorage(nullptr),
      clearTexImage(nullptr),
      clearTexSubImage(nullptr),

      bindTextureUnit(nullptr),
      blitNamedFramebuffer(nullptr),
      checkNamedFramebufferStatus(nullptr),
      clearNamedBufferData(nullptr),
      clearNamedBufferSubData(nullptr),
      clearNamedFramebufferfi(nullptr),
      clearNamedFramebufferfv(nullptr),
      clearNamedFramebufferiv(nullptr),
      clearNamedFramebufferuiv(nullptr),
      clipControl(nullptr),
      compressedTextureSubImage1D(nullptr),
      compressedTextureSubImage2D(nullptr),
      compressedTextureSubImage3D(nullptr),
      copyNamedBufferSubData(nullptr),
      copyTextureSubImage1D(nullptr),
      copyTextureSubImage2D(nullptr),
      copyTextureSubImage3D(nullptr),
      createBuffers(nullptr),
      createFramebuffers(nullptr),
      createProgramPipelines(nullptr),
      createQueries(nullptr),
      createRenderbuffers(nullptr),
      createSamplers(nullptr),
      createTextures(nullptr),
      createTransformFeedbacks(nullptr),
      createVertexArrays(nullptr),
      disableVertexArrayAttrib(nullptr),
      enableVertexArrayAttrib(nullptr),
      flushMappedNamedBufferRange(nullptr),
      generateTextureMipmap(nullptr),
      getCompressedTextureImage(nullptr),
      getCompressedTextureSubImage(nullptr),
      getGraphicsResetStatus(nullptr),
      getNamedBufferParameteri64v(nullptr),
      getNamedBufferParameteriv(nullptr),
      getNamedBufferPointerv(nullptr),
      getNamedBufferSubData(nullptr),
      getNamedFramebufferAttachmentParameteriv(nullptr),
      getNamedFramebufferParameteriv(nullptr),
      getNamedRenderbufferParameteriv(nullptr),
      getQueryBufferObjecti64v(nullptr),
      getQueryBufferObjectiv(nullptr),
      getQueryBufferObjectui64v(nullptr),
      getQueryBufferObjectuiv(nullptr),
      getTextureImage(nullptr),
      getTextureLevelParameterfv(nullptr),
      getTextureLevelParameteriv(nullptr),
      getTextureParameterIiv(nullptr),
      getTextureParameterIuiv(nullptr),
      getTextureParameterfv(nullptr),
      getTextureParameteriv(nullptr),
      getTextureSubImage(nullptr),
      getTransformFeedbacki64_v(nullptr),
      getTransformFeedbacki_v(nullptr),
      getTransformFeedbackiv(nullptr),
      getVertexArrayIndexed64iv(nullptr),
      getVertexArrayIndexediv(nullptr),
      getVertexArrayiv(nullptr),
      getnCompressedTexImage(nullptr),
      getnTexImage(nullptr),
      getnUniformdv(nullptr),
      getnUniformfv(nullptr),
      getnUniformiv(nullptr),
      getnUniformuiv(nullptr),
      invalidateNamedFramebufferData(nullptr),
      invalidateNamedFramebufferSubData(nullptr),
      mapNamedBuffer(nullptr),
      mapNamedBufferRange(nullptr),
      memoryBarrierByRegion(nullptr),
      namedBufferData(nullptr),
      namedBufferStorage(nullptr),
      namedBufferSubData(nullptr),
      namedFramebufferDrawBuffer(nullptr),
      namedFramebufferDrawBuffers(nullptr),
      namedFramebufferParameteri(nullptr),
      namedFramebufferReadBuffer(nullptr),
      namedFramebufferRenderbuffer(nullptr),
      namedFramebufferTexture(nullptr),
      namedFramebufferTextureLayer(nullptr),
      namedRenderbufferStorage(nullptr),
      namedRenderbufferStorageMultisample(nullptr),
      readnPixels(nullptr),
      textureBarrier(nullptr),
      textureBuffer(nullptr),
      textureBufferRange(nullptr),
      textureParameterIiv(nullptr),
      textureParameterIuiv(nullptr),
      textureParameterf(nullptr),
      textureParameterfv(nullptr),
      textureParameteri(nullptr),
      textureParameteriv(nullptr),
      textureStorage1D(nullptr),
      textureStorage2D(nullptr),
      textureStorage2DMultisample(nullptr),
      textureStorage3D(nullptr),
      textureStorage3DMultisample(nullptr),
      textureSubImage1D(nullptr),
      textureSubImage2D(nullptr),
      textureSubImage3D(nullptr),
      transformFeedbackBufferBase(nullptr),
      transformFeedbackBufferRange(nullptr),
      unmapNamedBuffer(nullptr),
      vertexArrayAttribBinding(nullptr),
      vertexArrayAttribFormat(nullptr),
      vertexArrayAttribIFormat(nullptr),
      vertexArrayAttribLFormat(nullptr),
      vertexArrayBindingDivisor(nullptr),
      vertexArrayElementBuffer(nullptr),
      vertexArrayVertexBuffer(nullptr),
      vertexArrayVertexBuffers(nullptr)
{
}

FunctionsGL::~FunctionsGL()
{
}

void FunctionsGL::initialize()
{
    // Grab the version number
    AssignGLEntryPoint(loadProcAddress("glGetString"), &getString);
    AssignGLEntryPoint(loadProcAddress("glGetIntegerv"), &getIntegerv);
    GetGLVersion(getString, &version, &standard);

    // Grab the GL extensions
    if (isAtLeastGL(gl::Version(3, 0)))
    {
        AssignGLEntryPoint(loadProcAddress("glGetStringi"), &getStringi);
        extensions = GetIndexedExtensions(getIntegerv, getStringi);
    }
    else
    {
        const char *exts = reinterpret_cast<const char*>(getString(GL_EXTENSIONS));
        angle::SplitStringAlongWhitespace(std::string(exts), &extensions);
    }

    // Check the context profile
    if (isAtLeastGL(gl::Version(3, 2)))
    {
        getIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    }
    else
    {
        profile = 0;
    }

    // clang-format off

    // Load extensions
    // Even though extensions are written against specific versions of GL, many drivers expose the extensions
    // in even older versions.  Always try loading the extensions regardless of GL version.

    // GL_NV_fence
    AssignGLExtensionEntryPoint(extensions, "GL_NV_fence", loadProcAddress("glDeleteFencesNV"), &deleteFencesNV);
    AssignGLExtensionEntryPoint(extensions, "GL_NV_fence", loadProcAddress("glGenFencesNV"), &genFencesNV);
    AssignGLExtensionEntryPoint(extensions, "GL_NV_fence", loadProcAddress("glIsFenceNV"), &isFenceNV);
    AssignGLExtensionEntryPoint(extensions, "GL_NV_fence", loadProcAddress("glTestFenceNV"), &testFenceNV);
    AssignGLExtensionEntryPoint(extensions, "GL_NV_fence", loadProcAddress("glGetFenceivNV"), &getFenceivNV);
    AssignGLExtensionEntryPoint(extensions, "GL_NV_fence", loadProcAddress("glFinishFenceNV"), &finishFenceNV);
    AssignGLExtensionEntryPoint(extensions, "GL_NV_fence", loadProcAddress("glSetFenceNV"), &setFenceNV);

    // GL_EXT_texture_storage
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_texture_storage", loadProcAddress("glTexStorage1DEXT"), &texStorage1D);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_texture_storage", loadProcAddress("glTexStorage2DEXT"), &texStorage2D);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_texture_storage GL_EXT_texture3D", loadProcAddress("glTexStorage3DEXT"), &texStorage3D);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_texture_storage GL_EXT_direct_state_access", loadProcAddress("glTextureStorage1DEXT"), &textureStorage1D);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_texture_storage GL_EXT_direct_state_access", loadProcAddress("glTextureStorage2DEXT"), &textureStorage2D);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_texture_storage GL_EXT_direct_state_access GL_EXT_texture3D", loadProcAddress("glTextureStorage3DEXT"), &textureStorage3D);

    // GL_ARB_vertex_array_object
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_vertex_array_object", loadProcAddress("glBindVertexArray"), &bindVertexArray);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_vertex_array_object", loadProcAddress("glDeleteVertexArrays"), &deleteVertexArrays);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_vertex_array_object", loadProcAddress("glGenVertexArrays"), &genVertexArrays);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_vertex_array_object", loadProcAddress("glIsVertexArray"), &isVertexArray);

    // GL_ARB_sync
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_sync", loadProcAddress("glClientWaitSync"), &clientWaitSync);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_sync", loadProcAddress("glDeleteSync"), &deleteSync);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_sync", loadProcAddress("glFenceSync"), &fenceSync);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_sync", loadProcAddress("glGetInteger64i_v"), &getInteger64i_v);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_sync", loadProcAddress("glGetInteger64v"), &getInteger64v);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_sync", loadProcAddress("glGetSynciv"), &getSynciv);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_sync", loadProcAddress("glIsSync"), &isSync);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_sync", loadProcAddress("glWaitSync"), &waitSync);

    // GL_EXT_framebuffer_object
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glIsRenderbufferEXT"), &isRenderbuffer);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glBindRenderbufferEXT"), &bindRenderbuffer);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glDeleteRenderbuffersEXT"), &deleteRenderbuffers);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glGenRenderbuffersEXT"), &genRenderbuffers);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glRenderbufferStorageEXT"), &renderbufferStorage);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glGetRenderbufferParameterivEXT"), &getRenderbufferParameteriv);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glIsFramebufferEXT"), &isFramebuffer);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glBindFramebufferEXT"), &bindFramebuffer);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glDeleteFramebuffersEXT"), &deleteFramebuffers);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glGenFramebuffersEXT"), &genFramebuffers);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glCheckFramebufferStatusEXT"), &checkFramebufferStatus);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glFramebufferTexture1DEXT"), &framebufferTexture1D);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glFramebufferTexture2DEXT"), &framebufferTexture2D);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glFramebufferTexture3DEXT"), &framebufferTexture3D);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glFramebufferRenderbufferEXT"), &framebufferRenderbuffer);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glGetFramebufferAttachmentParameterivEXT"), &getFramebufferAttachmentParameteriv);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_object", loadProcAddress("glGenerateMipmapEXT"), &generateMipmap);

    // GL_EXT_framebuffer_blit
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_framebuffer_blit", loadProcAddress("glBlitFramebufferEXT"), &blitFramebuffer);

    // GL_KHR_debug
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glDebugMessageControl"), &debugMessageControl);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glDebugMessageInsert"), &debugMessageInsert);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glDebugMessageCallback"), &debugMessageCallback);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glGetDebugMessageLog"), &getDebugMessageLog);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glGetPointerv"), &getPointerv);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glPushDebugGroup"), &pushDebugGroup);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glPopDebugGroup"), &popDebugGroup);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glObjectLabel"), &objectLabel);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glGetObjectLabel"), &getObjectLabel);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glObjectPtrLabel"), &objectPtrLabel);
    AssignGLExtensionEntryPoint(extensions, "GL_KHR_debug", loadProcAddress("glGetObjectPtrLabel"), &getObjectPtrLabel);

    // GL_ARB_internalformat_query
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_internalformat_query", loadProcAddress("glGetInternalformativ"), &getInternalformativ);

    // GL_ARB_ES2_compatibility
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_ES2_compatibility", loadProcAddress("glReleaseShaderCompiler"), &releaseShaderCompiler);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_ES2_compatibility", loadProcAddress("glShaderBinary"), &shaderBinary);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_ES2_compatibility", loadProcAddress("glGetShaderPrecisionFormat"), &getShaderPrecisionFormat);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_ES2_compatibility", loadProcAddress("glDepthRangef"), &depthRangef);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_ES2_compatibility", loadProcAddress("glClearDepthf"), &clearDepthf);

    // GL_ARB_instanced_arrays
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_instanced_arrays", loadProcAddress("glVertexAttribDivisorARB"), &vertexAttribDivisor);

    // GL_EXT_draw_instanced
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_draw_instanced", loadProcAddress("glDrawArraysInstancedEXT"), &drawArraysInstanced);
    AssignGLExtensionEntryPoint(extensions, "GL_EXT_draw_instanced", loadProcAddress("glDrawElementsInstancedEXT"), &drawElementsInstanced);

    // GL_ARB_draw_instanced
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_draw_instanced", loadProcAddress("glDrawArraysInstancedARB"), &drawArraysInstanced);
    AssignGLExtensionEntryPoint(extensions, "GL_ARB_draw_instanced", loadProcAddress("glDrawElementsInstancedARB"), &drawElementsInstanced);

    // 1.0
    if (isAtLeastGL(gl::Version(1, 0)))
    {
        AssignGLEntryPoint(loadProcAddress("glBlendFunc"), &blendFunc);
        AssignGLEntryPoint(loadProcAddress("glClear"), &clear);
        AssignGLEntryPoint(loadProcAddress("glClearColor"), &clearColor);
        AssignGLEntryPoint(loadProcAddress("glClearDepth"), &clearDepth);
        AssignGLEntryPoint(loadProcAddress("glClearStencil"), &clearStencil);
        AssignGLEntryPoint(loadProcAddress("glColorMask"), &colorMask);
        AssignGLEntryPoint(loadProcAddress("glCullFace"), &cullFace);
        AssignGLEntryPoint(loadProcAddress("glDepthFunc"), &depthFunc);
        AssignGLEntryPoint(loadProcAddress("glDepthMask"), &depthMask);
        AssignGLEntryPoint(loadProcAddress("glDepthRange"), &depthRange);
        AssignGLEntryPoint(loadProcAddress("glDisable"), &disable);
        AssignGLEntryPoint(loadProcAddress("glDrawBuffer"), &drawBuffer);
        AssignGLEntryPoint(loadProcAddress("glEnable"), &enable);
        AssignGLEntryPoint(loadProcAddress("glFinish"), &finish);
        AssignGLEntryPoint(loadProcAddress("glFlush"), &flush);
        AssignGLEntryPoint(loadProcAddress("glFrontFace"), &frontFace);
        AssignGLEntryPoint(loadProcAddress("glGetBooleanv"), &getBooleanv);
        AssignGLEntryPoint(loadProcAddress("glGetDoublev"), &getDoublev);
        AssignGLEntryPoint(loadProcAddress("glGetError"), &getError);
        AssignGLEntryPoint(loadProcAddress("glGetFloatv"), &getFloatv);
        AssignGLEntryPoint(loadProcAddress("glGetIntegerv"), &getIntegerv);
        AssignGLEntryPoint(loadProcAddress("glGetString"), &getString);
        AssignGLEntryPoint(loadProcAddress("glGetTexImage"), &getTexImage);
        AssignGLEntryPoint(loadProcAddress("glGetTexLevelParameterfv"), &getTexLevelParameterfv);
        AssignGLEntryPoint(loadProcAddress("glGetTexLevelParameteriv"), &getTexLevelParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetTexParameterfv"), &getTexParameterfv);
        AssignGLEntryPoint(loadProcAddress("glGetTexParameteriv"), &getTexParameteriv);
        AssignGLEntryPoint(loadProcAddress("glHint"), &hint);
        AssignGLEntryPoint(loadProcAddress("glIsEnabled"), &isEnabled);
        AssignGLEntryPoint(loadProcAddress("glLineWidth"), &lineWidth);
        AssignGLEntryPoint(loadProcAddress("glLogicOp"), &logicOp);
        AssignGLEntryPoint(loadProcAddress("glPixelStoref"), &pixelStoref);
        AssignGLEntryPoint(loadProcAddress("glPixelStorei"), &pixelStorei);
        AssignGLEntryPoint(loadProcAddress("glPointSize"), &pointSize);
        AssignGLEntryPoint(loadProcAddress("glPolygonMode"), &polygonMode);
        AssignGLEntryPoint(loadProcAddress("glReadBuffer"), &readBuffer);
        AssignGLEntryPoint(loadProcAddress("glReadPixels"), &readPixels);
        AssignGLEntryPoint(loadProcAddress("glScissor"), &scissor);
        AssignGLEntryPoint(loadProcAddress("glStencilFunc"), &stencilFunc);
        AssignGLEntryPoint(loadProcAddress("glStencilMask"), &stencilMask);
        AssignGLEntryPoint(loadProcAddress("glStencilOp"), &stencilOp);
        AssignGLEntryPoint(loadProcAddress("glTexImage1D"), &texImage1D);
        AssignGLEntryPoint(loadProcAddress("glTexImage2D"), &texImage2D);
        AssignGLEntryPoint(loadProcAddress("glTexParameterf"), &texParameterf);
        AssignGLEntryPoint(loadProcAddress("glTexParameterfv"), &texParameterfv);
        AssignGLEntryPoint(loadProcAddress("glTexParameteri"), &texParameteri);
        AssignGLEntryPoint(loadProcAddress("glTexParameteriv"), &texParameteriv);
        AssignGLEntryPoint(loadProcAddress("glViewport"), &viewport);
    }

    // 1.1
    if (isAtLeastGL(gl::Version(1, 1)))
    {
        AssignGLEntryPoint(loadProcAddress("glBindTexture"), &bindTexture);
        AssignGLEntryPoint(loadProcAddress("glCopyTexImage1D"), &copyTexImage1D);
        AssignGLEntryPoint(loadProcAddress("glCopyTexImage2D"), &copyTexImage2D);
        AssignGLEntryPoint(loadProcAddress("glCopyTexSubImage1D"), &copyTexSubImage1D);
        AssignGLEntryPoint(loadProcAddress("glCopyTexSubImage2D"), &copyTexSubImage2D);
        AssignGLEntryPoint(loadProcAddress("glDeleteTextures"), &deleteTextures);
        AssignGLEntryPoint(loadProcAddress("glDrawArrays"), &drawArrays);
        AssignGLEntryPoint(loadProcAddress("glDrawElements"), &drawElements);
        AssignGLEntryPoint(loadProcAddress("glGenTextures"), &genTextures);
        AssignGLEntryPoint(loadProcAddress("glIsTexture"), &isTexture);
        AssignGLEntryPoint(loadProcAddress("glPolygonOffset"), &polygonOffset);
        AssignGLEntryPoint(loadProcAddress("glTexSubImage1D"), &texSubImage1D);
        AssignGLEntryPoint(loadProcAddress("glTexSubImage2D"), &texSubImage2D);
    }

    // 1.2
    if (isAtLeastGL(gl::Version(1, 2)))
    {
        AssignGLEntryPoint(loadProcAddress("glBlendColor"), &blendColor);
        AssignGLEntryPoint(loadProcAddress("glBlendEquation"), &blendEquation);
        AssignGLEntryPoint(loadProcAddress("glCopyTexSubImage3D"), &copyTexSubImage3D);
        AssignGLEntryPoint(loadProcAddress("glDrawRangeElements"), &drawRangeElements);
        AssignGLEntryPoint(loadProcAddress("glTexImage3D"), &texImage3D);
        AssignGLEntryPoint(loadProcAddress("glTexSubImage3D"), &texSubImage3D);
    }

    // 1.3
    if (isAtLeastGL(gl::Version(1, 3)))
    {
        AssignGLEntryPoint(loadProcAddress("glActiveTexture"), &activeTexture);
        AssignGLEntryPoint(loadProcAddress("glCompressedTexImage1D"), &compressedTexImage1D);
        AssignGLEntryPoint(loadProcAddress("glCompressedTexImage2D"), &compressedTexImage2D);
        AssignGLEntryPoint(loadProcAddress("glCompressedTexImage3D"), &compressedTexImage3D);
        AssignGLEntryPoint(loadProcAddress("glCompressedTexSubImage1D"), &compressedTexSubImage1D);
        AssignGLEntryPoint(loadProcAddress("glCompressedTexSubImage2D"), &compressedTexSubImage2D);
        AssignGLEntryPoint(loadProcAddress("glCompressedTexSubImage3D"), &compressedTexSubImage3D);
        AssignGLEntryPoint(loadProcAddress("glGetCompressedTexImage"), &getCompressedTexImage);
        AssignGLEntryPoint(loadProcAddress("glSampleCoverage"), &sampleCoverage);
    }

    // 1.4
    if (isAtLeastGL(gl::Version(1, 4)))
    {
        AssignGLEntryPoint(loadProcAddress("glBlendFuncSeparate"), &blendFuncSeparate);
        AssignGLEntryPoint(loadProcAddress("glMultiDrawArrays"), &multiDrawArrays);
        AssignGLEntryPoint(loadProcAddress("glMultiDrawElements"), &multiDrawElements);
        AssignGLEntryPoint(loadProcAddress("glPointParameterf"), &pointParameterf);
        AssignGLEntryPoint(loadProcAddress("glPointParameterfv"), &pointParameterfv);
        AssignGLEntryPoint(loadProcAddress("glPointParameteri"), &pointParameteri);
        AssignGLEntryPoint(loadProcAddress("glPointParameteriv"), &pointParameteriv);
    }

    // 1.5
    if (isAtLeastGL(gl::Version(1, 5)))
    {
        AssignGLEntryPoint(loadProcAddress("glBeginQuery"), &beginQuery);
        AssignGLEntryPoint(loadProcAddress("glBindBuffer"), &bindBuffer);
        AssignGLEntryPoint(loadProcAddress("glBufferData"), &bufferData);
        AssignGLEntryPoint(loadProcAddress("glBufferSubData"), &bufferSubData);
        AssignGLEntryPoint(loadProcAddress("glDeleteBuffers"), &deleteBuffers);
        AssignGLEntryPoint(loadProcAddress("glDeleteQueries"), &deleteQueries);
        AssignGLEntryPoint(loadProcAddress("glEndQuery"), &endQuery);
        AssignGLEntryPoint(loadProcAddress("glGenBuffers"), &genBuffers);
        AssignGLEntryPoint(loadProcAddress("glGenQueries"), &genQueries);
        AssignGLEntryPoint(loadProcAddress("glGetBufferParameteriv"), &getBufferParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetBufferPointerv"), &getBufferPointerv);
        AssignGLEntryPoint(loadProcAddress("glGetBufferSubData"), &getBufferSubData);
        AssignGLEntryPoint(loadProcAddress("glGetQueryObjectiv"), &getQueryObjectiv);
        AssignGLEntryPoint(loadProcAddress("glGetQueryObjectuiv"), &getQueryObjectuiv);
        AssignGLEntryPoint(loadProcAddress("glGetQueryiv"), &getQueryiv);
        AssignGLEntryPoint(loadProcAddress("glIsBuffer"), &isBuffer);
        AssignGLEntryPoint(loadProcAddress("glIsQuery"), &isQuery);
        AssignGLEntryPoint(loadProcAddress("glMapBuffer"), &mapBuffer);
        AssignGLEntryPoint(loadProcAddress("glUnmapBuffer"), &unmapBuffer);
    }

    // 2.0
    if (isAtLeastGL(gl::Version(2, 0)))
    {
        AssignGLEntryPoint(loadProcAddress("glAttachShader"), &attachShader);
        AssignGLEntryPoint(loadProcAddress("glBindAttribLocation"), &bindAttribLocation);
        AssignGLEntryPoint(loadProcAddress("glBlendEquationSeparate"), &blendEquationSeparate);
        AssignGLEntryPoint(loadProcAddress("glCompileShader"), &compileShader);
        AssignGLEntryPoint(loadProcAddress("glCreateProgram"), &createProgram);
        AssignGLEntryPoint(loadProcAddress("glCreateShader"), &createShader);
        AssignGLEntryPoint(loadProcAddress("glDeleteProgram"), &deleteProgram);
        AssignGLEntryPoint(loadProcAddress("glDeleteShader"), &deleteShader);
        AssignGLEntryPoint(loadProcAddress("glDetachShader"), &detachShader);
        AssignGLEntryPoint(loadProcAddress("glDisableVertexAttribArray"), &disableVertexAttribArray);
        AssignGLEntryPoint(loadProcAddress("glDrawBuffers"), &drawBuffers);
        AssignGLEntryPoint(loadProcAddress("glEnableVertexAttribArray"), &enableVertexAttribArray);
        AssignGLEntryPoint(loadProcAddress("glGetActiveAttrib"), &getActiveAttrib);
        AssignGLEntryPoint(loadProcAddress("glGetActiveUniform"), &getActiveUniform);
        AssignGLEntryPoint(loadProcAddress("glGetAttachedShaders"), &getAttachedShaders);
        AssignGLEntryPoint(loadProcAddress("glGetAttribLocation"), &getAttribLocation);
        AssignGLEntryPoint(loadProcAddress("glGetProgramInfoLog"), &getProgramInfoLog);
        AssignGLEntryPoint(loadProcAddress("glGetProgramiv"), &getProgramiv);
        AssignGLEntryPoint(loadProcAddress("glGetShaderInfoLog"), &getShaderInfoLog);
        AssignGLEntryPoint(loadProcAddress("glGetShaderSource"), &getShaderSource);
        AssignGLEntryPoint(loadProcAddress("glGetShaderiv"), &getShaderiv);
        AssignGLEntryPoint(loadProcAddress("glGetUniformLocation"), &getUniformLocation);
        AssignGLEntryPoint(loadProcAddress("glGetUniformfv"), &getUniformfv);
        AssignGLEntryPoint(loadProcAddress("glGetUniformiv"), &getUniformiv);
        AssignGLEntryPoint(loadProcAddress("glGetVertexAttribPointerv"), &getVertexAttribPointerv);
        AssignGLEntryPoint(loadProcAddress("glGetVertexAttribdv"), &getVertexAttribdv);
        AssignGLEntryPoint(loadProcAddress("glGetVertexAttribfv"), &getVertexAttribfv);
        AssignGLEntryPoint(loadProcAddress("glGetVertexAttribiv"), &getVertexAttribiv);
        AssignGLEntryPoint(loadProcAddress("glIsProgram"), &isProgram);
        AssignGLEntryPoint(loadProcAddress("glIsShader"), &isShader);
        AssignGLEntryPoint(loadProcAddress("glLinkProgram"), &linkProgram);
        AssignGLEntryPoint(loadProcAddress("glShaderSource"), &shaderSource);
        AssignGLEntryPoint(loadProcAddress("glStencilFuncSeparate"), &stencilFuncSeparate);
        AssignGLEntryPoint(loadProcAddress("glStencilMaskSeparate"), &stencilMaskSeparate);
        AssignGLEntryPoint(loadProcAddress("glStencilOpSeparate"), &stencilOpSeparate);
        AssignGLEntryPoint(loadProcAddress("glUniform1f"), &uniform1f);
        AssignGLEntryPoint(loadProcAddress("glUniform1fv"), &uniform1fv);
        AssignGLEntryPoint(loadProcAddress("glUniform1i"), &uniform1i);
        AssignGLEntryPoint(loadProcAddress("glUniform1iv"), &uniform1iv);
        AssignGLEntryPoint(loadProcAddress("glUniform2f"), &uniform2f);
        AssignGLEntryPoint(loadProcAddress("glUniform2fv"), &uniform2fv);
        AssignGLEntryPoint(loadProcAddress("glUniform2i"), &uniform2i);
        AssignGLEntryPoint(loadProcAddress("glUniform2iv"), &uniform2iv);
        AssignGLEntryPoint(loadProcAddress("glUniform3f"), &uniform3f);
        AssignGLEntryPoint(loadProcAddress("glUniform3fv"), &uniform3fv);
        AssignGLEntryPoint(loadProcAddress("glUniform3i"), &uniform3i);
        AssignGLEntryPoint(loadProcAddress("glUniform3iv"), &uniform3iv);
        AssignGLEntryPoint(loadProcAddress("glUniform4f"), &uniform4f);
        AssignGLEntryPoint(loadProcAddress("glUniform4fv"), &uniform4fv);
        AssignGLEntryPoint(loadProcAddress("glUniform4i"), &uniform4i);
        AssignGLEntryPoint(loadProcAddress("glUniform4iv"), &uniform4iv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix2fv"), &uniformMatrix2fv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix3fv"), &uniformMatrix3fv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix4fv"), &uniformMatrix4fv);
        AssignGLEntryPoint(loadProcAddress("glUseProgram"), &useProgram);
        AssignGLEntryPoint(loadProcAddress("glValidateProgram"), &validateProgram);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib1d"), &vertexAttrib1d);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib1dv"), &vertexAttrib1dv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib1f"), &vertexAttrib1f);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib1fv"), &vertexAttrib1fv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib1s"), &vertexAttrib1s);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib1sv"), &vertexAttrib1sv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib2d"), &vertexAttrib2d);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib2dv"), &vertexAttrib2dv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib2f"), &vertexAttrib2f);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib2fv"), &vertexAttrib2fv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib2s"), &vertexAttrib2s);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib2sv"), &vertexAttrib2sv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib3d"), &vertexAttrib3d);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib3dv"), &vertexAttrib3dv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib3f"), &vertexAttrib3f);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib3fv"), &vertexAttrib3fv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib3s"), &vertexAttrib3s);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib3sv"), &vertexAttrib3sv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4Nbv"), &vertexAttrib4Nbv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4Niv"), &vertexAttrib4Niv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4Nsv"), &vertexAttrib4Nsv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4Nub"), &vertexAttrib4Nub);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4Nubv"), &vertexAttrib4Nubv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4Nuiv"), &vertexAttrib4Nuiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4Nusv"), &vertexAttrib4Nusv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4bv"), &vertexAttrib4bv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4d"), &vertexAttrib4d);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4dv"), &vertexAttrib4dv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4f"), &vertexAttrib4f);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4fv"), &vertexAttrib4fv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4iv"), &vertexAttrib4iv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4s"), &vertexAttrib4s);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4sv"), &vertexAttrib4sv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4ubv"), &vertexAttrib4ubv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4uiv"), &vertexAttrib4uiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttrib4usv"), &vertexAttrib4usv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribPointer"), &vertexAttribPointer);
    }

    // 2.1
    if (isAtLeastGL(gl::Version(2, 1)))
    {
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix2x3fv"), &uniformMatrix2x3fv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix2x4fv"), &uniformMatrix2x4fv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix3x2fv"), &uniformMatrix3x2fv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix3x4fv"), &uniformMatrix3x4fv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix4x2fv"), &uniformMatrix4x2fv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix4x3fv"), &uniformMatrix4x3fv);
    }

    // 3.0
    if (isAtLeastGL(gl::Version(3, 0)))
    {
        AssignGLEntryPoint(loadProcAddress("glBeginConditionalRender"), &beginConditionalRender);
        AssignGLEntryPoint(loadProcAddress("glBeginTransformFeedback"), &beginTransformFeedback);
        AssignGLEntryPoint(loadProcAddress("glBindBufferBase"), &bindBufferBase);
        AssignGLEntryPoint(loadProcAddress("glBindBufferRange"), &bindBufferRange);
        AssignGLEntryPoint(loadProcAddress("glBindFragDataLocation"), &bindFragDataLocation);
        AssignGLEntryPoint(loadProcAddress("glBindFramebuffer"), &bindFramebuffer);
        AssignGLEntryPoint(loadProcAddress("glBindRenderbuffer"), &bindRenderbuffer);
        AssignGLEntryPoint(loadProcAddress("glBindVertexArray"), &bindVertexArray);
        AssignGLEntryPoint(loadProcAddress("glBlitFramebuffer"), &blitFramebuffer);
        AssignGLEntryPoint(loadProcAddress("glCheckFramebufferStatus"), &checkFramebufferStatus);
        AssignGLEntryPoint(loadProcAddress("glClampColor"), &clampColor);
        AssignGLEntryPoint(loadProcAddress("glClearBufferfi"), &clearBufferfi);
        AssignGLEntryPoint(loadProcAddress("glClearBufferfv"), &clearBufferfv);
        AssignGLEntryPoint(loadProcAddress("glClearBufferiv"), &clearBufferiv);
        AssignGLEntryPoint(loadProcAddress("glClearBufferuiv"), &clearBufferuiv);
        AssignGLEntryPoint(loadProcAddress("glColorMaski"), &colorMaski);
        AssignGLEntryPoint(loadProcAddress("glDeleteFramebuffers"), &deleteFramebuffers);
        AssignGLEntryPoint(loadProcAddress("glDeleteRenderbuffers"), &deleteRenderbuffers);
        AssignGLEntryPoint(loadProcAddress("glDeleteVertexArrays"), &deleteVertexArrays);
        AssignGLEntryPoint(loadProcAddress("glDisablei"), &disablei);
        AssignGLEntryPoint(loadProcAddress("glEnablei"), &enablei);
        AssignGLEntryPoint(loadProcAddress("glEndConditionalRender"), &endConditionalRender);
        AssignGLEntryPoint(loadProcAddress("glEndTransformFeedback"), &endTransformFeedback);
        AssignGLEntryPoint(loadProcAddress("glFlushMappedBufferRange"), &flushMappedBufferRange);
        AssignGLEntryPoint(loadProcAddress("glFramebufferRenderbuffer"), &framebufferRenderbuffer);
        AssignGLEntryPoint(loadProcAddress("glFramebufferTexture1D"), &framebufferTexture1D);
        AssignGLEntryPoint(loadProcAddress("glFramebufferTexture2D"), &framebufferTexture2D);
        AssignGLEntryPoint(loadProcAddress("glFramebufferTexture3D"), &framebufferTexture3D);
        AssignGLEntryPoint(loadProcAddress("glFramebufferTextureLayer"), &framebufferTextureLayer);
        AssignGLEntryPoint(loadProcAddress("glGenFramebuffers"), &genFramebuffers);
        AssignGLEntryPoint(loadProcAddress("glGenRenderbuffers"), &genRenderbuffers);
        AssignGLEntryPoint(loadProcAddress("glGenVertexArrays"), &genVertexArrays);
        AssignGLEntryPoint(loadProcAddress("glGenerateMipmap"), &generateMipmap);
        AssignGLEntryPoint(loadProcAddress("glGetBooleani_v"), &getBooleani_v);
        AssignGLEntryPoint(loadProcAddress("glGetFragDataLocation"), &getFragDataLocation);
        AssignGLEntryPoint(loadProcAddress("glGetFramebufferAttachmentParameteriv"), &getFramebufferAttachmentParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetIntegeri_v"), &getIntegeri_v);
        AssignGLEntryPoint(loadProcAddress("glGetRenderbufferParameteriv"), &getRenderbufferParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetStringi"), &getStringi);
        AssignGLEntryPoint(loadProcAddress("glGetTexParameterIiv"), &getTexParameterIiv);
        AssignGLEntryPoint(loadProcAddress("glGetTexParameterIuiv"), &getTexParameterIuiv);
        AssignGLEntryPoint(loadProcAddress("glGetTransformFeedbackVarying"), &getTransformFeedbackVarying);
        AssignGLEntryPoint(loadProcAddress("glGetUniformuiv"), &getUniformuiv);
        AssignGLEntryPoint(loadProcAddress("glGetVertexAttribIiv"), &getVertexAttribIiv);
        AssignGLEntryPoint(loadProcAddress("glGetVertexAttribIuiv"), &getVertexAttribIuiv);
        AssignGLEntryPoint(loadProcAddress("glIsEnabledi"), &isEnabledi);
        AssignGLEntryPoint(loadProcAddress("glIsFramebuffer"), &isFramebuffer);
        AssignGLEntryPoint(loadProcAddress("glIsRenderbuffer"), &isRenderbuffer);
        AssignGLEntryPoint(loadProcAddress("glIsVertexArray"), &isVertexArray);
        AssignGLEntryPoint(loadProcAddress("glMapBufferRange"), &mapBufferRange);
        AssignGLEntryPoint(loadProcAddress("glRenderbufferStorage"), &renderbufferStorage);
        AssignGLEntryPoint(loadProcAddress("glRenderbufferStorageMultisample"), &renderbufferStorageMultisample);
        AssignGLEntryPoint(loadProcAddress("glTexParameterIiv"), &texParameterIiv);
        AssignGLEntryPoint(loadProcAddress("glTexParameterIuiv"), &texParameterIuiv);
        AssignGLEntryPoint(loadProcAddress("glTransformFeedbackVaryings"), &transformFeedbackVaryings);
        AssignGLEntryPoint(loadProcAddress("glUniform1ui"), &uniform1ui);
        AssignGLEntryPoint(loadProcAddress("glUniform1uiv"), &uniform1uiv);
        AssignGLEntryPoint(loadProcAddress("glUniform2ui"), &uniform2ui);
        AssignGLEntryPoint(loadProcAddress("glUniform2uiv"), &uniform2uiv);
        AssignGLEntryPoint(loadProcAddress("glUniform3ui"), &uniform3ui);
        AssignGLEntryPoint(loadProcAddress("glUniform3uiv"), &uniform3uiv);
        AssignGLEntryPoint(loadProcAddress("glUniform4ui"), &uniform4ui);
        AssignGLEntryPoint(loadProcAddress("glUniform4uiv"), &uniform4uiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI1i"), &vertexAttribI1i);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI1iv"), &vertexAttribI1iv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI1ui"), &vertexAttribI1ui);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI1uiv"), &vertexAttribI1uiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI2i"), &vertexAttribI2i);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI2iv"), &vertexAttribI2iv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI2ui"), &vertexAttribI2ui);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI2uiv"), &vertexAttribI2uiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI3i"), &vertexAttribI3i);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI3iv"), &vertexAttribI3iv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI3ui"), &vertexAttribI3ui);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI3uiv"), &vertexAttribI3uiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI4bv"), &vertexAttribI4bv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI4i"), &vertexAttribI4i);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI4iv"), &vertexAttribI4iv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI4sv"), &vertexAttribI4sv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI4ubv"), &vertexAttribI4ubv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI4ui"), &vertexAttribI4ui);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI4uiv"), &vertexAttribI4uiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribI4usv"), &vertexAttribI4usv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribIPointer"), &vertexAttribIPointer);
    }

    // 3.1
    if (isAtLeastGL(gl::Version(3, 1)))
    {
        AssignGLEntryPoint(loadProcAddress("glCopyBufferSubData"), &copyBufferSubData);
        AssignGLEntryPoint(loadProcAddress("glDrawArraysInstanced"), &drawArraysInstanced);
        AssignGLEntryPoint(loadProcAddress("glDrawElementsInstanced"), &drawElementsInstanced);
        AssignGLEntryPoint(loadProcAddress("glGetActiveUniformBlockName"), &getActiveUniformBlockName);
        AssignGLEntryPoint(loadProcAddress("glGetActiveUniformBlockiv"), &getActiveUniformBlockiv);
        AssignGLEntryPoint(loadProcAddress("glGetActiveUniformName"), &getActiveUniformName);
        AssignGLEntryPoint(loadProcAddress("glGetActiveUniformsiv"), &getActiveUniformsiv);
        AssignGLEntryPoint(loadProcAddress("glGetUniformBlockIndex"), &getUniformBlockIndex);
        AssignGLEntryPoint(loadProcAddress("glGetUniformIndices"), &getUniformIndices);
        AssignGLEntryPoint(loadProcAddress("glPrimitiveRestartIndex"), &primitiveRestartIndex);
        AssignGLEntryPoint(loadProcAddress("glTexBuffer"), &texBuffer);
        AssignGLEntryPoint(loadProcAddress("glUniformBlockBinding"), &uniformBlockBinding);
    }

    // 3.2
    if (isAtLeastGL(gl::Version(3, 2)))
    {
        AssignGLEntryPoint(loadProcAddress("glClientWaitSync"), &clientWaitSync);
        AssignGLEntryPoint(loadProcAddress("glDeleteSync"), &deleteSync);
        AssignGLEntryPoint(loadProcAddress("glDrawElementsBaseVertex"), &drawElementsBaseVertex);
        AssignGLEntryPoint(loadProcAddress("glDrawElementsInstancedBaseVertex"), &drawElementsInstancedBaseVertex);
        AssignGLEntryPoint(loadProcAddress("glDrawRangeElementsBaseVertex"), &drawRangeElementsBaseVertex);
        AssignGLEntryPoint(loadProcAddress("glFenceSync"), &fenceSync);
        AssignGLEntryPoint(loadProcAddress("glFramebufferTexture"), &framebufferTexture);
        AssignGLEntryPoint(loadProcAddress("glGetBufferParameteri64v"), &getBufferParameteri64v);
        AssignGLEntryPoint(loadProcAddress("glGetInteger64i_v"), &getInteger64i_v);
        AssignGLEntryPoint(loadProcAddress("glGetInteger64v"), &getInteger64v);
        AssignGLEntryPoint(loadProcAddress("glGetMultisamplefv"), &getMultisamplefv);
        AssignGLEntryPoint(loadProcAddress("glGetSynciv"), &getSynciv);
        AssignGLEntryPoint(loadProcAddress("glIsSync"), &isSync);
        AssignGLEntryPoint(loadProcAddress("glMultiDrawElementsBaseVertex"), &multiDrawElementsBaseVertex);
        AssignGLEntryPoint(loadProcAddress("glProvokingVertex"), &provokingVertex);
        AssignGLEntryPoint(loadProcAddress("glSampleMaski"), &sampleMaski);
        AssignGLEntryPoint(loadProcAddress("glTexImage2DMultisample"), &texImage2DMultisample);
        AssignGLEntryPoint(loadProcAddress("glTexImage3DMultisample"), &texImage3DMultisample);
        AssignGLEntryPoint(loadProcAddress("glWaitSync"), &waitSync);
    }

    // 3.3
    if (isAtLeastGL(gl::Version(3, 3)))
    {
        AssignGLEntryPoint(loadProcAddress("glBindFragDataLocationIndexed"), &bindFragDataLocationIndexed);
        AssignGLEntryPoint(loadProcAddress("glBindSampler"), &bindSampler);
        AssignGLEntryPoint(loadProcAddress("glDeleteSamplers"), &deleteSamplers);
        AssignGLEntryPoint(loadProcAddress("glGenSamplers"), &genSamplers);
        AssignGLEntryPoint(loadProcAddress("glGetFragDataIndex"), &getFragDataIndex);
        AssignGLEntryPoint(loadProcAddress("glGetQueryObjecti64v"), &getQueryObjecti64v);
        AssignGLEntryPoint(loadProcAddress("glGetQueryObjectui64v"), &getQueryObjectui64v);
        AssignGLEntryPoint(loadProcAddress("glGetSamplerParameterIiv"), &getSamplerParameterIiv);
        AssignGLEntryPoint(loadProcAddress("glGetSamplerParameterIuiv"), &getSamplerParameterIuiv);
        AssignGLEntryPoint(loadProcAddress("glGetSamplerParameterfv"), &getSamplerParameterfv);
        AssignGLEntryPoint(loadProcAddress("glGetSamplerParameteriv"), &getSamplerParameteriv);
        AssignGLEntryPoint(loadProcAddress("glIsSampler"), &isSampler);
        AssignGLEntryPoint(loadProcAddress("glQueryCounter"), &queryCounter);
        AssignGLEntryPoint(loadProcAddress("glSamplerParameterIiv"), &samplerParameterIiv);
        AssignGLEntryPoint(loadProcAddress("glSamplerParameterIuiv"), &samplerParameterIuiv);
        AssignGLEntryPoint(loadProcAddress("glSamplerParameterf"), &samplerParameterf);
        AssignGLEntryPoint(loadProcAddress("glSamplerParameterfv"), &samplerParameterfv);
        AssignGLEntryPoint(loadProcAddress("glSamplerParameteri"), &samplerParameteri);
        AssignGLEntryPoint(loadProcAddress("glSamplerParameteriv"), &samplerParameteriv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribDivisor"), &vertexAttribDivisor);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribP1ui"), &vertexAttribP1ui);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribP1uiv"), &vertexAttribP1uiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribP2ui"), &vertexAttribP2ui);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribP2uiv"), &vertexAttribP2uiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribP3ui"), &vertexAttribP3ui);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribP3uiv"), &vertexAttribP3uiv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribP4ui"), &vertexAttribP4ui);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribP4uiv"), &vertexAttribP4uiv);
    }

    // 4.0
    if (isAtLeastGL(gl::Version(4, 0)))
    {
        AssignGLEntryPoint(loadProcAddress("glBeginQueryIndexed"), &beginQueryIndexed);
        AssignGLEntryPoint(loadProcAddress("glBindTransformFeedback"), &bindTransformFeedback);
        AssignGLEntryPoint(loadProcAddress("glBlendEquationSeparatei"), &blendEquationSeparatei);
        AssignGLEntryPoint(loadProcAddress("glBlendEquationi"), &blendEquationi);
        AssignGLEntryPoint(loadProcAddress("glBlendFuncSeparatei"), &blendFuncSeparatei);
        AssignGLEntryPoint(loadProcAddress("glBlendFunci"), &blendFunci);
        AssignGLEntryPoint(loadProcAddress("glDeleteTransformFeedbacks"), &deleteTransformFeedbacks);
        AssignGLEntryPoint(loadProcAddress("glDrawArraysIndirect"), &drawArraysIndirect);
        AssignGLEntryPoint(loadProcAddress("glDrawElementsIndirect"), &drawElementsIndirect);
        AssignGLEntryPoint(loadProcAddress("glDrawTransformFeedback"), &drawTransformFeedback);
        AssignGLEntryPoint(loadProcAddress("glDrawTransformFeedbackStream"), &drawTransformFeedbackStream);
        AssignGLEntryPoint(loadProcAddress("glEndQueryIndexed"), &endQueryIndexed);
        AssignGLEntryPoint(loadProcAddress("glGenTransformFeedbacks"), &genTransformFeedbacks);
        AssignGLEntryPoint(loadProcAddress("glGetActiveSubroutineName"), &getActiveSubroutineName);
        AssignGLEntryPoint(loadProcAddress("glGetActiveSubroutineUniformName"), &getActiveSubroutineUniformName);
        AssignGLEntryPoint(loadProcAddress("glGetActiveSubroutineUniformiv"), &getActiveSubroutineUniformiv);
        AssignGLEntryPoint(loadProcAddress("glGetProgramStageiv"), &getProgramStageiv);
        AssignGLEntryPoint(loadProcAddress("glGetQueryIndexediv"), &getQueryIndexediv);
        AssignGLEntryPoint(loadProcAddress("glGetSubroutineIndex"), &getSubroutineIndex);
        AssignGLEntryPoint(loadProcAddress("glGetSubroutineUniformLocation"), &getSubroutineUniformLocation);
        AssignGLEntryPoint(loadProcAddress("glGetUniformSubroutineuiv"), &getUniformSubroutineuiv);
        AssignGLEntryPoint(loadProcAddress("glGetUniformdv"), &getUniformdv);
        AssignGLEntryPoint(loadProcAddress("glIsTransformFeedback"), &isTransformFeedback);
        AssignGLEntryPoint(loadProcAddress("glMinSampleShading"), &minSampleShading);
        AssignGLEntryPoint(loadProcAddress("glPatchParameterfv"), &patchParameterfv);
        AssignGLEntryPoint(loadProcAddress("glPatchParameteri"), &patchParameteri);
        AssignGLEntryPoint(loadProcAddress("glPauseTransformFeedback"), &pauseTransformFeedback);
        AssignGLEntryPoint(loadProcAddress("glResumeTransformFeedback"), &resumeTransformFeedback);
        AssignGLEntryPoint(loadProcAddress("glUniform1d"), &uniform1d);
        AssignGLEntryPoint(loadProcAddress("glUniform1dv"), &uniform1dv);
        AssignGLEntryPoint(loadProcAddress("glUniform2d"), &uniform2d);
        AssignGLEntryPoint(loadProcAddress("glUniform2dv"), &uniform2dv);
        AssignGLEntryPoint(loadProcAddress("glUniform3d"), &uniform3d);
        AssignGLEntryPoint(loadProcAddress("glUniform3dv"), &uniform3dv);
        AssignGLEntryPoint(loadProcAddress("glUniform4d"), &uniform4d);
        AssignGLEntryPoint(loadProcAddress("glUniform4dv"), &uniform4dv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix2dv"), &uniformMatrix2dv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix2x3dv"), &uniformMatrix2x3dv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix2x4dv"), &uniformMatrix2x4dv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix3dv"), &uniformMatrix3dv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix3x2dv"), &uniformMatrix3x2dv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix3x4dv"), &uniformMatrix3x4dv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix4dv"), &uniformMatrix4dv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix4x2dv"), &uniformMatrix4x2dv);
        AssignGLEntryPoint(loadProcAddress("glUniformMatrix4x3dv"), &uniformMatrix4x3dv);
        AssignGLEntryPoint(loadProcAddress("glUniformSubroutinesuiv"), &uniformSubroutinesuiv);
    }

    // 4.1
    if (isAtLeastGL(gl::Version(4, 1)))
    {
        AssignGLEntryPoint(loadProcAddress("glActiveShaderProgram"), &activeShaderProgram);
        AssignGLEntryPoint(loadProcAddress("glBindProgramPipeline"), &bindProgramPipeline);
        AssignGLEntryPoint(loadProcAddress("glClearDepthf"), &clearDepthf);
        AssignGLEntryPoint(loadProcAddress("glCreateShaderProgramv"), &createShaderProgramv);
        AssignGLEntryPoint(loadProcAddress("glDeleteProgramPipelines"), &deleteProgramPipelines);
        AssignGLEntryPoint(loadProcAddress("glDepthRangeArrayv"), &depthRangeArrayv);
        AssignGLEntryPoint(loadProcAddress("glDepthRangeIndexed"), &depthRangeIndexed);
        AssignGLEntryPoint(loadProcAddress("glDepthRangef"), &depthRangef);
        AssignGLEntryPoint(loadProcAddress("glGenProgramPipelines"), &genProgramPipelines);
        AssignGLEntryPoint(loadProcAddress("glGetDoublei_v"), &getDoublei_v);
        AssignGLEntryPoint(loadProcAddress("glGetFloati_v"), &getFloati_v);
        AssignGLEntryPoint(loadProcAddress("glGetProgramBinary"), &getProgramBinary);
        AssignGLEntryPoint(loadProcAddress("glGetProgramPipelineInfoLog"), &getProgramPipelineInfoLog);
        AssignGLEntryPoint(loadProcAddress("glGetProgramPipelineiv"), &getProgramPipelineiv);
        AssignGLEntryPoint(loadProcAddress("glGetShaderPrecisionFormat"), &getShaderPrecisionFormat);
        AssignGLEntryPoint(loadProcAddress("glGetVertexAttribLdv"), &getVertexAttribLdv);
        AssignGLEntryPoint(loadProcAddress("glIsProgramPipeline"), &isProgramPipeline);
        AssignGLEntryPoint(loadProcAddress("glProgramBinary"), &programBinary);
        AssignGLEntryPoint(loadProcAddress("glProgramParameteri"), &programParameteri);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform1d"), &programUniform1d);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform1dv"), &programUniform1dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform1f"), &programUniform1f);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform1fv"), &programUniform1fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform1i"), &programUniform1i);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform1iv"), &programUniform1iv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform1ui"), &programUniform1ui);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform1uiv"), &programUniform1uiv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform2d"), &programUniform2d);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform2dv"), &programUniform2dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform2f"), &programUniform2f);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform2fv"), &programUniform2fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform2i"), &programUniform2i);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform2iv"), &programUniform2iv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform2ui"), &programUniform2ui);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform2uiv"), &programUniform2uiv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform3d"), &programUniform3d);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform3dv"), &programUniform3dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform3f"), &programUniform3f);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform3fv"), &programUniform3fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform3i"), &programUniform3i);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform3iv"), &programUniform3iv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform3ui"), &programUniform3ui);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform3uiv"), &programUniform3uiv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform4d"), &programUniform4d);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform4dv"), &programUniform4dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform4f"), &programUniform4f);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform4fv"), &programUniform4fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform4i"), &programUniform4i);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform4iv"), &programUniform4iv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform4ui"), &programUniform4ui);
        AssignGLEntryPoint(loadProcAddress("glProgramUniform4uiv"), &programUniform4uiv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix2dv"), &programUniformMatrix2dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix2fv"), &programUniformMatrix2fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix2x3dv"), &programUniformMatrix2x3dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix2x3fv"), &programUniformMatrix2x3fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix2x4dv"), &programUniformMatrix2x4dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix2x4fv"), &programUniformMatrix2x4fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix3dv"), &programUniformMatrix3dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix3fv"), &programUniformMatrix3fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix3x2dv"), &programUniformMatrix3x2dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix3x2fv"), &programUniformMatrix3x2fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix3x4dv"), &programUniformMatrix3x4dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix3x4fv"), &programUniformMatrix3x4fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix4dv"), &programUniformMatrix4dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix4fv"), &programUniformMatrix4fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix4x2dv"), &programUniformMatrix4x2dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix4x2fv"), &programUniformMatrix4x2fv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix4x3dv"), &programUniformMatrix4x3dv);
        AssignGLEntryPoint(loadProcAddress("glProgramUniformMatrix4x3fv"), &programUniformMatrix4x3fv);
        AssignGLEntryPoint(loadProcAddress("glReleaseShaderCompiler"), &releaseShaderCompiler);
        AssignGLEntryPoint(loadProcAddress("glScissorArrayv"), &scissorArrayv);
        AssignGLEntryPoint(loadProcAddress("glScissorIndexed"), &scissorIndexed);
        AssignGLEntryPoint(loadProcAddress("glScissorIndexedv"), &scissorIndexedv);
        AssignGLEntryPoint(loadProcAddress("glShaderBinary"), &shaderBinary);
        AssignGLEntryPoint(loadProcAddress("glUseProgramStages"), &useProgramStages);
        AssignGLEntryPoint(loadProcAddress("glValidateProgramPipeline"), &validateProgramPipeline);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribL1d"), &vertexAttribL1d);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribL1dv"), &vertexAttribL1dv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribL2d"), &vertexAttribL2d);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribL2dv"), &vertexAttribL2dv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribL3d"), &vertexAttribL3d);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribL3dv"), &vertexAttribL3dv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribL4d"), &vertexAttribL4d);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribL4dv"), &vertexAttribL4dv);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribLPointer"), &vertexAttribLPointer);
        AssignGLEntryPoint(loadProcAddress("glViewportArrayv"), &viewportArrayv);
        AssignGLEntryPoint(loadProcAddress("glViewportIndexedf"), &viewportIndexedf);
        AssignGLEntryPoint(loadProcAddress("glViewportIndexedfv"), &viewportIndexedfv);
    }

    // 4.2
    if (isAtLeastGL(gl::Version(4, 2)))
    {
        AssignGLEntryPoint(loadProcAddress("glBindImageTexture"), &bindImageTexture);
        AssignGLEntryPoint(loadProcAddress("glDrawArraysInstancedBaseInstance"), &drawArraysInstancedBaseInstance);
        AssignGLEntryPoint(loadProcAddress("glDrawElementsInstancedBaseInstance"), &drawElementsInstancedBaseInstance);
        AssignGLEntryPoint(loadProcAddress("glDrawElementsInstancedBaseVertexBaseInstance"), &drawElementsInstancedBaseVertexBaseInstance);
        AssignGLEntryPoint(loadProcAddress("glDrawTransformFeedbackInstanced"), &drawTransformFeedbackInstanced);
        AssignGLEntryPoint(loadProcAddress("glDrawTransformFeedbackStreamInstanced"), &drawTransformFeedbackStreamInstanced);
        AssignGLEntryPoint(loadProcAddress("glGetActiveAtomicCounterBufferiv"), &getActiveAtomicCounterBufferiv);
        AssignGLEntryPoint(loadProcAddress("glGetInternalformativ"), &getInternalformativ);
        AssignGLEntryPoint(loadProcAddress("glMemoryBarrier"), &memoryBarrier);
        AssignGLEntryPoint(loadProcAddress("glTexStorage1D"), &texStorage1D);
        AssignGLEntryPoint(loadProcAddress("glTexStorage2D"), &texStorage2D);
        AssignGLEntryPoint(loadProcAddress("glTexStorage3D"), &texStorage3D);
    }

    // 4.3
    if (isAtLeastGL(gl::Version(4, 3)))
    {
        AssignGLEntryPoint(loadProcAddress("glBindVertexBuffer"), &bindVertexBuffer);
        AssignGLEntryPoint(loadProcAddress("glClearBufferData"), &clearBufferData);
        AssignGLEntryPoint(loadProcAddress("glClearBufferSubData"), &clearBufferSubData);
        AssignGLEntryPoint(loadProcAddress("glCopyImageSubData"), &copyImageSubData);
        AssignGLEntryPoint(loadProcAddress("glDebugMessageCallback"), &debugMessageCallback);
        AssignGLEntryPoint(loadProcAddress("glDebugMessageControl"), &debugMessageControl);
        AssignGLEntryPoint(loadProcAddress("glDebugMessageInsert"), &debugMessageInsert);
        AssignGLEntryPoint(loadProcAddress("glDispatchCompute"), &dispatchCompute);
        AssignGLEntryPoint(loadProcAddress("glDispatchComputeIndirect"), &dispatchComputeIndirect);
        AssignGLEntryPoint(loadProcAddress("glFramebufferParameteri"), &framebufferParameteri);
        AssignGLEntryPoint(loadProcAddress("glGetDebugMessageLog"), &getDebugMessageLog);
        AssignGLEntryPoint(loadProcAddress("glGetFramebufferParameteriv"), &getFramebufferParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetInternalformati64v"), &getInternalformati64v);
        AssignGLEntryPoint(loadProcAddress("glGetPointerv"), &getPointerv);
        AssignGLEntryPoint(loadProcAddress("glGetObjectLabel"), &getObjectLabel);
        AssignGLEntryPoint(loadProcAddress("glGetObjectPtrLabel"), &getObjectPtrLabel);
        AssignGLEntryPoint(loadProcAddress("glGetProgramInterfaceiv"), &getProgramInterfaceiv);
        AssignGLEntryPoint(loadProcAddress("glGetProgramResourceIndex"), &getProgramResourceIndex);
        AssignGLEntryPoint(loadProcAddress("glGetProgramResourceLocation"), &getProgramResourceLocation);
        AssignGLEntryPoint(loadProcAddress("glGetProgramResourceLocationIndex"), &getProgramResourceLocationIndex);
        AssignGLEntryPoint(loadProcAddress("glGetProgramResourceName"), &getProgramResourceName);
        AssignGLEntryPoint(loadProcAddress("glGetProgramResourceiv"), &getProgramResourceiv);
        AssignGLEntryPoint(loadProcAddress("glInvalidateBufferData"), &invalidateBufferData);
        AssignGLEntryPoint(loadProcAddress("glInvalidateBufferSubData"), &invalidateBufferSubData);
        AssignGLEntryPoint(loadProcAddress("glInvalidateFramebuffer"), &invalidateFramebuffer);
        AssignGLEntryPoint(loadProcAddress("glInvalidateSubFramebuffer"), &invalidateSubFramebuffer);
        AssignGLEntryPoint(loadProcAddress("glInvalidateTexImage"), &invalidateTexImage);
        AssignGLEntryPoint(loadProcAddress("glInvalidateTexSubImage"), &invalidateTexSubImage);
        AssignGLEntryPoint(loadProcAddress("glMultiDrawArraysIndirect"), &multiDrawArraysIndirect);
        AssignGLEntryPoint(loadProcAddress("glMultiDrawElementsIndirect"), &multiDrawElementsIndirect);
        AssignGLEntryPoint(loadProcAddress("glObjectLabel"), &objectLabel);
        AssignGLEntryPoint(loadProcAddress("glObjectPtrLabel"), &objectPtrLabel);
        AssignGLEntryPoint(loadProcAddress("glPopDebugGroup"), &popDebugGroup);
        AssignGLEntryPoint(loadProcAddress("glPushDebugGroup"), &pushDebugGroup);
        AssignGLEntryPoint(loadProcAddress("glShaderStorageBlockBinding"), &shaderStorageBlockBinding);
        AssignGLEntryPoint(loadProcAddress("glTexBufferRange"), &texBufferRange);
        AssignGLEntryPoint(loadProcAddress("glTexStorage2DMultisample"), &texStorage2DMultisample);
        AssignGLEntryPoint(loadProcAddress("glTexStorage3DMultisample"), &texStorage3DMultisample);
        AssignGLEntryPoint(loadProcAddress("glTextureView"), &textureView);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribBinding"), &vertexAttribBinding);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribFormat"), &vertexAttribFormat);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribIFormat"), &vertexAttribIFormat);
        AssignGLEntryPoint(loadProcAddress("glVertexAttribLFormat"), &vertexAttribLFormat);
        AssignGLEntryPoint(loadProcAddress("glVertexBindingDivisor"), &vertexBindingDivisor);
    }

    // 4.4
    if (isAtLeastGL(gl::Version(4, 4)))
    {
        AssignGLEntryPoint(loadProcAddress("glBindBuffersBase"), &bindBuffersBase);
        AssignGLEntryPoint(loadProcAddress("glBindBuffersRange"), &bindBuffersRange);
        AssignGLEntryPoint(loadProcAddress("glBindImageTextures"), &bindImageTextures);
        AssignGLEntryPoint(loadProcAddress("glBindSamplers"), &bindSamplers);
        AssignGLEntryPoint(loadProcAddress("glBindTextures"), &bindTextures);
        AssignGLEntryPoint(loadProcAddress("glBindVertexBuffers"), &bindVertexBuffers);
        AssignGLEntryPoint(loadProcAddress("glBufferStorage"), &bufferStorage);
        AssignGLEntryPoint(loadProcAddress("glClearTexImage"), &clearTexImage);
        AssignGLEntryPoint(loadProcAddress("glClearTexSubImage"), &clearTexSubImage);
    }

    // 4.5
    if (isAtLeastGL(gl::Version(4, 5)))
    {
        AssignGLEntryPoint(loadProcAddress("glBindTextureUnit"), &bindTextureUnit);
        AssignGLEntryPoint(loadProcAddress("glBlitNamedFramebuffer"), &blitNamedFramebuffer);
        AssignGLEntryPoint(loadProcAddress("glCheckNamedFramebufferStatus"), &checkNamedFramebufferStatus);
        AssignGLEntryPoint(loadProcAddress("glClearNamedBufferData"), &clearNamedBufferData);
        AssignGLEntryPoint(loadProcAddress("glClearNamedBufferSubData"), &clearNamedBufferSubData);
        AssignGLEntryPoint(loadProcAddress("glClearNamedFramebufferfi"), &clearNamedFramebufferfi);
        AssignGLEntryPoint(loadProcAddress("glClearNamedFramebufferfv"), &clearNamedFramebufferfv);
        AssignGLEntryPoint(loadProcAddress("glClearNamedFramebufferiv"), &clearNamedFramebufferiv);
        AssignGLEntryPoint(loadProcAddress("glClearNamedFramebufferuiv"), &clearNamedFramebufferuiv);
        AssignGLEntryPoint(loadProcAddress("glClipControl"), &clipControl);
        AssignGLEntryPoint(loadProcAddress("glCompressedTextureSubImage1D"), &compressedTextureSubImage1D);
        AssignGLEntryPoint(loadProcAddress("glCompressedTextureSubImage2D"), &compressedTextureSubImage2D);
        AssignGLEntryPoint(loadProcAddress("glCompressedTextureSubImage3D"), &compressedTextureSubImage3D);
        AssignGLEntryPoint(loadProcAddress("glCopyNamedBufferSubData"), &copyNamedBufferSubData);
        AssignGLEntryPoint(loadProcAddress("glCopyTextureSubImage1D"), &copyTextureSubImage1D);
        AssignGLEntryPoint(loadProcAddress("glCopyTextureSubImage2D"), &copyTextureSubImage2D);
        AssignGLEntryPoint(loadProcAddress("glCopyTextureSubImage3D"), &copyTextureSubImage3D);
        AssignGLEntryPoint(loadProcAddress("glCreateBuffers"), &createBuffers);
        AssignGLEntryPoint(loadProcAddress("glCreateFramebuffers"), &createFramebuffers);
        AssignGLEntryPoint(loadProcAddress("glCreateProgramPipelines"), &createProgramPipelines);
        AssignGLEntryPoint(loadProcAddress("glCreateQueries"), &createQueries);
        AssignGLEntryPoint(loadProcAddress("glCreateRenderbuffers"), &createRenderbuffers);
        AssignGLEntryPoint(loadProcAddress("glCreateSamplers"), &createSamplers);
        AssignGLEntryPoint(loadProcAddress("glCreateTextures"), &createTextures);
        AssignGLEntryPoint(loadProcAddress("glCreateTransformFeedbacks"), &createTransformFeedbacks);
        AssignGLEntryPoint(loadProcAddress("glCreateVertexArrays"), &createVertexArrays);
        AssignGLEntryPoint(loadProcAddress("glDisableVertexArrayAttrib"), &disableVertexArrayAttrib);
        AssignGLEntryPoint(loadProcAddress("glEnableVertexArrayAttrib"), &enableVertexArrayAttrib);
        AssignGLEntryPoint(loadProcAddress("glFlushMappedNamedBufferRange"), &flushMappedNamedBufferRange);
        AssignGLEntryPoint(loadProcAddress("glGenerateTextureMipmap"), &generateTextureMipmap);
        AssignGLEntryPoint(loadProcAddress("glGetCompressedTextureImage"), &getCompressedTextureImage);
        AssignGLEntryPoint(loadProcAddress("glGetCompressedTextureSubImage"), &getCompressedTextureSubImage);
        AssignGLEntryPoint(loadProcAddress("glGetGraphicsResetStatus"), &getGraphicsResetStatus);
        AssignGLEntryPoint(loadProcAddress("glGetNamedBufferParameteri64v"), &getNamedBufferParameteri64v);
        AssignGLEntryPoint(loadProcAddress("glGetNamedBufferParameteriv"), &getNamedBufferParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetNamedBufferPointerv"), &getNamedBufferPointerv);
        AssignGLEntryPoint(loadProcAddress("glGetNamedBufferSubData"), &getNamedBufferSubData);
        AssignGLEntryPoint(loadProcAddress("glGetNamedFramebufferAttachmentParameteriv"), &getNamedFramebufferAttachmentParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetNamedFramebufferParameteriv"), &getNamedFramebufferParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetNamedRenderbufferParameteriv"), &getNamedRenderbufferParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetQueryBufferObjecti64v"), &getQueryBufferObjecti64v);
        AssignGLEntryPoint(loadProcAddress("glGetQueryBufferObjectiv"), &getQueryBufferObjectiv);
        AssignGLEntryPoint(loadProcAddress("glGetQueryBufferObjectui64v"), &getQueryBufferObjectui64v);
        AssignGLEntryPoint(loadProcAddress("glGetQueryBufferObjectuiv"), &getQueryBufferObjectuiv);
        AssignGLEntryPoint(loadProcAddress("glGetTextureImage"), &getTextureImage);
        AssignGLEntryPoint(loadProcAddress("glGetTextureLevelParameterfv"), &getTextureLevelParameterfv);
        AssignGLEntryPoint(loadProcAddress("glGetTextureLevelParameteriv"), &getTextureLevelParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetTextureParameterIiv"), &getTextureParameterIiv);
        AssignGLEntryPoint(loadProcAddress("glGetTextureParameterIuiv"), &getTextureParameterIuiv);
        AssignGLEntryPoint(loadProcAddress("glGetTextureParameterfv"), &getTextureParameterfv);
        AssignGLEntryPoint(loadProcAddress("glGetTextureParameteriv"), &getTextureParameteriv);
        AssignGLEntryPoint(loadProcAddress("glGetTextureSubImage"), &getTextureSubImage);
        AssignGLEntryPoint(loadProcAddress("glGetTransformFeedbacki64_v"), &getTransformFeedbacki64_v);
        AssignGLEntryPoint(loadProcAddress("glGetTransformFeedbacki_v"), &getTransformFeedbacki_v);
        AssignGLEntryPoint(loadProcAddress("glGetTransformFeedbackiv"), &getTransformFeedbackiv);
        AssignGLEntryPoint(loadProcAddress("glGetVertexArrayIndexed64iv"), &getVertexArrayIndexed64iv);
        AssignGLEntryPoint(loadProcAddress("glGetVertexArrayIndexediv"), &getVertexArrayIndexediv);
        AssignGLEntryPoint(loadProcAddress("glGetVertexArrayiv"), &getVertexArrayiv);
        AssignGLEntryPoint(loadProcAddress("glGetnCompressedTexImage"), &getnCompressedTexImage);
        AssignGLEntryPoint(loadProcAddress("glGetnTexImage"), &getnTexImage);
        AssignGLEntryPoint(loadProcAddress("glGetnUniformdv"), &getnUniformdv);
        AssignGLEntryPoint(loadProcAddress("glGetnUniformfv"), &getnUniformfv);
        AssignGLEntryPoint(loadProcAddress("glGetnUniformiv"), &getnUniformiv);
        AssignGLEntryPoint(loadProcAddress("glGetnUniformuiv"), &getnUniformuiv);
        AssignGLEntryPoint(loadProcAddress("glInvalidateNamedFramebufferData"), &invalidateNamedFramebufferData);
        AssignGLEntryPoint(loadProcAddress("glInvalidateNamedFramebufferSubData"), &invalidateNamedFramebufferSubData);
        AssignGLEntryPoint(loadProcAddress("glMapNamedBuffer"), &mapNamedBuffer);
        AssignGLEntryPoint(loadProcAddress("glMapNamedBufferRange"), &mapNamedBufferRange);
        AssignGLEntryPoint(loadProcAddress("glMemoryBarrierByRegion"), &memoryBarrierByRegion);
        AssignGLEntryPoint(loadProcAddress("glNamedBufferData"), &namedBufferData);
        AssignGLEntryPoint(loadProcAddress("glNamedBufferStorage"), &namedBufferStorage);
        AssignGLEntryPoint(loadProcAddress("glNamedBufferSubData"), &namedBufferSubData);
        AssignGLEntryPoint(loadProcAddress("glNamedFramebufferDrawBuffer"), &namedFramebufferDrawBuffer);
        AssignGLEntryPoint(loadProcAddress("glNamedFramebufferDrawBuffers"), &namedFramebufferDrawBuffers);
        AssignGLEntryPoint(loadProcAddress("glNamedFramebufferParameteri"), &namedFramebufferParameteri);
        AssignGLEntryPoint(loadProcAddress("glNamedFramebufferReadBuffer"), &namedFramebufferReadBuffer);
        AssignGLEntryPoint(loadProcAddress("glNamedFramebufferRenderbuffer"), &namedFramebufferRenderbuffer);
        AssignGLEntryPoint(loadProcAddress("glNamedFramebufferTexture"), &namedFramebufferTexture);
        AssignGLEntryPoint(loadProcAddress("glNamedFramebufferTextureLayer"), &namedFramebufferTextureLayer);
        AssignGLEntryPoint(loadProcAddress("glNamedRenderbufferStorage"), &namedRenderbufferStorage);
        AssignGLEntryPoint(loadProcAddress("glNamedRenderbufferStorageMultisample"), &namedRenderbufferStorageMultisample);
        AssignGLEntryPoint(loadProcAddress("glReadnPixels"), &readnPixels);
        AssignGLEntryPoint(loadProcAddress("glTextureBarrier"), &textureBarrier);
        AssignGLEntryPoint(loadProcAddress("glTextureBuffer"), &textureBuffer);
        AssignGLEntryPoint(loadProcAddress("glTextureBufferRange"), &textureBufferRange);
        AssignGLEntryPoint(loadProcAddress("glTextureParameterIiv"), &textureParameterIiv);
        AssignGLEntryPoint(loadProcAddress("glTextureParameterIuiv"), &textureParameterIuiv);
        AssignGLEntryPoint(loadProcAddress("glTextureParameterf"), &textureParameterf);
        AssignGLEntryPoint(loadProcAddress("glTextureParameterfv"), &textureParameterfv);
        AssignGLEntryPoint(loadProcAddress("glTextureParameteri"), &textureParameteri);
        AssignGLEntryPoint(loadProcAddress("glTextureParameteriv"), &textureParameteriv);
        AssignGLEntryPoint(loadProcAddress("glTextureStorage1D"), &textureStorage1D);
        AssignGLEntryPoint(loadProcAddress("glTextureStorage2D"), &textureStorage2D);
        AssignGLEntryPoint(loadProcAddress("glTextureStorage2DMultisample"), &textureStorage2DMultisample);
        AssignGLEntryPoint(loadProcAddress("glTextureStorage3D"), &textureStorage3D);
        AssignGLEntryPoint(loadProcAddress("glTextureStorage3DMultisample"), &textureStorage3DMultisample);
        AssignGLEntryPoint(loadProcAddress("glTextureSubImage1D"), &textureSubImage1D);
        AssignGLEntryPoint(loadProcAddress("glTextureSubImage2D"), &textureSubImage2D);
        AssignGLEntryPoint(loadProcAddress("glTextureSubImage3D"), &textureSubImage3D);
        AssignGLEntryPoint(loadProcAddress("glTransformFeedbackBufferBase"), &transformFeedbackBufferBase);
        AssignGLEntryPoint(loadProcAddress("glTransformFeedbackBufferRange"), &transformFeedbackBufferRange);
        AssignGLEntryPoint(loadProcAddress("glUnmapNamedBuffer"), &unmapNamedBuffer);
        AssignGLEntryPoint(loadProcAddress("glVertexArrayAttribBinding"), &vertexArrayAttribBinding);
        AssignGLEntryPoint(loadProcAddress("glVertexArrayAttribFormat"), &vertexArrayAttribFormat);
        AssignGLEntryPoint(loadProcAddress("glVertexArrayAttribIFormat"), &vertexArrayAttribIFormat);
        AssignGLEntryPoint(loadProcAddress("glVertexArrayAttribLFormat"), &vertexArrayAttribLFormat);
        AssignGLEntryPoint(loadProcAddress("glVertexArrayBindingDivisor"), &vertexArrayBindingDivisor);
        AssignGLEntryPoint(loadProcAddress("glVertexArrayElementBuffer"), &vertexArrayElementBuffer);
        AssignGLEntryPoint(loadProcAddress("glVertexArrayVertexBuffer"), &vertexArrayVertexBuffer);
        AssignGLEntryPoint(loadProcAddress("glVertexArrayVertexBuffers"), &vertexArrayVertexBuffers);
    }

    // clang-format on
}

bool FunctionsGL::isAtLeastGL(const gl::Version &glVersion) const
{
    return standard == STANDARD_GL_DESKTOP && version >= glVersion;
}

bool FunctionsGL::isAtLeastGLES(const gl::Version &glesVersion) const
{
    return standard == STANDARD_GL_ES && version >= glesVersion;
}

bool FunctionsGL::hasExtension(const std::string &ext) const
{
    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}

bool FunctionsGL::hasGLExtension(const std::string &ext) const
{
    return standard == STANDARD_GL_DESKTOP && hasExtension(ext);
}

bool FunctionsGL::hasGLESExtension(const std::string &ext) const
{
    return standard == STANDARD_GL_ES && hasExtension(ext);
}

}

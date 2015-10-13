//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// State.cpp: Implements the State class, encapsulating raw GL state.

#include "libANGLE/State.h"

#include "libANGLE/Context.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Query.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/formatutils.h"

namespace gl
{

State::State()
{
    mMaxDrawBuffers = 0;
    mMaxCombinedTextureImageUnits = 0;

    // Initialize dirty bit masks
    // TODO(jmadill): additional ES3 state
    mUnpackStateBitMask.set(DIRTY_BIT_UNPACK_ALIGNMENT);
    mUnpackStateBitMask.set(DIRTY_BIT_UNPACK_ROW_LENGTH);
    mPackStateBitMask.set(DIRTY_BIT_PACK_ALIGNMENT);
    mPackStateBitMask.set(DIRTY_BIT_PACK_REVERSE_ROW_ORDER);
    mClearStateBitMask.set(DIRTY_BIT_RASTERIZER_DISCARD_ENABLED);
    mClearStateBitMask.set(DIRTY_BIT_SCISSOR_TEST_ENABLED);
    mClearStateBitMask.set(DIRTY_BIT_SCISSOR);
    mClearStateBitMask.set(DIRTY_BIT_VIEWPORT);
    mClearStateBitMask.set(DIRTY_BIT_CLEAR_COLOR);
    mClearStateBitMask.set(DIRTY_BIT_CLEAR_DEPTH);
    mClearStateBitMask.set(DIRTY_BIT_CLEAR_STENCIL);
    mClearStateBitMask.set(DIRTY_BIT_COLOR_MASK);
    mClearStateBitMask.set(DIRTY_BIT_DEPTH_MASK);
    mClearStateBitMask.set(DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
    mClearStateBitMask.set(DIRTY_BIT_STENCIL_WRITEMASK_BACK);
}

State::~State()
{
    reset();
}

void State::initialize(const Caps &caps, GLuint clientVersion)
{
    mMaxDrawBuffers = caps.maxDrawBuffers;
    mMaxCombinedTextureImageUnits = caps.maxCombinedTextureImageUnits;

    setColorClearValue(0.0f, 0.0f, 0.0f, 0.0f);

    mDepthClearValue = 1.0f;
    mStencilClearValue = 0;

    mRasterizer.rasterizerDiscard = false;
    mRasterizer.cullFace = false;
    mRasterizer.cullMode = GL_BACK;
    mRasterizer.frontFace = GL_CCW;
    mRasterizer.polygonOffsetFill = false;
    mRasterizer.polygonOffsetFactor = 0.0f;
    mRasterizer.polygonOffsetUnits = 0.0f;
    mRasterizer.pointDrawMode = false;
    mRasterizer.multiSample = false;
    mScissorTest = false;
    mScissor.x = 0;
    mScissor.y = 0;
    mScissor.width = 0;
    mScissor.height = 0;

    mBlend.blend = false;
    mBlend.sourceBlendRGB = GL_ONE;
    mBlend.sourceBlendAlpha = GL_ONE;
    mBlend.destBlendRGB = GL_ZERO;
    mBlend.destBlendAlpha = GL_ZERO;
    mBlend.blendEquationRGB = GL_FUNC_ADD;
    mBlend.blendEquationAlpha = GL_FUNC_ADD;
    mBlend.sampleAlphaToCoverage = false;
    mBlend.dither = true;

    mBlendColor.red = 0;
    mBlendColor.green = 0;
    mBlendColor.blue = 0;
    mBlendColor.alpha = 0;

    mDepthStencil.depthTest = false;
    mDepthStencil.depthFunc = GL_LESS;
    mDepthStencil.depthMask = true;
    mDepthStencil.stencilTest = false;
    mDepthStencil.stencilFunc = GL_ALWAYS;
    mDepthStencil.stencilMask = static_cast<GLuint>(-1);
    mDepthStencil.stencilWritemask = static_cast<GLuint>(-1);
    mDepthStencil.stencilBackFunc = GL_ALWAYS;
    mDepthStencil.stencilBackMask = static_cast<GLuint>(-1);
    mDepthStencil.stencilBackWritemask = static_cast<GLuint>(-1);
    mDepthStencil.stencilFail = GL_KEEP;
    mDepthStencil.stencilPassDepthFail = GL_KEEP;
    mDepthStencil.stencilPassDepthPass = GL_KEEP;
    mDepthStencil.stencilBackFail = GL_KEEP;
    mDepthStencil.stencilBackPassDepthFail = GL_KEEP;
    mDepthStencil.stencilBackPassDepthPass = GL_KEEP;

    mStencilRef = 0;
    mStencilBackRef = 0;

    mSampleCoverage = false;
    mSampleCoverageValue = 1.0f;
    mSampleCoverageInvert = false;
    mGenerateMipmapHint = GL_DONT_CARE;
    mFragmentShaderDerivativeHint = GL_DONT_CARE;

    mLineWidth = 1.0f;

    mViewport.x = 0;
    mViewport.y = 0;
    mViewport.width = 0;
    mViewport.height = 0;
    mNearZ = 0.0f;
    mFarZ = 1.0f;

    mBlend.colorMaskRed = true;
    mBlend.colorMaskGreen = true;
    mBlend.colorMaskBlue = true;
    mBlend.colorMaskAlpha = true;

    mActiveSampler = 0;

    mVertexAttribCurrentValues.resize(caps.maxVertexAttributes);

    mUniformBuffers.resize(caps.maxCombinedUniformBlocks);

    mSamplerTextures[GL_TEXTURE_2D].resize(caps.maxCombinedTextureImageUnits);
    mSamplerTextures[GL_TEXTURE_CUBE_MAP].resize(caps.maxCombinedTextureImageUnits);
    if (clientVersion >= 3)
    {
        // TODO: These could also be enabled via extension
        mSamplerTextures[GL_TEXTURE_2D_ARRAY].resize(caps.maxCombinedTextureImageUnits);
        mSamplerTextures[GL_TEXTURE_3D].resize(caps.maxCombinedTextureImageUnits);
    }

    mSamplers.resize(caps.maxCombinedTextureImageUnits);

    mActiveQueries[GL_ANY_SAMPLES_PASSED].set(NULL);
    mActiveQueries[GL_ANY_SAMPLES_PASSED_CONSERVATIVE].set(NULL);
    mActiveQueries[GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN].set(NULL);

    mProgram = NULL;

    mReadFramebuffer = NULL;
    mDrawFramebuffer = NULL;

    mPrimitiveRestart = false;
}

void State::reset()
{
    for (TextureBindingMap::iterator bindingVec = mSamplerTextures.begin(); bindingVec != mSamplerTextures.end(); bindingVec++)
    {
        TextureBindingVector &textureVector = bindingVec->second;
        for (size_t textureIdx = 0; textureIdx < textureVector.size(); textureIdx++)
        {
            textureVector[textureIdx].set(NULL);
        }
    }
    for (size_t samplerIdx = 0; samplerIdx < mSamplers.size(); samplerIdx++)
    {
        mSamplers[samplerIdx].set(NULL);
    }

    mArrayBuffer.set(NULL);
    mRenderbuffer.set(NULL);

    if (mProgram)
    {
        mProgram->release();
    }
    mProgram = NULL;

    mTransformFeedback.set(NULL);

    for (State::ActiveQueryMap::iterator i = mActiveQueries.begin(); i != mActiveQueries.end(); i++)
    {
        i->second.set(NULL);
    }

    mGenericUniformBuffer.set(NULL);
    for (BufferVector::iterator bufItr = mUniformBuffers.begin(); bufItr != mUniformBuffers.end(); ++bufItr)
    {
        bufItr->set(NULL);
    }

    mCopyReadBuffer.set(NULL);
    mCopyWriteBuffer.set(NULL);

    mPack.pixelBuffer.set(NULL);
    mUnpack.pixelBuffer.set(NULL);

    mProgram = NULL;

    // TODO(jmadill): Is this necessary?
    setAllDirtyBits();
}

const RasterizerState &State::getRasterizerState() const
{
    return mRasterizer;
}

const BlendState &State::getBlendState() const
{
    return mBlend;
}

const DepthStencilState &State::getDepthStencilState() const
{
    return mDepthStencil;
}

void State::setColorClearValue(float red, float green, float blue, float alpha)
{
    mColorClearValue.red = red;
    mColorClearValue.green = green;
    mColorClearValue.blue = blue;
    mColorClearValue.alpha = alpha;
    mDirtyBits.set(DIRTY_BIT_CLEAR_COLOR);
}

void State::setDepthClearValue(float depth)
{
    mDepthClearValue = depth;
    mDirtyBits.set(DIRTY_BIT_CLEAR_DEPTH);
}

void State::setStencilClearValue(int stencil)
{
    mStencilClearValue = stencil;
    mDirtyBits.set(DIRTY_BIT_CLEAR_STENCIL);
}

void State::setColorMask(bool red, bool green, bool blue, bool alpha)
{
    mBlend.colorMaskRed = red;
    mBlend.colorMaskGreen = green;
    mBlend.colorMaskBlue = blue;
    mBlend.colorMaskAlpha = alpha;
    mDirtyBits.set(DIRTY_BIT_COLOR_MASK);
}

void State::setDepthMask(bool mask)
{
    mDepthStencil.depthMask = mask;
    mDirtyBits.set(DIRTY_BIT_DEPTH_MASK);
}

bool State::isRasterizerDiscardEnabled() const
{
    return mRasterizer.rasterizerDiscard;
}

void State::setRasterizerDiscard(bool enabled)
{
    mRasterizer.rasterizerDiscard = enabled;
    mDirtyBits.set(DIRTY_BIT_RASTERIZER_DISCARD_ENABLED);
}

bool State::isCullFaceEnabled() const
{
    return mRasterizer.cullFace;
}

void State::setCullFace(bool enabled)
{
    mRasterizer.cullFace = enabled;
    mDirtyBits.set(DIRTY_BIT_CULL_FACE_ENABLED);
}

void State::setCullMode(GLenum mode)
{
    mRasterizer.cullMode = mode;
    mDirtyBits.set(DIRTY_BIT_CULL_FACE);
}

void State::setFrontFace(GLenum front)
{
    mRasterizer.frontFace = front;
    mDirtyBits.set(DIRTY_BIT_FRONT_FACE);
}

bool State::isDepthTestEnabled() const
{
    return mDepthStencil.depthTest;
}

void State::setDepthTest(bool enabled)
{
    mDepthStencil.depthTest = enabled;
    mDirtyBits.set(DIRTY_BIT_DEPTH_TEST_ENABLED);
}

void State::setDepthFunc(GLenum depthFunc)
{
     mDepthStencil.depthFunc = depthFunc;
     mDirtyBits.set(DIRTY_BIT_DEPTH_FUNC);
}

void State::setDepthRange(float zNear, float zFar)
{
    mNearZ = zNear;
    mFarZ = zFar;
    mDirtyBits.set(DIRTY_BIT_DEPTH_RANGE);
}

float State::getNearPlane() const
{
    return mNearZ;
}

float State::getFarPlane() const
{
    return mFarZ;
}

bool State::isBlendEnabled() const
{
    return mBlend.blend;
}

void State::setBlend(bool enabled)
{
    mBlend.blend = enabled;
    mDirtyBits.set(DIRTY_BIT_BLEND_ENABLED);
}

void State::setBlendFactors(GLenum sourceRGB, GLenum destRGB, GLenum sourceAlpha, GLenum destAlpha)
{
    mBlend.sourceBlendRGB = sourceRGB;
    mBlend.destBlendRGB = destRGB;
    mBlend.sourceBlendAlpha = sourceAlpha;
    mBlend.destBlendAlpha = destAlpha;
    mDirtyBits.set(DIRTY_BIT_BLEND_FUNCS);
}

void State::setBlendColor(float red, float green, float blue, float alpha)
{
    mBlendColor.red = red;
    mBlendColor.green = green;
    mBlendColor.blue = blue;
    mBlendColor.alpha = alpha;
    mDirtyBits.set(DIRTY_BIT_BLEND_COLOR);
}

void State::setBlendEquation(GLenum rgbEquation, GLenum alphaEquation)
{
    mBlend.blendEquationRGB = rgbEquation;
    mBlend.blendEquationAlpha = alphaEquation;
    mDirtyBits.set(DIRTY_BIT_BLEND_EQUATIONS);
}

const ColorF &State::getBlendColor() const
{
    return mBlendColor;
}

bool State::isStencilTestEnabled() const
{
    return mDepthStencil.stencilTest;
}

void State::setStencilTest(bool enabled)
{
    mDepthStencil.stencilTest = enabled;
    mDirtyBits.set(DIRTY_BIT_STENCIL_TEST_ENABLED);
}

void State::setStencilParams(GLenum stencilFunc, GLint stencilRef, GLuint stencilMask)
{
    mDepthStencil.stencilFunc = stencilFunc;
    mStencilRef = (stencilRef > 0) ? stencilRef : 0;
    mDepthStencil.stencilMask = stencilMask;
    mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_FRONT);
}

void State::setStencilBackParams(GLenum stencilBackFunc, GLint stencilBackRef, GLuint stencilBackMask)
{
    mDepthStencil.stencilBackFunc = stencilBackFunc;
    mStencilBackRef = (stencilBackRef > 0) ? stencilBackRef : 0;
    mDepthStencil.stencilBackMask = stencilBackMask;
    mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_BACK);
}

void State::setStencilWritemask(GLuint stencilWritemask)
{
    mDepthStencil.stencilWritemask = stencilWritemask;
    mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
}

void State::setStencilBackWritemask(GLuint stencilBackWritemask)
{
    mDepthStencil.stencilBackWritemask = stencilBackWritemask;
    mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_BACK);
}

void State::setStencilOperations(GLenum stencilFail, GLenum stencilPassDepthFail, GLenum stencilPassDepthPass)
{
    mDepthStencil.stencilFail = stencilFail;
    mDepthStencil.stencilPassDepthFail = stencilPassDepthFail;
    mDepthStencil.stencilPassDepthPass = stencilPassDepthPass;
    mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_FRONT);
}

void State::setStencilBackOperations(GLenum stencilBackFail, GLenum stencilBackPassDepthFail, GLenum stencilBackPassDepthPass)
{
    mDepthStencil.stencilBackFail = stencilBackFail;
    mDepthStencil.stencilBackPassDepthFail = stencilBackPassDepthFail;
    mDepthStencil.stencilBackPassDepthPass = stencilBackPassDepthPass;
    mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_BACK);
}

GLint State::getStencilRef() const
{
    return mStencilRef;
}

GLint State::getStencilBackRef() const
{
    return mStencilBackRef;
}

bool State::isPolygonOffsetFillEnabled() const
{
    return mRasterizer.polygonOffsetFill;
}

void State::setPolygonOffsetFill(bool enabled)
{
    mRasterizer.polygonOffsetFill = enabled;
    mDirtyBits.set(DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED);
}

void State::setPolygonOffsetParams(GLfloat factor, GLfloat units)
{
    // An application can pass NaN values here, so handle this gracefully
    mRasterizer.polygonOffsetFactor = factor != factor ? 0.0f : factor;
    mRasterizer.polygonOffsetUnits = units != units ? 0.0f : units;
    mDirtyBits.set(DIRTY_BIT_POLYGON_OFFSET);
}

bool State::isSampleAlphaToCoverageEnabled() const
{
    return mBlend.sampleAlphaToCoverage;
}

void State::setSampleAlphaToCoverage(bool enabled)
{
    mBlend.sampleAlphaToCoverage = enabled;
    mDirtyBits.set(DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED);
}

bool State::isSampleCoverageEnabled() const
{
    return mSampleCoverage;
}

void State::setSampleCoverage(bool enabled)
{
    mSampleCoverage = enabled;
    mDirtyBits.set(DIRTY_BIT_SAMPLE_COVERAGE_ENABLED);
}

void State::setSampleCoverageParams(GLclampf value, bool invert)
{
    mSampleCoverageValue = value;
    mSampleCoverageInvert = invert;
    mDirtyBits.set(DIRTY_BIT_SAMPLE_COVERAGE);
}

GLclampf State::getSampleCoverageValue() const
{
    return mSampleCoverageValue;
}

bool State::getSampleCoverageInvert() const
{
    return mSampleCoverageInvert;
}

bool State::isScissorTestEnabled() const
{
    return mScissorTest;
}

void State::setScissorTest(bool enabled)
{
    mScissorTest = enabled;
    mDirtyBits.set(DIRTY_BIT_SCISSOR_TEST_ENABLED);
}

void State::setScissorParams(GLint x, GLint y, GLsizei width, GLsizei height)
{
    mScissor.x = x;
    mScissor.y = y;
    mScissor.width = width;
    mScissor.height = height;
    mDirtyBits.set(DIRTY_BIT_SCISSOR);
}

const Rectangle &State::getScissor() const
{
    return mScissor;
}

bool State::isDitherEnabled() const
{
    return mBlend.dither;
}

void State::setDither(bool enabled)
{
    mBlend.dither = enabled;
    mDirtyBits.set(DIRTY_BIT_DITHER_ENABLED);
}

bool State::isPrimitiveRestartEnabled() const
{
    return mPrimitiveRestart;
}

void State::setPrimitiveRestart(bool enabled)
{
    mPrimitiveRestart = enabled;
    mDirtyBits.set(DIRTY_BIT_PRIMITIVE_RESTART_ENABLED);
}

void State::setEnableFeature(GLenum feature, bool enabled)
{
    switch (feature)
    {
      case GL_CULL_FACE:                     setCullFace(enabled);              break;
      case GL_POLYGON_OFFSET_FILL:           setPolygonOffsetFill(enabled);     break;
      case GL_SAMPLE_ALPHA_TO_COVERAGE:      setSampleAlphaToCoverage(enabled); break;
      case GL_SAMPLE_COVERAGE:               setSampleCoverage(enabled);        break;
      case GL_SCISSOR_TEST:                  setScissorTest(enabled);           break;
      case GL_STENCIL_TEST:                  setStencilTest(enabled);           break;
      case GL_DEPTH_TEST:                    setDepthTest(enabled);             break;
      case GL_BLEND:                         setBlend(enabled);                 break;
      case GL_DITHER:                        setDither(enabled);                break;
      case GL_PRIMITIVE_RESTART_FIXED_INDEX: setPrimitiveRestart(enabled);      break;
      case GL_RASTERIZER_DISCARD:            setRasterizerDiscard(enabled);     break;
      default:                               UNREACHABLE();
    }
}

bool State::getEnableFeature(GLenum feature)
{
    switch (feature)
    {
      case GL_CULL_FACE:                     return isCullFaceEnabled();
      case GL_POLYGON_OFFSET_FILL:           return isPolygonOffsetFillEnabled();
      case GL_SAMPLE_ALPHA_TO_COVERAGE:      return isSampleAlphaToCoverageEnabled();
      case GL_SAMPLE_COVERAGE:               return isSampleCoverageEnabled();
      case GL_SCISSOR_TEST:                  return isScissorTestEnabled();
      case GL_STENCIL_TEST:                  return isStencilTestEnabled();
      case GL_DEPTH_TEST:                    return isDepthTestEnabled();
      case GL_BLEND:                         return isBlendEnabled();
      case GL_DITHER:                        return isDitherEnabled();
      case GL_PRIMITIVE_RESTART_FIXED_INDEX: return isPrimitiveRestartEnabled();
      case GL_RASTERIZER_DISCARD:            return isRasterizerDiscardEnabled();
      default:                               UNREACHABLE(); return false;
    }
}

void State::setLineWidth(GLfloat width)
{
    mLineWidth = width;
    mDirtyBits.set(DIRTY_BIT_LINE_WIDTH);
}

float State::getLineWidth() const
{
    return mLineWidth;
}

void State::setGenerateMipmapHint(GLenum hint)
{
    mGenerateMipmapHint = hint;
    mDirtyBits.set(DIRTY_BIT_GENERATE_MIPMAP_HINT);
}

void State::setFragmentShaderDerivativeHint(GLenum hint)
{
    mFragmentShaderDerivativeHint = hint;
    mDirtyBits.set(DIRTY_BIT_SHADER_DERIVATIVE_HINT);
    // TODO: Propagate the hint to shader translator so we can write
    // ddx, ddx_coarse, or ddx_fine depending on the hint.
    // Ignore for now. It is valid for implementations to ignore hint.
}

void State::setViewportParams(GLint x, GLint y, GLsizei width, GLsizei height)
{
    mViewport.x = x;
    mViewport.y = y;
    mViewport.width = width;
    mViewport.height = height;
    mDirtyBits.set(DIRTY_BIT_VIEWPORT);
}

const Rectangle &State::getViewport() const
{
    return mViewport;
}

void State::setActiveSampler(unsigned int active)
{
    mActiveSampler = active;
}

unsigned int State::getActiveSampler() const
{
    return static_cast<unsigned int>(mActiveSampler);
}

void State::setSamplerTexture(GLenum type, Texture *texture)
{
    mSamplerTextures[type][mActiveSampler].set(texture);
}

Texture *State::getSamplerTexture(unsigned int sampler, GLenum type) const
{
    const auto it = mSamplerTextures.find(type);
    ASSERT(it != mSamplerTextures.end());
    ASSERT(sampler < it->second.size());
    return it->second[sampler].get();
}

GLuint State::getSamplerTextureId(unsigned int sampler, GLenum type) const
{
    const auto it = mSamplerTextures.find(type);
    ASSERT(it != mSamplerTextures.end());
    ASSERT(sampler < it->second.size());
    return it->second[sampler].id();
}

void State::detachTexture(const TextureMap &zeroTextures, GLuint texture)
{
    // Textures have a detach method on State rather than a simple
    // removeBinding, because the zero/null texture objects are managed
    // separately, and don't have to go through the Context's maps or
    // the ResourceManager.

    // [OpenGL ES 2.0.24] section 3.8 page 84:
    // If a texture object is deleted, it is as if all texture units which are bound to that texture object are
    // rebound to texture object zero

    for (TextureBindingMap::iterator bindingVec = mSamplerTextures.begin(); bindingVec != mSamplerTextures.end(); bindingVec++)
    {
        GLenum textureType = bindingVec->first;
        TextureBindingVector &textureVector = bindingVec->second;
        for (size_t textureIdx = 0; textureIdx < textureVector.size(); textureIdx++)
        {
            BindingPointer<Texture> &binding = textureVector[textureIdx];
            if (binding.id() == texture)
            {
                auto it = zeroTextures.find(textureType);
                ASSERT(it != zeroTextures.end());
                // Zero textures are the "default" textures instead of NULL
                binding.set(it->second.get());
            }
        }
    }

    // [OpenGL ES 2.0.24] section 4.4 page 112:
    // If a texture object is deleted while its image is attached to the currently bound framebuffer, then it is
    // as if Texture2DAttachment had been called, with a texture of 0, for each attachment point to which this
    // image was attached in the currently bound framebuffer.

    if (mReadFramebuffer)
    {
        mReadFramebuffer->detachTexture(texture);
    }

    if (mDrawFramebuffer)
    {
        mDrawFramebuffer->detachTexture(texture);
    }
}

void State::initializeZeroTextures(const TextureMap &zeroTextures)
{
    for (const auto &zeroTexture : zeroTextures)
    {
        auto &samplerTextureArray = mSamplerTextures[zeroTexture.first];

        for (size_t textureUnit = 0; textureUnit < samplerTextureArray.size(); ++textureUnit)
        {
            samplerTextureArray[textureUnit].set(zeroTexture.second.get());
        }
    }
}

void State::setSamplerBinding(GLuint textureUnit, Sampler *sampler)
{
    mSamplers[textureUnit].set(sampler);
}

GLuint State::getSamplerId(GLuint textureUnit) const
{
    ASSERT(textureUnit < mSamplers.size());
    return mSamplers[textureUnit].id();
}

Sampler *State::getSampler(GLuint textureUnit) const
{
    return mSamplers[textureUnit].get();
}

void State::detachSampler(GLuint sampler)
{
    // [OpenGL ES 3.0.2] section 3.8.2 pages 123-124:
    // If a sampler object that is currently bound to one or more texture units is
    // deleted, it is as though BindSampler is called once for each texture unit to
    // which the sampler is bound, with unit set to the texture unit and sampler set to zero.
    for (size_t textureUnit = 0; textureUnit < mSamplers.size(); textureUnit++)
    {
        BindingPointer<Sampler> &samplerBinding = mSamplers[textureUnit];
        if (samplerBinding.id() == sampler)
        {
            samplerBinding.set(NULL);
        }
    }
}

void State::setRenderbufferBinding(Renderbuffer *renderbuffer)
{
    mRenderbuffer.set(renderbuffer);
}

GLuint State::getRenderbufferId() const
{
    return mRenderbuffer.id();
}

Renderbuffer *State::getCurrentRenderbuffer()
{
    return mRenderbuffer.get();
}

void State::detachRenderbuffer(GLuint renderbuffer)
{
    // [OpenGL ES 2.0.24] section 4.4 page 109:
    // If a renderbuffer that is currently bound to RENDERBUFFER is deleted, it is as though BindRenderbuffer
    // had been executed with the target RENDERBUFFER and name of zero.

    if (mRenderbuffer.id() == renderbuffer)
    {
        mRenderbuffer.set(NULL);
    }

    // [OpenGL ES 2.0.24] section 4.4 page 111:
    // If a renderbuffer object is deleted while its image is attached to the currently bound framebuffer,
    // then it is as if FramebufferRenderbuffer had been called, with a renderbuffer of 0, for each attachment
    // point to which this image was attached in the currently bound framebuffer.

    Framebuffer *readFramebuffer = mReadFramebuffer;
    Framebuffer *drawFramebuffer = mDrawFramebuffer;

    if (readFramebuffer)
    {
        readFramebuffer->detachRenderbuffer(renderbuffer);
    }

    if (drawFramebuffer && drawFramebuffer != readFramebuffer)
    {
        drawFramebuffer->detachRenderbuffer(renderbuffer);
    }

}

void State::setReadFramebufferBinding(Framebuffer *framebuffer)
{
    mReadFramebuffer = framebuffer;
}

void State::setDrawFramebufferBinding(Framebuffer *framebuffer)
{
    mDrawFramebuffer = framebuffer;
}

Framebuffer *State::getTargetFramebuffer(GLenum target) const
{
    switch (target)
    {
    case GL_READ_FRAMEBUFFER_ANGLE:  return mReadFramebuffer;
    case GL_DRAW_FRAMEBUFFER_ANGLE:
    case GL_FRAMEBUFFER:             return mDrawFramebuffer;
    default:                         UNREACHABLE(); return NULL;
    }
}

Framebuffer *State::getReadFramebuffer()
{
    return mReadFramebuffer;
}

Framebuffer *State::getDrawFramebuffer()
{
    return mDrawFramebuffer;
}

const Framebuffer *State::getReadFramebuffer() const
{
    return mReadFramebuffer;
}

const Framebuffer *State::getDrawFramebuffer() const
{
    return mDrawFramebuffer;
}

bool State::removeReadFramebufferBinding(GLuint framebuffer)
{
    if (mReadFramebuffer != nullptr &&
        mReadFramebuffer->id() == framebuffer)
    {
        mReadFramebuffer = NULL;
        return true;
    }

    return false;
}

bool State::removeDrawFramebufferBinding(GLuint framebuffer)
{
    if (mReadFramebuffer != nullptr &&
        mDrawFramebuffer->id() == framebuffer)
    {
        mDrawFramebuffer = NULL;
        return true;
    }

    return false;
}

void State::setVertexArrayBinding(VertexArray *vertexArray)
{
    mVertexArray = vertexArray;
    mDirtyBits.set(DIRTY_BIT_VERTEX_ARRAY_BINDING);
    mDirtyBits.set(DIRTY_BIT_VERTEX_ARRAY_OBJECT);
}

GLuint State::getVertexArrayId() const
{
    ASSERT(mVertexArray != NULL);
    return mVertexArray->id();
}

VertexArray *State::getVertexArray() const
{
    ASSERT(mVertexArray != NULL);
    return mVertexArray;
}

bool State::removeVertexArrayBinding(GLuint vertexArray)
{
    if (mVertexArray->id() == vertexArray)
    {
        mVertexArray = NULL;
        mDirtyBits.set(DIRTY_BIT_VERTEX_ARRAY_BINDING);
        mDirtyBits.set(DIRTY_BIT_VERTEX_ARRAY_OBJECT);
        return true;
    }

    return false;
}

void State::setProgram(Program *newProgram)
{
    if (mProgram != newProgram)
    {
        if (mProgram)
        {
            mProgram->release();
        }

        mProgram = newProgram;

        if (mProgram)
        {
            newProgram->addRef();
        }
    }
}

Program *State::getProgram() const
{
    return mProgram;
}

void State::setTransformFeedbackBinding(TransformFeedback *transformFeedback)
{
    mTransformFeedback.set(transformFeedback);
}

TransformFeedback *State::getCurrentTransformFeedback() const
{
    return mTransformFeedback.get();
}

bool State::isTransformFeedbackActiveUnpaused() const
{
    gl::TransformFeedback *curTransformFeedback = getCurrentTransformFeedback();
    return curTransformFeedback && curTransformFeedback->isActive() && !curTransformFeedback->isPaused();
}

void State::detachTransformFeedback(GLuint transformFeedback)
{
    if (mTransformFeedback.id() == transformFeedback)
    {
        mTransformFeedback.set(NULL);
    }
}

bool State::isQueryActive() const
{
    for (State::ActiveQueryMap::const_iterator i = mActiveQueries.begin();
        i != mActiveQueries.end(); i++)
    {
        if (i->second.get() != NULL)
        {
            return true;
        }
    }

    return false;
}

void State::setActiveQuery(GLenum target, Query *query)
{
    mActiveQueries[target].set(query);
}

GLuint State::getActiveQueryId(GLenum target) const
{
    const Query *query = getActiveQuery(target);
    return (query ? query->id() : 0u);
}

Query *State::getActiveQuery(GLenum target) const
{
    const auto it = mActiveQueries.find(target);

    // All query types should already exist in the activeQueries map
    ASSERT(it != mActiveQueries.end());

    return it->second.get();
}

void State::setArrayBufferBinding(Buffer *buffer)
{
    mArrayBuffer.set(buffer);
}

GLuint State::getArrayBufferId() const
{
    return mArrayBuffer.id();
}

bool State::removeArrayBufferBinding(GLuint buffer)
{
    if (mArrayBuffer.id() == buffer)
    {
        mArrayBuffer.set(NULL);
        return true;
    }

    return false;
}

void State::setGenericUniformBufferBinding(Buffer *buffer)
{
    mGenericUniformBuffer.set(buffer);
}

void State::setIndexedUniformBufferBinding(GLuint index, Buffer *buffer, GLintptr offset, GLsizeiptr size)
{
    mUniformBuffers[index].set(buffer, offset, size);
}

GLuint State::getIndexedUniformBufferId(GLuint index) const
{
    ASSERT(static_cast<size_t>(index) < mUniformBuffers.size());

    return mUniformBuffers[index].id();
}

Buffer *State::getIndexedUniformBuffer(GLuint index) const
{
    ASSERT(static_cast<size_t>(index) < mUniformBuffers.size());

    return mUniformBuffers[index].get();
}

GLintptr State::getIndexedUniformBufferOffset(GLuint index) const
{
    ASSERT(static_cast<size_t>(index) < mUniformBuffers.size());

    return mUniformBuffers[index].getOffset();
}

GLsizeiptr State::getIndexedUniformBufferSize(GLuint index) const
{
    ASSERT(static_cast<size_t>(index) < mUniformBuffers.size());

    return mUniformBuffers[index].getSize();
}

void State::setCopyReadBufferBinding(Buffer *buffer)
{
    mCopyReadBuffer.set(buffer);
}

void State::setCopyWriteBufferBinding(Buffer *buffer)
{
    mCopyWriteBuffer.set(buffer);
}

void State::setPixelPackBufferBinding(Buffer *buffer)
{
    mPack.pixelBuffer.set(buffer);
}

void State::setPixelUnpackBufferBinding(Buffer *buffer)
{
    mUnpack.pixelBuffer.set(buffer);
}

Buffer *State::getTargetBuffer(GLenum target) const
{
    switch (target)
    {
      case GL_ARRAY_BUFFER:              return mArrayBuffer.get();
      case GL_COPY_READ_BUFFER:          return mCopyReadBuffer.get();
      case GL_COPY_WRITE_BUFFER:         return mCopyWriteBuffer.get();
      case GL_ELEMENT_ARRAY_BUFFER:      return getVertexArray()->getElementArrayBuffer().get();
      case GL_PIXEL_PACK_BUFFER:         return mPack.pixelBuffer.get();
      case GL_PIXEL_UNPACK_BUFFER:       return mUnpack.pixelBuffer.get();
      case GL_TRANSFORM_FEEDBACK_BUFFER: return mTransformFeedback->getGenericBuffer().get();
      case GL_UNIFORM_BUFFER:            return mGenericUniformBuffer.get();
      default: UNREACHABLE();            return NULL;
    }
}

void State::setEnableVertexAttribArray(unsigned int attribNum, bool enabled)
{
    getVertexArray()->enableAttribute(attribNum, enabled);
    mDirtyBits.set(DIRTY_BIT_VERTEX_ARRAY_OBJECT);
}

void State::setVertexAttribf(GLuint index, const GLfloat values[4])
{
    ASSERT(static_cast<size_t>(index) < mVertexAttribCurrentValues.size());
    mVertexAttribCurrentValues[index].setFloatValues(values);
    mDirtyBits.set(DIRTY_BIT_CURRENT_VALUE_0 + index);
}

void State::setVertexAttribu(GLuint index, const GLuint values[4])
{
    ASSERT(static_cast<size_t>(index) < mVertexAttribCurrentValues.size());
    mVertexAttribCurrentValues[index].setUnsignedIntValues(values);
    mDirtyBits.set(DIRTY_BIT_CURRENT_VALUE_0 + index);
}

void State::setVertexAttribi(GLuint index, const GLint values[4])
{
    ASSERT(static_cast<size_t>(index) < mVertexAttribCurrentValues.size());
    mVertexAttribCurrentValues[index].setIntValues(values);
    mDirtyBits.set(DIRTY_BIT_CURRENT_VALUE_0 + index);
}

void State::setVertexAttribState(unsigned int attribNum,
                                 Buffer *boundBuffer,
                                 GLint size,
                                 GLenum type,
                                 bool normalized,
                                 bool pureInteger,
                                 GLsizei stride,
                                 const void *pointer)
{
    getVertexArray()->setAttributeState(attribNum, boundBuffer, size, type, normalized, pureInteger, stride, pointer);
    mDirtyBits.set(DIRTY_BIT_VERTEX_ARRAY_OBJECT);
}

void State::setVertexAttribDivisor(GLuint index, GLuint divisor)
{
    getVertexArray()->setVertexAttribDivisor(index, divisor);
    mDirtyBits.set(DIRTY_BIT_VERTEX_ARRAY_OBJECT);
}

const VertexAttribCurrentValueData &State::getVertexAttribCurrentValue(unsigned int attribNum) const
{
    ASSERT(static_cast<size_t>(attribNum) < mVertexAttribCurrentValues.size());
    return mVertexAttribCurrentValues[attribNum];
}

const void *State::getVertexAttribPointer(unsigned int attribNum) const
{
    return getVertexArray()->getVertexAttribute(attribNum).pointer;
}

void State::setPackAlignment(GLint alignment)
{
    mPack.alignment = alignment;
    mDirtyBits.set(DIRTY_BIT_PACK_ALIGNMENT);
}

GLint State::getPackAlignment() const
{
    return mPack.alignment;
}

void State::setPackReverseRowOrder(bool reverseRowOrder)
{
    mPack.reverseRowOrder = reverseRowOrder;
    mDirtyBits.set(DIRTY_BIT_PACK_REVERSE_ROW_ORDER);
}

bool State::getPackReverseRowOrder() const
{
    return mPack.reverseRowOrder;
}

const PixelPackState &State::getPackState() const
{
    return mPack;
}

PixelPackState &State::getPackState()
{
    return mPack;
}

void State::setUnpackAlignment(GLint alignment)
{
    mUnpack.alignment = alignment;
    mDirtyBits.set(DIRTY_BIT_UNPACK_ALIGNMENT);
}

GLint State::getUnpackAlignment() const
{
    return mUnpack.alignment;
}

void State::setUnpackRowLength(GLint rowLength)
{
    mUnpack.rowLength = rowLength;
    mDirtyBits.set(DIRTY_BIT_UNPACK_ROW_LENGTH);
}

GLint State::getUnpackRowLength() const
{
    return mUnpack.rowLength;
}

const PixelUnpackState &State::getUnpackState() const
{
    return mUnpack;
}

PixelUnpackState &State::getUnpackState()
{
    return mUnpack;
}

void State::getBooleanv(GLenum pname, GLboolean *params)
{
    switch (pname)
    {
      case GL_SAMPLE_COVERAGE_INVERT:    *params = mSampleCoverageInvert;         break;
      case GL_DEPTH_WRITEMASK:           *params = mDepthStencil.depthMask;       break;
      case GL_COLOR_WRITEMASK:
        params[0] = mBlend.colorMaskRed;
        params[1] = mBlend.colorMaskGreen;
        params[2] = mBlend.colorMaskBlue;
        params[3] = mBlend.colorMaskAlpha;
        break;
      case GL_CULL_FACE:                 *params = mRasterizer.cullFace;          break;
      case GL_POLYGON_OFFSET_FILL:       *params = mRasterizer.polygonOffsetFill; break;
      case GL_SAMPLE_ALPHA_TO_COVERAGE:  *params = mBlend.sampleAlphaToCoverage;  break;
      case GL_SAMPLE_COVERAGE:           *params = mSampleCoverage;               break;
      case GL_SCISSOR_TEST:              *params = mScissorTest;                  break;
      case GL_STENCIL_TEST:              *params = mDepthStencil.stencilTest;     break;
      case GL_DEPTH_TEST:                *params = mDepthStencil.depthTest;       break;
      case GL_BLEND:                     *params = mBlend.blend;                  break;
      case GL_DITHER:                    *params = mBlend.dither;                 break;
      case GL_TRANSFORM_FEEDBACK_ACTIVE: *params = getCurrentTransformFeedback()->isActive() ? GL_TRUE : GL_FALSE; break;
      case GL_TRANSFORM_FEEDBACK_PAUSED: *params = getCurrentTransformFeedback()->isPaused() ? GL_TRUE : GL_FALSE; break;
      default:
        UNREACHABLE();
        break;
    }
}

void State::getFloatv(GLenum pname, GLfloat *params)
{
    // Please note: DEPTH_CLEAR_VALUE is included in our internal getFloatv implementation
    // because it is stored as a float, despite the fact that the GL ES 2.0 spec names
    // GetIntegerv as its native query function. As it would require conversion in any
    // case, this should make no difference to the calling application.
    switch (pname)
    {
      case GL_LINE_WIDTH:               *params = mLineWidth;                         break;
      case GL_SAMPLE_COVERAGE_VALUE:    *params = mSampleCoverageValue;               break;
      case GL_DEPTH_CLEAR_VALUE:        *params = mDepthClearValue;                   break;
      case GL_POLYGON_OFFSET_FACTOR:    *params = mRasterizer.polygonOffsetFactor;    break;
      case GL_POLYGON_OFFSET_UNITS:     *params = mRasterizer.polygonOffsetUnits;     break;
      case GL_DEPTH_RANGE:
        params[0] = mNearZ;
        params[1] = mFarZ;
        break;
      case GL_COLOR_CLEAR_VALUE:
        params[0] = mColorClearValue.red;
        params[1] = mColorClearValue.green;
        params[2] = mColorClearValue.blue;
        params[3] = mColorClearValue.alpha;
        break;
      case GL_BLEND_COLOR:
        params[0] = mBlendColor.red;
        params[1] = mBlendColor.green;
        params[2] = mBlendColor.blue;
        params[3] = mBlendColor.alpha;
        break;
      default:
        UNREACHABLE();
        break;
    }
}

void State::getIntegerv(const gl::Data &data, GLenum pname, GLint *params)
{
    if (pname >= GL_DRAW_BUFFER0_EXT && pname <= GL_DRAW_BUFFER15_EXT)
    {
        unsigned int colorAttachment = (pname - GL_DRAW_BUFFER0_EXT);
        ASSERT(colorAttachment < mMaxDrawBuffers);
        Framebuffer *framebuffer = mDrawFramebuffer;
        *params = framebuffer->getDrawBufferState(colorAttachment);
        return;
    }

    // Please note: DEPTH_CLEAR_VALUE is not included in our internal getIntegerv implementation
    // because it is stored as a float, despite the fact that the GL ES 2.0 spec names
    // GetIntegerv as its native query function. As it would require conversion in any
    // case, this should make no difference to the calling application. You may find it in
    // State::getFloatv.
    switch (pname)
    {
      case GL_ARRAY_BUFFER_BINDING:                     *params = mArrayBuffer.id();                              break;
      case GL_ELEMENT_ARRAY_BUFFER_BINDING:             *params = getVertexArray()->getElementArrayBuffer().id(); break;
        //case GL_FRAMEBUFFER_BINDING:                    // now equivalent to GL_DRAW_FRAMEBUFFER_BINDING_ANGLE
      case GL_DRAW_FRAMEBUFFER_BINDING_ANGLE:           *params = mDrawFramebuffer->id();                         break;
      case GL_READ_FRAMEBUFFER_BINDING_ANGLE:           *params = mReadFramebuffer->id();                         break;
      case GL_RENDERBUFFER_BINDING:                     *params = mRenderbuffer.id();                             break;
      case GL_VERTEX_ARRAY_BINDING:                     *params = mVertexArray->id();                             break;
      case GL_CURRENT_PROGRAM:                          *params = mProgram ? mProgram->id() : 0;                  break;
      case GL_PACK_ALIGNMENT:                           *params = mPack.alignment;                                break;
      case GL_PACK_REVERSE_ROW_ORDER_ANGLE:             *params = mPack.reverseRowOrder;                          break;
      case GL_UNPACK_ALIGNMENT:                         *params = mUnpack.alignment;                              break;
      case GL_UNPACK_ROW_LENGTH:                        *params = mUnpack.rowLength;                              break;
      case GL_GENERATE_MIPMAP_HINT:                     *params = mGenerateMipmapHint;                            break;
      case GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES:      *params = mFragmentShaderDerivativeHint;                  break;
      case GL_ACTIVE_TEXTURE:
          *params = (static_cast<GLint>(mActiveSampler) + GL_TEXTURE0);
          break;
      case GL_STENCIL_FUNC:                             *params = mDepthStencil.stencilFunc;                      break;
      case GL_STENCIL_REF:                              *params = mStencilRef;                                    break;
      case GL_STENCIL_VALUE_MASK:                       *params = clampToInt(mDepthStencil.stencilMask);          break;
      case GL_STENCIL_BACK_FUNC:                        *params = mDepthStencil.stencilBackFunc;                  break;
      case GL_STENCIL_BACK_REF:                         *params = mStencilBackRef;                                break;
      case GL_STENCIL_BACK_VALUE_MASK:                  *params = clampToInt(mDepthStencil.stencilBackMask);      break;
      case GL_STENCIL_FAIL:                             *params = mDepthStencil.stencilFail;                      break;
      case GL_STENCIL_PASS_DEPTH_FAIL:                  *params = mDepthStencil.stencilPassDepthFail;             break;
      case GL_STENCIL_PASS_DEPTH_PASS:                  *params = mDepthStencil.stencilPassDepthPass;             break;
      case GL_STENCIL_BACK_FAIL:                        *params = mDepthStencil.stencilBackFail;                  break;
      case GL_STENCIL_BACK_PASS_DEPTH_FAIL:             *params = mDepthStencil.stencilBackPassDepthFail;         break;
      case GL_STENCIL_BACK_PASS_DEPTH_PASS:             *params = mDepthStencil.stencilBackPassDepthPass;         break;
      case GL_DEPTH_FUNC:                               *params = mDepthStencil.depthFunc;                        break;
      case GL_BLEND_SRC_RGB:                            *params = mBlend.sourceBlendRGB;                          break;
      case GL_BLEND_SRC_ALPHA:                          *params = mBlend.sourceBlendAlpha;                        break;
      case GL_BLEND_DST_RGB:                            *params = mBlend.destBlendRGB;                            break;
      case GL_BLEND_DST_ALPHA:                          *params = mBlend.destBlendAlpha;                          break;
      case GL_BLEND_EQUATION_RGB:                       *params = mBlend.blendEquationRGB;                        break;
      case GL_BLEND_EQUATION_ALPHA:                     *params = mBlend.blendEquationAlpha;                      break;
      case GL_STENCIL_WRITEMASK:                        *params = clampToInt(mDepthStencil.stencilWritemask);     break;
      case GL_STENCIL_BACK_WRITEMASK:                   *params = clampToInt(mDepthStencil.stencilBackWritemask); break;
      case GL_STENCIL_CLEAR_VALUE:                      *params = mStencilClearValue;                             break;
      case GL_IMPLEMENTATION_COLOR_READ_TYPE:           *params = mReadFramebuffer->getImplementationColorReadType();   break;
      case GL_IMPLEMENTATION_COLOR_READ_FORMAT:         *params = mReadFramebuffer->getImplementationColorReadFormat(); break;
      case GL_SAMPLE_BUFFERS:
      case GL_SAMPLES:
        {
            gl::Framebuffer *framebuffer = mDrawFramebuffer;
            if (framebuffer->checkStatus(data) == GL_FRAMEBUFFER_COMPLETE)
            {
                switch (pname)
                {
                  case GL_SAMPLE_BUFFERS:
                    if (framebuffer->getSamples(data) != 0)
                    {
                        *params = 1;
                    }
                    else
                    {
                        *params = 0;
                    }
                    break;
                  case GL_SAMPLES:
                    *params = framebuffer->getSamples(data);
                    break;
                }
            }
            else
            {
                *params = 0;
            }
        }
        break;
      case GL_VIEWPORT:
        params[0] = mViewport.x;
        params[1] = mViewport.y;
        params[2] = mViewport.width;
        params[3] = mViewport.height;
        break;
      case GL_SCISSOR_BOX:
        params[0] = mScissor.x;
        params[1] = mScissor.y;
        params[2] = mScissor.width;
        params[3] = mScissor.height;
        break;
      case GL_CULL_FACE_MODE:                   *params = mRasterizer.cullMode;   break;
      case GL_FRONT_FACE:                       *params = mRasterizer.frontFace;  break;
      case GL_RED_BITS:
      case GL_GREEN_BITS:
      case GL_BLUE_BITS:
      case GL_ALPHA_BITS:
        {
            gl::Framebuffer *framebuffer = getDrawFramebuffer();
            const gl::FramebufferAttachment *colorbuffer = framebuffer->getFirstColorbuffer();

            if (colorbuffer)
            {
                switch (pname)
                {
                case GL_RED_BITS:   *params = colorbuffer->getRedSize();      break;
                case GL_GREEN_BITS: *params = colorbuffer->getGreenSize();    break;
                case GL_BLUE_BITS:  *params = colorbuffer->getBlueSize();     break;
                case GL_ALPHA_BITS: *params = colorbuffer->getAlphaSize();    break;
                }
            }
            else
            {
                *params = 0;
            }
        }
        break;
      case GL_DEPTH_BITS:
        {
            const gl::Framebuffer *framebuffer = getDrawFramebuffer();
            const gl::FramebufferAttachment *depthbuffer = framebuffer->getDepthbuffer();

            if (depthbuffer)
            {
                *params = depthbuffer->getDepthSize();
            }
            else
            {
                *params = 0;
            }
        }
        break;
      case GL_STENCIL_BITS:
        {
            const gl::Framebuffer *framebuffer = getDrawFramebuffer();
            const gl::FramebufferAttachment *stencilbuffer = framebuffer->getStencilbuffer();

            if (stencilbuffer)
            {
                *params = stencilbuffer->getStencilSize();
            }
            else
            {
                *params = 0;
            }
        }
        break;
      case GL_TEXTURE_BINDING_2D:
        ASSERT(mActiveSampler < mMaxCombinedTextureImageUnits);
        *params = getSamplerTextureId(static_cast<unsigned int>(mActiveSampler), GL_TEXTURE_2D);
        break;
      case GL_TEXTURE_BINDING_CUBE_MAP:
        ASSERT(mActiveSampler < mMaxCombinedTextureImageUnits);
        *params =
            getSamplerTextureId(static_cast<unsigned int>(mActiveSampler), GL_TEXTURE_CUBE_MAP);
        break;
      case GL_TEXTURE_BINDING_3D:
        ASSERT(mActiveSampler < mMaxCombinedTextureImageUnits);
        *params = getSamplerTextureId(static_cast<unsigned int>(mActiveSampler), GL_TEXTURE_3D);
        break;
      case GL_TEXTURE_BINDING_2D_ARRAY:
        ASSERT(mActiveSampler < mMaxCombinedTextureImageUnits);
        *params =
            getSamplerTextureId(static_cast<unsigned int>(mActiveSampler), GL_TEXTURE_2D_ARRAY);
        break;
      case GL_UNIFORM_BUFFER_BINDING:
        *params = mGenericUniformBuffer.id();
        break;
      case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
        *params = mTransformFeedback->getGenericBuffer().id();
        break;
      case GL_COPY_READ_BUFFER_BINDING:
        *params = mCopyReadBuffer.id();
        break;
      case GL_COPY_WRITE_BUFFER_BINDING:
        *params = mCopyWriteBuffer.id();
        break;
      case GL_PIXEL_PACK_BUFFER_BINDING:
        *params = mPack.pixelBuffer.id();
        break;
      case GL_PIXEL_UNPACK_BUFFER_BINDING:
        *params = mUnpack.pixelBuffer.id();
        break;
      default:
        UNREACHABLE();
        break;
    }
}

bool State::getIndexedIntegerv(GLenum target, GLuint index, GLint *data)
{
    switch (target)
    {
      case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
        if (static_cast<size_t>(index) < mTransformFeedback->getIndexedBufferCount())
        {
            *data = mTransformFeedback->getIndexedBuffer(index).id();
        }
        break;
      case GL_UNIFORM_BUFFER_BINDING:
        if (static_cast<size_t>(index) < mUniformBuffers.size())
        {
            *data = mUniformBuffers[index].id();
        }
        break;
      default:
        return false;
    }

    return true;
}

bool State::getIndexedInteger64v(GLenum target, GLuint index, GLint64 *data)
{
    switch (target)
    {
      case GL_TRANSFORM_FEEDBACK_BUFFER_START:
        if (static_cast<size_t>(index) < mTransformFeedback->getIndexedBufferCount())
        {
            *data = mTransformFeedback->getIndexedBuffer(index).getOffset();
        }
        break;
      case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
        if (static_cast<size_t>(index) < mTransformFeedback->getIndexedBufferCount())
        {
            *data = mTransformFeedback->getIndexedBuffer(index).getSize();
        }
        break;
      case GL_UNIFORM_BUFFER_START:
        if (static_cast<size_t>(index) < mUniformBuffers.size())
        {
            *data = mUniformBuffers[index].getOffset();
        }
        break;
      case GL_UNIFORM_BUFFER_SIZE:
        if (static_cast<size_t>(index) < mUniformBuffers.size())
        {
            *data = mUniformBuffers[index].getSize();
        }
        break;
      default:
        return false;
    }

    return true;
}

bool State::hasMappedBuffer(GLenum target) const
{
    if (target == GL_ARRAY_BUFFER)
    {
        const VertexArray *vao = getVertexArray();
        const auto &vertexAttribs = vao->getVertexAttributes();
        size_t maxEnabledAttrib = vao->getMaxEnabledAttribute();
        for (size_t attribIndex = 0; attribIndex < maxEnabledAttrib; attribIndex++)
        {
            const gl::VertexAttribute &vertexAttrib = vertexAttribs[attribIndex];
            gl::Buffer *boundBuffer = vertexAttrib.buffer.get();
            if (vertexAttrib.enabled && boundBuffer && boundBuffer->isMapped())
            {
                return true;
            }
        }

        return false;
    }
    else
    {
        Buffer *buffer = getTargetBuffer(target);
        return (buffer && buffer->isMapped());
    }
}

}

//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderergl_utils.cpp: Conversion functions and other utility routines
// specific to the OpenGL renderer.

#include "libANGLE/renderer/gl/renderergl_utils.h"

#include <limits>

#include "libANGLE/Caps.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/WorkaroundsGL.h"
#include "libANGLE/renderer/gl/formatutilsgl.h"

#include <algorithm>
#include <sstream>

namespace rx
{
static VendorID GetVendorID(const FunctionsGL *functions)
{
    std::string nativeVendorString(reinterpret_cast<const char *>(functions->getString(GL_VENDOR)));
    if (nativeVendorString.find("Intel") != std::string::npos)
    {
        return VENDOR_ID_INTEL;
    }
    else if (nativeVendorString.find("NVIDIA") != std::string::npos)
    {
        return VENDOR_ID_NVIDIA;
    }
    else if (nativeVendorString.find("ATI") != std::string::npos ||
             nativeVendorString.find("AMD") != std::string::npos)
    {
        return VENDOR_ID_AMD;
    }
    else
    {
        return VENDOR_ID_UNKNOWN;
    }
}

namespace nativegl_gl
{

static bool MeetsRequirements(const FunctionsGL *functions, const nativegl::SupportRequirement &requirements)
{
    for (const std::string &extension : requirements.requiredExtensions)
    {
        if (!functions->hasExtension(extension))
        {
            return false;
        }
    }

    if (functions->version >= requirements.version)
    {
        return true;
    }
    else if (!requirements.versionExtensions.empty())
    {
        for (const std::string &extension : requirements.versionExtensions)
        {
            if (!functions->hasExtension(extension))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

static gl::TextureCaps GenerateTextureFormatCaps(const FunctionsGL *functions, GLenum internalFormat)
{
    gl::TextureCaps textureCaps;

    const nativegl::InternalFormat &formatInfo = nativegl::GetInternalFormatInfo(internalFormat, functions->standard);
    textureCaps.texturable = MeetsRequirements(functions, formatInfo.texture);
    textureCaps.filterable = textureCaps.texturable && MeetsRequirements(functions, formatInfo.filter);
    textureCaps.renderable = MeetsRequirements(functions, formatInfo.framebufferAttachment);

    // glGetInternalformativ is not available until version 4.2 but may be available through the 3.0
    // extension GL_ARB_internalformat_query
    if (textureCaps.renderable && functions->getInternalformativ)
    {
        GLint numSamples = 0;
        functions->getInternalformativ(GL_RENDERBUFFER, internalFormat, GL_NUM_SAMPLE_COUNTS, 1, &numSamples);

        if (numSamples > 0)
        {
            std::vector<GLint> samples(numSamples);
            functions->getInternalformativ(GL_RENDERBUFFER, internalFormat, GL_SAMPLES,
                                           static_cast<GLsizei>(samples.size()), &samples[0]);
            for (size_t sampleIndex = 0; sampleIndex < samples.size(); sampleIndex++)
            {
                textureCaps.sampleCounts.insert(samples[sampleIndex]);
            }
        }
    }

    return textureCaps;
}

static GLint QuerySingleGLInt(const FunctionsGL *functions, GLenum name)
{
    GLint result;
    functions->getIntegerv(name, &result);
    return result;
}

static GLint QueryGLIntRange(const FunctionsGL *functions, GLenum name, size_t index)
{
    GLint result[2];
    functions->getIntegerv(name, result);
    return result[index];
}

static GLint64 QuerySingleGLInt64(const FunctionsGL *functions, GLenum name)
{
    GLint64 result;
    functions->getInteger64v(name, &result);
    return result;
}

static GLfloat QuerySingleGLFloat(const FunctionsGL *functions, GLenum name)
{
    GLfloat result;
    functions->getFloatv(name, &result);
    return result;
}

static GLfloat QueryGLFloatRange(const FunctionsGL *functions, GLenum name, size_t index)
{
    GLfloat result[2];
    functions->getFloatv(name, result);
    return result[index];
}

static gl::TypePrecision QueryTypePrecision(const FunctionsGL *functions, GLenum shaderType, GLenum precisionType)
{
    gl::TypePrecision precision;
    functions->getShaderPrecisionFormat(shaderType, precisionType, precision.range, &precision.precision);
    return precision;
}

static void LimitVersion(gl::Version *curVersion, const gl::Version &maxVersion)
{
    if (*curVersion >= maxVersion)
    {
        *curVersion = maxVersion;
    }
}

void GenerateCaps(const FunctionsGL *functions, gl::Caps *caps, gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions, gl::Version *maxSupportedESVersion)
{
    // Texture format support checks
    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        gl::TextureCaps textureCaps = GenerateTextureFormatCaps(functions, internalFormat);
        textureCapsMap->insert(internalFormat, textureCaps);

        if (gl::GetInternalFormatInfo(internalFormat).compressed)
        {
            caps->compressedTextureFormats.push_back(internalFormat);
        }
    }

    // Start by assuming ES3 support and work down
    *maxSupportedESVersion = gl::Version(3, 0);

    // Table 6.28, implementation dependent values
    if (functions->isAtLeastGL(gl::Version(4, 3)) || functions->hasGLExtension("GL_ARB_ES3_compatibility") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxElementIndex = QuerySingleGLInt64(functions, GL_MAX_ELEMENT_INDEX);
    }
    else
    {
        // Doesn't affect ES3 support, can use a pre-defined limit
        caps->maxElementIndex = static_cast<GLint64>(std::numeric_limits<unsigned int>::max());
    }

    if (functions->isAtLeastGL(gl::Version(1, 2)) ||
        functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_OES_texture_3D"))
    {
        caps->max3DTextureSize = QuerySingleGLInt(functions, GL_MAX_3D_TEXTURE_SIZE);
    }
    else
    {
        // Can't support ES3 without 3D textures
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    caps->max2DTextureSize = QuerySingleGLInt(functions, GL_MAX_TEXTURE_SIZE); // GL 1.0 / ES 2.0
    caps->maxCubeMapTextureSize = QuerySingleGLInt(functions, GL_MAX_CUBE_MAP_TEXTURE_SIZE); // GL 1.3 / ES 2.0

    if (functions->isAtLeastGL(gl::Version(3, 0)) || functions->hasGLExtension("GL_EXT_texture_array") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxArrayTextureLayers = QuerySingleGLInt(functions, GL_MAX_ARRAY_TEXTURE_LAYERS);
    }
    else
    {
        // Can't support ES3 without array textures
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(1, 5)) || functions->hasGLExtension("GL_EXT_texture_lod_bias") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxLODBias = QuerySingleGLFloat(functions, GL_MAX_TEXTURE_LOD_BIAS);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 0)) || functions->hasGLExtension("GL_EXT_framebuffer_object") ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxRenderbufferSize = QuerySingleGLInt(functions, GL_MAX_RENDERBUFFER_SIZE);
        caps->maxColorAttachments = QuerySingleGLInt(functions, GL_MAX_COLOR_ATTACHMENTS);
    }
    else
    {
        // Can't support ES2 without framebuffers and renderbuffers
        LimitVersion(maxSupportedESVersion, gl::Version(0, 0));
    }

    if (functions->isAtLeastGL(gl::Version(2, 0)) || functions->hasGLExtension("ARB_draw_buffers") ||
        functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_EXT_draw_buffers"))
    {
        caps->maxDrawBuffers = QuerySingleGLInt(functions, GL_MAX_DRAW_BUFFERS);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    caps->maxViewportWidth = QueryGLIntRange(functions, GL_MAX_VIEWPORT_DIMS, 0); // GL 1.0 / ES 2.0
    caps->maxViewportHeight = QueryGLIntRange(functions, GL_MAX_VIEWPORT_DIMS, 1); // GL 1.0 / ES 2.0

    if (functions->standard == STANDARD_GL_DESKTOP &&
        (functions->profile & GL_CONTEXT_CORE_PROFILE_BIT) != 0)
    {
        // Desktop GL core profile deprecated the GL_ALIASED_POINT_SIZE_RANGE query.  Use
        // GL_POINT_SIZE_RANGE instead.
        caps->minAliasedPointSize = QueryGLFloatRange(functions, GL_POINT_SIZE_RANGE, 0);
        caps->maxAliasedPointSize = QueryGLFloatRange(functions, GL_POINT_SIZE_RANGE, 1);
    }
    else
    {
        caps->minAliasedPointSize = QueryGLFloatRange(functions, GL_ALIASED_POINT_SIZE_RANGE, 0);
        caps->maxAliasedPointSize = QueryGLFloatRange(functions, GL_ALIASED_POINT_SIZE_RANGE, 1);
    }

    caps->minAliasedLineWidth = QueryGLFloatRange(functions, GL_ALIASED_LINE_WIDTH_RANGE, 0); // GL 1.2 / ES 2.0
    caps->maxAliasedLineWidth = QueryGLFloatRange(functions, GL_ALIASED_LINE_WIDTH_RANGE, 1); // GL 1.2 / ES 2.0

    // Table 6.29, implementation dependent values (cont.)
    if (functions->isAtLeastGL(gl::Version(1, 2)) ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxElementsIndices = QuerySingleGLInt(functions, GL_MAX_ELEMENTS_INDICES);
        caps->maxElementsVertices = QuerySingleGLInt(functions, GL_MAX_ELEMENTS_VERTICES);
    }
    else
    {
        // Doesn't impact supported version
    }

    // glGetShaderPrecisionFormat is not available until desktop GL version 4.1 or GL_ARB_ES2_compatibility exists
    if (functions->isAtLeastGL(gl::Version(4, 1)) || functions->hasGLExtension("GL_ARB_ES2_compatibility") ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->vertexHighpFloat = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_HIGH_FLOAT);
        caps->vertexMediumpFloat = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_MEDIUM_FLOAT);
        caps->vertexLowpFloat = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_LOW_FLOAT);
        caps->fragmentHighpFloat = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_HIGH_FLOAT);
        caps->fragmentMediumpFloat = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_MEDIUM_FLOAT);
        caps->fragmentLowpFloat = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_LOW_FLOAT);
        caps->vertexHighpInt = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_HIGH_INT);
        caps->vertexMediumpInt = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_MEDIUM_INT);
        caps->vertexLowpInt = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_LOW_INT);
        caps->fragmentHighpInt = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_HIGH_INT);
        caps->fragmentMediumpInt = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_MEDIUM_INT);
        caps->fragmentLowpInt = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_LOW_INT);
    }
    else
    {
        // Doesn't impact supported version, set some default values
        caps->vertexHighpFloat.setIEEEFloat();
        caps->vertexMediumpFloat.setIEEEFloat();
        caps->vertexLowpFloat.setIEEEFloat();
        caps->fragmentHighpFloat.setIEEEFloat();
        caps->fragmentMediumpFloat.setIEEEFloat();
        caps->fragmentLowpFloat.setIEEEFloat();
        caps->vertexHighpInt.setTwosComplementInt(32);
        caps->vertexMediumpInt.setTwosComplementInt(32);
        caps->vertexLowpInt.setTwosComplementInt(32);
        caps->fragmentHighpInt.setTwosComplementInt(32);
        caps->fragmentMediumpInt.setTwosComplementInt(32);
        caps->fragmentLowpInt.setTwosComplementInt(32);
    }

    if (functions->isAtLeastGL(gl::Version(3, 2)) || functions->hasGLExtension("GL_ARB_sync") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxServerWaitTimeout = QuerySingleGLInt64(functions, GL_MAX_SERVER_WAIT_TIMEOUT);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Table 6.31, implementation dependent vertex shader limits
    if (functions->isAtLeastGL(gl::Version(2, 0)) ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxVertexAttributes = QuerySingleGLInt(functions, GL_MAX_VERTEX_ATTRIBS);
        caps->maxVertexUniformComponents = QuerySingleGLInt(functions, GL_MAX_VERTEX_UNIFORM_COMPONENTS);
        caps->maxVertexTextureImageUnits = QuerySingleGLInt(functions, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
    }
    else
    {
        // Can't support ES2 version without these caps
        LimitVersion(maxSupportedESVersion, gl::Version(0, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 1)) || functions->hasGLExtension("GL_ARB_ES2_compatibility") ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxVertexUniformVectors = QuerySingleGLInt(functions, GL_MAX_VERTEX_UNIFORM_VECTORS);
    }
    else
    {
        // Doesn't limit ES version, GL_MAX_VERTEX_UNIFORM_COMPONENTS / 4 is acceptable.
        caps->maxVertexUniformVectors = caps->maxVertexUniformComponents / 4;
    }

    if (functions->isAtLeastGL(gl::Version(3, 1)) || functions->hasGLExtension("GL_ARB_uniform_buffer_object") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxVertexUniformBlocks = QuerySingleGLInt(functions, GL_MAX_VERTEX_UNIFORM_BLOCKS);
    }
    else
    {
        // Can't support ES3 without uniform blocks
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 2)) ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxVertexOutputComponents = QuerySingleGLInt(functions, GL_MAX_VERTEX_OUTPUT_COMPONENTS);
    }
    else
    {
        // There doesn't seem, to be a desktop extension to add this cap, maybe it could be given a safe limit
        // instead of limiting the supported ES version.
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Table 6.32, implementation dependent fragment shader limits
    if (functions->isAtLeastGL(gl::Version(2, 0)) ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxFragmentUniformComponents = QuerySingleGLInt(functions, GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
        caps->maxTextureImageUnits = QuerySingleGLInt(functions, GL_MAX_TEXTURE_IMAGE_UNITS);
    }
    else
    {
        // Can't support ES2 version without these caps
        LimitVersion(maxSupportedESVersion, gl::Version(0, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 1)) || functions->hasGLExtension("GL_ARB_ES2_compatibility") ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxFragmentUniformVectors = QuerySingleGLInt(functions, GL_MAX_FRAGMENT_UNIFORM_VECTORS);
    }
    else
    {
        // Doesn't limit ES version, GL_MAX_FRAGMENT_UNIFORM_COMPONENTS / 4 is acceptable.
        caps->maxFragmentUniformVectors = caps->maxFragmentUniformComponents / 4;
    }

    if (functions->isAtLeastGL(gl::Version(3, 1)) || functions->hasGLExtension("GL_ARB_uniform_buffer_object") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxFragmentUniformBlocks = QuerySingleGLInt(functions, GL_MAX_FRAGMENT_UNIFORM_BLOCKS);
    }
    else
    {
        // Can't support ES3 without uniform blocks
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 2)) ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxFragmentInputComponents = QuerySingleGLInt(functions, GL_MAX_FRAGMENT_INPUT_COMPONENTS);
    }
    else
    {
        // There doesn't seem, to be a desktop extension to add this cap, maybe it could be given a safe limit
        // instead of limiting the supported ES version.
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 0)) ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->minProgramTexelOffset = QuerySingleGLInt(functions, GL_MIN_PROGRAM_TEXEL_OFFSET);
        caps->maxProgramTexelOffset = QuerySingleGLInt(functions, GL_MAX_PROGRAM_TEXEL_OFFSET);
    }
    else
    {
        // Can't support ES3 without texel offset, could possibly be emulated in the shader
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Table 6.33, implementation dependent aggregate shader limits
    if (functions->isAtLeastGL(gl::Version(3, 1)) || functions->hasGLExtension("GL_ARB_uniform_buffer_object") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxUniformBufferBindings = QuerySingleGLInt(functions, GL_MAX_UNIFORM_BUFFER_BINDINGS);
        caps->maxUniformBlockSize = QuerySingleGLInt64(functions, GL_MAX_UNIFORM_BLOCK_SIZE);
        caps->uniformBufferOffsetAlignment = QuerySingleGLInt(functions, GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
        caps->maxCombinedUniformBlocks = caps->maxVertexUniformBlocks + caps->maxFragmentInputComponents;
        caps->maxCombinedVertexUniformComponents = QuerySingleGLInt64(functions, GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
        caps->maxCombinedFragmentUniformComponents = QuerySingleGLInt64(functions, GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);
    }
    else
    {
        // Can't support ES3 without uniform blocks
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 0)) || functions->hasGLExtension("GL_ARB_ES3_compatibility") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxVaryingComponents = QuerySingleGLInt(functions, GL_MAX_VARYING_COMPONENTS);
    }
    else if (functions->isAtLeastGL(gl::Version(2, 0)))
    {
        caps->maxVaryingComponents = QuerySingleGLInt(functions, GL_MAX_VARYING_FLOATS);
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(0, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 1)) || functions->hasGLExtension("GL_ARB_ES2_compatibility") ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxVaryingVectors = QuerySingleGLInt(functions, GL_MAX_VARYING_VECTORS);
    }
    else
    {
        // Doesn't limit ES version, GL_MAX_VARYING_COMPONENTS / 4 is acceptable.
        caps->maxVaryingVectors = caps->maxVaryingComponents / 4;
    }

    // Determine the max combined texture image units by adding the vertex and fragment limits.  If
    // the real cap is queried, it would contain the limits for shader types that are not available to ES.
    caps->maxCombinedTextureImageUnits = caps->maxVertexTextureImageUnits + caps->maxTextureImageUnits;

    // Table 6.34, implementation dependent transform feedback limits
    if (functions->isAtLeastGL(gl::Version(3, 0)) || functions->hasGLExtension("GL_EXT_transform_feedback") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxTransformFeedbackInterleavedComponents = QuerySingleGLInt(functions, GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS);
        caps->maxTransformFeedbackSeparateAttributes = QuerySingleGLInt(functions, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS);
        caps->maxTransformFeedbackSeparateComponents = QuerySingleGLInt(functions, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS);
    }
    else
    {
        // Can't support ES3 without transform feedback
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Table 6.35, Framebuffer Dependent Values
    if (functions->isAtLeastGL(gl::Version(3, 0)) || functions->hasGLExtension("GL_EXT_framebuffer_multisample") ||
        functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_EXT_multisampled_render_to_texture"))
    {
        caps->maxSamples = QuerySingleGLInt(functions, GL_MAX_SAMPLES);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Extension support
    extensions->setTextureExtensionSupport(*textureCapsMap);
    extensions->elementIndexUint = functions->standard == STANDARD_GL_DESKTOP ||
                                   functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_OES_element_index_uint");
    extensions->readFormatBGRA = functions->isAtLeastGL(gl::Version(1, 2)) || functions->hasGLExtension("GL_EXT_bgra") ||
                                 functions->hasGLESExtension("GL_EXT_read_format_bgra");
    extensions->mapBuffer = functions->isAtLeastGL(gl::Version(1, 5)) ||
                            functions->hasGLESExtension("GL_OES_mapbuffer");
    extensions->mapBufferRange = functions->isAtLeastGL(gl::Version(3, 0)) || functions->hasGLExtension("GL_ARB_map_buffer_range") ||
                                 functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_EXT_map_buffer_range");
    extensions->textureNPOT = functions->standard == STANDARD_GL_DESKTOP ||
                              functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_OES_texture_npot");
    extensions->drawBuffers = functions->isAtLeastGL(gl::Version(2, 0)) || functions->hasGLExtension("ARB_draw_buffers") ||
                              functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_EXT_draw_buffers");
    extensions->textureStorage = true;
    extensions->textureFilterAnisotropic = functions->hasGLExtension("GL_EXT_texture_filter_anisotropic") || functions->hasGLESExtension("GL_EXT_texture_filter_anisotropic");
    extensions->maxTextureAnisotropy = extensions->textureFilterAnisotropic ? QuerySingleGLFloat(functions, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT) : 0.0f;
    extensions->fence = functions->hasGLExtension("GL_NV_fence") || functions->hasGLESExtension("GL_NV_fence");
    extensions->blendMinMax = functions->isAtLeastGL(gl::Version(1, 5)) || functions->hasGLExtension("GL_EXT_blend_minmax") ||
                              functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_EXT_blend_minmax");
    extensions->framebufferBlit = (functions->blitFramebuffer != nullptr);
    extensions->framebufferMultisample = caps->maxSamples > 0;
    extensions->standardDerivatives = functions->isAtLeastGL(gl::Version(2, 0)) || functions->hasGLExtension("GL_ARB_fragment_shader") ||
                                      functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_OES_standard_derivatives");
    extensions->shaderTextureLOD = functions->isAtLeastGL(gl::Version(3, 0)) || functions->hasGLExtension("GL_ARB_shader_texture_lod") ||
                                   functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_EXT_shader_texture_lod");
    extensions->fragDepth = functions->standard == STANDARD_GL_DESKTOP ||
                            functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_EXT_frag_depth");
    extensions->fboRenderMipmap = functions->isAtLeastGL(gl::Version(3, 0)) || functions->hasGLExtension("GL_EXT_framebuffer_object") ||
                                  functions->isAtLeastGLES(gl::Version(3, 0)) || functions->hasGLESExtension("GL_OES_fbo_render_mipmap");
    extensions->instancedArrays = functions->isAtLeastGL(gl::Version(3, 1)) ||
                                  (functions->hasGLExtension("GL_ARB_instanced_arrays") &&
                                   (functions->hasGLExtension("GL_ARB_draw_instanced") ||
                                    functions->hasGLExtension("GL_EXT_draw_instanced"))) ||
                                  functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                  functions->hasGLESExtension("GL_EXT_instanced_arrays");
    extensions->unpackSubimage = functions->standard == STANDARD_GL_DESKTOP ||
                                 functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                 functions->hasGLESExtension("GL_EXT_unpack_subimage");
    extensions->packSubimage = functions->standard == STANDARD_GL_DESKTOP ||
                               functions->isAtLeastGLES(gl::Version(3, 0)) ||
                               functions->hasGLESExtension("GL_NV_pack_subimage");
}

void GenerateWorkarounds(const FunctionsGL *functions, WorkaroundsGL *workarounds)
{
    VendorID vendor = GetVendorID(functions);

    // Don't use 1-bit alpha formats on desktop GL with AMD or Intel drivers.
    workarounds->avoid1BitAlphaTextureFormats =
        functions->standard == STANDARD_GL_DESKTOP &&
        (vendor == VENDOR_ID_AMD || vendor == VENDOR_ID_INTEL);

    workarounds->rgba4IsNotSupportedForColorRendering =
        functions->standard == STANDARD_GL_DESKTOP && vendor == VENDOR_ID_INTEL;

    workarounds->doesSRGBClearsOnLinearFramebufferAttachments =
        functions->standard == STANDARD_GL_DESKTOP &&
        (vendor == VENDOR_ID_INTEL || vendor == VENDOR_ID_AMD);
}

}

}

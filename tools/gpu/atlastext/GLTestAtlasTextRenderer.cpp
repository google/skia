/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLUtil.h"
#include "tools/gpu/atlastext/GLTestAtlasTextRenderer.h"
#include "tools/gpu/atlastext/TestAtlasTextRenderer.h"
#include "tools/gpu/gl/GLTestContext.h"

using sk_gpu_test::GLTestContext;

namespace {

class GLTestAtlasTextRenderer : public sk_gpu_test::TestAtlasTextRenderer {
public:
    GLTestAtlasTextRenderer(std::unique_ptr<GLTestContext>);

    void* createTexture(AtlasFormat, int width, int height) override;

    void deleteTexture(void* textureHandle) override;

    void setTextureData(void* textureHandle, const void* data, int x, int y, int width, int height,
                        size_t rowBytes) override;

    void drawSDFGlyphs(void* targetHandle, void* textureHandle, const SDFVertex vertices[],
                       int quadCnt) override;

    void* makeTargetHandle(int width, int height) override;

    void targetDeleted(void* targetHandle) override;

    SkBitmap readTargetHandle(void* targetHandle) override;

    void clearTarget(void* targetHandle, uint32_t color) override;

    bool initialized() const { return 0 != fProgram; }

private:
    struct AtlasTexture {
        GrGLuint fID;
        AtlasFormat fFormat;
        int fWidth;
        int fHeight;
    };

    struct Target {
        GrGLuint fFBOID;
        GrGLuint fRBID;
        int fWidth;
        int fHeight;
    };

    std::unique_ptr<GLTestContext> fContext;
    GrGLuint fProgram = 0;
    GrGLint fDstScaleAndTranslateLocation = 0;
    GrGLint fAtlasInvSizeLocation = 0;
    GrGLint fSamplerLocation = 0;
};

#define callgl(NAME, ...) fContext->gl()->fFunctions.f##NAME(__VA_ARGS__)
#define checkgl()                                               \
    do {                                                        \
        static constexpr auto line = __LINE__;                  \
        auto error = fContext->gl()->fFunctions.fGetError();    \
        if (error != GR_GL_NO_ERROR) {                          \
            SkDebugf("GL ERROR: 0x%x, line %d\n", error, line); \
        }                                                       \
    } while (false)

GLTestAtlasTextRenderer::GLTestAtlasTextRenderer(std::unique_ptr<GLTestContext> context)
        : fContext(std::move(context)) {
    auto restore = fContext->makeCurrentAndAutoRestore();

    // First check whether the GL is supported so we can avoid spammy failures on systems
    // where the GL simply doesn't work with this class.
    const char* versionStr = reinterpret_cast<const char*>(callgl(GetString, GR_GL_VERSION));
    auto version = GrGLGetVersionFromString(versionStr);
    auto standard = GrGLGetStandardInUseFromString(versionStr);
    switch (standard) {
        case kWebGL_GrGLStandard:
        case kNone_GrGLStandard:
            return;
        case kGLES_GrGLStandard:
            if (version < GR_GL_VER(3, 0)) {
                return;
            }
            break;
        case kGL_GrGLStandard: {
            if (version < GR_GL_VER(4, 3)) {
                return;
            }
            GrGLint profileMask;
            callgl(GetIntegerv, GR_GL_CONTEXT_PROFILE_MASK, &profileMask);
            if (profileMask & GR_GL_CONTEXT_CORE_PROFILE_BIT) {
                return;
            }
        }
    }

    auto vs = callgl(CreateShader, GR_GL_VERTEX_SHADER);
    static constexpr char kGLVersionString[] = "#version 430 compatibility";
    static constexpr char kGLESVersionString[] = "#version 300 es";
    GrGLint lengths[2];
    const GrGLchar* strings[2];
    switch (fContext->gl()->fStandard) {
        case kGL_GrGLStandard:
            strings[0] = kGLVersionString;
            lengths[0] = static_cast<GrGLint>(SK_ARRAY_COUNT(kGLVersionString)) - 1;
            break;
        case kGLES_GrGLStandard:
            strings[0] = kGLESVersionString;
            lengths[0] = static_cast<GrGLint>(SK_ARRAY_COUNT(kGLESVersionString)) - 1;
            break;
        default:
            strings[0] = nullptr;
            lengths[0] = 0;
            break;
    }

    static constexpr const char kVS[] = R"(
        precision highp float;

        uniform vec4 uDstScaleAndTranslate;
        uniform vec2 uAtlasInvSize;

        layout (location = 0) in vec3 inPosition;
        layout (location = 1) in vec4 inColor;
        layout (location = 2) in uvec2 inTextureCoords;

        out vec2 vTexCoord;
        out vec4 vColor;
        out vec2 vIntTexCoord;

        void main() {
            vec2 intCoords;
            // floor(vec2) doesn't seem to work on some ES devices.
            intCoords.x = floor(float(inTextureCoords.x));
            intCoords.y = floor(float(inTextureCoords.y));
            vTexCoord = intCoords * uAtlasInvSize;
            vIntTexCoord = intCoords;
            vColor = inColor;
            gl_Position = vec4(inPosition.x * uDstScaleAndTranslate.x + uDstScaleAndTranslate.y,
                               inPosition.y * uDstScaleAndTranslate.z + uDstScaleAndTranslate.w,
                               0.0, inPosition.z);
        }
    )";
    strings[1] = kVS;
    lengths[1] = SK_ARRAY_COUNT(kVS) - 1;
    callgl(ShaderSource, vs, 2, strings, lengths);
    callgl(CompileShader, vs);
    GrGLint compileStatus;
    callgl(GetShaderiv, vs, GR_GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GR_GL_FALSE) {
        GrGLint logLength;
        callgl(GetShaderiv, vs, GR_GL_INFO_LOG_LENGTH, &logLength);
        std::unique_ptr<GrGLchar[]> log(new GrGLchar[logLength + 1]);
        log[logLength] = '\0';
        callgl(GetShaderInfoLog, vs, logLength, &logLength, log.get());
        SkDebugf("Vertex Shader failed to compile\n%s", log.get());
        callgl(DeleteShader, vs);
        return;
    }

    auto fs = callgl(CreateShader, GR_GL_FRAGMENT_SHADER);
    static constexpr const char kFS[] = R"(
        precision highp float;

        uniform sampler2D uSampler;

        in vec2 vTexCoord;
        in vec4 vColor;
        in vec2 vIntTexCoord;

        layout (location = 0) out vec4 outColor;

        void main() {
            float sdfValue = texture(uSampler, vTexCoord).r;
            float distance = 7.96875 * (sdfValue - 0.50196078431000002);
            vec2 dist_grad = vec2(dFdx(distance), dFdy(distance));
            vec2 Jdx = dFdx(vIntTexCoord);
            vec2 Jdy = dFdy(vIntTexCoord);
            float dg_len2 = dot(dist_grad, dist_grad);
            if (dg_len2 < 0.0001) {
                dist_grad = vec2(0.7071, 0.7071);
            } else {
                dist_grad = dist_grad * inversesqrt(dg_len2);
            }
            vec2 grad = vec2(dist_grad.x * Jdx.x + dist_grad.y * Jdy.x,
                             dist_grad.x * Jdx.y + dist_grad.y * Jdy.y);
            float afwidth = abs(0.65000000000000002 * length(grad));
            float value = smoothstep(-afwidth, afwidth, distance);
            outColor = value * vec4(vColor.rgb * vColor.a, vColor.a);
        }
    )";
    strings[1] = kFS;
    lengths[1] = SK_ARRAY_COUNT(kFS) - 1;
    callgl(ShaderSource, fs, 2, strings, lengths);
    callgl(CompileShader, fs);
    callgl(GetShaderiv, fs, GR_GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GR_GL_FALSE) {
        GrGLint logLength;
        callgl(GetShaderiv, fs, GR_GL_INFO_LOG_LENGTH, &logLength);
        std::unique_ptr<GrGLchar[]> log(new GrGLchar[logLength + 1]);
        log[logLength] = '\0';
        callgl(GetShaderInfoLog, fs, logLength, &logLength, log.get());
        SkDebugf("Fragment Shader failed to compile\n%s", log.get());
        callgl(DeleteShader, vs);
        callgl(DeleteShader, fs);
        return;
    }

    fProgram = callgl(CreateProgram);
    if (!fProgram) {
        callgl(DeleteShader, vs);
        callgl(DeleteShader, fs);
        return;
    }

    callgl(AttachShader, fProgram, vs);
    callgl(AttachShader, fProgram, fs);
    callgl(LinkProgram, fProgram);
    GrGLint linkStatus;
    callgl(GetProgramiv, fProgram, GR_GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GR_GL_FALSE) {
        GrGLint logLength = 0;
        callgl(GetProgramiv, vs, GR_GL_INFO_LOG_LENGTH, &logLength);
        std::unique_ptr<GrGLchar[]> log(new GrGLchar[logLength + 1]);
        log[logLength] = '\0';
        callgl(GetProgramInfoLog, vs, logLength, &logLength, log.get());
        SkDebugf("Program failed to link\n%s", log.get());
        callgl(DeleteShader, vs);
        callgl(DeleteShader, fs);
        callgl(DeleteProgram, fProgram);
        fProgram = 0;
        return;
    }
    fDstScaleAndTranslateLocation = callgl(GetUniformLocation, fProgram, "uDstScaleAndTranslate");
    fAtlasInvSizeLocation = callgl(GetUniformLocation, fProgram, "uAtlasInvSize");
    fSamplerLocation = callgl(GetUniformLocation, fProgram, "uSampler");
    if (fDstScaleAndTranslateLocation < 0 || fAtlasInvSizeLocation < 0 || fSamplerLocation < 0) {
        callgl(DeleteShader, vs);
        callgl(DeleteShader, fs);
        callgl(DeleteProgram, fProgram);
        fProgram = 0;
    }

    checkgl();
}

inline bool atlas_format_to_gl_types(SkAtlasTextRenderer::AtlasFormat format,
                                     GrGLenum* internalFormat, GrGLenum* externalFormat,
                                     GrGLenum* type) {
    switch (format) {
        case SkAtlasTextRenderer::AtlasFormat::kA8:
            *internalFormat = GR_GL_R8;
            *externalFormat = GR_GL_RED;
            *type = GR_GL_UNSIGNED_BYTE;
            return true;
    }
    return false;
}

inline int atlas_format_bytes_per_pixel(SkAtlasTextRenderer::AtlasFormat format) {
    switch (format) {
        case SkAtlasTextRenderer::AtlasFormat::kA8:
            return 1;
    }
    return 0;
}

void* GLTestAtlasTextRenderer::createTexture(AtlasFormat format, int width, int height) {
    GrGLenum internalFormat;
    GrGLenum externalFormat;
    GrGLenum type;
    if (!atlas_format_to_gl_types(format, &internalFormat, &externalFormat, &type)) {
        return nullptr;
    }
    auto restore = fContext->makeCurrentAndAutoRestore();

    GrGLuint id;
    callgl(GenTextures, 1, &id);
    if (!id) {
        return nullptr;
    }

    callgl(BindTexture, GR_GL_TEXTURE_2D, id);
    callgl(TexImage2D, GR_GL_TEXTURE_2D, 0, internalFormat, width, height, 0, externalFormat, type,
           nullptr);
    checkgl();

    AtlasTexture* atlas = new AtlasTexture;
    atlas->fID = id;
    atlas->fFormat = format;
    atlas->fWidth = width;
    atlas->fHeight = height;
    return atlas;
}

void GLTestAtlasTextRenderer::deleteTexture(void* textureHandle) {
    auto restore = fContext->makeCurrentAndAutoRestore();

    auto* atlasTexture = reinterpret_cast<const AtlasTexture*>(textureHandle);

    callgl(DeleteTextures, 1, &atlasTexture->fID);
    checkgl();

    delete atlasTexture;
}

void GLTestAtlasTextRenderer::setTextureData(void* textureHandle, const void* data, int x, int y,
                                             int width, int height, size_t rowBytes) {
    auto restore = fContext->makeCurrentAndAutoRestore();

    auto atlasTexture = reinterpret_cast<const AtlasTexture*>(textureHandle);

    GrGLenum internalFormat;
    GrGLenum externalFormat;
    GrGLenum type;
    if (!atlas_format_to_gl_types(atlasTexture->fFormat, &internalFormat, &externalFormat, &type)) {
        return;
    }
    int bpp = atlas_format_bytes_per_pixel(atlasTexture->fFormat);
    GrGLint rowLength = static_cast<GrGLint>(rowBytes / bpp);
    if (static_cast<size_t>(rowLength * bpp) != rowBytes) {
        return;
    }
    callgl(PixelStorei, GR_GL_UNPACK_ALIGNMENT, 1);
    callgl(PixelStorei, GR_GL_UNPACK_ROW_LENGTH, rowLength);
    callgl(BindTexture, GR_GL_TEXTURE_2D, atlasTexture->fID);
    callgl(TexSubImage2D, GR_GL_TEXTURE_2D, 0, x, y, width, height, externalFormat, type, data);
    checkgl();
}

void GLTestAtlasTextRenderer::drawSDFGlyphs(void* targetHandle, void* textureHandle,
                                            const SDFVertex vertices[], int quadCnt) {
    auto restore = fContext->makeCurrentAndAutoRestore();

    auto target = reinterpret_cast<const Target*>(targetHandle);
    auto atlas = reinterpret_cast<const AtlasTexture*>(textureHandle);

    callgl(UseProgram, fProgram);

    callgl(ActiveTexture, GR_GL_TEXTURE0);
    callgl(BindTexture, GR_GL_TEXTURE_2D, atlas->fID);
    callgl(TexParameteri, GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MAG_FILTER, GR_GL_LINEAR);
    callgl(TexParameteri, GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MIN_FILTER, GR_GL_LINEAR);

    float uniformScaleAndTranslate[4] = {2.f / target->fWidth, -1.f, 2.f / target->fHeight, -1.f};
    callgl(Uniform4fv, fDstScaleAndTranslateLocation, 1, uniformScaleAndTranslate);
    callgl(Uniform2f, fAtlasInvSizeLocation, 1.f / atlas->fWidth, 1.f / atlas->fHeight);
    callgl(Uniform1i, fSamplerLocation, 0);

    callgl(BindFramebuffer, GR_GL_FRAMEBUFFER, target->fFBOID);
    callgl(Viewport, 0, 0, target->fWidth, target->fHeight);

    callgl(Enable, GR_GL_BLEND);
    callgl(BlendFunc, GR_GL_ONE, GR_GL_ONE_MINUS_SRC_ALPHA);
    callgl(Disable, GR_GL_DEPTH_TEST);

    callgl(BindVertexArray, 0);
    callgl(BindBuffer, GR_GL_ARRAY_BUFFER, 0);
    callgl(BindBuffer, GR_GL_ELEMENT_ARRAY_BUFFER, 0);
    callgl(VertexAttribPointer, 0, 3, GR_GL_FLOAT, GR_GL_FALSE, sizeof(SDFVertex), vertices);
    size_t colorOffset = 3 * sizeof(float);
    callgl(VertexAttribPointer, 1, 4, GR_GL_UNSIGNED_BYTE, GR_GL_TRUE, sizeof(SDFVertex),
           reinterpret_cast<const char*>(vertices) + colorOffset);
    size_t texOffset = colorOffset + sizeof(uint32_t);
    callgl(VertexAttribIPointer, 2, 2, GR_GL_UNSIGNED_SHORT, sizeof(SDFVertex),
           reinterpret_cast<const char*>(vertices) + texOffset);
    callgl(EnableVertexAttribArray, 0);
    callgl(EnableVertexAttribArray, 1);
    callgl(EnableVertexAttribArray, 2);

    std::unique_ptr<uint16_t[]> indices(new uint16_t[quadCnt * 6]);
    for (int q = 0; q < quadCnt; ++q) {
        indices[q * 6 + 0] = 0 + 4 * q;
        indices[q * 6 + 1] = 1 + 4 * q;
        indices[q * 6 + 2] = 2 + 4 * q;
        indices[q * 6 + 3] = 2 + 4 * q;
        indices[q * 6 + 4] = 1 + 4 * q;
        indices[q * 6 + 5] = 3 + 4 * q;
    }
    callgl(DrawElements, GR_GL_TRIANGLES, 6 * quadCnt, GR_GL_UNSIGNED_SHORT, indices.get());
    checkgl();
}

void* GLTestAtlasTextRenderer::makeTargetHandle(int width, int height) {
    auto restore = fContext->makeCurrentAndAutoRestore();

    GrGLuint fbo;
    callgl(GenFramebuffers, 1, &fbo);
    if (!fbo) {
        return nullptr;
    }
    GrGLuint rb;
    callgl(GenRenderbuffers, 1, &rb);
    if (!rb) {
        callgl(DeleteFramebuffers, 1, &fbo);
        return nullptr;
    }
    callgl(BindFramebuffer, GR_GL_FRAMEBUFFER, fbo);
    callgl(BindRenderbuffer, GR_GL_RENDERBUFFER, rb);
    callgl(RenderbufferStorage, GR_GL_RENDERBUFFER, GR_GL_RGBA8, width, height);
    callgl(FramebufferRenderbuffer, GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0, GR_GL_RENDERBUFFER,
           rb);
    GrGLenum status = callgl(CheckFramebufferStatus, GR_GL_FRAMEBUFFER);
    if (GR_GL_FRAMEBUFFER_COMPLETE != status) {
        callgl(DeleteFramebuffers, 1, &fbo);
        callgl(DeleteRenderbuffers, 1, &rb);
        return nullptr;
    }
    callgl(Disable, GR_GL_SCISSOR_TEST);
    callgl(ClearColor, 0, 0, 0, 0.0);
    callgl(Clear, GR_GL_COLOR_BUFFER_BIT);
    checkgl();
    Target* target = new Target;
    target->fFBOID = fbo;
    target->fRBID = rb;
    target->fWidth = width;
    target->fHeight = height;
    return target;
}

void GLTestAtlasTextRenderer::targetDeleted(void* targetHandle) {
    auto restore = fContext->makeCurrentAndAutoRestore();

    Target* target = reinterpret_cast<Target*>(targetHandle);
    callgl(DeleteFramebuffers, 1, &target->fFBOID);
    callgl(DeleteRenderbuffers, 1, &target->fRBID);
    delete target;
}

SkBitmap GLTestAtlasTextRenderer::readTargetHandle(void* targetHandle) {
    auto restore = fContext->makeCurrentAndAutoRestore();

    Target* target = reinterpret_cast<Target*>(targetHandle);

    auto info =
            SkImageInfo::Make(target->fWidth, target->fHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bmp;
    bmp.setInfo(info, sizeof(uint32_t) * target->fWidth);
    bmp.allocPixels();

    callgl(BindFramebuffer, GR_GL_FRAMEBUFFER, target->fFBOID);
    callgl(ReadPixels, 0, 0, target->fWidth, target->fHeight, GR_GL_RGBA, GR_GL_UNSIGNED_BYTE,
           bmp.getPixels());
    checkgl();
    return bmp;
}

void GLTestAtlasTextRenderer::clearTarget(void* targetHandle, uint32_t color) {
    auto restore = fContext->makeCurrentAndAutoRestore();

    Target* target = reinterpret_cast<Target*>(targetHandle);
    callgl(BindFramebuffer, GR_GL_FRAMEBUFFER, target->fFBOID);
    callgl(Disable, GR_GL_SCISSOR_TEST);
    float r = ((color >>  0) & 0xff) / 255.f;
    float g = ((color >>  8) & 0xff) / 255.f;
    float b = ((color >> 16) & 0xff) / 255.f;
    float a = ((color >> 24) & 0xff) / 255.f;
    callgl(ClearColor, r, g, b, a);
    callgl(Clear, GR_GL_COLOR_BUFFER_BIT);
}

}  // anonymous namespace

namespace sk_gpu_test {

sk_sp<TestAtlasTextRenderer> MakeGLTestAtlasTextRenderer() {
    std::unique_ptr<GLTestContext> context(CreatePlatformGLTestContext(kGL_GrGLStandard));
    if (!context) {
        context.reset(CreatePlatformGLTestContext(kGLES_GrGLStandard));
    }
    if (!context) {
        return nullptr;
    }
    auto restorer = context->makeCurrentAndAutoRestore();
    auto renderer = sk_make_sp<GLTestAtlasTextRenderer>(std::move(context));
    return renderer->initialized() ? std::move(renderer) : nullptr;
}

}  // namespace sk_gpu_test

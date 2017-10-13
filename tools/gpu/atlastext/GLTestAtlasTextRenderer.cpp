/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLTestAtlasTextRenderer.h"
#include "SkAtlasTextRenderer.h"
#include "../gl/GLTestContext.h"
#include "gl/GrGLDefines.h"

using sk_gpu_test::GLTestContext;

namespace {

class GLTestAtlasTextRenderer : public SkAtlasTextRenderer {
public:
    GLTestAtlasTextRenderer(std::unique_ptr<GLTestContext>);

    void* createTexture(AtlasFormat, int width, int height) override;

    void deleteTexture(void* textureHandle) override;

    void setTextureData(void* textureHandle, void* data, int x, int y, int width, int height, size_t rowBytes) override;

    void drawSDFGlyphs(void* targetHandle, void* textureHandle, const SDFVertex vertices[], int quadCnt) override;

private:
    struct AtlasTexture {
        GrGLuint fID;
        AtlasFormat fFormat;
    };

    std::unique_ptr<GLTestContext> fContext;
    GrGLuint fProgram;
};

#define callgl(NAME, ...) fContext->gl()->fFunctions.f ## NAME(__VA_ARGS__);

GLTestAtlasTextRenderer::GLTestAtlasTextRenderer(std::unique_ptr<GLTestContext> context) : fContext(std::move(context)) {
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
    }

    static constexpr const char kVS[] = R"(
        layout (location = 0) uniform vec4 uDstScaleAndTranslate;
        layout (location = 1) uniform vec2 uAtlasInvSize;

        layout (location = 0) in vec2 inPosition;
        layout (location = 1) in uvec2 inTextureCoords;

        out vec2 vTexCoord;
        out vec2 vIntTexCoord;

        void main() {
            vec2 pos2 = inPosition;
            // The lowest bit of each coord value is used to store a texture index, which we ignore.
            vec2 intCoords = floor(inTextureCoords >> 1);
            vTexCoord = intCoords * uAtlasInvSize;
            vIntTexCoord = intCoords;
            gl_Position = vec4(pos2.x * uDstScaleAndTranslate.x + uDstScaleAndTranslate.y,
                               pos2.y * uDstScaleAndTranslate.z + uDstScaleAndTranslate.w,
                               0.0, 1.0);
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
    }

    auto fs = callgl(CreateShader, GR_GL_FRAGMENT_SHADER);
    static constexpr const char kFS[] = R"(
        uniform sampler2D uSampler;

        in vec2 vTexCoord;
        in vec2 vIntTexCoord;

        layout (location = 0) out vec4 outColor;

        void main() {
            vec4 outputCoverage_Stage0;
            float sdfValue = texture(uSampler, vTexCoord).a;
            float distance = 7.96875 * (sdfValue - 0.50196078431000002);
            vec2 dist_grad = vec2(dFdx(distance), dFdy(distance));
            vec2 Jdx = dFdx(vIntTexCoord);
            vec2 Jdy = dFdy(vIntTexCoord);
            float dg_len2 = dot(dist_grad, dist_grad);
            dist_grad = dist_grad * inversesqrt(dg_len2);
            vec2 grad = vec2(dist_grad.x * Jdx.x + dist_grad.y * Jdy.x,
                             dist_grad.x * Jdx.y + dist_grad.y * Jdy.y);
            float afwidth = abs(0.65000000000000002 * length(grad));
            float value = smoothstep(-afwidth, afwidth, distance);
            outColor = vec4(value);
        }
    )";
    strings[1] = kFS;
    lengths[1] = SK_ARRAY_COUNT(kFS) - 1;
    callgl(ShaderSource, fs, 2, strings,lengths)
    callgl(CompileShader, fs);
    callgl(GetShaderiv, fs, GR_GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GR_GL_FALSE) {
        GrGLint logLength;
        callgl(GetShaderiv, fs, GR_GL_INFO_LOG_LENGTH, &logLength);
        std::unique_ptr<GrGLchar[]> log(new GrGLchar[logLength + 1]);
        log[logLength] = '\0';
        callgl(GetShaderInfoLog, fs, logLength, &logLength, log.get());
        SkDebugf("Fragment Shader failed to compile\n%s", log.get());
    }

    fProgram = callgl(CreateProgram);

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
    }
}

inline bool atlas_format_to_gl_types(SkAtlasTextRenderer::AtlasFormat format,
                                     GrGLenum* internalFormat, GrGLenum* externalFormat,
                                     GrGLenum* type) {
    switch (format) {
        case SkAtlasTextRenderer::AtlasFormat::kA8:
            *internalFormat = GR_GL_ALPHA8;
            *externalFormat = GR_GL_ALPHA;
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

    GrGLuint id;
    callgl(GenTextures, 1, &id);
    if (!id) {
        return nullptr;
    }

    callgl(BindTexture, GR_GL_TEXTURE_2D, id);
    callgl(TexImage2D, GR_GL_TEXTURE_2D, 0, internalFormat, width, height, 0, externalFormat, type,
       nullptr);

    AtlasTexture atlas  = {.fID = id, .fFormat = format};
    return new AtlasTexture(atlas);
}

void GLTestAtlasTextRenderer::deleteTexture(void* textureHandle) {
    const AtlasTexture* atlasTexture = reinterpret_cast<const AtlasTexture*>(textureHandle);

    callgl(DeleteTextures, 1, &atlasTexture->fID);
    delete atlasTexture;
}

void GLTestAtlasTextRenderer::setTextureData(void* textureHandle, void* data, int x, int y,
                                             int width, int height, size_t rowBytes) {
    const AtlasTexture* atlasTexture = reinterpret_cast<const AtlasTexture*>(textureHandle);

    GrGLenum internalFormat;
    GrGLenum externalFormat;
    GrGLenum type;
    if (!atlas_format_to_gl_types(atlasTexture->fFormat, &internalFormat, &externalFormat, &type)) {
        return;
    }
    int bpp = atlas_format_bytes_per_pixel(atlasTexture->fFormat);
    GrGLint rowLength  = static_cast<GrGLint>(rowBytes / bpp);
    if (static_cast<size_t>(rowLength * bpp) != rowBytes) {
        return;
    }
    callgl(PixelStorei, GR_GL_UNPACK_ALIGNMENT, 1);
    callgl(PixelStorei, GR_GL_UNPACK_ROW_LENGTH, rowLength);
    callgl(BindTexture, GR_GL_TEXTURE_2D, atlasTexture->fID);
    callgl(TexSubImage2D, GR_GL_TEXTURE_2D, 0, x, y, width, height,  externalFormat, type, data);
}

void GLTestAtlasTextRenderer::drawSDFGlyphs(void* targetHandle, void* textureHandle,
                                            const SDFVertex vertices[], int quadCnt) {

    callgl();
}

}  // anonymous namespace

namespace sk_gpu_test {

std::unique_ptr<SkAtlasTextRenderer> MakeGLTestAtlasTextRenderer() {
    std::unique_ptr<GLTestContext> context(CreatePlatformGLTestContext(kGL_GrGLStandard));
    if (!context) {
        context.reset(CreatePlatformGLTestContext(kGLES_GrGLStandard));
    }
    if (!context) {
        return nullptr;
    }
    return std::unique_ptr<SkAtlasTextRenderer>(new GLTestAtlasTextRenderer(std::move(context)));
}

}  // anonymous namespace


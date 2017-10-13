/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLTestAtlasTextRenderer.h"
#include "SkAtlasTextRenderer.h"
#include "../private/SkTHash.h"
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
    //SkTHashMap<GrGLuint, const AtlasTexture*> fAtlases;
};

#define gl(NAME, ...) fContext->gl()->fFunctions.f ## NAME(__VA_ARGS__);

GLTestAtlasTextRenderer::GLTestAtlasTextRenderer(std::unique_ptr<GLTestContext> context) : fContext(std::move(context)) {
    auto vs = gl(CreateShader, GR_GL_VERTEX_SHADER);
    static constexpr const char kVS[] = R"(
        #version 420 compatibility
        uniform vec4 uDstScaleAndTranslate;
        uniform vec2 uAtlasInvSize;

        in vec2 inPosition;
        in uvec2 inTextureCoords;

        out vec2 vTexCoord;
        out float vTexIndex;
        out vec2 vIntTexCoord;

        void main() {
            vinColor_Stage0 = inColor;
            vec2 pos2 = inPosition;
            vec2 indexTexCoords = vec2(float(inTextureCoords.x), float(inTextureCoords.y));
            vec2 intCoords = floor(0.5 * indexTexCoords);
            vec2 diff = indexTexCoords - 2.0 * intCoords;
            float texIdx = 2.0 * diff.x + diff.y;
            vTexCoord = intCoords * uAtlasInvSize;
            vTexIndex = texIdx;
            vIntTexCoord = intCoords;
            gl_Position = vec4(pos2.x * uDstScaleAndTranslate.x + uDstScaleAndTranslate.y, pos2.y * uDstScaleAndTranslate.z + uDstScaleAndTranslate.w, 0.0, 1.0);
        }
    )";
    static const GrGLchar* kVSChars = kVS;
    static constexpr GrGLint kVSLength = SK_ARRAY_COUNT(kVS);
    gl(ShaderSource, vs, 1, &kVSChars, &kVSLength)
    gl(CompileShader, vs);
    GrGLint compileStatus;
    gl(GetShaderiv, vs, GR_GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GR_GL_FALSE) {
        GrGLint logLength;
        gl(GetShaderiv, vs, GR_GL_INFO_LOG_LENGTH, &logLength);
        std::unique_ptr<GrGLchar[]> log(new GrGLchar[logLength + 1]);
        log[logLength] = '\0';
        gl(GetShaderInfoLog, vs, logLength, &logLength, log.get());
        SkDebugf("Vertex Shader failed to compile\n%s", log.get());
    }

    auto fs = gl(CreateShader, GR_GL_FRAGMENT_SHADER);
    static constexpr const char kFS[] = R"(
        #version 420 compatibility

        uniform sampler2D uSampler;

        in vec2 vTexCoord;
        in float vTexIndex;
        in vec2 vIntTextCoord;

        out vec4 outColor;

        void main() {
            vec4 outputCoverage_Stage0;
            float sdfValue = texture(uSampler, uv).a;
            float distance = 7.96875 * (sdfValue - 0.50196078431000002);
            float afwidth = abs(0.65000000000000002 * dFdx(vIntTexCoord.x));
            float val = smoothstep(-afwidth, afwidth, distance);
            outputCoverage_Stage0 = vec4(val);
            outColor = vec4(value);
        }
    )";
    static const GrGLchar* kFSChars = kFS;
    static constexpr GrGLint kFSLength = SK_ARRAY_COUNT(kFS);
    gl(ShaderSource, fs, 1, &kFSChars, &kFSLength)
    gl(CompileShader, fs);
    gl(GetShaderiv, vs, GR_GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GR_GL_FALSE) {
        GrGLint logLength;
        gl(GetShaderiv, vs, GR_GL_INFO_LOG_LENGTH, &logLength);
        std::unique_ptr<GrGLchar[]> log(new GrGLchar[logLength + 1]);
        log[logLength] = '\0';
        gl(GetShaderInfoLog, vs, logLength, &logLength, log.get());
        SkDebugf("Fragment Shader failed to compile\n%s", log.get());
    }

    fProgram = gl(CreateProgram);

    gl(AttachShader, fProgram, vs);
    gl(AttachShader, fProgram, fs);
    gl(LinkProgram, fProgram);
    GrGLint linkStatus;
    gl(GetProgramiv, fProgram, GR_GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GR_GL_FALSE) {
        GrGLint logLength;
        gl(GetProgramiv, vs, GR_GL_INFO_LOG_LENGTH, &logLength);
        std::unique_ptr<GrGLchar[]> log(new GrGLchar[logLength + 1]);
        log[logLength] = '\0';
        gl(GetProgramInfoLog, vs, logLength, &logLength, log.get());
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
    gl(GenTextures, 1, &id);
    if (!id) {
        return nullptr;
    }

    gl(BindTexture, GR_GL_TEXTURE_2D, id);
    gl(TexImage2D, GR_GL_TEXTURE_2D, 0, internalFormat, width, height, 0, externalFormat, type,
       nullptr);

    AtlasTexture atlas  = {.fID = id, .fFormat = format};
    return new AtlasTexture(atlas);
}

void GLTestAtlasTextRenderer::deleteTexture(void* textureHandle) {
    const AtlasTexture* atlasTexture = reinterpret_cast<const AtlasTexture*>(textureHandle);

    gl(DeleteTextures, 1, &atlasTexture->fID);
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
    gl(PixelStorei, GR_GL_UNPACK_ALIGNMENT, 1);
    gl(PixelStorei, GR_GL_UNPACK_ROW_LENGTH, rowLength);
    gl(BindTexture, GR_GL_TEXTURE_2D, atlasTexture->fID);
    gl(TexSubImage2D, GR_GL_TEXTURE_2D, 0, x, y, width, height,  externalFormat, type, data);
}

void GLTestAtlasTextRenderer::drawSDFGlyphs(void* targetHandle, void* textureHandle,
                                            const SDFVertex vertices[], int quadCnt) {

}

}  // anonymous namespace

namespace sk_gpu_test {

std::unique_ptr<SkAtlasTextRenderer> MakeGLTestTextRenderer() {
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

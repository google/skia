/*
    Copyright 2010 Google Inc.

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


#include "GrGpuD3D9.h"
#include "GrGpuVertex.h"

void d3dCheckErr(HRESULT hr) {
    GrAssert(SUCCEEDED(hr));
}

#if GR_DEBUG
    #define GR_D3D9(OBJ, X) d3dCheckErr(OBJ-> X);
#else
    #define GR_D3D9(OBJ, X) OBJ-> X;
#endif

#if GR_SCALAR_IS_FIXED
    //mobile d3d allows 3 component fixed point verts
    #error "fixed is unsupported in D3D9"
#elif GR_SCALAR_IS_FLOAT
    #define FVF_POS_TYPE D3DFVF_XYZ
    #define FVF_TEX_TYPE (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(1))
#else
    #error "unknown GPU type"
#endif

#define FVF_COL_TYPE D3DFVF_DIFFUSE

#if GR_TEXT_SCALAR_IS_FIXED
    //mobile d3d allows 3 component fixed point verts
    #error "fixed is unsupported in D3D9"
#elif GR_TEXT_SCALAR_IS_FLOAT
    #define FVF_POS_TYPE_TEXT D3DFVF_XYZ
    #define FVF_TEX_TYPE_TEXT (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(1))
#elif GR_TEXT_SCALAR_IS_USHORT
    #error "positions must be float in fixed-pipe D3D9"
#else
    #error "unknown GPU text type"
#endif

static const int STAGE_NUM_VERTS = 512;
static const int STAGE_VERTEX_SIZE = sizeof(GrPoint)*STAGE_NUM_VERTS;
static const int STAGE_INDEX_SIZE  = 2*STAGE_NUM_VERTS*2*3; 
                                  // 2 bytes per index * 
                                  // 2 triangles per vert (euler char) * 3
                                  // 3 indices/triangle

static const D3DTRANSFORMSTATETYPE gMatrixMode2D3D9Matrix[] = {
    D3DTS_WORLD,     // kModelView_MatrixMode
    D3DTS_TEXTURE0,  // kTexture_MatrixMode
};

static const D3DPRIMITIVETYPE gPrimType2D3D9PrimType[] = {
    D3DPT_TRIANGLELIST,
    D3DPT_TRIANGLESTRIP,
    D3DPT_TRIANGLEFAN,
    D3DPT_POINTLIST,
    D3DPT_LINELIST,
    D3DPT_LINESTRIP,
};

const GrGpuD3D9::VertDecls GrGpuD3D9::gVertFlags2VertDeclIdx[] = {
    kPosOnly_VertDecl,              // no flags
    kTex_VertDecl,                  // kTexCoord_VertFlag
    kColors_VertDecl,               // kColors_VertFlag
    kTexAndColors_VertDecl,         // kColors_VertFlag & kColors_VertFlag
    kInvalid_VertDecl,              // kPositionAsTexCoord_VertFlag
    kPosAsTex_VertDecl,             // kPositionAsTexCoord_VertFlag & kTexCoord_VertFlag
    kInvalid_VertDecl,              // kPositionAsTexCoord_VertFlag & kColors_VertFlag
    kPosAsTexAndColors_VertDecl     // kPositionAsTexCoord_VertFlag & kTexCoord_VertFlag & kColors_VertFlag
};

const DWORD GrGpuD3D9::gDeclToFVFs[] = {
    FVF_POS_TYPE, // kPosOnly_VertDecl
    FVF_POS_TYPE | FVF_TEX_TYPE, // kTex_VertDecl
    FVF_POS_TYPE | FVF_COL_TYPE, // kColors_VertDecl
    FVF_POS_TYPE | FVF_TEX_TYPE | FVF_COL_TYPE, // kTexAndColors_VertDecl
    FVF_POS_TYPE, // kPosAsTex_VertDecl
    FVF_POS_TYPE | FVF_COL_TYPE, // kPosAsTexAndColors_VertDecl
};

const DWORD GrGpuD3D9::gTextFVF = FVF_POS_TYPE_TEXT | FVF_TEX_TYPE_TEXT;

#if   (SK_A32_SHIFT == 24) && (SK_R32_SHIFT == 16) && \
      (SK_G32_SHIFT == 8) && (SK_B32_SHIFT == 0)
    #define GR_D3D9_32BPP_COLOR_FORMAT D3DFMT_A8R8G8B8
#elif (SK_A32_SHIFT == 24) && (SK_B32_SHIFT == 16) && \
      (SK_G32_SHIFT == 8) && (SK_R32_SHIFT == 0)
    #define GR_D3D9_32BPP_COLOR_FORMAT D3DFMT_A8B8G8R8
#else
    #error "Skia's 32bit color format is not understood by D3D9."
#endif

static const DWORD gXfermodeCoeff2Blend[] = {
    D3DBLEND_ZERO,
    D3DBLEND_ONE,
    D3DBLEND_SRCCOLOR,
    D3DBLEND_INVSRCCOLOR,
    D3DBLEND_DESTCOLOR,
    D3DBLEND_INVDESTCOLOR,
    D3DBLEND_SRCALPHA,
    D3DBLEND_INVSRCALPHA,
    D3DBLEND_DESTALPHA,
    D3DBLEND_INVDESTALPHA,
};

static const DWORD gTileMode2D3D9Wrap[] = {
    D3DTADDRESS_CLAMP,
    D3DTADDRESS_WRAP,
    D3DTADDRESS_MIRROR
};

static bool can_be_texture(GrTexture::PixelConfig config, D3DFORMAT* format) {
    switch (config) {
        case GrTexture::kRGBA_8888_PixelConfig:
            *format = GR_D3D9_32BPP_COLOR_FORMAT;
            break;
        case GrTexture::kRGB_565_PixelConfig:
            *format = D3DFMT_R5G6B5;
            break;
        case GrTexture::kRGBA_4444_PixelConfig:
            *format = D3DFMT_A4R4G4B4;
            break;
        case GrTexture::kIndex_8_PixelConfig:
            // we promote index to argb32
            *format = GR_D3D9_32BPP_COLOR_FORMAT;
            break;
        case GrTexture::kAlpha_8_PixelConfig:
            *format = D3DFMT_A8;
            break;
        default:
            return false;
    }
    return true;
}

static int format_bytes(D3DFORMAT format) {
    switch (format) {
        case GR_D3D9_32BPP_COLOR_FORMAT:
            return 4;
        case D3DFMT_R5G6B5:
            return 2;
        case D3DFMT_A4R4G4B4:
            return 2;
        case D3DFMT_A8:
            return 1;
        default:
            GrAssert(!"Unexpected D3D format!");
            return 0;
    }
}

uint32_t vertex_to_primitive_count(GrGpu::PrimitiveTypes type, 
                                   uint32_t vertexCount) {
    switch (type) {
        case GrGpu::kTriangles_PrimitiveType:
            return vertexCount / 3;
        case GrGpu::kTriangleStrip_PrimitiveType: // fallthru
        case GrGpu::kTriangleFan_PrimitiveType:
            return vertexCount > 2 ? vertexCount - 2 : 0;
        case GrGpu::kPoints_PrimitiveType:
            return vertexCount;
        case GrGpu::kLines_PrimitiveType:
            return vertexCount / 2;
        case GrGpu::kLineStrip_PrimitiveType:
            return vertexCount > 1 ? vertexCount - 1 : 0;
        default:
            GrAssert(!"Unknown primitive type!");
            return 0;
    }
}

void gr_matrix_to_d3d_matrix(D3DMATRIX* d3dmat, GrMatrix& grmat) {
    d3dmat->_11 = grmat[GrMatrix::kScaleX];
    d3dmat->_21 = grmat[GrMatrix::kSkewX];
    d3dmat->_31 = 0;
    d3dmat->_41 = grmat[GrMatrix::kTransX];

    d3dmat->_12 = grmat[GrMatrix::kSkewY];
    d3dmat->_22 = grmat[GrMatrix::kScaleY];
    d3dmat->_32 = 0;
    d3dmat->_42 = grmat[GrMatrix::kTransY];
    
    d3dmat->_13 = 0;
    d3dmat->_23 = 0;
    d3dmat->_33 = 1;
    d3dmat->_43 = 0;

    d3dmat->_14 = grmat[GrMatrix::kPersp0];
    d3dmat->_24 = grmat[GrMatrix::kPersp1];
    d3dmat->_34 = 0;
    d3dmat->_44 = grmat[GrMatrix::kPersp2];
}

bool color_and_stencil_compatible(const D3DSURFACE_DESC& rtDesc, 
                                  const D3DSURFACE_DESC& dsDesc) {
    return (rtDesc.Width  <= dsDesc.Width)  &&
           (rtDesc.Height <= dsDesc.Height) &&
           (rtDesc.MultiSampleType == dsDesc.MultiSampleType) &&
           (rtDesc.MultiSampleQuality == dsDesc.MultiSampleQuality);
}

int format_stencil_bits(D3DFORMAT format) {
    switch (format) {
    case D3DFMT_D24S8:
    case D3DFMT_D24FS8:
    case D3DFMT_S8_LOCKABLE:
        return 8;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

GrGpuD3D9::GrGpuD3D9(IDirect3DDevice9* device) : GrGpu(), fDevice(device) {
    GrPrintf("----------------------- create GrGpuD3D9 %p --------------\n", this);

    fDeviceEx = NULL;
    fDevice->QueryInterface(__uuidof(::IDirect3DDevice9Ex), (void**)&fDeviceEx);

    fLastBlendOff = false;
    GR_D3D9(fDevice, SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

    GR_D3D9(fDevice, SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE));
    GR_D3D9(fDevice, SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
    GR_D3D9(fDevice, SetRenderState(D3DRS_LIGHTING, FALSE));
    GR_D3D9(fDevice, SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    GR_D3D9(fDevice, SetRenderState(D3DRS_COLORVERTEX, TRUE));

    fLastVertexState.fFlagBits = 0;
    GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2));
    GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2));
    GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
    GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
    GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CONSTANT));
    GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CONSTANT));

    fLastVertFVF = -1;

    fLastColorArg1 = D3DTA_TEXTURE;

    fLastDrawState.fSamplerState.fFilter = false;
    GR_D3D9(fDevice, SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    GR_D3D9(fDevice, SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    GR_D3D9(fDevice, SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    fNextDrawState.fSamplerState.fWrapX = (GrGpu::WrapModes)-1; // illegal
    fNextDrawState.fSamplerState.fWrapY = (GrGpu::WrapModes)-1; // illegal

    GR_D3D9(fDevice, SetPixelShader(NULL));
    GR_D3D9(fDevice, SetVertexShader(NULL));

    fLastDrawState.fFlagBits = 0;    
    GR_D3D9(fDevice, SetRenderState(D3DRS_DITHERENABLE, FALSE));
    GR_D3D9(fDevice, SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE));

    // illegal values
    fLastDrawState.fSrcBlend = (BlendCoeff)-1;
    fLastDrawState.fDstBlend = (BlendCoeff)-1;
    fLastDrawState.fColor = GrColor_ILLEGAL;
    fLastDrawState.fPointSize = fLastDrawState.fLineWidth = -1;
    fLastDrawState.fTexture = NULL;
    
    GR_D3D9(fDevice,GetRenderTarget(0,&fDefaultRenderTarget.fColor));
    fLastDrawState.fRenderTarget = (GrRenderTarget*) &fDefaultRenderTarget;
    GrAssert(NULL != fDefaultRenderTarget.fColor);

    // We need a stencil buffer to do path rendering.
    D3DSURFACE_DESC rtDesc;
    GR_D3D9(fDefaultRenderTarget.fColor, GetDesc(&rtDesc));
    fDefaultRenderTarget.fStencil = NULL;
    GR_D3D9(fDevice, GetDepthStencilSurface(&fDefaultRenderTarget.fStencil));
    // make sure any existing depth stencil is compatible with the rendertarget
    // and has at least 8 bits of stencil
    if (NULL != fDefaultRenderTarget.fStencil) {
        D3DSURFACE_DESC dsDesc;
        GR_D3D9(fDefaultRenderTarget.fStencil, GetDesc(&dsDesc));
        if (!color_and_stencil_compatible(rtDesc, dsDesc) ||
            format_stencil_bits(dsDesc.Format) < 8) {
            fDefaultRenderTarget.fStencil = NULL;
        } else {
            // add a ref so that we can safely Release in destructor
            fDefaultRenderTarget.fStencil->AddRef();
        }
    }    
    if (NULL == fDefaultRenderTarget.fStencil) {
        fDefaultRenderTarget.fStencil = createStencil(rtDesc.Width, 
                                                      rtDesc.Height,
                                                      rtDesc.MultiSampleType,
                                                      rtDesc.MultiSampleQuality);
        GrAssert(NULL != fDefaultRenderTarget.fStencil);
        GR_D3D9(fDevice, SetDepthStencilSurface(fDefaultRenderTarget.fStencil));
    }

    fLastDrawState.fScissorRect.setEmpty();
    RECT rect;
    rect.left = rect.right = rect.top = rect.bottom = 0;
    GR_D3D9(fDevice,SetScissorRect(&rect));
    
    D3DMATRIX identity;
    memset(&identity, 0, sizeof(identity));
    identity._11 = identity._22 = identity._33 = identity._44 = 1.f;
    for (int i = 0; i < kMatrixModeCount; i++) {
        fLastDrawState.fMatrixModeCache[i].setIdentity();
        GR_D3D9(fDevice, SetTransform(gMatrixMode2D3D9Matrix[i], &identity));
    }
    GR_D3D9(fDevice, 
        SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    fLastTexGen = false;

    fLastDrawState.fViewportW = -1;
    fLastDrawState.fViewportH = -1;

    GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILENABLE, FALSE));
    GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILMASK, 0xffffffff));
    GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff));
    GR_D3D9(fDevice, SetRenderState(D3DRS_COLORWRITEENABLE, 0xf));
    fLastDrawState.fDrawMode = kOther_DrawMode;
    fLastDrawState.fPathPass = (PathPass)-1;
    fLastDrawState.fReverseFill = false;
     
    fNextDrawState = fLastDrawState;
    fNextVertexState = fLastVertexState;

    fLastIndexBuffer = NULL;

    fStageVBuffer = (GrD3D9VertexBuffer*) 
                        this->createVertexBuffer(STAGE_VERTEX_SIZE, true);
    GrAssert(NULL != fStageVBuffer);

    fStageIBuffer = (GrD3D9IndexBuffer*) 
                        this->createIndexBuffer(STAGE_INDEX_SIZE, true);
    GrAssert(NULL != fStageIBuffer);

    TextureDesc dummyDesc = {
        0, 
        kNone_AALevel, 
        1, 
        1, 
        false,
        GrTexture::kAlpha_8_PixelConfig
    };
    fDummyTexture = (GrD3D9Texture*) this->createTexture(dummyDesc, NULL);
    GrAssert(NULL != fDummyTexture);

    fNPOTTextureSupport = kFull_NPOTTextureType;
    D3DCAPS9 caps;
    GR_D3D9(fDevice, GetDeviceCaps(&caps));
    fSingleStencilPassForWinding = 
        0 != (caps.StencilCaps & D3DSTENCILCAPS_TWOSIDED);
    GrAssert(D3DSTENCILOP_INVERT & caps.StencilCaps);
    GrAssert(D3DSTENCILOP_INCR & caps.StencilCaps);
    GrAssert(D3DSTENCILOP_DECR & caps.StencilCaps);
    GrAssert(D3DSTENCILOP_ZERO & caps.StencilCaps);

    // start off with all zeros, keep this after fNextDrawState assignment
    eraseStencil();
    fDefaultRenderTarget.fClearStencil = false;
}

GrGpuD3D9::~GrGpuD3D9() {
    fStageVBuffer->unref();
    fStageIBuffer->unref();
    fDummyTexture->unref();
    // Currently we are assuming that the default render target
    // existed before our constructor was called. We don't ever create
    // it and we never add a ref to it, so don't release.
    // We do create a stencil buffer if needed, though.
    fDefaultRenderTarget.fStencil->Release();
}

IDirect3DSurface9* GrGpuD3D9::createStencil(uint32_t width, 
                                            uint32_t height,
                                            D3DMULTISAMPLE_TYPE msType,
                                            DWORD msQual) {
    IDirect3DSurface9* dsSurface = NULL;
    // Direct3D9 Ex adds a stencil only format.
    if (NULL != fDeviceEx) {
        GR_D3D9(fDeviceEx, CreateDepthStencilSurfaceEx(width, height,
                                                       D3DFMT_S8_LOCKABLE, 
                                                       msType, msQual, FALSE, 
                                                       &dsSurface, NULL, 
                                                       D3DUSAGE_DEPTHSTENCIL));
        fDeviceEx->Release();
    }
    if (NULL == dsSurface) {
        fDevice->CreateDepthStencilSurface(width, height, D3DFMT_D24S8, msType, 
                                           msQual, FALSE, &dsSurface, NULL);
    }
    return dsSurface;
}

GrTexture* GrGpuD3D9::createTexture(const TextureDesc& desc,
                                    const void* srcData) {
    D3DFORMAT d3dformat;
    bool renderTarget = (desc.fFlags & kRenderTarget_TextureFlag);
    if (desc.fAALevel != kNone_AALevel  && renderTarget) {
        GrPrintf("Requested AA RT/Tex but not yet implemented in D3D.");
    }
    if (can_be_texture(desc.fFormat, &d3dformat)) {
        DWORD usage = desc.fDynamic ? D3DUSAGE_DYNAMIC : 0;
        usage |= renderTarget ? D3DUSAGE_RENDERTARGET : 0;
        IDirect3DTexture9* d3dTex = NULL;
    
        GR_D3D9(fDevice, CreateTexture(desc.fWidth, desc.fHeight, 1, usage, 
                                       d3dformat, D3DPOOL_DEFAULT, &d3dTex, 
                                       NULL));

        // In D3D9 the depth-stencil can be larger but not smaller than the RT
        IDirect3DSurface9* depthStencil = NULL;
        D3DSURFACE_DESC dsDesc;
        fDefaultRenderTarget.fStencil->GetDesc(&dsDesc);        
        // check if existing depth stencil is compatible
        if ((renderTarget) && 
                ((desc.fWidth > dsDesc.Width) || 
                 (desc.fHeight > dsDesc.Height) ||
                 (dsDesc.MultiSampleType != D3DMULTISAMPLE_NONE))) {
            depthStencil = createStencil(desc.fWidth, desc.fHeight, 
                                         D3DMULTISAMPLE_NONE, 0);
            GrAssert(NULL != depthStencil);
        }
        if (d3dTex) {
            GrD3D9Texture* texture = new GrD3D9Texture(desc.fWidth, 
                                                       desc.fHeight, 
                                                       desc.fFormat,
                                                       d3dTex, 
                                                       depthStencil,
                                                       true,
                                                       this);
            if (NULL != srcData) {
                texture->uploadTextureData(0, 0, desc.fWidth, 
                                           desc.fHeight, srcData);
            }
            return texture;
        }
    }
    return NULL;
}

GrVertexBuffer* GrGpuD3D9::createVertexBuffer(uint32_t size, bool dynamic) {
    DWORD usage = (dynamic & kRenderTarget_TextureFlag) ? D3DUSAGE_DYNAMIC : 0;
    usage |= D3DUSAGE_WRITEONLY;
    IDirect3DVertexBuffer9* vbuffer = NULL;
    GR_D3D9(fDevice, CreateVertexBuffer(size, usage, 0, 
                                        D3DPOOL_DEFAULT, &vbuffer, NULL));
    if (vbuffer) {
        return new GrD3D9VertexBuffer(size, dynamic, vbuffer, this);
    }
    return NULL;
}

GrIndexBuffer* GrGpuD3D9::createIndexBuffer(uint32_t size, bool dynamic) {
    DWORD usage = (dynamic & kRenderTarget_TextureFlag) ? D3DUSAGE_DYNAMIC : 0;
    usage |= D3DUSAGE_WRITEONLY;
    IDirect3DIndexBuffer9* ibuffer = NULL;
    GR_D3D9(fDevice, CreateIndexBuffer(size, usage, D3DFMT_INDEX16, 
                                       D3DPOOL_DEFAULT, &ibuffer, NULL));
    if (ibuffer) {
        return new GrD3D9IndexBuffer(size, dynamic, ibuffer, this);
    }
    return NULL;
}

bool GrGpuD3D9::flushGraphicsState(PrimitiveTypes type) {
    GrAssert(fNextDrawState.fViewportW != -1);

    if (fNextDrawState.fDrawMode == kRadialTexture_DrawMode ||
        fNextDrawState.fDrawMode == kSweepTexture_DrawMode ||
        fNextDrawState.fDrawMode == kTwoPointRadialTexture_DrawMode) {
        unimpl("Fixed pipe doesn't support radial/sweep gradient");
        return false;
    }

    uint32_t stateDiff = fNextDrawState.fFlagBits ^ fLastDrawState.fFlagBits;

    if (fLastDrawState.fRenderTarget != fNextDrawState.fRenderTarget) {
        setRenderTargetImm();
        GrD3D9RenderTarget& rt = *(GrD3D9RenderTarget*)fNextDrawState.fRenderTarget;
        if (rt.fClearStencil) {
            eraseStencil();
            rt.fClearStencil = false;
        }
        // may need to change how AA is handled.
        stateDiff |= (1 << kAntialias_StateFlag);
    }

    if (stateDiff)
    {
        if (stateDiff & (1<<kDither_StateFlag)) {
            GR_D3D9(fDevice,SetRenderState(D3DRS_DITHERENABLE, 
                    (fNextDrawState.fFlagBits & (1<<kDither_StateFlag)) ? 
                        TRUE : 
                        FALSE));
        }
        if (stateDiff & (1<<kAntialias_StateFlag)) {
            DWORD aa = fNextDrawState.fFlagBits & (1<<kAntialias_StateFlag) ? 
                                TRUE : FALSE;
            GrD3D9RenderTarget& rt = *(GrD3D9RenderTarget*)fNextDrawState.fRenderTarget;
            D3DSURFACE_DESC desc;
            rt.fColor->GetDesc(&desc);
            if (desc.MultiSampleType != D3DMULTISAMPLE_NONE) {
                GR_D3D9(fDevice, SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, aa));
            } else {
                GR_D3D9(fDevice, SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, aa));
            }
        }
        fLastDrawState.fFlagBits = fNextDrawState.fFlagBits;
    }
    bool blendOff = canDisableBlend();
    if (fLastBlendOff != blendOff) {
        GR_D3D9(fDevice, SetRenderState(D3DRS_ALPHABLENDENABLE, 
                         blendOff ? FALSE : TRUE));
        fLastBlendOff = blendOff;
    }
    if (!blendOff) {
        if (fLastDrawState.fSrcBlend != fNextDrawState.fSrcBlend) {
            GR_D3D9(fDevice, SetRenderState(D3DRS_SRCBLEND, 
                                 gXfermodeCoeff2Blend[fNextDrawState.fSrcBlend]));
            fLastDrawState.fSrcBlend = fNextDrawState.fSrcBlend;
        }
        if (fLastDrawState.fDstBlend != fNextDrawState.fDstBlend) {
            GR_D3D9(fDevice, SetRenderState(D3DRS_DESTBLEND, 
                                gXfermodeCoeff2Blend[fNextDrawState.fDstBlend]));
            fLastDrawState.fDstBlend = fNextDrawState.fDstBlend;
        }
    }

    // bind texture and set sampler state
    if (fNextVertexState.fFlagBits & (1 << kTexCoord_VertFlag)) {
        GrD3D9Texture* nextTexture = (GrD3D9Texture*)fNextDrawState.fTexture;
        if (NULL != nextTexture) {
            if (fLastDrawState.fTexture != nextTexture) {
                GR_D3D9(fDevice, SetTexture(0, nextTexture->texture()));
                DWORD nextColorArg1 = nextTexture->format() == D3DFMT_A8 ?
                                    (D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE) :
                                    D3DTA_TEXTURE;                        
                if (fLastColorArg1 != nextColorArg1) {                    
                    GR_D3D9(fDevice, SetTextureStageState(0, 
                                                          D3DTSS_COLORARG1, 
                                                          nextColorArg1));
                    fLastColorArg1 = nextColorArg1;
                }
                fLastDrawState.fTexture = nextTexture;
            }

            if (fLastDrawState.fSamplerState.fFilter != 
                    fNextDrawState.fSamplerState.fFilter) {
                DWORD filter = fNextDrawState.fSamplerState.fFilter ?
                                                       D3DTEXF_LINEAR :
                                                       D3DTEXF_POINT;
                GR_D3D9(fDevice, SetSamplerState(0, D3DSAMP_MAGFILTER, filter));
                GR_D3D9(fDevice, SetSamplerState(0, D3DSAMP_MINFILTER, filter));
                fLastDrawState.fSamplerState.fFilter = 
                    fNextDrawState.fSamplerState.fFilter;
            }
            if (fLastDrawState.fSamplerState.fWrapX != 
                    fNextDrawState.fSamplerState.fWrapX) {
                GR_D3D9(fDevice, SetSamplerState(0, D3DSAMP_ADDRESSU, 
                    gTileMode2D3D9Wrap[fNextDrawState.fSamplerState.fWrapX]));
                fLastDrawState.fSamplerState.fWrapX = 
                    fNextDrawState.fSamplerState.fWrapX;
            }
            if (fLastDrawState.fSamplerState.fWrapY != 
                    fNextDrawState.fSamplerState.fWrapY) {
                GR_D3D9(fDevice, SetSamplerState(0, D3DSAMP_ADDRESSV, 
                    gTileMode2D3D9Wrap[fNextDrawState.fSamplerState.fWrapY]));
                fLastDrawState.fSamplerState.fWrapY = 
                    fNextDrawState.fSamplerState.fWrapY;
            }
        } else {
            GrAssert(!"Rendering with texture vert flag set but no bound texture");
            if (NULL != fLastDrawState.fTexture) {            
                GR_D3D9(fDevice,SetTexture(0, NULL));
    //            GrPrintf("---- bindtexture 0\n");
                fLastDrawState.fTexture = NULL;
            }
        }
    }

    // check for circular rendering
    GrAssert(!(fNextVertexState.fFlagBits & (1 << kTexCoord_VertFlag)) ||
             NULL == fNextDrawState.fRenderTarget || 
             NULL == fNextDrawState.fTexture ||
             fNextDrawState.fTexture->asRenderTarget() != fNextDrawState.fRenderTarget);

    if ((type == GrGpu::kLineStrip_PrimitiveType ||
         type == GrGpu::kLines_PrimitiveType) &&
        fLastDrawState.fLineWidth != fNextDrawState.fLineWidth) {
        // D3D9 doesn't support wide lines!
        //GrAssert(fNextDrawState.fLineWidth == 1);
    }

    bool stencilChange = 
                fLastDrawState.fPathPass != fNextDrawState.fPathPass ||
                (kNone_PathPass != fNextDrawState.fPathPass &&
                 fLastDrawState.fReverseFill != fNextDrawState.fReverseFill);
                        
    if (stencilChange) {
        switch (fNextDrawState.fPathPass) {
        case kNone_PathPass:
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILENABLE, FALSE));
            GR_D3D9(fDevice, SetRenderState(D3DRS_COLORWRITEENABLE, 0xf));
            if (!fSingleStencilPassForWinding) {
                GR_D3D9(fDevice, SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
            }
            break;
        case kEvenOddStencil_PathPass:
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILENABLE, TRUE));
            if (fSingleStencilPassForWinding) {
                GR_D3D9(fDevice, SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE));
            } else {
                GR_D3D9(fDevice, SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
            }
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INVERT));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INVERT));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INVERT));
            GR_D3D9(fDevice, SetRenderState(D3DRS_COLORWRITEENABLE, 0x0));
            break;
        case kEvenOddColor_PathPass:
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILENABLE, TRUE));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILREF, 0xffffffff));
            if (fSingleStencilPassForWinding) {
                GR_D3D9(fDevice, SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE));
            } else {
                GR_D3D9(fDevice, SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
            }
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFUNC, 
                fNextDrawState.fReverseFill ? D3DCMP_NOTEQUAL : D3DCMP_EQUAL));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO));
            GR_D3D9(fDevice, SetRenderState(D3DRS_COLORWRITEENABLE, 0xf));
            break;
        case kWindingStencil1_PathPass:
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILENABLE, TRUE));
            if (fSingleStencilPassForWinding) {
                GR_D3D9(fDevice, SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE));
                GR_D3D9(fDevice, SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS));
                GR_D3D9(fDevice, SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_DECR));
                GR_D3D9(fDevice, SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR));
                GR_D3D9(fDevice, SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_DECR));
            } else {
                GR_D3D9(fDevice, SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
            }
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS));            
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCR));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR));
            GR_D3D9(fDevice, SetRenderState(D3DRS_COLORWRITEENABLE, 0x0));
            break;
        case kWindingStencil2_PathPass:
            GrAssert(!fSingleStencilPassForWinding);
            GR_D3D9(fDevice, SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILENABLE, TRUE));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS));            
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_DECR));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR));
            GR_D3D9(fDevice, SetRenderState(D3DRS_COLORWRITEENABLE, 0x0));
            break;
        case kWindingColor_PathPass:
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILENABLE, TRUE));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILREF, 0x00000000));
            if (fSingleStencilPassForWinding) {
                GR_D3D9(fDevice, SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE));
            } else {
                GR_D3D9(fDevice, SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
            }
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFUNC, 
                    fNextDrawState.fReverseFill ? D3DCMP_EQUAL : D3DCMP_NOTEQUAL));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO));
            GR_D3D9(fDevice, SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO));
            GR_D3D9(fDevice, SetRenderState(D3DRS_COLORWRITEENABLE, 0xf));
            break;
        default:
            GrAssert(!"Unexpected path pass.");
            break;
        }
        fLastDrawState.fPathPass = fNextDrawState.fPathPass;
        fLastDrawState.fReverseFill = fNextDrawState.fReverseFill;
    }
    fLastDrawState.fDrawMode    = fNextDrawState.fDrawMode;
    
    //////////////////////////////////////////////////////////////////////////
    // Fixed pipe stuff

    DWORD vertFVF = (fNextDrawState.fDrawMode == kTextGlyphs_DrawMode) ? 
                gTextFVF :
                gDeclToFVFs[gVertFlags2VertDeclIdx[fNextVertexState.fFlagBits]];
    GrAssert(-1 != vertFVF);
    if (fLastVertFVF != vertFVF) {
        GR_D3D9(fDevice, SetFVF(vertFVF));
        fLastVertFVF = vertFVF;
    }

    // this has to stay after the set render target because
    // setrendertarget resets the viewport
    if (fLastDrawState.fViewportW != fNextDrawState.fViewportW ||
        fLastDrawState.fViewportH != fNextDrawState.fViewportH) {
        D3DVIEWPORT9 vp;
        vp.X = vp.Y = 0;
        vp.MinZ = 0; vp.MaxZ = 1;
        vp.Width  = fNextDrawState.fViewportW;
        vp.Height = fNextDrawState.fViewportH;
        GR_D3D9(fDevice, SetViewport(&vp));
        
        D3DMATRIX mat;
        sk_bzero(&mat, sizeof(mat));
    
        float invW = 1.f / fNextDrawState.fViewportW;
        float invH = 1.f / fNextDrawState.fViewportH;

        mat._11 = 2.f * invW;
        mat._22 = -2.f * invH;
        //mat._33 = -1.f;
        mat._33 = 1.f;
        mat._44 = 1;
    
        // included here is 1/2 pixel adjustment because
        // d3d9 puts pixel *centers* at integer offset in viewport space.
        mat._41 = -1.f - invW;
        mat._42 = 1.f + invH;

        GR_D3D9(fDevice, SetTransform(D3DTS_PROJECTION, &mat));

        fLastDrawState.fViewportW = fNextDrawState.fViewportW;
        fLastDrawState.fViewportH = fNextDrawState.fViewportH;
    }

    uint32_t vertDiff = fNextVertexState.fFlagBits ^ fLastVertexState.fFlagBits;

    if (vertDiff) {
        if (vertDiff & (1 << kTexCoord_VertFlag)) {            
            if (fNextVertexState.fFlagBits & (1 << kTexCoord_VertFlag)) {
                GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_COLOROP, 
                                                      D3DTOP_MODULATE));
                GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_ALPHAOP, 
                                                      D3DTOP_MODULATE));
            } else {
                GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_COLOROP, 
                                                      D3DTOP_SELECTARG2));
                GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_ALPHAOP, 
                                                      D3DTOP_SELECTARG2));
            }
        }
        if (vertDiff & (1 << kColors_VertFlag)) {
            if (fNextVertexState.fFlagBits & (1 << kColors_VertFlag)) {
                GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_COLORARG2,
                                                      D3DTA_CURRENT));
                GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_ALPHAARG2, 
                                                      D3DTA_CURRENT));
            } else {
                GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_COLORARG2, 
                                                      D3DTA_CONSTANT));
                GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_ALPHAARG2, 
                                                      D3DTA_CONSTANT));                
            }
        }
        fLastVertexState.fFlagBits = fNextVertexState.fFlagBits;
    }
 
    if (fLastDrawState.fTexture != fDummyTexture &&
        !(fNextVertexState.fFlagBits & ((1 << kColors_VertFlag) || 
                                     (1 << kColors_VertFlag)))) {
        GR_D3D9(fDevice, SetTexture(0, fDummyTexture->texture()));
        fLastDrawState.fTexture = fDummyTexture;
    }

    if (fLastDrawState.fPointSize != fNextDrawState.fPointSize) {
        GR_D3D9(fDevice,SetRenderState(D3DRS_POINTSIZE, 
                                       *(DWORD*)&fNextDrawState.fPointSize));
        fLastDrawState.fPointSize = fNextDrawState.fPointSize;
    }
    
    if (!(fNextVertexState.fFlagBits & (1 << kColors_VertFlag)) &&
        fLastDrawState.fColor != fNextDrawState.fColor) {
        GR_D3D9(fDevice, SetTextureStageState(0, D3DTSS_CONSTANT, 
                                              fNextDrawState.fColor));
        fLastDrawState.fColor = fNextDrawState.fColor;
    }
    bool mvChanged = fLastDrawState.fMatrixModeCache[kModelView_MatrixMode] != 
                     fNextDrawState.fMatrixModeCache[kModelView_MatrixMode];
    if (mvChanged) {
        D3DMATRIX mat;
        gr_matrix_to_d3d_matrix(&mat, 
            fNextDrawState.fMatrixModeCache[kModelView_MatrixMode]);
        GR_D3D9(fDevice, SetTransform(
            gMatrixMode2D3D9Matrix[kModelView_MatrixMode], &mat));
        fLastDrawState.fMatrixModeCache[kModelView_MatrixMode] = 
            fNextDrawState.fMatrixModeCache[kModelView_MatrixMode];
    }

    // Since fixed-pipe FVF doesn't allow using positions
    // as tex coords like vertex decls do in for shaders, we
    // use tex gen
    // D3D9 tex coord gen uses the camera space vertex pos as the 
    // texture coordinates. We want to use the worldspace pos so
    // we invert the view matrix as part of the texture matrix.
    if ((fNextVertexState.fFlagBits & (1 << kTexCoord_VertFlag))) {
        bool texGen = 0 != (fNextVertexState.fFlagBits & 
                      (1 << kPositionAsTexCoord_VertFlag));

        bool texGenChange = fLastTexGen != texGen;

        if (fLastDrawState.fMatrixModeCache[kTexture_MatrixMode] != 
            fNextDrawState.fMatrixModeCache[kTexture_MatrixMode] || 
            texGenChange ||
            (texGen && mvChanged)) {
            GrMatrix* m;
            GrMatrix temp;
            D3DMATRIX d3dMat;
            if (texGen) {
                fNextDrawState.fMatrixModeCache[kModelView_MatrixMode].
                    invert(&temp);
                temp.postConcat(fNextDrawState.fMatrixModeCache[kTexture_MatrixMode]);
                m =  &temp;
            } else {
                m = &fNextDrawState.fMatrixModeCache[kTexture_MatrixMode];
            }

            gr_matrix_to_d3d_matrix(&d3dMat, *m);
            GR_D3D9(fDevice, SetTransform(
                gMatrixMode2D3D9Matrix[kTexture_MatrixMode], &d3dMat));
            fLastDrawState.fMatrixModeCache[kTexture_MatrixMode] = 
                fNextDrawState.fMatrixModeCache[kTexture_MatrixMode];
            if (texGenChange) {
                GR_D3D9(fDevice, SetTextureStageState(0, 
                            D3DTSS_TEXCOORDINDEX,
                            texGen ? D3DTSS_TCI_CAMERASPACEPOSITION : 0));
            }
            fLastTexGen = texGen;
        }
    }
    return true;
}

void GrGpuD3D9::flushScissor() {
    if (fLastDrawState.fScissorRect != fNextDrawState.fScissorRect) {
        RECT rect;
        rect.left = fNextDrawState.fScissorRect.fLeft;
        rect.right = fNextDrawState.fScissorRect.fRight;
        rect.top = fNextDrawState.fScissorRect.fTop;
        rect.bottom = fNextDrawState.fScissorRect.fBottom;
    
        GR_D3D9(fDevice, SetScissorRect(&rect));
        fLastDrawState.fScissorRect != fNextDrawState.fScissorRect;
    }

}

void GrGpuD3D9::eraseColor(GrColor color) {
    
    DWORD clr = D3DCLEAR_TARGET;

    if (fLastDrawState.fRenderTarget != fNextDrawState.fRenderTarget) {
        setRenderTargetImm();
        // In D3D9 setting the render target resets the viewport
        fLastDrawState.fViewportH = -1;
        GrD3D9RenderTarget& rt = *(GrD3D9RenderTarget*)fNextDrawState.fRenderTarget;
        if ((NULL != rt.fStencil) && rt.fClearStencil) {
            clr |= D3DCLEAR_STENCIL;
        }
    }

    D3DCOLOR d3dColor = D3DCOLOR_ARGB(GrColorUnpackA(color),
                                      GrColorUnpackR(color),
                                      GrColorUnpackG(color),
                                      GrColorUnpackB(color));
     
    // we enable the scissor in the preamble and flush 
    // assumes it is always enabled
    GR_D3D9(fDevice, SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE));
    GR_D3D9(fDevice, Clear(0, NULL, clr, d3dColor, 0.f, 0x0));    
    GR_D3D9(fDevice, SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE));
}

void GrGpuD3D9::eraseStencil() {
    if (fLastDrawState.fRenderTarget != fNextDrawState.fRenderTarget) {
        setRenderTargetImm();
        // In D3D9 setting the render target resets the viewport
        fLastDrawState.fViewportH = -1;
    }

    // we enable the scissor in the preamble and flush 
    // assumes it is always enabled
    GR_D3D9(fDevice, SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE));
    GR_D3D9(fDevice,Clear(0, NULL, D3DCLEAR_STENCIL, 0x0, 0.f, 0x0));    
    GR_D3D9(fDevice, SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE));
}

GrD3D9VertexBuffer* GrGpuD3D9::setupVBufferStage(int vsize,
                                                 int* baseVertex,
                                                 int vertexCount,
                                                 DrawModes mode) {
    GrD3D9VertexBuffer*vbuf;
    if (vsize*(vertexCount) > STAGE_VERTEX_SIZE) {
        GrPrintf("Staging vertex buffer is too small!");
        vbuf = (GrD3D9VertexBuffer*) createVertexBuffer(vsize*vertexCount, true);
        if (NULL == vbuf) {
            GrAssert(!"Temporary vertex buffer failed!");
            return NULL;
        }
    } else {
        vbuf = fStageVBuffer;
        // add a reference so that caller can unref without deleting
        vbuf->ref();
    }
    void* vptr = vbuf->lock();
    if (NULL == vptr) {
        GrAssert(!"Locking staging/temp vbuffer failed!");
        vbuf->unref();
        return NULL;
    }
    vptr = (char*) vptr;
    
    if (mode == kTextGlyphs_DrawMode) {
        GrAssert((fNextVertexState.fFlagBits & (1 << kTexCoord_VertFlag)) &&
                    !(fNextVertexState.fFlagBits & (1 << kColors_VertFlag)) &&
                    !(fNextVertexState.fFlagBits & 
                                      (1 << kPositionAsTexCoord_VertFlag)));
        GrAssert(sizeof(GrGpuTextVertex) == 2*sizeof(float));
        for (int i = 0; i < vertexCount; ++i) {
            GrGpuTextVertex* posxy = (GrGpuTextVertex*)((char*)vptr + i*vsize);
            float* posz  = (float*)posxy + 2;
            GrGpuTextVertex* tex = (GrGpuTextVertex*)(posz+1);
            *posxy = *((GrGpuTextVertex*)fNextVertexState.fArrays.positions + i + *baseVertex);
            *posz = .5;
            *tex = *((GrGpuTextVertex*)fNextVertexState.fArrays.texCoords + i + *baseVertex);
        }
    } else {
        switch (fNextVertexState.fFlagBits) {
            GrAssert(sizeof(GrPoint) == 2*sizeof(float));
            case 0: // position only
            case (1 << kTexCoord_VertFlag) | 
                 (1 << kPositionAsTexCoord_VertFlag): 
                for (int i = 0; i < vertexCount; ++i) {
                    GrPoint* posxy = (GrPoint*)((char*)vptr + i*vsize);
                    float* posz = (float*)(posxy + 1);
                    *posxy = *((GrPoint*)fNextVertexState.fArrays.positions + i + *baseVertex);
                    *posz = .5;
                }
                break;
            case (1 << kTexCoord_VertFlag):
                for (int i = 0; i < vertexCount; ++i) {
                    GrPoint* posxy = (GrPoint*)((char*)vptr + i*vsize);
                    float* posz = (float*)(posxy + 1);
                    GrPoint* tex = (GrPoint*)(posz + 1);
                    *posxy = *((GrPoint*)fNextVertexState.fArrays.positions + i + *baseVertex);
                    *posz = .5;
                    *tex = *((GrPoint*)fNextVertexState.fArrays.texCoords + i + *baseVertex);
                }
                break;
            case (1 << kColors_VertFlag):
            case (1 << kTexCoord_VertFlag) |
                 (1 << kPositionAsTexCoord_VertFlag) |
                 (1 << kColors_VertFlag):
                for (int i = 0; i < vertexCount; ++i) {
                    GrPoint* posxy = (GrPoint*)((char*)vptr + i*vsize);
                    float* posz = (float*)(posxy + 1);
                    uint32_t* col = (uint32_t*)(posz + 1);
                    *posxy = *((GrPoint*)fNextVertexState.fArrays.positions + i + *baseVertex);
                    *posz = .5;
                    *col =  *((uint32_t*)fNextVertexState.fArrays.colors + i + *baseVertex);
                }
                break;
            case (1 << kTexCoord_VertFlag) | (1 << kColors_VertFlag):
                for (int i = 0; i < vertexCount; ++i) {
                    GrPoint* posxy = (GrPoint*)((char*)vptr + i*vsize);
                    float* posz = (float*)posxy + 2;
                    uint32_t* col = (uint32_t*)(posz + 1);
                    GrPoint* tex = (GrPoint*)(col + 1);
                    *posxy = *((GrPoint*)fNextVertexState.fArrays.positions + i + *baseVertex);
                    *posz = .5;
                    *col =  *((uint32_t*)fNextVertexState.fArrays.colors + i + *baseVertex);
                    *tex = *((GrPoint*)fNextVertexState.fArrays.texCoords + i + *baseVertex);
                }
                break;
            default:
                GrAssert(!"Unexpected vertex flags!");
        }
    }
    *baseVertex = 0;
    vbuf->unlock();
    return vbuf;
}

void GrGpuD3D9::setRenderTargetImm() {
    GrD3D9RenderTarget& rt = *(GrD3D9RenderTarget*)fNextDrawState.fRenderTarget;
    GrAssert(NULL != rt.fColor);
    GR_D3D9(fDevice, SetRenderTarget(0, 
            (IDirect3DSurface9*) rt.fColor));
    if (NULL != rt.fStencil) {
        GR_D3D9(fDevice, SetDepthStencilSurface(rt.fStencil));
    } else {
        GrAssert(NULL != fDefaultRenderTarget.fStencil);
        GR_DEBUGCODE(D3DSURFACE_DESC rtDesc;)
        GR_DEBUGCODE(D3DSURFACE_DESC dsDesc;)
        GR_DEBUGCODE(GR_D3D9(rt.fColor,       GetDesc(&rtDesc));)
        GR_DEBUGCODE(GR_D3D9(fDefaultRenderTarget.fStencil, \
                        GetDesc(&dsDesc));)
        GR_DEBUGCODE(GrAssert(color_and_stencil_compatible(rtDesc, 
                                                            dsDesc));)
        GR_D3D9(fDevice, 
            SetDepthStencilSurface(fDefaultRenderTarget.fStencil));        
    }
    fLastDrawState.fRenderTarget = fNextDrawState.fRenderTarget;
}

GrD3D9IndexBuffer* GrGpuD3D9::setupIBufferStage(int* startIndex, int indexCount,
                                                const uint16_t* indices) {
    GrD3D9IndexBuffer* ibuf;
    if (indexCount*2 > STAGE_INDEX_SIZE) {
        GrPrintf("Staging index buffer is too small!");
        ibuf = (GrD3D9IndexBuffer*) createIndexBuffer(indexCount*2, true);
        if (NULL == ibuf) {
            GrAssert(!"Temporary index buffer is too small!");
            return NULL;
        }
    } else {
        ibuf = fStageIBuffer;
        // add a reference so that caller can unref without deleting
        ibuf->ref();
    }
    void* iptr = ibuf->lock();
    if (NULL == iptr) {
        GrAssert(!"Locking staging/temp ibuffer failed!");
        ibuf->unref();
        return NULL;
    }
    memcpy(iptr, indices + *startIndex, 2*indexCount);
    *startIndex = 0;
    ibuf->unlock();
    return ibuf;
}

int GrGpuD3D9::vertexSize(int vertFlagBits, GrGpu::DrawModes mode) {
    if (mode == kTextGlyphs_DrawMode) {
        return 5*sizeof(float);
    } else {
        switch (vertFlagBits) {
            case 0: // position only
            case (1 << kTexCoord_VertFlag) | 
                 (1 << kPositionAsTexCoord_VertFlag):
                return 3*sizeof(float);
            case (1 << kTexCoord_VertFlag):
                return 5*sizeof(float);
            case (1 << kColors_VertFlag):
            case (1 << kTexCoord_VertFlag) | 
                 (1 << kPositionAsTexCoord_VertFlag) | 
                 (1 << kColors_VertFlag):
                return 3*sizeof(float) + 4;
            case (1 << kTexCoord_VertFlag) | (1 << kColors_VertFlag):
                return 5*sizeof(float) + 4;
            default:
                GrAssert(!"Unexpected vertex flags!");
                return 0;
        }
    }
}

void GrGpuD3D9::drawIndexArrayApi(PrimitiveTypes type,
                                  int baseVertex,
                                  int vertexCount,
                                  int indexCount,
                                  const uint16_t* indexArray,
                                  bool redrawHint) {
    int vsize = vertexSize(fNextVertexState.fFlagBits, fNextDrawState.fDrawMode);

    GrD3D9VertexBuffer* vbuf;
    if (fNextVertexState.fUsingBuffer) {
        vbuf = (GrD3D9VertexBuffer*) fNextVertexState.fBuffer;
    } else {
        vbuf = setupVBufferStage(vsize, &baseVertex, vertexCount,
                                 fNextDrawState.fDrawMode);
        if (NULL == vbuf) {
            return;
        }
    }
    int startIndex = 0;
    GrD3D9IndexBuffer* ibuf = 
                    setupIBufferStage(&startIndex, indexCount, indexArray);    
    if (NULL == ibuf) {
        vbuf->unref();
    }

    GR_D3D9(fDevice,SetStreamSource(0, vbuf->buffer(), 0, vsize));
    GR_D3D9(fDevice,SetIndices(ibuf->buffer()));
    GR_D3D9(fDevice,DrawIndexedPrimitive(gPrimType2D3D9PrimType[type], 
                                            baseVertex, 0, vertexCount, startIndex,
                                            vertex_to_primitive_count(type,
                                                            indexCount)));
    vbuf->unref();
    ibuf->unref();
}

void GrGpuD3D9::drawIndexBufferApi(PrimitiveTypes type,
                                   int baseVertex,
                                   int startIndex,
                                   int vertexCount,
                                   int indexCount,                                   
                                   GrIndexBuffer* indexBuffer,
                                   bool redrawHint) {
    int vsize = vertexSize(fNextVertexState.fFlagBits, fNextDrawState.fDrawMode);
    GrD3D9VertexBuffer* vbuf;
    bool unrefVBuf = false;
    if (fNextVertexState.fUsingBuffer) {
        vbuf = (GrD3D9VertexBuffer*) fNextVertexState.fBuffer;
    } else {
        vbuf = setupVBufferStage(vsize, &baseVertex, vertexCount, 
                                 fNextDrawState.fDrawMode);
        if (NULL == vbuf) {
            return;
        }
    }
    GR_D3D9(fDevice,SetStreamSource(0, vbuf->buffer(), 0, vsize));
    GR_D3D9(fDevice,SetIndices(((GrD3D9IndexBuffer*)indexBuffer)->buffer()));
    GR_D3D9(fDevice,DrawIndexedPrimitive(gPrimType2D3D9PrimType[type], 
                                         baseVertex, 0, vertexCount, startIndex,
                                         vertex_to_primitive_count(type, 
                                                                  indexCount)));
    vbuf->unref();
}

void GrGpuD3D9::drawNonIndexedApi(PrimitiveTypes type,
                                  int baseVertex,
                                  int vertexCount,
                                  bool redrawHint) {
    
    int vsize = vertexSize(fNextVertexState.fFlagBits, fNextDrawState.fDrawMode);
    GrD3D9VertexBuffer* vbuf;

    if (fNextVertexState.fUsingBuffer) {
        vbuf = (GrD3D9VertexBuffer*) fNextVertexState.fBuffer;
    } else {
        vbuf = setupVBufferStage(vsize, &baseVertex, vertexCount, 
                                 fNextDrawState.fDrawMode);
    }
    GR_D3D9(fDevice,SetStreamSource(0, vbuf->buffer(), 0, vsize));
    GR_D3D9(fDevice,DrawPrimitive(gPrimType2D3D9PrimType[type], baseVertex, 
                                 vertex_to_primitive_count(type, vertexCount)));
    vbuf->unref();
}

void GrGpuD3D9::notifyVertexBufferBind(GrD3D9VertexBuffer* buffer) {
}

void GrGpuD3D9::notifyVertexBufferDelete(GrD3D9VertexBuffer* buffer) {
    if (fNextVertexState.fUsingBuffer && fNextVertexState.fBuffer == buffer) {
        fNextVertexState.fBuffer = NULL;
    }
}

void GrGpuD3D9::notifyIndexBufferBind(GrD3D9IndexBuffer* buffer) {
}

void GrGpuD3D9::notifyIndexBufferDelete(GrD3D9IndexBuffer* buffer) {
    if (fLastIndexBuffer == buffer) {
        fLastIndexBuffer = NULL;
    }
}

void GrGpuD3D9::notifyTextureDelete(GrD3D9Texture* texture) {
    if (fNextDrawState.fTexture == texture ||
        fLastDrawState.fTexture == texture) {
        fNextDrawState.fTexture = NULL;
        fLastDrawState.fTexture = NULL;
        GR_D3D9(fDevice, SetTexture(0, NULL));
    }
    if (fNextDrawState.fRenderTarget == texture->asRenderTarget() || 
        fLastDrawState.fRenderTarget == texture->asRenderTarget()) {        
        fNextDrawState.fRenderTarget = (GrRenderTarget*) &fDefaultRenderTarget;
        setRenderTargetImm();
    }
}

void GrGpuD3D9::notifyTextureRemoveRenderTarget(GrD3D9Texture* texture) {
    if (fNextDrawState.fRenderTarget == texture->asRenderTarget() || 
        fLastDrawState.fRenderTarget == texture->asRenderTarget()) {        
        fNextDrawState.fRenderTarget = (GrRenderTarget*) &fDefaultRenderTarget;
        setRenderTargetImm();
    }
}

////////////////////////////////////////////////////////////////////////////////

GrD3D9Texture::GrD3D9Texture(uint32_t width, 
                             uint32_t height,
                             GrTexture::PixelConfig config,
                             IDirect3DTexture9* texture,
                             IDirect3DSurface9* stencil,
                             bool clearStencil,
                             GrGpuD3D9* gpuD3D9) : 
        INHERITED(width, height, width, height, config),
        fTexture(texture),
        fStencil(stencil),
        fGpuD3D9(gpuD3D9) {    
    GrAssert(NULL != fTexture);
    fTexture->GetLevelDesc(0, &fDesc);
    
    if (fDesc.Usage & D3DUSAGE_RENDERTARGET) {
        BOOL result = fTexture->GetSurfaceLevel(0, &fRenderTarget.fColor);
        GrAssert(S_OK == result && NULL != fRenderTarget.fColor);
        fRenderTarget.fStencil = stencil;
        fRenderTarget.fClearStencil = clearStencil;
    } else {
        GrAssert(NULL == stencil);
        fRenderTarget.fColor = NULL;
        fRenderTarget.fStencil = NULL;
        fRenderTarget.fClearStencil = false;
    }
}

GrD3D9Texture::~GrD3D9Texture() {
    fGpuD3D9->notifyTextureDelete(this);
    if (NULL != fRenderTarget.fColor) {
        fRenderTarget.fColor->Release();
    }
    if (NULL != fRenderTarget.fStencil) {
        fRenderTarget.fStencil->Release();
    }
    if (NULL != fTexture) {
        fTexture->Release();
    }
}
    
void GrD3D9Texture::abandon() {
    GrAssert(NULL != fTexture);
    // release on device already deleted the objects?
    fTexture = NULL;
    fRenderTarget.fColor = NULL;
    fRenderTarget.fStencil = NULL;
}

bool GrD3D9Texture::isRenderTarget() {
    GrAssert(NULL != fTexture);
    return (fDesc.Usage & D3DUSAGE_RENDERTARGET);
}

void GrD3D9Texture::removeRenderTarget() {
    fGpuD3D9->notifyTextureRemoveRenderTarget(this);
    if (NULL != fRenderTarget.fColor) {
        fRenderTarget.fColor->Release();
        fRenderTarget.fColor = NULL;
    }
    if (NULL != fRenderTarget.fStencil) {
        fRenderTarget.fStencil->Release();
        fRenderTarget.fStencil = NULL;
    }
    fRenderTarget.fClearStencil = false;
}

void GrD3D9Texture::uploadTextureData(uint32_t x,
                                      uint32_t y,
                                      uint32_t width,
                                      uint32_t height,
                                      const void* srcData) {
    GrAssert(NULL != fTexture);
    HRESULT hr;
#if 0 // is it ever beneficial to lock the texture directly?
    if (fDesc.Usage & D3DUSAGE_DYNAMIC) {
        D3DLOCKED_RECT lock;
        RECT rect;
        rect.left = x;
        rect.right = x + width;
        rect.top = y;
        rect.bottom = y + height;
        hr = fTexture->LockRect(0, &lock, &rect, 0);
        if (FAILED(hr)) {
            GrAssert(!"Failed to lock texture!");
            return;
        }
        int bpp = format_bytes(fDesc.Format);
        if (lock.Pitch == width * bpp) {
            memcpy(lock.pBits, srcData, width*height*bpp);
        } else {
            for (uint32_t y = 0; y < height; ++y) {
                memcpy((char*)lock.pBits + y * lock.Pitch,
                       (char*)srcData + y*width*bpp, width*bpp);
            }
        }
        hr = fTexture->UnlockRect(0);
        GrAssert(SUCCEEDED(hr));
    } else 
#endif
    {
        // should the temp textures be cached 
        // somewhere so we aren't recreating them?
        IDirect3DDevice9* device;
        hr = fTexture->GetDevice(&device);
        if (FAILED(hr) || NULL == device) {
            GrAssert("getting device from texture failed!");
            return;
        }
        IDirect3DTexture9* tempTexture;
        GR_D3D9(device, CreateTexture(width, height, 1, 0, fDesc.Format, 
                                   D3DPOOL_SYSTEMMEM, &tempTexture, NULL));
        GrAssert(NULL != tempTexture);
        IDirect3DSurface9* tempSurface;
        GR_D3D9(tempTexture, GetSurfaceLevel(0, &tempSurface));
        GrAssert(NULL != tempTexture);

        D3DLOCKED_RECT lock;
        GR_D3D9(tempSurface, LockRect(&lock, NULL, 0));
        GrAssert(NULL != lock.pBits);

        // For 4444 D3D uses ARGB for while Skia uses RGBA
        if (D3DFMT_A4R4G4B4 == fDesc.Format) {
            WORD* src = (WORD*)srcData;
            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < width; ++x, ++src) {
                    WORD* dst = (WORD*)((char*)lock.pBits + y * lock.Pitch)+x;
                    *dst = ((0xfff0 & *src) >> 4) | ((0x000f & *src) << 12);                    
                }
            }
        } else {
            int bpp = format_bytes(fDesc.Format);        
            if (lock.Pitch == width * bpp) {
                memcpy(lock.pBits, srcData, width*height*bpp);
            } else {
                for (uint32_t y = 0; y < height; ++y) {
                    memcpy((char*)lock.pBits + y * lock.Pitch,
                           (char*)srcData + y*width*bpp, width*bpp);
                }
            }
        }
        GR_D3D9(tempSurface, UnlockRect());

        IDirect3DSurface9* level0;
        GR_D3D9(fTexture, GetSurfaceLevel(0, &level0));

        POINT xy = {x, y};
        GR_D3D9(device, UpdateSurface(tempSurface, NULL, level0, &xy));

        tempSurface->Release();
        tempTexture->Release();
        level0->Release();
        device->Release();
    }
}

////////////////////////////////////////////////////////////////////////////////

GrD3D9VertexBuffer::GrD3D9VertexBuffer(uint32_t size, 
                                       bool dynamic,
                                       IDirect3DVertexBuffer9* vbuffer,
                                       GrGpuD3D9* gpuD3D9) :
            INHERITED(size, dynamic),
            fBuffer(vbuffer),
            fLocked(false),
            fGpuD3D9(gpuD3D9) {
    HRESULT hr = fBuffer->GetDesc(&fDesc);
    GrAssert(SUCCEEDED(hr));
}

GrD3D9VertexBuffer::~GrD3D9VertexBuffer() {
    fGpuD3D9->notifyVertexBufferDelete(this);
    if (NULL != fBuffer) {
        fBuffer->Release();
    }
}

void GrD3D9VertexBuffer::abandon() {
    GrAssert(NULL != fBuffer);
    fBuffer = NULL;
}

void* GrD3D9VertexBuffer::lock() {
    GrAssert(NULL != fBuffer);
    HRESULT hr;
    void* data = NULL;
    hr = fBuffer->Lock(0, fDesc.Size, &data, D3DLOCK_DISCARD);
    fLocked = SUCCEEDED(hr);
    GrAssert(fLocked && NULL != data);
    return data;
}

void GrD3D9VertexBuffer::unlock() {
    GrAssert(fLocked);
    HRESULT hr = fBuffer->Unlock();
    GrAssert(SUCCEEDED(hr));
}

bool GrD3D9VertexBuffer::isLocked() {
    return fLocked;
}

bool GrD3D9VertexBuffer::updateData(const void* src, uint32_t srcSizeInBytes) {
    GrAssert(srcSizeInBytes <= fDesc.Size);
    HRESULT hr;
    void* data;
    hr = fBuffer->Lock(0, fDesc.Size, &data, D3DLOCK_DISCARD);
    GrAssert(SUCCEEDED(hr));
    if (SUCCEEDED(hr)) {
        fLocked = true;
        GrAssert(NULL != data);
        memcpy(data, src, srcSizeInBytes);
        hr = fBuffer->Unlock();
        fLocked = SUCCEEDED(hr);
        GrAssert(!fLocked);
        return !fLocked;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

GrD3D9IndexBuffer::GrD3D9IndexBuffer(uint32_t size, 
                                     bool dynamic,
                                     IDirect3DIndexBuffer9* ibuffer,
                                     GrGpuD3D9* gpuD3D9) :
            INHERITED(size, dynamic),
            fBuffer(ibuffer),
            fLocked(false),
            fGpuD3D9(gpuD3D9) {
    HRESULT hr = fBuffer->GetDesc(&fDesc);
    GrAssert(SUCCEEDED(hr));
}

GrD3D9IndexBuffer::~GrD3D9IndexBuffer() {
    fGpuD3D9->notifyIndexBufferDelete(this);
    if (NULL != fBuffer) {
        fBuffer->Release();
    }
}

void GrD3D9IndexBuffer::abandon() {
    GrAssert(NULL != fBuffer);
    fBuffer = NULL;
}

void* GrD3D9IndexBuffer::lock() {
    GrAssert(NULL != fBuffer);
    HRESULT hr;
    void* data = NULL;
    hr = fBuffer->Lock(0, fDesc.Size, &data, D3DLOCK_DISCARD);
    fLocked = SUCCEEDED(hr);
    GrAssert(fLocked && NULL != data);
    return data;
}

void GrD3D9IndexBuffer::unlock() {
    GrAssert(fLocked);
    HRESULT hr = fBuffer->Unlock();
    GrAssert(SUCCEEDED(hr));
}

bool GrD3D9IndexBuffer::isLocked() {
    return fLocked;
}

bool GrD3D9IndexBuffer::updateData(const void* src, uint32_t srcSizeInBytes) {
    GrAssert(srcSizeInBytes <= fDesc.Size);
    HRESULT hr;
    void* data;
    hr = fBuffer->Lock(0, fDesc.Size, &data, D3DLOCK_DISCARD);
    GrAssert(SUCCEEDED(hr));
    if (SUCCEEDED(hr)) {
        fLocked = true;
        GrAssert(NULL != data);
        memcpy(data, src, srcSizeInBytes);
        hr = fBuffer->Unlock();
        fLocked = SUCCEEDED(hr);
        GrAssert(!fLocked);
        return !fLocked;
    }
    return false;
}

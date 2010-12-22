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


#ifndef GrGpuD3D9_DEFINED
#define GrGpuD3D9_DEFINED

#include <Windows.h>
#include <d3d9.h>

#include "GrGpu.h"

class GrD3D9VertexBuffer;
class GrD3D9IndexBuffer;
class GrD3D9Texture;

// For D3D9 GrRenderTarget casts to a (GrD3D9RenderTarget*)
struct GrD3D9RenderTarget {
    IDirect3DSurface9* fColor;
    IDirect3DSurface9* fStencil;
    bool fClearStencil;
};

// GrGpu implementation for D3D9 fixed pipeline. 
// Known needed improvements:
//      vertex/index buffers need to be better managed:
//          use no_overwrite and walk down VB/IB until reach end and wrap
//      take advantage of the redrawHint and don't recopy vertex/idx data
//      User created vertex buffers must have position Z values 
//          (required for fixed pipeline) but there is no way to communicate 
//          this now
//      We create a temporary sysmem surface for each texture update.
//      split this out into fixed/shader subclasses (use vdecls for shaders)
class GrGpuD3D9 : public GrGpu {
public:
            GrGpuD3D9(IDirect3DDevice9* device);
    virtual ~GrGpuD3D9();

    // overrides from GrGpu
    virtual GrTexture* createTexture(const TextureDesc& desc,
                                     const void* srcData);
    virtual GrVertexBuffer* createVertexBuffer(uint32_t size, bool dynamic);
    virtual GrIndexBuffer* createIndexBuffer(uint32_t size, bool dynamic);

    virtual void eraseColor(GrColor color);
    virtual void eraseStencil();

protected:
    // overrides from GrGpu
    virtual bool flushGraphicsState(PrimitiveTypes type);
    virtual void drawIndexArrayApi(PrimitiveTypes type,
                                   int baseVertex,
                                   int vertexCount,
                                   int indexCount,
                                   const uint16_t* indexArray,
                                   bool redrawHint);
    virtual void drawIndexBufferApi(PrimitiveTypes type,
                                    int baseVertex,
                                    int startIndex,
                                    int vertexCount,
                                    int indexCount,
                                    GrIndexBuffer* indexBuffer,
                                    bool redrawHint);
    virtual void drawNonIndexedApi(PrimitiveTypes type,
                                   int baseVertex,
                                   int indexCount,
                                   bool redrawHint);
    virtual void flushScissor();

private:

    // baseVertex may be modified while setting up the stage
    GrD3D9VertexBuffer* setupVBufferStage(int vsize, int* baseVertex, 
                                          int vertexCount, DrawModes mode);
    GrD3D9IndexBuffer* setupIBufferStage(int* startIndex, int indexCount, 
                                         const uint16_t* indices);
    static int vertexSize(int vertFlagBits, GrGpu::DrawModes mode);
    static bool positionsOnly(int vertFlagBits);

    // notify callbacks to update state tracking when related
    // objects are bound to the device or deleted outside of the class
    void notifyVertexBufferBind(GrD3D9VertexBuffer* buffer);
    void notifyVertexBufferDelete(GrD3D9VertexBuffer* buffer);
    void notifyIndexBufferBind(GrD3D9IndexBuffer* buffer);
    void notifyIndexBufferDelete(GrD3D9IndexBuffer* buffer);
    void notifyTextureDelete(GrD3D9Texture* texture);
    void notifyTextureRemoveRenderTarget(GrD3D9Texture* texture);

    IDirect3DSurface9* createStencil(uint32_t width, 
                                     uint32_t height,
                                     D3DMULTISAMPLE_TYPE msType,
                                     DWORD msQual);

    void setRenderTargetImm();

    friend class GrD3D9VertexBuffer;
    friend class GrD3D9IndexBuffer;
    friend class GrD3D9Texture;

    GrIndexBuffer*                  fLastIndexBuffer;
    
    // used to track the COLORARG1 value for tex stage 0
    // needs to use ALPHAREPLICATE when using alpha-only textures
    DWORD                           fLastColorArg1;

    IDirect3DDevice9*               fDevice;
    // We may use Ex functionality if this is a Ex device
    IDirect3DDevice9Ex*             fDeviceEx;

    enum VertDecls {
        kInvalid_VertDecl = -1,
        kPosOnly_VertDecl = 0,
        kTex_VertDecl,
        kColors_VertDecl,
        kTexAndColors_VertDecl,
        kPosAsTex_VertDecl,
        kPosAsTexAndColors_VertDecl,
        kVertDeclCount
    };

    static const VertDecls gVertFlags2VertDeclIdx[];
    static const DWORD gDeclToFVFs[];
    static const DWORD gTextFVF;
    
    DWORD fLastVertFVF;

    bool fLastBlendOff;

    // D3D allows user pointers in place of buffers for vertex/index data
    // but it doesn't allow:
    //  -multiple streams (non-interleaved) ~ this will be resolved when we 
    //                                        go AoS with our verts
    //  -mixing user pointer verts with index buffer (or vice versa)
    // So we use these staging buffers
    GrD3D9VertexBuffer* fStageVBuffer;
    GrD3D9IndexBuffer*  fStageIBuffer;

    // did we use texture coordinate generation at the last flush
    bool fLastTexGen;

    GrD3D9RenderTarget fDefaultRenderTarget;

    // We use texture stage 0 to set a constant color
    // D3D disables the stage if NULL is bound (even when the ops don't
    // reference the texture). So we have a 1x1 dummy texture that
    // gets set when drawing constant color with no texture
    GrD3D9Texture* fDummyTexture;
};

class GrD3D9Texture : public GrTexture {
protected:
    GrD3D9Texture(uint32_t width, 
                  uint32_t height, 
                  PixelConfig config,
                  IDirect3DTexture9* texture,
                  IDirect3DSurface9* stencil,
                  bool clearStencil,
                  GrGpuD3D9* gpuD3D9);
public:
    virtual ~GrD3D9Texture();
    
    // overloads of GrTexture
    virtual void abandon();
    virtual bool isRenderTarget();
    virtual GrRenderTarget* asRenderTarget() 
                                    { return (GrRenderTarget*) &fRenderTarget; }
    virtual void removeRenderTarget();
    virtual void uploadTextureData(uint32_t x,
                                   uint32_t y,
                                   uint32_t width,
                                   uint32_t height,
                                   const void* srcData);
    IDirect3DTexture9* texture() const { return fTexture; }
    IDirect3DSurface9* stencil() const { return fStencil; }
    D3DFORMAT format() const { return fDesc.Format; }
private:
    IDirect3DTexture9*      fTexture;
    GrD3D9RenderTarget      fRenderTarget;
    IDirect3DSurface9*      fStencil;
    D3DSURFACE_DESC         fDesc;
    GrGpuD3D9*              fGpuD3D9;

    friend class GrGpuD3D9;

    typedef GrTexture INHERITED;
};

class GrD3D9VertexBuffer : public GrVertexBuffer {
protected:
    GrD3D9VertexBuffer(uint32_t size, 
                       bool dynamic,
                       IDirect3DVertexBuffer9* vbuffer,
                       GrGpuD3D9* gpuD3D9);
public:
    virtual ~GrD3D9VertexBuffer();
    
    IDirect3DVertexBuffer9* buffer() const { return fBuffer; }

    // overrides of GrVertexBuffer
    virtual void abandon();
    virtual void* lock();
    virtual void unlock();
    virtual bool isLocked();
    virtual bool updateData(const void* src, uint32_t srcSizeInBytes);

private:
    IDirect3DVertexBuffer9* fBuffer;
    D3DVERTEXBUFFER_DESC    fDesc;
    bool                    fLocked;
    GrGpuD3D9*              fGpuD3D9;

    friend class GrGpuD3D9;

    typedef GrVertexBuffer INHERITED;
};

class GrD3D9IndexBuffer : public GrIndexBuffer {
protected:
    GrD3D9IndexBuffer(uint32_t size, 
                      bool dynamic,
                      IDirect3DIndexBuffer9* vbuffer,
                      GrGpuD3D9* gpuD3D9);
public:
    virtual ~GrD3D9IndexBuffer();

    IDirect3DIndexBuffer9* buffer() const { return fBuffer; }

    // overrides of GrIndexBuffer
    virtual void abandon();
    virtual void* lock();
    virtual void unlock();
    virtual bool isLocked();
    virtual bool updateData(const void* src, uint32_t srcSizeInBytes);
private:
    IDirect3DIndexBuffer9*  fBuffer;
    D3DINDEXBUFFER_DESC     fDesc;
    bool                    fLocked;
    GrGpuD3D9*              fGpuD3D9;

    friend class GrGpuD3D9;

    typedef GrIndexBuffer INHERITED;
};

#endif


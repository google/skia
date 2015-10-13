//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// InputLayoutCache.cpp: Defines InputLayoutCache, a class that builds and caches
// D3D11 input layouts.

#include "libANGLE/renderer/d3d/d3d11/InputLayoutCache.h"

#include "common/utilities.h"
#include "libANGLE/Program.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"
#include "libANGLE/renderer/d3d/ProgramD3D.h"
#include "libANGLE/renderer/d3d/VertexDataManager.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/ShaderExecutable11.h"
#include "libANGLE/renderer/d3d/d3d11/VertexBuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "third_party/murmurhash/MurmurHash3.h"

namespace rx
{

namespace
{

gl::InputLayout GetInputLayout(
    const TranslatedAttribute *translatedAttributes[gl::MAX_VERTEX_ATTRIBS],
    size_t attributeCount)
{
    gl::InputLayout inputLayout(attributeCount, gl::VERTEX_FORMAT_INVALID);

    for (size_t attributeIndex = 0; attributeIndex < attributeCount; ++attributeIndex)
    {
        const TranslatedAttribute *translatedAttribute = translatedAttributes[attributeIndex];

        if (translatedAttribute->active)
        {
            inputLayout[attributeIndex] = gl::GetVertexFormatType(
                *translatedAttribute->attribute, translatedAttribute->currentValueType);
        }
    }
    return inputLayout;
}

GLenum GetGLSLAttributeType(const std::vector<sh::Attribute> &shaderAttributes, int index)
{
    // Count matrices differently
    for (const sh::Attribute &attrib : shaderAttributes)
    {
        if (attrib.location == -1)
        {
            continue;
        }

        GLenum transposedType = gl::TransposeMatrixType(attrib.type);
        int rows              = gl::VariableRowCount(transposedType);

        if (index >= attrib.location && index < attrib.location + rows)
        {
            return transposedType;
        }
    }

    UNREACHABLE();
    return GL_NONE;
}

const unsigned int kDefaultCacheSize = 1024;

struct PackedAttribute
{
    uint8_t attribType;
    uint8_t semanticIndex;
    uint8_t vertexFormatType;
    uint8_t divisor;
};

} // anonymous namespace

void InputLayoutCache::PackedAttributeLayout::addAttributeData(
    GLenum glType,
    UINT semanticIndex,
    gl::VertexFormatType vertexFormatType,
    unsigned int divisor)
{
    gl::AttributeType attribType = gl::GetAttributeType(glType);

    PackedAttribute packedAttrib;
    packedAttrib.attribType = static_cast<uint8_t>(attribType);
    packedAttrib.semanticIndex = static_cast<uint8_t>(semanticIndex);
    packedAttrib.vertexFormatType = static_cast<uint8_t>(vertexFormatType);
    packedAttrib.divisor = static_cast<uint8_t>(divisor);

    ASSERT(static_cast<gl::AttributeType>(packedAttrib.attribType) == attribType);
    ASSERT(static_cast<UINT>(packedAttrib.semanticIndex) == semanticIndex);
    ASSERT(static_cast<gl::VertexFormatType>(packedAttrib.vertexFormatType) == vertexFormatType);
    ASSERT(static_cast<unsigned int>(packedAttrib.divisor) == divisor);

    static_assert(sizeof(uint32_t) == sizeof(PackedAttribute), "PackedAttributes must be 32-bits exactly.");

    attributeData[numAttributes++] = gl::bitCast<uint32_t>(packedAttrib);
}

bool InputLayoutCache::PackedAttributeLayout::operator<(const PackedAttributeLayout &other) const
{
    if (numAttributes != other.numAttributes)
    {
        return numAttributes < other.numAttributes;
    }

    if (flags != other.flags)
    {
        return flags < other.flags;
    }

    return memcmp(attributeData, other.attributeData, sizeof(uint32_t) * numAttributes) < 0;
}

InputLayoutCache::InputLayoutCache()
    : mCacheSize(kDefaultCacheSize)
{
    mCounter = 0;
    mDevice = NULL;
    mDeviceContext = NULL;
    mCurrentIL = NULL;
    for (unsigned int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        mCurrentBuffers[i] = NULL;
        mCurrentVertexStrides[i] = static_cast<UINT>(-1);
        mCurrentVertexOffsets[i] = static_cast<UINT>(-1);
    }
    mPointSpriteVertexBuffer = NULL;
    mPointSpriteIndexBuffer = NULL;
}

InputLayoutCache::~InputLayoutCache()
{
    clear();
}

void InputLayoutCache::initialize(ID3D11Device *device, ID3D11DeviceContext *context)
{
    clear();
    mDevice = device;
    mDeviceContext = context;
    mFeatureLevel = device->GetFeatureLevel();
}

void InputLayoutCache::clear()
{
    for (auto &layout : mLayoutMap)
    {
        SafeRelease(layout.second);
    }
    mLayoutMap.clear();
    SafeRelease(mPointSpriteVertexBuffer);
    SafeRelease(mPointSpriteIndexBuffer);
    markDirty();
}

void InputLayoutCache::markDirty()
{
    mCurrentIL = NULL;
    for (unsigned int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        mCurrentBuffers[i] = NULL;
        mCurrentVertexStrides[i] = static_cast<UINT>(-1);
        mCurrentVertexOffsets[i] = static_cast<UINT>(-1);
    }
}

gl::Error InputLayoutCache::applyVertexBuffers(const std::vector<TranslatedAttribute> &unsortedAttributes,
                                               GLenum mode, gl::Program *program, SourceIndexData *sourceInfo)
{
    ProgramD3D *programD3D = GetImplAs<ProgramD3D>(program);

    int sortedSemanticIndices[gl::MAX_VERTEX_ATTRIBS];
    const TranslatedAttribute *sortedAttributes[gl::MAX_VERTEX_ATTRIBS] = { nullptr };
    programD3D->sortAttributesByLayout(unsortedAttributes, sortedSemanticIndices, sortedAttributes);
    bool programUsesInstancedPointSprites = programD3D->usesPointSize() && programD3D->usesInstancedPointSpriteEmulation();
    bool instancedPointSpritesActive = programUsesInstancedPointSprites && (mode == GL_POINTS);
    bool indexedPointSpriteEmulationActive = instancedPointSpritesActive && (sourceInfo != nullptr);

    const auto &semanticToLocation = programD3D->getAttributesByLayout();

    if (!mDevice || !mDeviceContext)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal input layout cache is not initialized.");
    }

    unsigned int inputElementCount = 0;
    D3D11_INPUT_ELEMENT_DESC inputElements[gl::MAX_VERTEX_ATTRIBS];
    PackedAttributeLayout layout;

    static const char* semanticName = "TEXCOORD";

    unsigned int firstIndexedElement = gl::MAX_VERTEX_ATTRIBS;
    unsigned int firstInstancedElement = gl::MAX_VERTEX_ATTRIBS;
    unsigned int nextAvailableInputSlot = 0;

    const std::vector<sh::Attribute> &shaderAttributes = program->getAttributes();

    for (unsigned int i = 0; i < unsortedAttributes.size(); i++)
    {
        if (sortedAttributes[i]->active)
        {
            D3D11_INPUT_CLASSIFICATION inputClass = sortedAttributes[i]->divisor > 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
            // If rendering points and instanced pointsprite emulation is being used, the inputClass is required to be configured as per instance data
            inputClass = instancedPointSpritesActive ? D3D11_INPUT_PER_INSTANCE_DATA : inputClass;

            gl::VertexFormatType vertexFormatType = gl::GetVertexFormatType(*sortedAttributes[i]->attribute, sortedAttributes[i]->currentValueType);
            const d3d11::VertexFormat &vertexFormatInfo = d3d11::GetVertexFormatInfo(vertexFormatType, mFeatureLevel);

            inputElements[inputElementCount].SemanticName = semanticName;
            inputElements[inputElementCount].SemanticIndex = sortedSemanticIndices[i];
            inputElements[inputElementCount].Format = vertexFormatInfo.nativeFormat;
            inputElements[inputElementCount].InputSlot = i;
            inputElements[inputElementCount].AlignedByteOffset = 0;
            inputElements[inputElementCount].InputSlotClass = inputClass;
            inputElements[inputElementCount].InstanceDataStepRate = instancedPointSpritesActive ? 1 : sortedAttributes[i]->divisor;

            if (inputClass == D3D11_INPUT_PER_VERTEX_DATA && firstIndexedElement == gl::MAX_VERTEX_ATTRIBS)
            {
                firstIndexedElement = inputElementCount;
            }
            else if (inputClass == D3D11_INPUT_PER_INSTANCE_DATA && firstInstancedElement == gl::MAX_VERTEX_ATTRIBS)
            {
                firstInstancedElement = inputElementCount;
            }

            // Record the type of the associated vertex shader vector in our key
            // This will prevent mismatched vertex shaders from using the same input layout
            GLenum glslElementType = GetGLSLAttributeType(
                shaderAttributes, semanticToLocation[sortedSemanticIndices[i]]);

            layout.addAttributeData(glslElementType,
                                    sortedSemanticIndices[i],
                                    vertexFormatType,
                                    sortedAttributes[i]->divisor);

            inputElementCount++;
            nextAvailableInputSlot = i + 1;
        }
    }

    // Instanced PointSprite emulation requires additional entries in the
    // inputlayout to support the vertices that make up the pointsprite quad.
    // We do this even if mode != GL_POINTS, since the shader signature has these inputs, and the input layout must match the shader
    if (programUsesInstancedPointSprites)
    {
        inputElements[inputElementCount].SemanticName = "SPRITEPOSITION";
        inputElements[inputElementCount].SemanticIndex = 0;
        inputElements[inputElementCount].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        inputElements[inputElementCount].InputSlot = nextAvailableInputSlot;
        inputElements[inputElementCount].AlignedByteOffset = 0;
        inputElements[inputElementCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        inputElements[inputElementCount].InstanceDataStepRate = 0;

        // The new elements are D3D11_INPUT_PER_VERTEX_DATA data so the indexed element
        // tracking must be applied.  This ensures that the instancing specific
        // buffer swapping logic continues to work.
        if (firstIndexedElement == gl::MAX_VERTEX_ATTRIBS)
        {
            firstIndexedElement = inputElementCount;
        }

        inputElementCount++;

        inputElements[inputElementCount].SemanticName = "SPRITETEXCOORD";
        inputElements[inputElementCount].SemanticIndex = 0;
        inputElements[inputElementCount].Format = DXGI_FORMAT_R32G32_FLOAT;
        inputElements[inputElementCount].InputSlot = nextAvailableInputSlot;
        inputElements[inputElementCount].AlignedByteOffset = sizeof(float) * 3;
        inputElements[inputElementCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        inputElements[inputElementCount].InstanceDataStepRate = 0;

        inputElementCount++;
    }

    // On 9_3, we must ensure that slot 0 contains non-instanced data.
    // If slot 0 currently contains instanced data then we swap it with a non-instanced element.
    // Note that instancing is only available on 9_3 via ANGLE_instanced_arrays, since 9_3 doesn't support OpenGL ES 3.0.
    // As per the spec for ANGLE_instanced_arrays, not all attributes can be instanced simultaneously, so a non-instanced element must exist.
    ASSERT(!(mFeatureLevel <= D3D_FEATURE_LEVEL_9_3 && firstIndexedElement == gl::MAX_VERTEX_ATTRIBS));
    bool moveFirstIndexedIntoSlotZero = mFeatureLevel <= D3D_FEATURE_LEVEL_9_3 && firstInstancedElement == 0 && firstIndexedElement != gl::MAX_VERTEX_ATTRIBS;

    if (moveFirstIndexedIntoSlotZero)
    {
        inputElements[firstInstancedElement].InputSlot = inputElements[firstIndexedElement].InputSlot;
        inputElements[firstIndexedElement].InputSlot = 0;

        // Instanced PointSprite emulation uses multiple layout entries across a single vertex buffer.
        // If an index swap is performed, we need to ensure that all elements get the proper InputSlot.
        if (programUsesInstancedPointSprites)
        {
            inputElements[firstIndexedElement + 1].InputSlot = 0;
        }
    }

    if (programUsesInstancedPointSprites)
    {
        layout.flags |= PackedAttributeLayout::FLAG_USES_INSTANCED_SPRITES;
    }

    if (moveFirstIndexedIntoSlotZero)
    {
        layout.flags |= PackedAttributeLayout::FLAG_MOVE_FIRST_INDEXED;
    }

    if (instancedPointSpritesActive)
    {
        layout.flags |= PackedAttributeLayout::FLAG_INSTANCED_SPRITES_ACTIVE;
    }

    ID3D11InputLayout *inputLayout = nullptr;

    auto layoutMapIt = mLayoutMap.find(layout);
    if (layoutMapIt != mLayoutMap.end())
    {
        inputLayout = layoutMapIt->second;
    }
    else
    {
        const gl::InputLayout &shaderInputLayout =
            GetInputLayout(sortedAttributes, unsortedAttributes.size());

        ShaderExecutableD3D *shader = nullptr;
        gl::Error error = programD3D->getVertexExecutableForInputLayout(shaderInputLayout, &shader, nullptr);
        if (error.isError())
        {
            return error;
        }

        ShaderExecutableD3D *shader11 = GetAs<ShaderExecutable11>(shader);

        D3D11_INPUT_ELEMENT_DESC descs[gl::MAX_VERTEX_ATTRIBS];
        for (unsigned int j = 0; j < inputElementCount; ++j)
        {
            descs[j] = inputElements[j];
        }

        HRESULT result = mDevice->CreateInputLayout(descs, inputElementCount, shader11->getFunction(), shader11->getLength(), &inputLayout);
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal input layout, HRESULT: 0x%08x", result);
        }

        if (mLayoutMap.size() >= mCacheSize)
        {
            TRACE("Overflowed the limit of %u input layouts, purging half the cache.", mCacheSize);

            // Randomly release every second element
            auto it = mLayoutMap.begin();
            while (it != mLayoutMap.end())
            {
                it++;
                if (it != mLayoutMap.end())
                {
                    // Calling std::map::erase invalidates the current iterator, so make a copy.
                    auto eraseIt = it++;
                    SafeRelease(eraseIt->second);
                    mLayoutMap.erase(eraseIt);
                }
            }
        }

        mLayoutMap[layout] = inputLayout;
    }

    if (inputLayout != mCurrentIL)
    {
        mDeviceContext->IASetInputLayout(inputLayout);
        mCurrentIL = inputLayout;
    }

    bool dirtyBuffers = false;
    unsigned int minDiff            = gl::MAX_VERTEX_ATTRIBS;
    unsigned int maxDiff            = 0;
    unsigned int nextAvailableIndex = 0;

    for (unsigned int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        ID3D11Buffer *buffer = NULL;
        UINT vertexStride = 0;
        UINT vertexOffset = 0;

        if (i < unsortedAttributes.size() && sortedAttributes[i]->active)
        {
            VertexBuffer11 *vertexBuffer = GetAs<VertexBuffer11>(sortedAttributes[i]->vertexBuffer);
            Buffer11 *bufferStorage = sortedAttributes[i]->storage ? GetAs<Buffer11>(sortedAttributes[i]->storage) : NULL;

            // If indexed pointsprite emulation is active, then we need to take a less efficent code path.
            // Emulated indexed pointsprite rendering requires that the vertex buffers match exactly to
            // the indices passed by the caller.  This could expand or shrink the vertex buffer depending
            // on the number of points indicated by the index list or how many duplicates are found on the index list.
            if (bufferStorage == nullptr)
            {
                buffer = vertexBuffer->getBuffer();
            }
            else if (indexedPointSpriteEmulationActive)
            {
                if (sourceInfo->srcBuffer != nullptr)
                {
                    const uint8_t *bufferData = nullptr;
                    gl::Error error = sourceInfo->srcBuffer->getData(&bufferData);
                    if (error.isError())
                    {
                        return error;
                    }
                    ASSERT(bufferData != nullptr);

                    ptrdiff_t offset = reinterpret_cast<ptrdiff_t>(sourceInfo->srcIndices);
                    sourceInfo->srcBuffer = nullptr;
                    sourceInfo->srcIndices = bufferData + offset;
                }

                buffer = bufferStorage->getEmulatedIndexedBuffer(sourceInfo, sortedAttributes[i]);
            }
            else
            {
                buffer = bufferStorage->getBuffer(BUFFER_USAGE_VERTEX_OR_TRANSFORM_FEEDBACK);
            }

            vertexStride = sortedAttributes[i]->stride;
            vertexOffset = sortedAttributes[i]->offset;
        }

        if (buffer != mCurrentBuffers[i] || vertexStride != mCurrentVertexStrides[i] ||
            vertexOffset != mCurrentVertexOffsets[i])
        {
            dirtyBuffers = true;
            minDiff      = std::min(minDiff, i);
            maxDiff      = std::max(maxDiff, i);

            mCurrentBuffers[i] = buffer;
            mCurrentVertexStrides[i] = vertexStride;
            mCurrentVertexOffsets[i] = vertexOffset;
        }

        // If a non null ID3D11Buffer is being assigned to mCurrentBuffers,
        // then the next available index needs to be tracked to ensure
        // that any instanced pointsprite emulation buffers will be properly packed.
        if (buffer)
        {
            nextAvailableIndex = i + 1;
        }
    }

    // Instanced PointSprite emulation requires two additional ID3D11Buffers.
    // A vertex buffer needs to be created and added to the list of current buffers,
    // strides and offsets collections.  This buffer contains the vertices for a single
    // PointSprite quad.
    // An index buffer also needs to be created and applied because rendering instanced
    // data on D3D11 FL9_3 requires DrawIndexedInstanced() to be used.
    // Shaders that contain gl_PointSize and used without the GL_POINTS rendering mode
    // require a vertex buffer because some drivers cannot handle missing vertex data
    // and will TDR the system.
    if (programUsesInstancedPointSprites)
    {
        HRESULT result = S_OK;
        const UINT pointSpriteVertexStride = sizeof(float) * 5;

        if (!mPointSpriteVertexBuffer)
        {
            static const float pointSpriteVertices[] =
            {
                // Position        // TexCoord
               -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
               -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
               -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
            };

            D3D11_SUBRESOURCE_DATA vertexBufferData = { pointSpriteVertices, 0, 0 };
            D3D11_BUFFER_DESC vertexBufferDesc;
            vertexBufferDesc.ByteWidth = sizeof(pointSpriteVertices);
            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            vertexBufferDesc.CPUAccessFlags = 0;
            vertexBufferDesc.MiscFlags = 0;
            vertexBufferDesc.StructureByteStride = 0;

            result = mDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &mPointSpriteVertexBuffer);
            if (FAILED(result))
            {
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create instanced pointsprite emulation vertex buffer, HRESULT: 0x%08x", result);
            }
        }

        mCurrentBuffers[nextAvailableIndex] = mPointSpriteVertexBuffer;
        // Set the stride to 0 if GL_POINTS mode is not being used to instruct the driver
        // to avoid indexing into the vertex buffer.
        mCurrentVertexStrides[nextAvailableIndex] =
            instancedPointSpritesActive ? pointSpriteVertexStride : 0;
        mCurrentVertexOffsets[nextAvailableIndex] = 0;

        if (!mPointSpriteIndexBuffer)
        {
            // Create an index buffer and set it for pointsprite rendering
            static const unsigned short pointSpriteIndices[] =
            {
                0, 1, 2, 3, 4, 5,
            };

            D3D11_SUBRESOURCE_DATA indexBufferData = { pointSpriteIndices, 0, 0 };
            D3D11_BUFFER_DESC indexBufferDesc;
            indexBufferDesc.ByteWidth = sizeof(pointSpriteIndices);
            indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            indexBufferDesc.CPUAccessFlags = 0;
            indexBufferDesc.MiscFlags = 0;
            indexBufferDesc.StructureByteStride = 0;

            result = mDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &mPointSpriteIndexBuffer);
            if (FAILED(result))
            {
                SafeRelease(mPointSpriteVertexBuffer);
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create instanced pointsprite emulation index buffer, HRESULT: 0x%08x", result);
            }
        }

        if (instancedPointSpritesActive)
        {
            // The index buffer is applied here because Instanced PointSprite emulation uses
            // the a non-indexed rendering path in ANGLE (DrawArrays).  This means that
            // applyIndexBuffer()
            // on the renderer will not be called and setting this buffer here ensures that the
            // rendering
            // path will contain the correct index buffers.
            mDeviceContext->IASetIndexBuffer(mPointSpriteIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        }
    }

    if (moveFirstIndexedIntoSlotZero)
    {
        // In this case, we swapped the slots of the first instanced element and the first indexed element, to ensure
        // that the first slot contains non-instanced data (required by Feature Level 9_3).
        // We must also swap the corresponding buffers sent to IASetVertexBuffers so that the correct data is sent to each slot.
        std::swap(mCurrentBuffers[firstIndexedElement], mCurrentBuffers[firstInstancedElement]);
        std::swap(mCurrentVertexStrides[firstIndexedElement], mCurrentVertexStrides[firstInstancedElement]);
        std::swap(mCurrentVertexOffsets[firstIndexedElement], mCurrentVertexOffsets[firstInstancedElement]);
    }

    if (dirtyBuffers)
    {
        ASSERT(minDiff <= maxDiff && maxDiff < gl::MAX_VERTEX_ATTRIBS);
        mDeviceContext->IASetVertexBuffers(minDiff, maxDiff - minDiff + 1, mCurrentBuffers + minDiff,
                                           mCurrentVertexStrides + minDiff, mCurrentVertexOffsets + minDiff);
    }

    return gl::Error(GL_NO_ERROR);
}

}

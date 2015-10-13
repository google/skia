//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// D3D11EmulatedIndexedBufferTest:
//   Tests to validate our D3D11 support for emulating an indexed
//   vertex buffer.
//

#include "libANGLE/angletypes.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"

using namespace angle;

namespace
{

class D3D11EmulatedIndexedBufferTest : public ANGLETest
{
  protected:

    void SetUp() override
    {
        ANGLETest::SetUp();
        ASSERT_EQ(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, GetParam().getRenderer());

        gl::Context *context = reinterpret_cast<gl::Context *>(getEGLWindow()->getContext());
        mRenderer = rx::GetAs<rx::Renderer11>(context->getRenderer());

        mSourceBuffer = new rx::Buffer11(mRenderer);
        GLfloat testData[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
        gl::Error error = mSourceBuffer->setData(testData, sizeof(testData), GL_STATIC_DRAW);
        ASSERT_FALSE(error.isError());

        mTranslatedAttribute.offset = 0;
        mTranslatedAttribute.stride = sizeof(GLfloat);

        GLubyte indices[] = {0, 0, 3, 4, 2, 1, 1};

        for (size_t i = 0; i < _countof(indices); i++)
        {
            mExpectedExpandedData.push_back(testData[indices[i]]);
            mubyteIndices.push_back(indices[i]);
            muintIndices.push_back(indices[i]);
            mushortIndices.push_back(indices[i]);
        }
    }

    void TearDown() override
    {
        SafeDelete(mSourceBuffer);
        ANGLETest::TearDown();
    }

    void createMappableCompareBufferFromEmulatedBuffer(ID3D11Buffer *sourceBuffer, GLuint size, ID3D11Buffer **mappableBuffer)
    {
        *mappableBuffer = nullptr;

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth = size;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        bufferDesc.Usage = D3D11_USAGE_STAGING;
        bufferDesc.BindFlags = 0;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

        HRESULT hr = mRenderer->getDevice()->CreateBuffer(&bufferDesc, nullptr, mappableBuffer);
        ASSERT_TRUE(SUCCEEDED(hr));

        D3D11_BOX srcBox;
        srcBox.left = 0;
        srcBox.right = size;
        srcBox.top = 0;
        srcBox.bottom = 1;
        srcBox.front = 0;
        srcBox.back = 1;

        mRenderer->getDeviceContext()->CopySubresourceRegion(*mappableBuffer, 0, 0, 0, 0, sourceBuffer, 0, &srcBox);
    }

    void compareContents(ID3D11Buffer *actual)
    {
        ID3D11Buffer *compareBuffer = nullptr;
        createMappableCompareBufferFromEmulatedBuffer(
            actual, sizeof(GLfloat) * static_cast<GLuint>(mExpectedExpandedData.size()),
            &compareBuffer);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = mRenderer->getDeviceContext()->Map(compareBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
        ASSERT_TRUE(SUCCEEDED(hr));

        GLfloat* compareData = static_cast<GLfloat*>(mappedResource.pData);
        for (size_t i = 0; i < mExpectedExpandedData.size(); i++)
        {
            EXPECT_EQ(mExpectedExpandedData[i], compareData[i]);
        }

        mRenderer->getDeviceContext()->Unmap(compareBuffer, 0);
        SafeRelease(compareBuffer);
    }

    void emulateAndCompare(rx::SourceIndexData *srcData)
    {
        ID3D11Buffer* emulatedBuffer = mSourceBuffer->getEmulatedIndexedBuffer(srcData, &mTranslatedAttribute);
        ASSERT_TRUE(emulatedBuffer != nullptr);

        compareContents(emulatedBuffer);
    }

  protected:
    rx::Buffer11 *mSourceBuffer;
    rx::Renderer11 *mRenderer;
    rx::TranslatedAttribute mTranslatedAttribute;
    std::vector<GLfloat> mExpectedExpandedData;
    std::vector<GLubyte> mubyteIndices;
    std::vector<GLuint> muintIndices;
    std::vector<GLushort> mushortIndices;
};

// This tests that a GL_UNSIGNED_BYTE indices list can be successfully expanded
// into a valid emulated indexed buffer.
TEST_P(D3D11EmulatedIndexedBufferTest, TestNativeToExpandedUsingGLubyteIndices)
{
    rx::SourceIndexData srcData = {nullptr, mubyteIndices.data(),
                                   static_cast<unsigned int>(mubyteIndices.size()),
                                   GL_UNSIGNED_BYTE, false};
    emulateAndCompare(&srcData);
}

// This tests that a GL_UNSIGNED_SHORT indices list can be successfully expanded
// into a valid emulated indexed buffer.
TEST_P(D3D11EmulatedIndexedBufferTest, TestNativeToExpandedUsingGLushortIndices)
{
    rx::SourceIndexData srcData = {nullptr, mushortIndices.data(),
                                   static_cast<unsigned int>(mushortIndices.size()),
                                   GL_UNSIGNED_SHORT, false};
    emulateAndCompare(&srcData);
}

// This tests that a GL_UNSIGNED_INT indices list can be successfully expanded
// into a valid emulated indexed buffer.
TEST_P(D3D11EmulatedIndexedBufferTest, TestNativeToExpandedUsingGLuintIndices)
{
    rx::SourceIndexData srcData = {nullptr, muintIndices.data(),
                                   static_cast<unsigned int>(muintIndices.size()), GL_UNSIGNED_INT,
                                   false};
    emulateAndCompare(&srcData);
}

// This tests verifies that a Buffer11 contents remain unchanged after calling getEmulatedIndexedBuffer
TEST_P(D3D11EmulatedIndexedBufferTest, TestSourceBufferRemainsUntouchedAfterExpandOperation)
{
    // Copy the original source buffer before any expand calls have been made
    rx::Buffer11 *cleanSourceBuffer = new rx::Buffer11(mRenderer);
    cleanSourceBuffer->copySubData(mSourceBuffer, 0, 0, mSourceBuffer->getSize());

    // Do a basic exanded and compare test.
    rx::SourceIndexData srcData = {nullptr, muintIndices.data(),
                                   static_cast<unsigned int>(muintIndices.size()), GL_UNSIGNED_INT,
                                   false};
    emulateAndCompare(&srcData);

    const uint8_t *sourceBufferMem = nullptr;
    const uint8_t *cleanBufferMem = nullptr;

    gl::Error error = mSourceBuffer->getData(&sourceBufferMem);
    ASSERT_FALSE(error.isError());

    error = cleanSourceBuffer->getData(&cleanBufferMem);
    ASSERT_FALSE(error.isError());

    int result = memcmp(sourceBufferMem, cleanBufferMem, cleanSourceBuffer->getSize());
    ASSERT_EQ(result, 0);

    SafeDelete(cleanSourceBuffer);
}

ANGLE_INSTANTIATE_TEST(D3D11EmulatedIndexedBufferTest,
                       ES2_D3D11());

} // anonymous namespace

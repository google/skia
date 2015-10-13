//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/utilities.h"
#include "libANGLE/ImageIndex.h"

using namespace gl;

namespace
{

static const GLint minMip = 0;
static const GLint maxMip = 4;
static const GLint minLayer = 1;
static const GLint maxLayer = 3;

TEST(ImageIndexTest, Iterator2D)
{
    ImageIndexIterator iter = ImageIndexIterator::Make2D(minMip, maxMip);

    ASSERT_GE(0, minMip);

    for (GLint mip = minMip; mip < maxMip; mip++)
    {
        EXPECT_TRUE(iter.hasNext());
        ImageIndex current = iter.current();
        ImageIndex nextIndex = iter.next();

        EXPECT_EQ(static_cast<GLenum>(GL_TEXTURE_2D), nextIndex.type);
        EXPECT_EQ(mip, nextIndex.mipIndex);
        EXPECT_FALSE(nextIndex.hasLayer());

        // Also test current
        EXPECT_EQ(current.type, nextIndex.type);
        EXPECT_EQ(current.mipIndex, nextIndex.mipIndex);
        EXPECT_EQ(current.layerIndex, nextIndex.layerIndex);
    }
}

TEST(ImageIndexTest, IteratorCube)
{
    ImageIndexIterator iter = ImageIndexIterator::MakeCube(0, 4);

    ASSERT_GE(0, minMip);

    for (GLint mip = minMip; mip < maxMip; mip++)
    {
        for (GLint layer = 0; layer < 6; layer++)
        {
            EXPECT_TRUE(iter.hasNext());
            ImageIndex nextIndex = iter.next();

            GLenum cubeTarget = LayerIndexToCubeMapTextureTarget(layer);

            EXPECT_EQ(cubeTarget, nextIndex.type);
            EXPECT_EQ(mip, nextIndex.mipIndex);
            EXPECT_EQ(layer, nextIndex.layerIndex);
            EXPECT_TRUE(nextIndex.hasLayer());
        }
    }
}

TEST(ImageIndexTest, Iterator3D)
{
    ImageIndexIterator iter = ImageIndexIterator::Make3D(minMip, maxMip, minLayer, maxLayer);

    ASSERT_GE(0, minMip);

    for (GLint mip = minMip; mip < maxMip; mip++)
    {
        for (GLint layer = minLayer; layer < maxLayer; layer++)
        {
            EXPECT_TRUE(iter.hasNext());
            ImageIndex nextIndex = iter.next();

            EXPECT_EQ(static_cast<GLenum>(GL_TEXTURE_3D), nextIndex.type);
            EXPECT_EQ(mip, nextIndex.mipIndex);
            EXPECT_EQ(layer, nextIndex.layerIndex);
            EXPECT_TRUE(nextIndex.hasLayer());
        }
    }
}

TEST(ImageIndexTest, Iterator2DArray)
{
    GLsizei layerCounts[] = { 1, 3, 5, 2 };

    ImageIndexIterator iter = ImageIndexIterator::Make2DArray(minMip, maxMip, layerCounts);

    ASSERT_GE(0, minMip);
    ASSERT_EQ(ArraySize(layerCounts), static_cast<size_t>(maxMip));

    for (GLint mip = minMip; mip < maxMip; mip++)
    {
        for (GLint layer = 0; layer < layerCounts[mip]; layer++)
        {
            EXPECT_TRUE(iter.hasNext());
            ImageIndex nextIndex = iter.next();

            EXPECT_EQ(static_cast<GLenum>(GL_TEXTURE_2D_ARRAY), nextIndex.type);
            EXPECT_EQ(mip, nextIndex.mipIndex);
            EXPECT_EQ(layer, nextIndex.layerIndex);
            EXPECT_TRUE(nextIndex.hasLayer());
        }
    }
}

} // namespace

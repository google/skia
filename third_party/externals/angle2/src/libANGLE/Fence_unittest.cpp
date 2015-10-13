//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "libANGLE/Fence.h"
#include "libANGLE/renderer/FenceNVImpl.h"
#include "libANGLE/renderer/FenceSyncImpl.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgumentPointee;

namespace {

//
// FenceNV tests
//

class MockFenceNVImpl : public rx::FenceNVImpl
{
  public:
    virtual ~MockFenceNVImpl() { destroy(); }

    MOCK_METHOD1(set, gl::Error(GLenum));
    MOCK_METHOD1(test, gl::Error(GLboolean *));
    MOCK_METHOD0(finish, gl::Error());

    MOCK_METHOD0(destroy, void());
};

class FenceNVTest : public testing::Test
{
  protected:
    virtual void SetUp()
    {
        mImpl = new MockFenceNVImpl;
        EXPECT_CALL(*mImpl, destroy());
        mFence = new gl::FenceNV(mImpl);
    }

    virtual void TearDown()
    {
        delete mFence;
    }

    MockFenceNVImpl *mImpl;
    gl::FenceNV* mFence;
};

TEST_F(FenceNVTest, DestructionDeletesImpl)
{
    MockFenceNVImpl* impl = new MockFenceNVImpl;
    EXPECT_CALL(*impl, destroy()).Times(1).RetiresOnSaturation();

    gl::FenceNV* fence = new gl::FenceNV(impl);
    delete fence;

    // Only needed because the mock is leaked if bugs are present,
    // which logs an error, but does not cause the test to fail.
    // Ordinarily mocks are verified when destroyed.
    testing::Mock::VerifyAndClear(impl);
}

TEST_F(FenceNVTest, SetAndTestBehavior)
{
    EXPECT_CALL(*mImpl, set(_))
        .WillOnce(Return(gl::Error(GL_NO_ERROR)))
        .RetiresOnSaturation();
    EXPECT_FALSE(mFence->isSet());
    mFence->set(GL_ALL_COMPLETED_NV);
    EXPECT_TRUE(mFence->isSet());
    // Fake the behavior of testing the fence before and after it's passed.
    EXPECT_CALL(*mImpl, test(_))
        .WillOnce(DoAll(SetArgumentPointee<0>(GL_FALSE),
                        Return(gl::Error(GL_NO_ERROR))))
        .WillOnce(DoAll(SetArgumentPointee<0>(GL_TRUE),
                        Return(gl::Error(GL_NO_ERROR))))
        .RetiresOnSaturation();
    GLboolean out;
    mFence->test(&out);
    EXPECT_EQ(GL_FALSE, out);
    mFence->test(&out);
    EXPECT_EQ(GL_TRUE, out);
}

//
// FenceSync tests
//

class MockFenceSyncImpl : public rx::FenceSyncImpl
{
  public:
    virtual ~MockFenceSyncImpl() { destroy(); }

    MOCK_METHOD2(set, gl::Error(GLenum, GLbitfield));
    MOCK_METHOD3(clientWait, gl::Error(GLbitfield, GLuint64, GLenum *));
    MOCK_METHOD2(serverWait, gl::Error(GLbitfield, GLuint64));
    MOCK_METHOD1(getStatus, gl::Error(GLint *));

    MOCK_METHOD0(destroy, void());
};

class FenceSyncTest : public testing::Test
{
  protected:
    virtual void SetUp()
    {
        mImpl = new MockFenceSyncImpl;
        EXPECT_CALL(*mImpl, destroy());
        mFence = new gl::FenceSync(mImpl, 1);
        mFence->addRef();
    }

    virtual void TearDown()
    {
        mFence->release();
    }

    MockFenceSyncImpl *mImpl;
    gl::FenceSync* mFence;
};

TEST_F(FenceSyncTest, DestructionDeletesImpl)
{
    MockFenceSyncImpl* impl = new MockFenceSyncImpl;
    EXPECT_CALL(*impl, destroy()).Times(1).RetiresOnSaturation();

    gl::FenceSync* fence = new gl::FenceSync(impl, 1);
    fence->addRef();
    fence->release();

    // Only needed because the mock is leaked if bugs are present,
    // which logs an error, but does not cause the test to fail.
    // Ordinarily mocks are verified when destroyed.
    testing::Mock::VerifyAndClear(impl);
}

TEST_F(FenceSyncTest, SetAndGetStatusBehavior)
{
    EXPECT_CALL(*mImpl, set(_, _))
        .WillOnce(Return(gl::Error(GL_NO_ERROR)))
        .RetiresOnSaturation();
    mFence->set(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    EXPECT_EQ(static_cast<GLenum>(GL_SYNC_GPU_COMMANDS_COMPLETE), mFence->getCondition());
    // Fake the behavior of testing the fence before and after it's passed.
    EXPECT_CALL(*mImpl, getStatus(_))
        .WillOnce(DoAll(SetArgumentPointee<0>(GL_UNSIGNALED),
                        Return(gl::Error(GL_NO_ERROR))))
        .WillOnce(DoAll(SetArgumentPointee<0>(GL_SIGNALED),
                        Return(gl::Error(GL_NO_ERROR))))
        .RetiresOnSaturation();
    GLint out;
    mFence->getStatus(&out);
    EXPECT_EQ(GL_UNSIGNALED, out);
    mFence->getStatus(&out);
    EXPECT_EQ(GL_SIGNALED, out);
}

} // namespace

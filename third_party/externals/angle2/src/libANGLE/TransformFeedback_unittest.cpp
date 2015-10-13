//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Caps.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/renderer/BufferImpl_mock.h"
#include "libANGLE/renderer/TransformFeedbackImpl_mock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgumentPointee;

namespace
{

class TransformFeedbackTest : public testing::Test
{
  protected:
    virtual void SetUp()
    {
        // Set a reasonable number of tf attributes
        mCaps.maxTransformFeedbackSeparateAttributes = 8;

        mImpl = new rx::MockTransformFeedbackImpl;
        EXPECT_CALL(*mImpl, destructor());
        mFeedback = new gl::TransformFeedback(mImpl, 1, mCaps);
        mFeedback->addRef();
    }

    virtual void TearDown()
    {
        mFeedback->release();
    }

    rx::MockTransformFeedbackImpl* mImpl;
    gl::TransformFeedback* mFeedback;
    gl::Caps mCaps;
};

TEST_F(TransformFeedbackTest, DestructionDeletesImpl)
{
    rx::MockTransformFeedbackImpl* impl = new rx::MockTransformFeedbackImpl;
    EXPECT_CALL(*impl, destructor()).Times(1).RetiresOnSaturation();

    gl::TransformFeedback* feedback = new gl::TransformFeedback(impl, 1, mCaps);
    feedback->addRef();
    feedback->release();

    // Only needed because the mock is leaked if bugs are present,
    // which logs an error, but does not cause the test to fail.
    // Ordinarily mocks are verified when destroyed.
    testing::Mock::VerifyAndClear(impl);
}

TEST_F(TransformFeedbackTest, SideEffectsOfStartAndStop)
{
    testing::InSequence seq;

    EXPECT_FALSE(mFeedback->isActive());
    EXPECT_CALL(*mImpl, begin(GL_TRIANGLES));
    mFeedback->begin(GL_TRIANGLES);
    EXPECT_TRUE(mFeedback->isActive());
    EXPECT_EQ(static_cast<GLenum>(GL_TRIANGLES), mFeedback->getPrimitiveMode());
    EXPECT_CALL(*mImpl, end());
    mFeedback->end();
    EXPECT_FALSE(mFeedback->isActive());
}

TEST_F(TransformFeedbackTest, SideEffectsOfPauseAndResume)
{
    testing::InSequence seq;

    EXPECT_FALSE(mFeedback->isActive());
    EXPECT_CALL(*mImpl, begin(GL_TRIANGLES));
    mFeedback->begin(GL_TRIANGLES);
    EXPECT_FALSE(mFeedback->isPaused());
    EXPECT_CALL(*mImpl, pause());
    mFeedback->pause();
    EXPECT_TRUE(mFeedback->isPaused());
    EXPECT_CALL(*mImpl, resume());
    mFeedback->resume();
    EXPECT_FALSE(mFeedback->isPaused());
    EXPECT_CALL(*mImpl, end());
    mFeedback->end();
}

TEST_F(TransformFeedbackTest, BufferBinding)
{
    rx::MockBufferImpl *bufferImpl = new rx::MockBufferImpl;
    gl::Buffer *buffer = new gl::Buffer(bufferImpl, 1);
    EXPECT_CALL(*bufferImpl, destructor()).Times(1).RetiresOnSaturation();

    static const size_t bindIndex = 0;

    rx::MockTransformFeedbackImpl *feedbackImpl = new rx::MockTransformFeedbackImpl;
    EXPECT_CALL(*feedbackImpl, destructor()).Times(1).RetiresOnSaturation();

    gl::TransformFeedback *feedback = new gl::TransformFeedback(feedbackImpl, 1, mCaps);

    EXPECT_EQ(feedback->getIndexedBufferCount(), mCaps.maxTransformFeedbackSeparateAttributes);

    EXPECT_CALL(*feedbackImpl, bindGenericBuffer(_));
    feedback->bindGenericBuffer(buffer);
    EXPECT_EQ(feedback->getGenericBuffer().get(), buffer);

    EXPECT_CALL(*feedbackImpl, bindIndexedBuffer(_, _));
    feedback->bindIndexedBuffer(bindIndex, buffer, 0, 1);
    for (size_t i = 0; i < feedback->getIndexedBufferCount(); i++)
    {
        if (i == bindIndex)
        {
            EXPECT_EQ(feedback->getIndexedBuffer(i).get(), buffer);
        }
        else
        {
            EXPECT_EQ(feedback->getIndexedBuffer(i).get(), nullptr);
        }
    }

    feedback->addRef();
    feedback->release();

    testing::Mock::VerifyAndClear(bufferImpl);
}

} // namespace

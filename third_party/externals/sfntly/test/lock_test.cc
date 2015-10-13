/*
 * Copyright 2011 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>

#include "gtest/gtest.h"
#include "sfntly/port/lock.h"
#include "test/platform_thread.h"

namespace sfntly {

// Basic test to make sure that Acquire()/Unlock()/Try() don't crash

class BasicLockTestThread : public PlatformThread::Delegate {
 public:
  BasicLockTestThread(Lock* lock) : lock_(lock), acquired_(0) {}

  virtual void ThreadMain() {
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      lock_->Unlock();
    }
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      PlatformThread::Sleep(rand() % 20);
      lock_->Unlock();
    }
    for (int i = 0; i < 10; i++) {
      if (lock_->Try()) {
        acquired_++;
        PlatformThread::Sleep(rand() % 20);
        lock_->Unlock();
      }
    }
  }

  int acquired() const { return acquired_; }

 private:
  Lock* lock_;
  int acquired_;

  NO_COPY_AND_ASSIGN(BasicLockTestThread);
};

bool BasicLockTest() {
  Lock lock;
  BasicLockTestThread thread(&lock);
  PlatformThreadHandle handle = kNullThreadHandle;

  EXPECT_TRUE(PlatformThread::Create(&thread, &handle));

  int acquired = 0;
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    lock.Unlock();
  }
  for (int i = 0; i < 10; i++) {
    lock.Acquire();
    acquired++;
    PlatformThread::Sleep(rand() % 20);
    lock.Unlock();
  }
  for (int i = 0; i < 10; i++) {
    if (lock.Try()) {
      acquired++;
      PlatformThread::Sleep(rand() % 20);
      lock.Unlock();
    }
  }
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    PlatformThread::Sleep(rand() % 20);
    lock.Unlock();
  }

  PlatformThread::Join(handle);

  EXPECT_GE(acquired, 20);
  EXPECT_GE(thread.acquired(), 20);

  return true;
}

// Test that Try() works as expected -------------------------------------------

class TryLockTestThread : public PlatformThread::Delegate {
 public:
  TryLockTestThread(Lock* lock) : lock_(lock), got_lock_(false) {}

  virtual void ThreadMain() {
    got_lock_ = lock_->Try();
    if (got_lock_)
      lock_->Unlock();
  }

  bool got_lock() const { return got_lock_; }

 private:
  Lock* lock_;
  bool got_lock_;

  NO_COPY_AND_ASSIGN(TryLockTestThread);
};

bool TryLockTest() {
  Lock lock;

  EXPECT_TRUE(lock.Try());
  // We now have the lock....

  // This thread will not be able to get the lock.
  {
    TryLockTestThread thread(&lock);
    PlatformThreadHandle handle = kNullThreadHandle;

    EXPECT_TRUE(PlatformThread::Create(&thread, &handle));

    PlatformThread::Join(handle);

    EXPECT_FALSE(thread.got_lock());
  }

  lock.Unlock();

  // This thread will....
  {
    TryLockTestThread thread(&lock);
    PlatformThreadHandle handle = kNullThreadHandle;

    EXPECT_TRUE(PlatformThread::Create(&thread, &handle));

    PlatformThread::Join(handle);

    EXPECT_TRUE(thread.got_lock());
    // But it released it....
    EXPECT_TRUE(lock.Try());
  }

  lock.Unlock();
  return true;
}

// Tests that locks actually exclude -------------------------------------------

class MutexLockTestThread : public PlatformThread::Delegate {
 public:
  MutexLockTestThread(Lock* lock, int* value) : lock_(lock), value_(value) {}

  // Static helper which can also be called from the main thread.
  static void DoStuff(Lock* lock, int* value) {
    for (int i = 0; i < 40; i++) {
      lock->Acquire();
      int v = *value;
      PlatformThread::Sleep(rand() % 10);
      *value = v + 1;
      lock->Unlock();
    }
  }

  virtual void ThreadMain() {
    DoStuff(lock_, value_);
  }

 private:
  Lock* lock_;
  int* value_;

  NO_COPY_AND_ASSIGN(MutexLockTestThread);
};

bool MutexTwoThreads() {
  Lock lock;
  int value = 0;

  MutexLockTestThread thread(&lock, &value);
  PlatformThreadHandle handle = kNullThreadHandle;

  EXPECT_TRUE(PlatformThread::Create(&thread, &handle));

  MutexLockTestThread::DoStuff(&lock, &value);

  PlatformThread::Join(handle);

  EXPECT_EQ(2 * 40, value);
  return true;
}

bool MutexFourThreads() {
  Lock lock;
  int value = 0;

  MutexLockTestThread thread1(&lock, &value);
  MutexLockTestThread thread2(&lock, &value);
  MutexLockTestThread thread3(&lock, &value);
  PlatformThreadHandle handle1 = kNullThreadHandle;
  PlatformThreadHandle handle2 = kNullThreadHandle;
  PlatformThreadHandle handle3 = kNullThreadHandle;

  EXPECT_TRUE(PlatformThread::Create(&thread1, &handle1));
  EXPECT_TRUE(PlatformThread::Create(&thread2, &handle2));
  EXPECT_TRUE(PlatformThread::Create(&thread3, &handle3));

  MutexLockTestThread::DoStuff(&lock, &value);

  PlatformThread::Join(handle1);
  PlatformThread::Join(handle2);
  PlatformThread::Join(handle3);

  EXPECT_EQ(4 * 40, value);
  return true;
}

}  // namespace sfntly

TEST(LockTest, Basic) {
  ASSERT_TRUE(sfntly::BasicLockTest());
}

TEST(LockTest, TryLock) {
  ASSERT_TRUE(sfntly::TryLockTest());
}

TEST(LockTest, Mutex) {
  ASSERT_TRUE(sfntly::MutexTwoThreads());
  ASSERT_TRUE(sfntly::MutexFourThreads());
}

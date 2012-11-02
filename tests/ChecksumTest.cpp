
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkChecksum.h"
#include "SkConsistentChecksum.h"

namespace skiatest {
    class ChecksumTestClass : public Test {
    public:
        static Test* Factory(void*) {return SkNEW(ChecksumTestClass); }
    protected:
        virtual void onGetName(SkString* name) { name->set("Checksum"); }
        virtual void onRun(Reporter* reporter) {
            this->fReporter = reporter;
            RunTest();
        }
    private:
        enum Algorithm {
            kSkChecksum,
            kSkConsistentChecksum
        };

        // Call Compute(data, size) on the appropriate checksum algorithm,
        // depending on this->fWhichAlgorithm.
        uint32_t ComputeChecksum(uint32_t* data, size_t size) {
            // Our checksum algorithms require 32-bit aligned data.
            // If either of these tests fail, then the algorithm
            // doesn't have a chance.
            REPORTER_ASSERT_MESSAGE(fReporter,
                                    reinterpret_cast<uintptr_t>(data) % 4 == 0,
                                    "test data pointer is not 32-bit aligned");
            REPORTER_ASSERT_MESSAGE(fReporter, SkIsAlign4(size),
                                    "test data size is not 32-bit aligned");

            switch(fWhichAlgorithm) {
            case kSkChecksum:
                return SkChecksum::Compute(data, size);
            case kSkConsistentChecksum:
                return SkConsistentChecksum::Compute(data, size);
            default:
                SkString message("fWhichAlgorithm has unknown value ");
                message.appendf("%d", fWhichAlgorithm);
                fReporter->reportFailed(message);
            }
            // we never get here
            return 0;
        }

        // Confirm that the checksum algorithm (specified by fWhichAlgorithm)
        // generates the same results if called twice over the same data.
        void TestChecksumSelfConsistency(size_t buf_size) {
            SkAutoMalloc storage(buf_size);
            uint32_t*    ptr = (uint32_t*)storage.get();
            char*        cptr = (char*)ptr;

            sk_bzero(ptr, buf_size);
            uint32_t prev = 0;

            // assert that as we change values (from 0 to non-zero) in
            // our buffer, we get a different value
            for (size_t i = 0; i < buf_size; ++i) {
                cptr[i] = (i & 0x7f) + 1; // need some non-zero value here

                // Try checksums of different-sized chunks, but always
                // 32-bit aligned and big enough to contain all the
                // nonzero bytes.
                size_t checksum_size = (((i/4)+1)*4);
                REPORTER_ASSERT(fReporter, checksum_size <= buf_size);

                uint32_t curr = ComputeChecksum(ptr, checksum_size);
                REPORTER_ASSERT(fReporter, prev != curr);
                uint32_t again = ComputeChecksum(ptr, checksum_size);
                REPORTER_ASSERT(fReporter, again == curr);
                prev = curr;
            }
        }

        // Return the checksum of a portion of this static test data
        // (8 bytes repeated twice): "1234567812345678"
        uint32_t GetTestDataChecksum(size_t offset, size_t len) {
            static char testbytes[] = "1234567812345678";
            uint32_t* ptr = reinterpret_cast<uint32_t*>(testbytes);
            return ComputeChecksum(ptr, len);
        }

        void RunTest() {
            // Test self-consistency of checksum algorithms.
            fWhichAlgorithm = kSkChecksum;
            REPORTER_ASSERT(fReporter,
                            GetTestDataChecksum(0, 8) ==
                            GetTestDataChecksum(8, 8));
            TestChecksumSelfConsistency(128);
            fWhichAlgorithm = kSkConsistentChecksum;
            REPORTER_ASSERT(fReporter,
                            GetTestDataChecksum(0, 8) ==
                            GetTestDataChecksum(8, 8));
            TestChecksumSelfConsistency(128);

            // Test checksum results that should be consistent across
            // versions and platforms.
            fWhichAlgorithm = kSkChecksum;
            REPORTER_ASSERT(fReporter, ComputeChecksum(NULL, 0) == 0);
            fWhichAlgorithm = kSkConsistentChecksum;
            REPORTER_ASSERT(fReporter, ComputeChecksum(NULL, 0) == 0);
            REPORTER_ASSERT(fReporter, GetTestDataChecksum(0, 8) == 0xa12fac2c);
            REPORTER_ASSERT(fReporter, GetTestDataChecksum(8, 8) == 0xa12fac2c);
            REPORTER_ASSERT(fReporter, GetTestDataChecksum(8, 4) == 0x34333231);
        }

        Reporter* fReporter;
        Algorithm fWhichAlgorithm;
    };

    static TestRegistry gReg(ChecksumTestClass::Factory);
}

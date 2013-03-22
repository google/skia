/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Error codes used by gmmain.cpp.
 */

namespace skiagm {

    /**
     * The complete list of error types we might encounter in GM.
     */
    enum ErrorType {
#if SK_SUPPORT_GPU
        kNoGpuContext_ErrorType,
#endif
        kImageMismatch_ErrorType,
        kMissingExpectations_ErrorType,
        kWritingReferenceImage_ErrorType,
        kLast_ErrorType = kWritingReferenceImage_ErrorType
    };

    /**
     * A combination of 0 or more ErrorTypes.
     */
    class ErrorCombination {
    public:
        ErrorCombination() : fBitfield(0) {}
        ErrorCombination(const ErrorType type) : fBitfield(1 << type) {}

        /**
         * Returns true iff there are NO errors.
         */
        bool isEmpty() const {
            return (0 == this->fBitfield);
        }

        /**
         * Adds this ErrorType to this ErrorCombination.
         */
        void add(const ErrorType type) {
            this->fBitfield |= (1 << type);
        }

        /**
         * Adds all ErrorTypes in "other" to this ErrorCombination.
         */
        void add(const ErrorCombination other) {
            this->fBitfield |= other.fBitfield;
        }

        /**
         * Returns true iff this ErrorCombination includes this ErrorType.
         */
        bool includes(const ErrorType type) const {
            return !(0 == (this->fBitfield & (1 << type)));
        }

        /**
         * Returns a new ErrorCombination, which includes the union of all
         * ErrorTypes in two ErrorCombination objects (this and other).
         */
        ErrorCombination plus(const ErrorCombination& other) const {
            ErrorCombination retval;
            retval.fBitfield = this->fBitfield | other.fBitfield;
            return retval;
        }

        /**
         * Returns a new ErrorCombination, which is a copy of "this"
         * but with all ErrorTypes in "other" removed.
         */
        ErrorCombination minus(const ErrorCombination& other) const {
            ErrorCombination retval;
            retval.fBitfield = this->fBitfield & ~(other.fBitfield);
            return retval;
        }

    private:
        int fBitfield;
    };

    // No errors at all.
    const static ErrorCombination kEmpty_ErrorCombination;
}

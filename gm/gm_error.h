/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Error codes used by gmmain.cpp.
 */

#ifndef gm_error_DEFINED
#define gm_error_DEFINED

#include "gm.h"

namespace skiagm {

    /**
     * The complete list of error types we might encounter in GM.
     */
    enum ErrorType {
        // Even though kNoGpuContext_ErrorType only occurs when SK_SUPPORT_GPU
        // is turned on, we always include this type in our enum so that
        // reports will be consistent whether SK_SUPPORT_GPU is turned on
        // or off (as long as the number of these errors is 0).
        kNoGpuContext_ErrorType,

        kIntentionallySkipped_ErrorType,
        kRenderModeMismatch_ErrorType,
        kGeneratePdfFailed_ErrorType,
        kExpectationsMismatch_ErrorType,
        kMissingExpectations_ErrorType,
        kWritingReferenceImage_ErrorType,
        kLast_ErrorType = kWritingReferenceImage_ErrorType
    };

    /**
     * Returns the name of the given ErrorType.
     */
    static const char *getErrorTypeName(ErrorType type) {
        switch(type) {
        case kNoGpuContext_ErrorType:
            return "NoGpuContext";
        case kIntentionallySkipped_ErrorType:
            return "IntentionallySkipped";
        case kRenderModeMismatch_ErrorType:
            return "RenderModeMismatch";
        case kGeneratePdfFailed_ErrorType:
            return "GeneratePdfFailed";
        case kExpectationsMismatch_ErrorType:
            return "ExpectationsMismatch";
        case kMissingExpectations_ErrorType:
            return "MissingExpectations";
        case kWritingReferenceImage_ErrorType:
            return "WritingReferenceImage";
        }
        // control should never reach here
        SkDEBUGFAIL("getErrorTypeName() called with unknown type");
        return "Unknown";
    }

    /**
     * Fills in "type" with the ErrorType associated with name "name".
     * Returns true if we found one, false if it is an unknown type name.
     */
    static bool getErrorTypeByName(const char name[], ErrorType *type) {
        for (int typeInt = 0; typeInt <= kLast_ErrorType; typeInt++) {
            ErrorType thisType = static_cast<ErrorType>(typeInt);
            const char *thisTypeName = getErrorTypeName(thisType);
            if (0 == strcmp(thisTypeName, name)) {
                *type = thisType;
                return true;
            }
        }
        return false;
    }

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
         * Returns a string representation of all ErrorTypes in this
         * ErrorCombination.
         *
         * @param separator text with which to separate ErrorType names
         */
        SkString asString(const char separator[]) const {
            SkString s;
            for (int typeInt = 0; typeInt <= kLast_ErrorType; typeInt++) {
                ErrorType type = static_cast<ErrorType>(typeInt);
                if (this->includes(type)) {
                    if (!s.isEmpty()) {
                        s.append(separator);
                    }
                    s.append(getErrorTypeName(type));
                }
            }
            return s;
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

#endif // ifndef gm_error_DEFINED

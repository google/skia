/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Token_DEFINED
#define skgpu_Token_DEFINED

#include <cstdint>

class GrOpFlushState;
class TestingUploadTarget;
namespace skgpu::graphite {
    class RecorderPriv;
}

namespace skgpu {

/**
 * A generic token used to sequence operations within a Recorder.
 */
class Token {
public:
    static Token InvalidToken() { return Token(0); }

    Token(const Token&) = default;
    Token& operator=(const Token&) = default;

    bool operator==(const Token& that) const { return fSequenceNumber == that.fSequenceNumber; }
    bool operator!=(const Token& that) const { return !(*this == that); }
    bool operator<(const Token that) const { return fSequenceNumber < that.fSequenceNumber; }
    bool operator<=(const Token that) const { return fSequenceNumber <= that.fSequenceNumber; }
    bool operator>(const Token that) const { return fSequenceNumber > that.fSequenceNumber; }
    bool operator>=(const Token that) const { return fSequenceNumber >= that.fSequenceNumber; }

    Token& operator++() {
        ++fSequenceNumber;
        return *this;
    }
    Token operator++(int) {
        auto old = fSequenceNumber;
        ++fSequenceNumber;
        return Token(old);
    }

    Token next() const { return Token(fSequenceNumber + 1); }

    /** Returns the raw value for debugging and comparison. */
    uint64_t value() const { return fSequenceNumber; }

    /** Is this token in the [start, end] inclusive interval? */
    bool inInterval(const Token& start, const Token& end) {
        return *this >= start && *this <= end;
    }

private:
    explicit Token(uint64_t sequenceNumber) : fSequenceNumber(sequenceNumber) {}
    uint64_t fSequenceNumber;

    friend class TokenTracker; // Allow TokenTracker to construct one
};

/**
 * Encapsulates the incrementing and distribution of Tokens for a Recorder.
 */
class TokenTracker {
public:
    /**
     * Gets the token one beyond the last token that has been flushed.
     * This represents the ID for the *current* batch of work being recorded.
     */
    Token nextFlushToken() const { return fCurrentFlushToken.next(); }

    /**
     * Gets the token that was *just issued*. This represents the ID of the
     * flush that was most recently completed.
     */
    Token currentFlushToken() const { return fCurrentFlushToken; }

    /**
     * Gets the next draw token.
     */
    Token nextDrawToken() const { return fCurrentDrawToken.next(); }

private:
    // Only these classes get to increment the token counters
    friend class ::GrOpFlushState;               // Ganesh
    friend class ::TestingUploadTarget;          // ``
    friend class skgpu::graphite::RecorderPriv;  // Graphite

    Token issueDrawToken() { return ++fCurrentDrawToken; }
    Token issueFlushToken() { return ++fCurrentFlushToken; }

    Token fCurrentDrawToken = Token::InvalidToken();
    Token fCurrentFlushToken = Token::InvalidToken();
};

} // namespace skgpu

#endif // skgpu_Token_DEFINED

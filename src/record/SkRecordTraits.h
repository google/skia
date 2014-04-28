#include "SkRecords.h"
#include "SkTLogic.h"

// Type traits that are useful for working with SkRecords.

namespace SkRecords {

namespace {

// Abstracts away whether the T is optional or not.
template <typename T> const T* as_ptr(const SkRecords::Optional<T>& x) { return x; }
template <typename T> const T* as_ptr(const T& x) { return &x; }

}  // namespace

// Gets the paint from any command that may have one.
template <typename Command> const SkPaint* GetPaint(const Command& x) { return as_ptr(x.paint); }

// Have a paint?  You are a draw command!
template <typename Command> struct IsDraw {
    SK_CREATE_MEMBER_DETECTOR(paint);
    static const bool value = HasMember_paint<Command>::value;
};

// Have a clip op?  You are a clip command.
template <typename Command> struct IsClip {
    SK_CREATE_MEMBER_DETECTOR(op);
    static const bool value = HasMember_op<Command>::value;
};

}  // namespace SkRecords

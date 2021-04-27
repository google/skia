#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S1 {
    int x;
};
thread bool operator==(thread const S1& left, thread const S1& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const S1& left, thread const S1& right) {
    return !(left == right);
}
struct S2 {
    S1 x;
};
thread bool operator==(thread const S2& left, thread const S2& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const S2& left, thread const S2& right) {
    return !(left == right);
}
struct S3 {
    S2 x;
};
thread bool operator==(thread const S3& left, thread const S3& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const S3& left, thread const S3& right) {
    return !(left == right);
}
struct S4 {
    S3 x;
};
thread bool operator==(thread const S4& left, thread const S4& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const S4& left, thread const S4& right) {
    return !(left == right);
}
struct S5 {
    S4 x;
};
thread bool operator==(thread const S5& left, thread const S5& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const S5& left, thread const S5& right) {
    return !(left == right);
}
struct S6 {
    S5 x;
};
thread bool operator==(thread const S6& left, thread const S6& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const S6& left, thread const S6& right) {
    return !(left == right);
}
struct S7 {
    S6 x;
};
thread bool operator==(thread const S7& left, thread const S7& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const S7& left, thread const S7& right) {
    return !(left == right);
}
struct S8 {
    S7 x;
};
thread bool operator==(thread const S8& left, thread const S8& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const S8& left, thread const S8& right) {
    return !(left == right);
}
struct SA1 {
    array<int, 8> x;
};
thread bool operator==(thread const SA1& left, thread const SA1& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const SA1& left, thread const SA1& right) {
    return !(left == right);
}
struct SA2 {
    array<SA1, 7> x;
};
thread bool operator==(thread const SA2& left, thread const SA2& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const SA2& left, thread const SA2& right) {
    return !(left == right);
}
struct SA3 {
    array<SA2, 6> x;
};
thread bool operator==(thread const SA3& left, thread const SA3& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const SA3& left, thread const SA3& right) {
    return !(left == right);
}
struct SA4 {
    array<SA3, 5> x;
};
thread bool operator==(thread const SA4& left, thread const SA4& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const SA4& left, thread const SA4& right) {
    return !(left == right);
}
struct SA5 {
    array<SA4, 4> x;
};
thread bool operator==(thread const SA5& left, thread const SA5& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const SA5& left, thread const SA5& right) {
    return !(left == right);
}
struct SA6 {
    array<SA5, 3> x;
};
thread bool operator==(thread const SA6& left, thread const SA6& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const SA6& left, thread const SA6& right) {
    return !(left == right);
}
struct SA7 {
    array<SA6, 2> x;
};
thread bool operator==(thread const SA7& left, thread const SA7& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const SA7& left, thread const SA7& right) {
    return !(left == right);
}
struct SA8 {
    array<SA7, 1> x;
};
thread bool operator==(thread const SA8& left, thread const SA8& right) {
    return (left.x == right.x);
}
thread bool operator!=(thread const SA8& left, thread const SA8& right) {
    return !(left == right);
}
struct Inputs {
    S8 s8;
    array<SA8, 9> sa8;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


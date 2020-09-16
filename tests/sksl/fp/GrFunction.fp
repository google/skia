uniform half4 color;

half4 flip(half4 c) {
    // Prevent the function from being inlined by making it
    // sufficiently large, and by calling it more than once.
    int x = 42;
    ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;
    ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;
    ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;
    return c.abgr;
}

void main() {
    sk_OutColor = flip(flip(flip(color)));
}

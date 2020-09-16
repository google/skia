uniform half4 color;

half4 flip(half4 c) {
    return c.abgr;
}

void main() {
    sk_OutColor = flip(color);
}

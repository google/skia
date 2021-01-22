uniform half4 color;

half4 flip(half4 c) {
    return c.abgr;
}

half4 main() {
    return flip(color);
}

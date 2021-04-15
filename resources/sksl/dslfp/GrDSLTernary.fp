half4 main() {
    float4 a = half4(1, 2, 3, 4);
    float4 b = half4(5, 6, 7, 8);

    int x = (a.x != b.y) ? 123 : 456;
    int y = (a.y <= b.z) ? 234 : 567;

    return (x == y) ? half4(1) : half4(0);
}

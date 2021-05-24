// Test that '%' is expanded to '%%' in emitCode
uniform int value;
half4 main() {
    return half4(half(1 % value));
}

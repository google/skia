// Test that '%' is expanded to '%%' in emitCode
half4 main() {
    return half4(half(1 % int(sqrt(2))));
}

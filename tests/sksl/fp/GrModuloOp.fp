// Test that '%' is expanded to '%%' in emitCode
half4 main() {
    sk_OutColor.r = half(1 % int(sqrt(2)));
}

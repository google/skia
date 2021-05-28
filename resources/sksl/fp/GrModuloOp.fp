// Test that '%' is expanded to '%%' in emitCode
uniform int unknownInput;
half4 main() {
    return half4(unknownInput % 7);
}

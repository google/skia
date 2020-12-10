// Test that '%' is expanded to '%%' in emitCode
void main() {
    sk_OutColor.r = half(1 % int(sqrt(2)));
}

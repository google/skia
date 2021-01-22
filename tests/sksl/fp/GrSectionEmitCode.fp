half x = 10;
@emitCode { fragBuilder->codeAppendf("half y = %d\n", x * 2); }
void main() {
    sk_OutColor = half4(1);
}

half x = 10;
@emitCode { fragBuilder->codeAppendf("half y = %d\n", x * 2); }
half4 main() {
    return half4(1);
}

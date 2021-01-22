int factorial(int x) {
    return (x <= 1) ? 1 : x * factorial(x - 1);
}

half4 main() {
    return half4(factorial(7));
}

int factorial(int x) {
    return (x <= 1) ? 1 : x * factorial(x - 1);
}

void main() {
    sk_OutColor = half4(factorial(7));
}

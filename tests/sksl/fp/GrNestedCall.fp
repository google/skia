/*#pragma settings NoInline*/

half4 func1();
half4 func2();
half4 func3();

half4 func1() {
    return func2();
}

half4 func2() {
    return func3();
}

half4 func3() {
    return half4(1);
}

void main() {
    sk_OutColor = func1();
}

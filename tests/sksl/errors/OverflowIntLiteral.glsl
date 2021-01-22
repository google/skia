
out vec4 sk_FragColor;
int intMin = -2147483648;
int intMinMinusOne = -2147483649;
int intMax = 2147483647;
int intMaxPlusOne = 2147483648;
void main() {
    sk_FragColor.x = float(intMin);
    sk_FragColor.x = float(intMax);
    sk_FragColor.x = float(intMinMinusOne);
    sk_FragColor.x = float(intMaxPlusOne);
}

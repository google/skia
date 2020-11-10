
out vec4 sk_FragColor;
uint uintMin = 0u;
uint uintMinMinusOne = 4294967295u;
uint uintMax = 4294967295u;
uint uintMaxPlusOne = 0u;
void main() {
    sk_FragColor.x = float(uintMin);
    sk_FragColor.x = float(uintMinMinusOne);
    sk_FragColor.x = float(uintMax);
    sk_FragColor.x = float(uintMaxPlusOne);
}

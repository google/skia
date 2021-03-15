
out vec4 sk_FragColor;
uniform int a;
uniform uint b;
void main() {
    sk_FragColor.x = float(findMSB(a));
    sk_FragColor.y = float(findMSB(b));
}

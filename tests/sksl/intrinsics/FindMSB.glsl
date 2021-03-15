
out vec4 sk_FragColor;
int a;
uint b;
void main() {
    sk_FragColor.x = float(findMSB(a));
    sk_FragColor.y = float(findMSB(b));
}

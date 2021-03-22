
out vec4 sk_FragColor;
void outParameter_vh(inout float x) {
    x *= 2.0;
}
void main() {
    float x = 1.0;
    outParameter_vh(x);
    sk_FragColor.x = x;
}

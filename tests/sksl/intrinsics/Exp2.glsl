
out vec4 sk_FragColor;
uniform float a;
void main() {
    sk_FragColor.x = exp2(a);
}

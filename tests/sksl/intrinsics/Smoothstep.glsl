
out vec4 sk_FragColor;
uniform float a;
uniform float b;
uniform float c;
void main() {
    sk_FragColor.x = smoothstep(a, b, c);
}

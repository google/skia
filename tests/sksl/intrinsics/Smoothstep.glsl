
out vec4 sk_FragColor;
float a;
float b;
float c;
void main() {
    sk_FragColor.x = smoothstep(a, b, c);
}

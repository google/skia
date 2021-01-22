
out vec4 sk_FragColor;
in float a;
in float b;
in float c;
in vec4 d;
in vec4 e;
in vec4 f;
void main() {
    sk_FragColor.x = faceforward(a, b, c);
    sk_FragColor = faceforward(d, e, f);
}


out vec4 sk_FragColor;
uniform float a;
uniform float b;
uniform float c;
uniform vec4 d;
uniform vec4 e;
uniform vec4 f;
void main() {
    sk_FragColor.x = faceforward(a, b, c);
    sk_FragColor = faceforward(d, e, f);
}

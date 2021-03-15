
out vec4 sk_FragColor;
uniform float a;
uniform float b;
uniform float c;
uniform vec4 d;
uniform vec4 e;
void main() {
    sk_FragColor.x = refract(a, b, c);
    sk_FragColor = refract(d, e, c);
}

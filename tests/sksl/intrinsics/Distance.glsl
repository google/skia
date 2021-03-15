
out vec4 sk_FragColor;
uniform float a;
uniform float b;
uniform vec4 c;
uniform vec4 d;
void main() {
    sk_FragColor.x = distance(a, b);
    sk_FragColor.x = distance(c, d);
}


out vec4 sk_FragColor;
uniform float a;
uniform float b;
uniform vec4 c;
uniform vec4 d;
void main() {
    sk_FragColor.x = mod(a, b);
    sk_FragColor = mod(c, b);
    sk_FragColor = mod(c, d);
}

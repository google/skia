
out vec4 sk_FragColor;
uniform float a;
uniform float b;
uniform vec4 c;
uniform vec4 d;
void main() {
    sk_FragColor.x = atan(a);
    sk_FragColor.x = atan(a, b);
    sk_FragColor = atan(c);
    sk_FragColor = atan(c, d);
}


out vec4 sk_FragColor;
float a;
float b;
vec4 c;
vec4 d;
void main() {
    sk_FragColor.x = mod(a, b);
    sk_FragColor = mod(c, b);
    sk_FragColor = mod(c, d);
}

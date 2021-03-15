
out vec4 sk_FragColor;
float a;
float b;
vec4 c;
vec4 d;
void main() {
    sk_FragColor.x = atan(a);
    sk_FragColor.x = atan(a, b);
    sk_FragColor = atan(c);
    sk_FragColor = atan(c, d);
}

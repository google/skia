
out vec4 sk_FragColor;
float a;
float b;
vec4 c;
vec4 d;
void main() {
    sk_FragColor.x = reflect(a, b);
    sk_FragColor = reflect(c, d);
}

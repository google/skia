
out vec4 sk_FragColor;
float a;
float b;
float c;
vec4 d;
vec4 e;
void main() {
    sk_FragColor.x = refract(a, b, c);
    sk_FragColor = refract(d, e, c);
}

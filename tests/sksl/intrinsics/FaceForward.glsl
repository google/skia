
out vec4 sk_FragColor;
float a;
float b;
float c;
vec4 d;
vec4 e;
vec4 f;
void main() {
    sk_FragColor.x = faceforward(a, b, c);
    sk_FragColor = faceforward(d, e, f);
}

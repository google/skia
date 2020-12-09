
out vec4 sk_FragColor;
mat4 a;
void main() {
    sk_FragColor = inverse(a)[0];
}

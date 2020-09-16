
out vec4 sk_FragColor;
uniform float x;
uniform float y;
void main() {
    float z = pow(x + 1.0, y + 2.0);
    sk_FragColor = vec4(z);
}

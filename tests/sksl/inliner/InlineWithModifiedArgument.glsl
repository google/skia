
out vec4 sk_FragColor;
float parameterWrite(float x) {
    x *= 2.0;
    return x;
}
void main() {
    sk_FragColor.x = parameterWrite(1.0);
}


out vec4 sk_FragColor;
void main() {
    float b = 2.0;

    float d = 3.0;
    b++;
    d++;
    sk_FragColor = vec4(b, b, d, d);
}

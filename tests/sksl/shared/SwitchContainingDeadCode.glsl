
out vec4 sk_FragColor;
void main() {
    float x;
    {
        x = 1.0;
        x = 2.0;
    }
    sk_FragColor = vec4(x);
}

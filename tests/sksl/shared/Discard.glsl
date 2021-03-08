
out vec4 sk_FragColor;
void main() {
    float x;
    {
        x = 1.0;
        discard;
    }
    sk_FragColor = vec4(x);
}

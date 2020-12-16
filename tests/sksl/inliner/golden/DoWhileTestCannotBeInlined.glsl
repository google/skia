
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(0.0);
    for (; ; ) {
        {
            sk_FragColor += vec4(0.125);
        }
        bool _0_shouldLoop;
        _0_shouldLoop = sk_FragColor.x < 0.5;

        if (!_0_shouldLoop) break;

    }
}

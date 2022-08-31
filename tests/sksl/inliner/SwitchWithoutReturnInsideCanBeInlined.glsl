
out vec4 sk_FragColor;
uniform int value;
void main() {
    vec4 _0_result = vec4(1.0);
    switch (value) {
        case 0:
            _0_result = vec4(0.5);
    }
    sk_FragColor = _0_result;
}

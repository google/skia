
out vec4 sk_FragColor;
uniform int value;
void main() {
    vec4 _0_switchy;
    {
        vec4 _1_result = vec4(1.0);
        switch (value) {
            case 0:
                _1_result = vec4(0.5);
        }
        _0_switchy = _1_result;
    }

    sk_FragColor = _0_switchy;

}

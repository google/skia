
out vec4 sk_FragColor;
uniform int value;
void main() {
    vec4 _0_switchy;
    for (int _1_loop = 0;_1_loop < 1; _1_loop++) {
        switch (value) {
            case 0:
                {
                    _0_switchy = vec4(0.5);
                    continue;
                }
        }
        {
            _0_switchy = vec4(1.0);
            continue;
        }
    }
    sk_FragColor = _0_switchy;

}

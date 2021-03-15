
out vec4 sk_FragColor;
vec4 helper();
void main() {
    vec4 _0_helper;
    for (int _1_loop = 0;_1_loop < 1; _1_loop++) {
        int _2_temp = 1;
        switch (_2_temp) {
            case 0:
                {
                    _0_helper = vec4(0.0, 0.0, 0.0, 1.0);
                    continue;
                }
            case 1:
                {
                    _0_helper = vec4(0.5, 0.5, 0.5, 1.0);
                    continue;
                }
            case 2:
                {
                    _0_helper = vec4(1.0);
                    continue;
                }
            default:
                {
                    _0_helper = vec4(1.0, 0.0, 0.0, 1.0);
                    continue;
                }
        }
    }
    sk_FragColor = _0_helper;
}

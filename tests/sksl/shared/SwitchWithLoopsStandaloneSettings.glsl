
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_with_continue_in_loop_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            for (int i = 0;i < 10; ++i) {
                ++val;
                continue;
                ++val;
            }
        default:
            ++val;
    }
    return val == 11;
}
bool loop_with_break_in_switch_bi(int x) {
    int val = 0;
    for (int i = 0;i < 10; ++i) {
        switch (x) {
            case 1:
                ++val;
                break;
            default:
                return false;
        }
        ++val;
    }
    return val == 20;
}
vec4 main() {
    int x = int(colorGreen.y);
    int _0_val = 0;
    switch (x) {
        case 1:
            for (int _1_i = 0;_1_i < 10; ++_1_i) {
                ++_0_val;
                break;
                ++_0_val;
            }
        default:
            ++_0_val;
    }
    return (_0_val == 2 && switch_with_continue_in_loop_bi(x)) && loop_with_break_in_switch_bi(x) ? colorGreen : colorRed;
}

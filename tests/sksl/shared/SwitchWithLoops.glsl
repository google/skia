#version 400
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_with_continue_in_loop_bi(int x) {
    int val = 0;
    int _tmpSwitchValue1 = x, _tmpSwitchFallthrough0 = 0;
    for (int _tmpSwitchLoop2 = 0; _tmpSwitchLoop2 < 1; _tmpSwitchLoop2++) {
        if ((_tmpSwitchValue1 == 1)) {
            for (int i = 0;i < 10; ++i) {
                ++val;
                continue;
            }
            _tmpSwitchFallthrough0 = 1;
        }
        ++val;
    }
    return val == 11;
}
bool loop_with_break_in_switch_bi(int x) {
    int val = 0;
    for (int i = 0;i < 10; ++i) {
        int _tmpSwitchValue4 = x, _tmpSwitchFallthrough3 = 0;
        for (int _tmpSwitchLoop5 = 0; _tmpSwitchLoop5 < 1; _tmpSwitchLoop5++) {
            if ((_tmpSwitchValue4 == 1)) {
                ++val;
                break;
                _tmpSwitchFallthrough3 = 1;
            }
            return false;
        }
        ++val;
    }
    return val == 20;
}
vec4 main() {
    int x = int(colorGreen.y);
    int _0_val = 0;
    int _tmpSwitchValue7 = x, _tmpSwitchFallthrough6 = 0;
    for (int _tmpSwitchLoop8 = 0; _tmpSwitchLoop8 < 1; _tmpSwitchLoop8++) {
        if ((_tmpSwitchValue7 == 1)) {
            for (int _1_i = 0;_1_i < 10; ++_1_i) {
                ++_0_val;
                break;
            }
            _tmpSwitchFallthrough6 = 1;
        }
        ++_0_val;
    }
    return (_0_val == 2 && switch_with_continue_in_loop_bi(x)) && loop_with_break_in_switch_bi(x) ? colorGreen : colorRed;
}

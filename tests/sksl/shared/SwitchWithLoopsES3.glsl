#version 400
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_with_continue_in_while_loop_bi(int x) {
    int val = 0;
    int i = 0;
    int _tmpSwitchValue1 = x, _tmpSwitchFallthrough0 = 0;
    for (int _tmpSwitchLoop2 = 0; _tmpSwitchLoop2 < 1; _tmpSwitchLoop2++) {
        if ((_tmpSwitchValue1 == 1)) {
            while (i < 10) {
                ++i;
                ++val;
                continue;
            }
            _tmpSwitchFallthrough0 = 1;
        }
        ++val;
    }
    return val == 11;
}
bool while_loop_with_break_in_switch_bi(int x) {
    int val = 0;
    int i = 0;
    while (i < 10) {
        ++i;
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
bool switch_with_break_in_do_while_loop_bi(int x) {
    int val = 0;
    int i = 0;
    int _tmpSwitchValue7 = x, _tmpSwitchFallthrough6 = 0;
    for (int _tmpSwitchLoop8 = 0; _tmpSwitchLoop8 < 1; _tmpSwitchLoop8++) {
        if ((_tmpSwitchValue7 == 1)) {
            do {
                ++i;
                ++val;
                break;
            } while (i < 10);
            _tmpSwitchFallthrough6 = 1;
        }
        ++val;
    }
    return val == 2;
}
bool switch_with_continue_in_do_while_loop_bi(int x) {
    int val = 0;
    int i = 0;
    int _tmpSwitchValue10 = x, _tmpSwitchFallthrough9 = 0;
    for (int _tmpSwitchLoop11 = 0; _tmpSwitchLoop11 < 1; _tmpSwitchLoop11++) {
        if ((_tmpSwitchValue10 == 1)) {
            do {
                ++i;
                ++val;
                continue;
            } while (i < 10);
            _tmpSwitchFallthrough9 = 1;
        }
        ++val;
    }
    return val == 11;
}
bool do_while_loop_with_break_in_switch_bi(int x) {
    int val = 0;
    int i = 0;
    do {
        ++i;
        int _tmpSwitchValue13 = x, _tmpSwitchFallthrough12 = 0;
        for (int _tmpSwitchLoop14 = 0; _tmpSwitchLoop14 < 1; _tmpSwitchLoop14++) {
            if ((_tmpSwitchValue13 == 1)) {
                ++val;
                break;
                _tmpSwitchFallthrough12 = 1;
            }
            return false;
        }
        ++val;
    } while (i < 10);
    return val == 20;
}
vec4 main() {
    int x = int(colorGreen.y);
    int _0_val = 0;
    int _1_i = 0;
    int _tmpSwitchValue16 = x, _tmpSwitchFallthrough15 = 0;
    for (int _tmpSwitchLoop17 = 0; _tmpSwitchLoop17 < 1; _tmpSwitchLoop17++) {
        if ((_tmpSwitchValue16 == 1)) {
            while (_1_i < 10) {
                ++_1_i;
                ++_0_val;
                break;
            }
            _tmpSwitchFallthrough15 = 1;
        }
        ++_0_val;
    }
    return ((((_0_val == 2 && switch_with_continue_in_while_loop_bi(x)) && while_loop_with_break_in_switch_bi(x)) && switch_with_break_in_do_while_loop_bi(x)) && switch_with_continue_in_do_while_loop_bi(x)) && do_while_loop_with_break_in_switch_bi(x) ? colorGreen : colorRed;
}

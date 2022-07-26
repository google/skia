
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_with_continue_in_while_loop_bi(int x) {
    int val = 0;
    int i = 0;
    switch (x) {
        case 1:
            while (i < 10) {
                ++i;
                ++val;
                continue;
            }
        default:
            ++val;
    }
    return val == 11;
}
bool while_loop_with_break_in_switch_bi(int x) {
    int val = 0;
    int i = 0;
    while (i < 10) {
        ++i;
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
bool switch_with_break_in_do_while_loop_bi(int x) {
    int val = 0;
    int i = 0;
    switch (x) {
        case 1:
            do {
                ++i;
                ++val;
                break;
            } while (i < 10);
        default:
            ++val;
    }
    return val == 2;
}
bool switch_with_continue_in_do_while_loop_bi(int x) {
    int val = 0;
    int i = 0;
    switch (x) {
        case 1:
            do {
                ++i;
                ++val;
                continue;
            } while (i < 10);
        default:
            ++val;
    }
    return val == 11;
}
bool do_while_loop_with_break_in_switch_bi(int x) {
    int val = 0;
    int i = 0;
    do {
        ++i;
        switch (x) {
            case 1:
                ++val;
                break;
            default:
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
    switch (x) {
        case 1:
            while (_1_i < 10) {
                ++_1_i;
                ++_0_val;
                break;
            }
        default:
            ++_0_val;
    }
    return ((((_0_val == 2 && switch_with_continue_in_while_loop_bi(x)) && while_loop_with_break_in_switch_bi(x)) && switch_with_break_in_do_while_loop_bi(x)) && switch_with_continue_in_do_while_loop_bi(x)) && do_while_loop_with_break_in_switch_bi(x) ? colorGreen : colorRed;
}

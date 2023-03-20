
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool return_in_one_case_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            ++val;
            return false;
        default:
            ++val;
    }
    return val == 1;
}
bool return_in_default_bi(int x) {
    switch (x) {
        case 0:
        default:
            return true;
    }
}
bool return_in_every_case_bi(int x) {
    switch (x) {
        case 1:
            return false;
        default:
            return true;
    }
}
bool return_in_every_case_no_default_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            return false;
        case 2:
            return true;
    }
    ++val;
    return val == 1;
}
bool case_has_break_before_return_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            break;
        case 2:
            return true;
        default:
            return true;
    }
    ++val;
    return val == 1;
}
bool case_has_break_after_return_bi(int x) {
    switch (x) {
        case 1:
            return false;
        case 2:
            return true;
        default:
            return true;
    }
}
bool no_return_in_default_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            return false;
        case 2:
            return true;
        default:
            break;
    }
    ++val;
    return val == 1;
}
bool empty_default_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            return false;
        case 2:
            return true;
        default:
            break;
    }
    ++val;
    return val == 1;
}
bool return_with_fallthrough_bi(int x) {
    switch (x) {
        case 1:
        case 2:
            return true;
        default:
            return false;
    }
}
bool fallthrough_ends_in_break_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
        case 2:
            break;
        default:
            return false;
    }
    ++val;
    return val == 1;
}
bool fallthrough_to_default_with_break_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
        case 2:
        default:
            break;
    }
    ++val;
    return val == 1;
}
bool fallthrough_to_default_with_return_bi(int x) {
    switch (x) {
        case 1:
        case 2:
        default:
            return true;
    }
}
bool fallthrough_with_loop_break_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            for (int i = 0;i < 5; ++i) {
                ++val;
                break;
            }
        case 2:
        default:
            return true;
    }
}
bool fallthrough_with_loop_continue_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            for (int i = 0;i < 5; ++i) {
                ++val;
                continue;
            }
        case 2:
        default:
            return true;
    }
}
vec4 main() {
    int x = int(colorGreen.y);
    return ((((((((((((return_in_one_case_bi(x) && return_in_default_bi(x)) && return_in_every_case_bi(x)) && return_in_every_case_no_default_bi(x)) && case_has_break_before_return_bi(x)) && case_has_break_after_return_bi(x)) && no_return_in_default_bi(x)) && empty_default_bi(x)) && return_with_fallthrough_bi(x)) && fallthrough_ends_in_break_bi(x)) && fallthrough_to_default_with_break_bi(x)) && fallthrough_to_default_with_return_bi(x)) && fallthrough_with_loop_break_bi(x)) && fallthrough_with_loop_continue_bi(x) ? colorGreen : colorRed;
}


out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float unknownInput;
bool for_inside_body() {
    for (int x = 0;x <= 10; ++x) {
        return true;
    }
}
bool after_for_body() {
    for (int x = 0;x <= 10; ++x) {
    }
    return true;
}
bool for_with_double_sided_conditional_return() {
    for (int x = 0;x <= 10; ++x) {
        if (unknownInput == 1.0) return true; else return true;
    }
}
bool switch_with_all_returns() {
    switch (int(unknownInput)) {
        case 1:
            return true;
        case 2:
            return true;
        default:
            return true;
    }
}
bool switch_only_default() {
    switch (int(unknownInput)) {
        default:
            return true;
    }
}
bool switch_fallthrough() {
    switch (int(unknownInput)) {
        case 1:
            return true;
        case 2:
        default:
            return true;
    }
}
bool switch_fallthrough_twice() {
    switch (int(unknownInput)) {
        case 1:
        case 2:
        default:
            return true;
    }
}
bool switch_with_break_in_loop() {
    switch (int(unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                break;
            }
        default:
            return true;
    }
}
bool switch_with_continue_in_loop() {
    switch (int(unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                continue;
            }
        default:
            return true;
    }
}
bool switch_with_if_that_returns() {
    switch (int(unknownInput)) {
        case 1:
            if (unknownInput == 123.0) return true; else return true;
        default:
            return true;
    }
}
vec4 main() {
    bool _0_return_on_both_sides;
    if (unknownInput == 1.0) _0_return_on_both_sides = true; else _0_return_on_both_sides = true;
    return (((((((((_0_return_on_both_sides && for_inside_body()) && after_for_body()) && for_with_double_sided_conditional_return()) && switch_with_all_returns()) && switch_only_default()) && switch_fallthrough()) && switch_fallthrough_twice()) && switch_with_break_in_loop()) && switch_with_continue_in_loop()) && switch_with_if_that_returns() ? colorGreen : colorRed;


}

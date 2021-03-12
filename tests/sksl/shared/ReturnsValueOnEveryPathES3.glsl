
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float unknownInput;
bool return_on_both_sides() {
    if (unknownInput == 1.0) return true; else return true;
}
bool for_inside_body() {
    for (int x = 0;x <= 10; ++x) {
        return true;
    }
}
bool after_for_body() {
    for (int x = 0;x <= 10; ++x) {
        true;
    }
    return true;
}
bool for_with_double_sided_conditional_return() {
    for (int x = 0;x <= 10; ++x) {
        if (unknownInput == 1.0) return true; else return true;
    }
}
bool if_else_chain() {
    if (unknownInput == 1.0) return true; else if (unknownInput == 2.0) return false; else if (unknownInput == 3.0) return true; else if (unknownInput == 4.0) return false; else return true;
}
bool conditional_inside_while_loop() {
    while (unknownInput == 123.0) {
        return true;
    }
}
bool inside_do_loop() {
    do {
        return true;
    } while (true);
}
bool inside_while_loop() {
    while (true) {
        return true;
    }
}
bool after_do_loop() {
    do {
        break;
    } while (true);
    return true;
}
bool after_while_loop() {
    while (true) {
        break;
    }
    return true;
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
bool switch_with_one_sided_if_then_fallthrough() {
    switch (int(unknownInput)) {
        case 1:
            if (unknownInput == 123.0) return true;
        default:
            return true;
    }
}
vec4 main() {
    return (((((((((((((((((true && return_on_both_sides()) && for_inside_body()) && after_for_body()) && for_with_double_sided_conditional_return()) && if_else_chain()) && conditional_inside_while_loop()) && inside_do_loop()) && inside_while_loop()) && after_do_loop()) && after_while_loop()) && switch_with_all_returns()) && switch_only_default()) && switch_fallthrough()) && switch_fallthrough_twice()) && switch_with_break_in_loop()) && switch_with_continue_in_loop()) && switch_with_if_that_returns()) && switch_with_one_sided_if_then_fallthrough() ? colorGreen : colorRed;
}

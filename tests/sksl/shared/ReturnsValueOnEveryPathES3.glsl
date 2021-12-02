
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float unknownInput;
bool inside_while_loop_b() {
    while (unknownInput == 123.0) {
        return false;
    }
    return true;
}
bool inside_infinite_do_loop_b() {
    do {
        return true;
    } while (true);
}
bool inside_infinite_while_loop_b() {
    while (true) {
        return true;
    }
}
bool after_do_loop_b() {
    do {
        break;
    } while (true);
    return true;
}
bool after_while_loop_b() {
    while (true) {
        break;
    }
    return true;
}
bool switch_with_all_returns_b() {
    switch (int(unknownInput)) {
        case 1:
            return true;
        case 2:
            return false;
        default:
            return false;
    }
}
bool switch_fallthrough_b() {
    switch (int(unknownInput)) {
        case 1:
            return true;
        case 2:
        default:
            return false;
    }
}
bool switch_fallthrough_twice_b() {
    switch (int(unknownInput)) {
        case 1:
        case 2:
        default:
            return true;
    }
}
bool switch_with_break_in_loop_b() {
    switch (int(unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                break;
            }
        default:
            return true;
    }
}
bool switch_with_continue_in_loop_b() {
    switch (int(unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                continue;
            }
        default:
            return true;
    }
}
bool switch_with_if_that_returns_b() {
    switch (int(unknownInput)) {
        case 1:
            if (unknownInput == 123.0) return false; else return true;
        default:
            return true;
    }
}
bool switch_with_one_sided_if_then_fallthrough_b() {
    switch (int(unknownInput)) {
        case 1:
            if (unknownInput == 123.0) return false;
        default:
            return true;
    }
}
vec4 main() {
    return ((((((((((inside_while_loop_b() && inside_infinite_do_loop_b()) && inside_infinite_while_loop_b()) && after_do_loop_b()) && after_while_loop_b()) && switch_with_all_returns_b()) && switch_fallthrough_b()) && switch_fallthrough_twice_b()) && switch_with_break_in_loop_b()) && switch_with_continue_in_loop_b()) && switch_with_if_that_returns_b()) && switch_with_one_sided_if_then_fallthrough_b() ? colorGreen : colorRed;
}

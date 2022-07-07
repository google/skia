
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
bool return_in_some_cases_bi(int x) {
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
    int val = 0;
    switch (x) {
        case 1:
        case 2:
            return true;
        default:
            break;
    }
    ++val;
    return val == 1;
}
vec4 main() {
    int x = int(colorGreen.y);
    return ((((return_in_one_case_bi(x) && return_in_default_bi(x)) && return_in_every_case_bi(x)) && return_in_every_case_no_default_bi(x)) && return_in_some_cases_bi(x)) && return_with_fallthrough_bi(x) ? colorGreen : colorRed;
}

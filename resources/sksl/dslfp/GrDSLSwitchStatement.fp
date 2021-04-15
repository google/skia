half4 main() {
    half4 color = half4(0);

    switch (int(color.r)) {
        case 0: ++color.a; // fallthrough
        case 1: break;
        case 2: return half4(0);
        case 3: // fallthrough
        case 4: ++color.r; // fallthrough
        case 5:  { ++color.b; break; }
        default: { --color.g; } break;
    }

    switch (int(color.r)) {
        case 1: break;
        case 0: color.r = color.b = 1;
    }

    @switch (10) {
        case 0:  color.r = 1; break;
        case 10: color.g = 1; break;
        case 20: color.b = 1; break;
    }

    return color;
}

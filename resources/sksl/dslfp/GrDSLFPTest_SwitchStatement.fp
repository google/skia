half4 main() {
    half4 color = half4(0);

    switch (int(color.r)) { // will take case 0
        case 0: ++color.g; // fallthrough
        case 1: break;
        case 2: return half4(0);
        case 3: // fallthrough
        case 4: ++color.r; // fallthrough
        case 5:  { ++color.b; } break;
        default: { --color.g; break; }
    }

    switch (int(color.g)) { // will take case 1
        case 1: break;
        case 0: { color.r = 1; color.b = 1; }
    }

    @switch (10) {
        case 0:  color.r = color.g; break;
        case 20: color.b = color.g; break;
        case 10: color.a = color.g; break;
    }

    return color;
}

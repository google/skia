layout(key) in int ten; // always equals 10

half4 main() {
    int4 color = int4(0);

    switch (color.r) { // will take case 0
        case 0: ++color.g; // fallthrough
        case 1: break;
        case 2: return half4(0);
        case 3: // fallthrough
        case 4: ++color.r; // fallthrough
        case 5:  { ++color.b; } break;
        default: { --color.g; break; }
    }

    switch (color.g) { // will take case 1
        case 1: break;
        case 0: { color.r = 1; color.b = 1; }
    }

    @switch (ten) {
        case 0:  color.r = color.g; break;
        case 20: color.b = color.g; break;
        case 10: color.a = color.g; break;
    }

    return half4(color);
}

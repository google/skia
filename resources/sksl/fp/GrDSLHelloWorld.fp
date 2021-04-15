/* HELLO WORLD */

half4 main() {
    half h;
    h = 123;
    half4 h4 = half4(1,2,3,h);

    h4 *= half4(2);
    h4 *= half4(0.5, 0.5, 0.5, 0.5);

    if (h > h4.x) {
        h4.y += 1;
    } else if (h > h4.y) {
        h4.y /= h4.z;
    } else {
        h4.y -= h4.w;
    }

    if (h > h4.x)
        h4.y += 1;
    else if (h > h4.y)
        h4.y /= h4.z;
    else
        h4.y -= h4.w;

    for (int x=0; x<5; ++x) {
        h4.z *= h;
    }

    int y = 1;
    while (y < 5) ++h4.x, y += 1;

    do { h4.y--;; y++; continue; } while (y < 10);

    switch (y) {
        case 5: { return h4; }
        case 9: // fallthrough
        case 10: ++y; // fallthrough
        case 11: --y; break;
        default: ++y;
    }

    return half4(1);
}


out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    float huge = 1000000000.0 * (1000000000.0 * (1000000000.0 * (1000000000.0 * (1000000000.0 * (1000000000.0 * (1000000000.0 * (1000000000.0 * (1000000000.0 * (1000000000.0 * 9.9999996169031625e+35)))))))));
    int hugeI = int(16384 * (2 * (2 * (2 * (2 * (2 * (2 * 1073741824)))))));
    uint hugeU = uint(16384 * (2 * (2 * (2 * (2 * (2 * (2 * 1073741824)))))));
    int hugeS = 8192 * 262144;
    uint hugeUS = 8192u * 0u;
    int hugeNI = int(-16384 * (2 * (2 * (2 * (2 * (2 * (2 * 1073741824)))))));
    int hugeNS = -8192 * 262144;
    return colorGreen * (clamp(huge, 0.0, 1.0) * (clamp(float(hugeI), 0.0, 1.0) * (clamp(float(hugeU), 0.0, 1.0) * (clamp(float(hugeS), 0.0, 1.0) * (clamp(float(hugeUS), 0.0, 1.0) * (clamp(float(hugeNI), 0.0, 1.0) * clamp(float(hugeNS), 0.0, 1.0)))))));
}

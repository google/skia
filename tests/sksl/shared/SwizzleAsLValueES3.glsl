
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
int gAccessCount = 0;
int Z_i() {
    ++gAccessCount;
    return 0;
}
vec4 main() {
    vec4 array[1];
    array[Z_i()] = colorGreen * 0.5;
    array[Z_i()].w = 2.0;
    array[Z_i()].y *= 4.0;
    array[Z_i()].yzw *= mat3(0.5);
    array[Z_i()].zywx += vec4(0.25, 0.0, 0.0, 0.75);
    array[Z_i()].x += array[Z_i()].w <= 1.0 ? array[Z_i()].z : float(Z_i());
    return gAccessCount == 8 && array[0] == vec4(1.0, 1.0, 0.25, 1.0) ? colorGreen : colorRed;
}

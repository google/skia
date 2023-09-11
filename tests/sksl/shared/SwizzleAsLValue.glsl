
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 scalar;
    vec4 array[1];
    scalar = colorGreen * 0.5;
    scalar.w = 2.0;
    scalar.y *= 4.0;
    scalar.yzw *= mat3(0.5);
    scalar.zywx += vec4(0.25, 0.0, 0.0, 0.75);
    scalar.x += scalar.w <= 1.0 ? scalar.z : 0.0;
    array[0] = colorGreen * 0.5;
    array[0].w = 2.0;
    array[0].y *= 4.0;
    array[0].yzw *= mat3(0.5);
    array[0].zywx += vec4(0.25, 0.0, 0.0, 0.75);
    array[0].x += array[0].w <= 1.0 ? array[0].z : 0.0;
    return scalar == vec4(1.0, 1.0, 0.25, 1.0) && array[0] == vec4(1.0, 1.0, 0.25, 1.0) ? colorGreen : colorRed;
}

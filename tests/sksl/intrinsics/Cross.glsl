
out vec4 sk_FragColor;
uniform mat3 testMatrix3x3;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec3 expected1 = vec3(-3.0, 6.0, -3.0);
    const vec3 expected2 = vec3(6.0, -12.0, 6.0);
    return cross(testMatrix3x3[0], testMatrix3x3[1]) == expected1 && cross(testMatrix3x3[2], testMatrix3x3[0]) == expected2 ? colorGreen : colorRed;
}

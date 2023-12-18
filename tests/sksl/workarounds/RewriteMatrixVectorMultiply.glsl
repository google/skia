#version 400
out vec4 sk_FragColor;
uniform mat4 testMatrix4x4;
uniform vec4 testValues;
vec4 main() {
    mat4 m44 = mat4(123.0);
    vec4 v4 = vec4(0.0, 1.0, 2.0, 3.0);
    return m44 * v4 + testMatrix4x4 * testValues;
}

#version 400
out vec4 sk_FragColor;
vec4 main() {
    mat4 m44 = mat4(123.0);
    vec4 v4 = vec4(0.0, 1.0, 2.0, 3.0);
    return ((m44[0] * v4.x + m44[1] * v4.y) + m44[2] * v4.z) + m44[3] * v4.w;
}

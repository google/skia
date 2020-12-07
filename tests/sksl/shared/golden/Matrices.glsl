
out vec4 sk_FragColor;
void main() {
    mat3x4 z = mat2x4(1.0) * mat3x2(1.0, 0.0, 0.0, 1.0, 2.0, 2.0);
    vec3 v1 = mat3(1.0) * vec3(2.0);
    vec3 v2 = vec3(2.0) * mat3(1.0);
    sk_FragColor = vec4(z[0].x, v1 + v2);
    mat2 m5 = mat2(mat2(1.0, 2.0, 3.0, 4.0)[0].x);
    sk_FragColor = vec4((((((((((mat2(1.0, 2.0, 3.0, 4.0)[0].x + mat2(vec4(0.0))[0].x) + mat2(1.0, 2.0, 3.0, 4.0)[0].x) + mat2(1.0)[0].x) + m5[0].x) + mat2(1.0, 2.0, 3.0, 4.0)[0].x) + mat2(5.0, 6.0, 7.0, 8.0)[0].x) + mat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0)[0].x) + mat3(1.0)[0].x) + mat4(1.0)[0].x) + mat4(2.0)[0].x);
}

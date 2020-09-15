
out vec4 sk_FragColor;
void main() {
    mat3x4 z = mat2x4(1.0) * mat3x2(1.0, 0.0, 0.0, 1.0, vec2(2.0, 2.0));
    vec3 v1 = mat3(1.0) * vec3(2.0);
    vec3 v2 = vec3(2.0) * mat3(1.0);
    sk_FragColor = vec4(z[0].x, v1 + v2);
}

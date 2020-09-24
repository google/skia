
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(1.0, 2.0, 3.0, 4.0);
    sk_FragColor = vec4(mat2(2.0)[0][0], mat3(3.0)[0][0], mat4(4.0)[0][0], 1.0);
    sk_FragColor = vec4(1.0, 2.0, 3.0, 4.0);
    sk_FragColor = vec4(1.0, 2.0, 3.0, 4.0);
    sk_FragColor = vec4(mat2(2.0)[0][0], mat3(3.0)[0][0], mat4(4.0)[0][0], 1.0);
    sk_FragColor = vec4(1.0, bvec2(false).x ? 1.0 : 0.0, bvec3(true).x ? 1.0 : 0.0, bvec4(false).x ? 1.0 : 0.0);
}


out vec4 sk_FragColor;
void main() {
    sk_FragColor.x = mat2(mat3(1.0))[0].x;
    sk_FragColor.x = mat2(mat4(1.0))[0].x;
    sk_FragColor.x = mat3(mat4(1.0))[0].x;
    sk_FragColor.x = mat3(mat2(1.0))[0].x;
    sk_FragColor.x = mat3(mat2x3(1.0))[0].x;
    sk_FragColor.x = mat3(mat3x2(1.0))[0].x;
    sk_FragColor.x = mat4(mat3(mat2(1.0)))[0].x;
    sk_FragColor.x = mat4(mat4x3(mat4x2(1.0)))[0].x;
    sk_FragColor.x = mat4(mat3x4(mat2x4(1.0)))[0].x;
    sk_FragColor.x = mat2x4(mat4x2(1.0))[0].x;
    sk_FragColor.x = mat4x2(mat2x4(1.0))[0].x;
}


out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(6.0, 6.0, 7.0, 8.0);
    sk_FragColor = vec4(7.0, 9.0, 9.0, 9.0);
    sk_FragColor = vec4(9.0, 9.0, 10.0, 10.0);
    sk_FragColor.xyz = vec3(6.0, 6.0, 6.0);
    sk_FragColor.xy = vec2(3.0, 3.0);
    sk_FragColor.x = 6.0;
    sk_FragColor = vec4(6.0, 6.0, 7.0, 8.0);
    sk_FragColor = vec4(-7.0, -9.0, -9.0, -9.0);
    sk_FragColor = vec4(9.0, 9.0, 10.0, 10.0);
    sk_FragColor.xyz = vec3(6.0, 6.0, 6.0);
    sk_FragColor.xy = vec2(8.0, 8.0);
    sk_FragColor = vec4(2.0, 1.0, 0.5, 0.25);

    ivec4 _0_result;
    _0_result = ivec4(6, 6, 7, 8);
    _0_result = ivec4(7, 9, 9, 9);
    _0_result = ivec4(9, 9, 10, 10);
    _0_result.xyz = ivec3(6, 6, 6);
    _0_result.xy = ivec2(3, 3);
    _0_result.x = 6;
    _0_result = ivec4(6, 6, 7, 8);
    _0_result = ivec4(-7, -9, -9, -9);
    _0_result = ivec4(9, 9, 10, 10);
    _0_result.xyz = ivec3(6, 6, 6);
    _0_result.xy = ivec2(1, 1);
    _0_result.xyz = ivec3(2, 1, 0);
    sk_FragColor = vec4(_0_result);

}

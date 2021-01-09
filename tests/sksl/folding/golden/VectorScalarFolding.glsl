
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(6.0, 6.0, 7.0, 8.0);
    sk_FragColor = vec4(7.0, 9.0, 9.0, 9.0);
    sk_FragColor = vec4(9.0, 9.0, 10.0, 10.0);
    sk_FragColor.xyz = vec3(6.0, 6.0, 6.0);
    sk_FragColor.xy = vec2(3.0, 3.0);
    sk_FragColor.x = 6.0;
    sk_FragColor = 5.0 + vec4(vec2(1.0), 2.0, 3.0);
    sk_FragColor = 1.0 - vec4(8.0, vec3(10.0));
    sk_FragColor = 1.0 + vec4(vec2(8.0), vec2(9.0));
    sk_FragColor.xyz = 3.0 * vec3(2.0);
    sk_FragColor.xy = 4.0 / vec2(12.0);
    sk_FragColor.x = (2.0 / vec4(12.0)).y;

    ivec4 _0_result;
    _0_result = ivec4(6, 6, 7, 8);
    _0_result = ivec4(7, 9, 9, 9);
    _0_result = ivec4(9, 9, 10, 10);
    _0_result.xyz = ivec3(6, 6, 6);
    _0_result.xy = ivec2(3, 3);
    _0_result.x = 6;
    _0_result = 5 + ivec4(ivec2(1), 2, 3);
    _0_result = 1 - ivec4(8, ivec3(10));
    _0_result = 1 + ivec4(ivec2(8), ivec2(9));
    _0_result.xyz = 3 * ivec3(2);
    _0_result.xy = 4 / ivec2(12);
    _0_result.x = (2 / ivec4(12)).y;
    sk_FragColor = vec4(_0_result);

}

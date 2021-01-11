
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(vec2(1.0), 2.0, 3.0) + 5.0;
    sk_FragColor = vec4(8.0, vec3(10.0)) - 1.0;
    sk_FragColor = vec4(vec2(8.0), vec2(9.0)) + 1.0;
    sk_FragColor.xyz = vec3(2.0) * 3.0;
    sk_FragColor.xy = vec2(12.0) / 4.0;
    sk_FragColor.x = (vec4(12.0) / 2.0).y;
    sk_FragColor = 5.0 + vec4(vec2(1.0), 2.0, 3.0);
    sk_FragColor = 1.0 - vec4(8.0, vec3(10.0));
    sk_FragColor = 1.0 + vec4(vec2(8.0), vec2(9.0));
    sk_FragColor.xyz = 3.0 * vec3(2.0);
    sk_FragColor.xy = 4.0 / vec2(0.5);
    sk_FragColor = 20.0 / vec4(10.0, 20.0, 40.0, 80.0);

    ivec4 _0_result;
    _0_result = ivec4(ivec2(1), 2, 3) + 5;
    _0_result = ivec4(8, ivec3(10)) - 1;
    _0_result = ivec4(ivec2(8), ivec2(9)) + 1;
    _0_result.xyz = ivec3(2) * 3;
    _0_result.xy = ivec2(12) / 4;
    _0_result.x = (ivec4(12) / 2).y;
    _0_result = 5 + ivec4(ivec2(1), 2, 3);
    _0_result = 1 - ivec4(8, ivec3(10));
    _0_result = 1 + ivec4(ivec2(8), ivec2(9));
    _0_result.xyz = 3 * ivec3(2);
    _0_result.xy = 4 / ivec2(4);
    _0_result.xyz = 20 / ivec3(10, 20, 40);
    sk_FragColor = vec4(_0_result);

}

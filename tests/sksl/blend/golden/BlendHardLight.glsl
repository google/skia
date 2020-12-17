#version 400
out vec4 sk_FragColor;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
in vec4 src;
in vec4 dst;
void main() {
    vec4 _2_result = vec4(_blend_overlay_component(dst.xw, src.xw), _blend_overlay_component(dst.yw, src.yw), _blend_overlay_component(dst.zw, src.zw), dst.w + (1.0 - dst.w) * src.w);
    _2_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
    sk_FragColor = _2_result;


}

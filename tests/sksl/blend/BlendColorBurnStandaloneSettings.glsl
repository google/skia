
const float sk_PrivGuardedDivideEpsilon = false ? 9.9999999392252903e-09 : 0.0;
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float guarded_divide_Qhhh(float n, float d);
float color_burn_component_Qhh2h2(vec2 s, vec2 d);
vec4 blend_color_burn_h4h4h4(vec4 src, vec4 dst);
float guarded_divide_Qhhh(float n, float d) {
    return n / (d + sk_PrivGuardedDivideEpsilon);
}
float color_burn_component_Qhh2h2(vec2 s, vec2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    } else if (s.x == 0.0) {
        return d.x * (1.0 - s.y);
    } else {
        float delta = max(0.0, d.y - guarded_divide_Qhhh((d.y - d.x) * s.y, s.x));
        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
vec4 blend_color_burn_h4h4h4(vec4 src, vec4 dst) {
    return vec4(color_burn_component_Qhh2h2(src.xw, dst.xw), color_burn_component_Qhh2h2(src.yw, dst.yw), color_burn_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
void main() {
    sk_FragColor = blend_color_burn_h4h4h4(src, dst);
}

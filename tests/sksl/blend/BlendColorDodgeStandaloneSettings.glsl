
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float guarded_divide_Qhhh(float n, float d);
float color_dodge_component_Qhh2h2(vec2 s, vec2 d);
float guarded_divide_Qhhh(float n, float d) {
    return n / (d + sk_PrivkGuardedDivideEpsilon);
}
float color_dodge_component_Qhh2h2(vec2 s, vec2 d) {
    float dxScale = float(d.x == 0.0 ? 0 : 1);
    float delta = dxScale * min(d.y, abs(s.y - s.x) >= sk_PrivkMinNormalHalf ? guarded_divide_Qhhh(d.x * s.y, s.y - s.x) : d.y);
    return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
}
void main() {
    sk_FragColor = vec4(color_dodge_component_Qhh2h2(src.xw, dst.xw), color_dodge_component_Qhh2h2(src.yw, dst.yw), color_dodge_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}

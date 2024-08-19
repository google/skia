
const float sk_PrivkGuardedDivideEpsilon = true ? 1e-08 : 0.0;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float guarded_divide_Qhhh(float n, float d);
float color_burn_component_Qhh2h2(vec2 s, vec2 d);
float color_dodge_component_Qhh2h2(vec2 s, vec2 d);
float soft_light_component_Qhh2h2(vec2 s, vec2 d);
float guarded_divide_Qhhh(float n, float d) {
    return n / (d + sk_PrivkGuardedDivideEpsilon);
}
float color_burn_component_Qhh2h2(vec2 s, vec2 d) {
    float dyTerm = d.y == d.x ? d.y : 0.0;
    float delta = abs(s.x) >= sk_PrivkMinNormalHalf ? d.y - min(d.y, guarded_divide_Qhhh((d.y - d.x) * s.y, s.x)) : dyTerm;
    return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
}
float color_dodge_component_Qhh2h2(vec2 s, vec2 d) {
    float dxScale = float(d.x == 0.0 ? 0 : 1);
    float delta = dxScale * min(d.y, abs(s.y - s.x) >= sk_PrivkMinNormalHalf ? guarded_divide_Qhhh(d.x * s.y, s.y - s.x) : d.y);
    return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
}
float soft_light_component_Qhh2h2(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        return (((d.x * d.x) * (s.y - 2.0 * s.x)) / (d.y + sk_PrivkGuardedDivideEpsilon) + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);
    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        return (((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x) / (DaSqd + sk_PrivkGuardedDivideEpsilon);
    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
void main() {
    sk_FragColor = vec4(color_dodge_component_Qhh2h2(src.xw, dst.xw), color_dodge_component_Qhh2h2(src.yw, dst.yw), color_dodge_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    sk_FragColor = vec4(color_burn_component_Qhh2h2(src.xw, dst.xw), color_burn_component_Qhh2h2(src.yw, dst.yw), color_burn_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    sk_FragColor = dst.w == 0.0 ? src : vec4(soft_light_component_Qhh2h2(src.xw, dst.xw), soft_light_component_Qhh2h2(src.yw, dst.yw), soft_light_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}


const float sk_PrivGuardedDivideEpsilon = false ? 9.9999999392252903e-09 : 0.0;
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float guarded_divide_Qhhh(float n, float d);
float color_burn_component_Qhh2h2(vec2 s, vec2 d);
float color_dodge_component_Qhh2h2(vec2 s, vec2 d);
float soft_light_component_Qhh2h2(vec2 s, vec2 d);
vec4 blend_color_burn_h4h4h4(vec4 src, vec4 dst);
vec4 blend_color_dodge_h4h4h4(vec4 src, vec4 dst);
vec4 blend_soft_light_h4h4h4(vec4 src, vec4 dst);
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
float color_dodge_component_Qhh2h2(vec2 s, vec2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            delta = min(d.y, guarded_divide_Qhhh(d.x * s.y, delta));
            return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
    }
}
float soft_light_component_Qhh2h2(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        return (guarded_divide_Qhhh((d.x * d.x) * (s.y - 2.0 * s.x), d.y) + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);
    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        return guarded_divide_Qhhh(((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x, DaSqd);
    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
vec4 blend_color_burn_h4h4h4(vec4 src, vec4 dst) {
    return vec4(color_burn_component_Qhh2h2(src.xw, dst.xw), color_burn_component_Qhh2h2(src.yw, dst.yw), color_burn_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
vec4 blend_color_dodge_h4h4h4(vec4 src, vec4 dst) {
    return vec4(color_dodge_component_Qhh2h2(src.xw, dst.xw), color_dodge_component_Qhh2h2(src.yw, dst.yw), color_dodge_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
vec4 blend_soft_light_h4h4h4(vec4 src, vec4 dst) {
    return dst.w == 0.0 ? src : vec4(soft_light_component_Qhh2h2(src.xw, dst.xw), soft_light_component_Qhh2h2(src.yw, dst.yw), soft_light_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
void main() {
    sk_FragColor = blend_color_dodge_h4h4h4(src, dst);
    sk_FragColor = blend_color_burn_h4h4h4(src, dst);
    sk_FragColor = blend_soft_light_h4h4h4(src, dst);
}

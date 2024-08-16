
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float color_dodge_component_Qhh2h2(vec2 s, vec2 d);
float color_dodge_component_Qhh2h2(vec2 s, vec2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            delta = min(d.y, (d.x * s.y) / (delta + sk_PrivkGuardedDivideEpsilon));
            return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
    }
}
void main() {
    sk_FragColor = vec4(color_dodge_component_Qhh2h2(src.xw, dst.xw), color_dodge_component_Qhh2h2(src.yw, dst.yw), color_dodge_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}


out vec4 sk_FragColor;
layout (set = 0) uniform mat4 colorXform;
layout (binding = 0) uniform sampler2D s;
void main() {
    vec4 tmpColor;
    sk_FragColor = (tmpColor = texture(s, vec2(1.0)) , colorXform != mat4(1.0) ? vec4(clamp((colorXform * vec4(tmpColor.xyz, 1.0)).xyz, 0.0, tmpColor.w), tmpColor.w) : tmpColor);
}

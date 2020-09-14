
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp mat4 colorXform;
uniform sampler2D sampler;
void main() {
    highp vec4 tmpColor;
    sk_FragColor = (tmpColor = texture(sampler, vec2(1.0)) , colorXform != mat4(1.0) ? vec4(clamp((colorXform * vec4(tmpColor.xyz, 1.0)).xyz, 0.0, tmpColor.w), tmpColor.w) : tmpColor);
}

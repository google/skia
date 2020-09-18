#version 310es
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform sampler2D tex;
in highp vec2 texcoord;
in highp ivec2 offset;
void main() {
    highp int scalar = offset.y;
    sk_FragColor = texture(tex, texcoord + vec2(offset * scalar));
}

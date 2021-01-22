#version 400
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform sampler2D tex;
in highp vec2 texcoord;
in mediump ivec2 offset;
void main() {
    mediump int scalar = offset.y;
    sk_FragColor = texture(tex, texcoord + vec2(offset * scalar));
}

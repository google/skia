
precision mediump float;
precision mediump sampler2D;
out highp int id;
void main() {
    id = gl_VertexID;
}

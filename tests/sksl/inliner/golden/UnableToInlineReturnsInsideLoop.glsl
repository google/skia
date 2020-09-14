
precision mediump float;
precision mediump sampler2D;
void cantActuallyInline(inout highp int x) {
    for (; ; ) {
        ++x;
        if (x > 10) return;
    }
}
void main() {
    highp int x = 0;
    cantActuallyInline(x);
}

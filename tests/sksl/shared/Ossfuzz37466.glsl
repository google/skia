
float foo_ff(float v[2]) {
    return v[0] = v[1];
}
void main() {
    float y[2];
    foo_ff(y);
}

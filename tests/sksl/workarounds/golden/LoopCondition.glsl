#version 400
void main() {
    int c = 0;
    for (int i = 0;(i < 4 || c < 10) && true; ++i) {
        c += 1;
    }
}

layout(location=1) out int id;

noinline int fn() {
    return sk_VertexID;
}

void main() {
    id = fn();
}

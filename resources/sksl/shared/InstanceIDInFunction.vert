layout(location=1) out int id;

noinline int fn() {
    return sk_InstanceID;
}

void main() {
    id = fn();
}

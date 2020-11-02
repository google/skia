
out vec4 sk_FragColor;
vec4 this_function_is_defined_before_use() {
    return vec4(1.0);
}
void main() {
    sk_FragColor = this_function_is_defined_before_use();
    sk_FragColor = this_function_is_defined_after_use();
}
vec4 this_function_is_defined_after_use() {
    return vec4(2.0);
}

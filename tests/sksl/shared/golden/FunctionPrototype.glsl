
out vec4 sk_FragColor;
float this_function_is_prototyped_at_the_start_and_never_defined();
vec4 this_function_is_defined_before_use();
vec4 this_function_is_defined_after_use();
vec4 this_function_is_defined_before_use() {
    return vec4(1.0);
}
bool this_function_is_prototyped_in_the_middle_and_never_defined(mat4 a);
void main() {
    sk_FragColor = this_function_is_defined_before_use();
    sk_FragColor = this_function_is_defined_after_use();
}
vec4 this_function_is_defined_after_use() {
    return vec4(2.0);
}
ivec3 this_function_is_prototyped_at_the_very_end_and_never_defined(mat2x3 x, bvec2 y);

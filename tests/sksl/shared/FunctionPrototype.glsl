
out vec4 sk_FragColor;
uniform vec4 colorGreen;
float this_function_is_prototyped_at_the_start_and_never_defined_f();
vec4 this_function_is_defined_before_use_h4h4(vec4 x);
vec4 this_function_is_defined_after_use_h4h4(vec4 x);
vec4 this_function_is_defined_near_the_end_h4h4(vec4 x);
vec4 main();
bool this_function_is_prototyped_in_the_middle_and_never_defined_bf44(mat4 a);
ivec3 this_function_is_prototyped_at_the_very_end_and_never_defined_i3h22b2(mat2 x, bvec2 y);
vec4 this_function_is_defined_before_use_h4h4(vec4 x) {
    return -x;
}
vec4 main() {
    return this_function_is_defined_before_use_h4h4(-colorGreen);
}

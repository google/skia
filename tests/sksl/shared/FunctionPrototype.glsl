
out vec4 sk_FragColor;
uniform vec4 colorGreen;
float this_function_is_prototyped_at_the_start_and_never_defined();
vec4 this_function_is_defined_before_use(vec4 x);
vec4 this_function_is_defined_after_use(vec4 x);
bool this_function_is_prototyped_in_the_middle_and_never_defined(mat4 a);
ivec3 this_function_is_prototyped_at_the_very_end_and_never_defined(mat2 x, bvec2 y);
vec4 main() {
    return colorGreen;
}

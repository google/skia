### Compilation failed:

error: 1: SPIR-V validation error: Uniform OpVariable <id> '6[%arr]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %arr = OpVariable %_ptr_Uniform__arr__arr__arr_float_int_3_int_2_int_1 Uniform

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %arr "arr"
OpName %main "main"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_3 ArrayStride 16
OpDecorate %_arr__arr_float_int_3_int_2 ArrayStride 48
OpDecorate %_arr__arr__arr_float_int_3_int_2_int_1 ArrayStride 96
OpDecorate %arr DescriptorSet 0
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%_arr_float_int_3 = OpTypeArray %float %int_3
%int_2 = OpConstant %int 2
%_arr__arr_float_int_3_int_2 = OpTypeArray %_arr_float_int_3 %int_2
%int_1 = OpConstant %int 1
%_arr__arr__arr_float_int_3_int_2_int_1 = OpTypeArray %_arr__arr_float_int_3_int_2 %int_1
%_ptr_Uniform__arr__arr__arr_float_int_3_int_2_int_1 = OpTypePointer Uniform %_arr__arr__arr_float_int_3_int_2_int_1
%arr = OpVariable %_ptr_Uniform__arr__arr__arr_float_int_3_int_2_int_1 Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%main = OpFunction %void None %17
%18 = OpLabel
OpReturn
OpFunctionEnd

1 error

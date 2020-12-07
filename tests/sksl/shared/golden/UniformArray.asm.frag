### Compilation failed:

error: SPIR-V validation error: Uniform OpVariable <id> '6[%arr]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %arr = OpVariable %_ptr_Uniform__arr_float_int_3 Uniform

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
OpDecorate %arr DescriptorSet 0
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%_arr_float_int_3 = OpTypeArray %float %int_3
%_ptr_Uniform__arr_float_int_3 = OpTypePointer Uniform %_arr_float_int_3
%arr = OpVariable %_ptr_Uniform__arr_float_int_3 Uniform
%void = OpTypeVoid
%13 = OpTypeFunction %void
%main = OpFunction %void None %13
%14 = OpLabel
OpReturn
OpFunctionEnd

1 error

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "arr"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_3 ArrayStride 16
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %6 Binding 0
OpDecorate %6 DescriptorSet 0
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%_arr_float_int_3 = OpTypeArray %float %int_3
%_UniformBuffer = OpTypeStruct %_arr_float_int_3
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%6 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%main = OpFunction %void None %14
%15 = OpLabel
OpReturn
OpFunctionEnd

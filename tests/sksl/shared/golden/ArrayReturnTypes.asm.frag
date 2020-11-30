### Compilation failed:

error: 1: SPIR-V validation error: OpEntryPoint Entry Point <id> '2[%main]'s function return type is not void.
  OpEntryPoint Fragment %main "main" %sk_Clockwise

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %_arr__arr_float_int_2_int_2 ArrayStride 32
OpDecorate %26 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%8 = OpTypeFunction %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_arr__arr_float_int_2_int_2 = OpTypeArray %_arr_float_int_2 %int_2
%_ptr_Function__arr__arr_float_int_2_int_2 = OpTypePointer Function %_arr__arr_float_int_2_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
%main = OpFunction %v4float None %8
%9 = OpLabel
%10 = OpVariable %_ptr_Function__arr__arr_float_int_2_int_2 Function
%27 = OpVariable %_ptr_Function__arr__arr_float_int_2_int_2 Function
%34 = OpVariable %_ptr_Function__arr__arr_float_int_2_int_2 Function
%40 = OpVariable %_ptr_Function__arr__arr_float_int_2_int_2 Function
%18 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
%21 = OpCompositeConstruct %_arr_float_int_2 %float_3 %float_4
%22 = OpCompositeConstruct %_arr__arr_float_int_2_int_2 %18 %21
OpStore %10 %22
%24 = OpAccessChain %_ptr_Function_float %10 %int_0 %int_0
%26 = OpLoad %float %24
%28 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
%29 = OpCompositeConstruct %_arr_float_int_2 %float_3 %float_4
%30 = OpCompositeConstruct %_arr__arr_float_int_2_int_2 %28 %29
OpStore %27 %30
%32 = OpAccessChain %_ptr_Function_float %27 %int_0 %int_1
%33 = OpLoad %float %32
%35 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
%36 = OpCompositeConstruct %_arr_float_int_2 %float_3 %float_4
%37 = OpCompositeConstruct %_arr__arr_float_int_2_int_2 %35 %36
OpStore %34 %37
%38 = OpAccessChain %_ptr_Function_float %34 %int_1 %int_0
%39 = OpLoad %float %38
%41 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
%42 = OpCompositeConstruct %_arr_float_int_2 %float_3 %float_4
%43 = OpCompositeConstruct %_arr__arr_float_int_2_int_2 %41 %42
OpStore %40 %43
%44 = OpAccessChain %_ptr_Function_float %40 %int_1 %int_1
%45 = OpLoad %float %44
%46 = OpCompositeConstruct %v4float %26 %33 %39 %45
OpReturnValue %46
OpFunctionEnd

1 error

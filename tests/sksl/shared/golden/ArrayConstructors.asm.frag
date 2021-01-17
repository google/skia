### Compilation failed:

error: SPIR-V validation error: Expected total number of Constituents to be equal to the number of elements of Result Type array
  %33 = OpCompositeConstruct %_arr_mat4v4float_int_1

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %test1 "test1"
OpName %test2 "test2"
OpName %test3 "test3"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %_arr_v2float_int_2 ArrayStride 16
OpDecorate %_arr_mat4v4float_int_1 ArrayStride 64
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Private__arr_float_int_4 = OpTypePointer Private %_arr_float_int_4
%test1 = OpVariable %_ptr_Private__arr_float_int_4 Private
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v2float = OpTypeVector %float 2
%int_2 = OpConstant %int 2
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Private__arr_v2float_int_2 = OpTypePointer Private %_arr_v2float_int_2
%test2 = OpVariable %_ptr_Private__arr_v2float_int_2 Private
%25 = OpConstantComposite %v2float %float_1 %float_2
%26 = OpConstantComposite %v2float %float_3 %float_4
%mat4v4float = OpTypeMatrix %v4float 4
%int_1 = OpConstant %int 1
%_arr_mat4v4float_int_1 = OpTypeArray %mat4v4float %int_1
%_ptr_Private__arr_mat4v4float_int_1 = OpTypePointer Private %_arr_mat4v4float_int_1
%test3 = OpVariable %_ptr_Private__arr_mat4v4float_int_1 Private
%void = OpTypeVoid
%35 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Private_v2float = OpTypePointer Private %v2float
%_ptr_Private_v4float = OpTypePointer Private %v4float
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %35
%36 = OpLabel
%19 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %test1 %19
%27 = OpCompositeConstruct %_arr_v2float_int_2 %25 %26
OpStore %test2 %27
%33 = OpCompositeConstruct %_arr_mat4v4float_int_1
OpStore %test3 %33
%38 = OpAccessChain %_ptr_Private_float %test1 %int_0
%40 = OpLoad %float %38
%41 = OpAccessChain %_ptr_Private_v2float %test2 %int_0
%43 = OpLoad %v2float %41
%44 = OpCompositeExtract %float %43 0
%45 = OpFAdd %float %40 %44
%46 = OpAccessChain %_ptr_Private_v4float %test3 %int_0 %int_0
%48 = OpLoad %v4float %46
%49 = OpCompositeExtract %float %48 0
%50 = OpFAdd %float %45 %49
%51 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %51 %50
OpReturn
OpFunctionEnd

1 error

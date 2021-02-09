OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %i "i"
OpName %i4 "i4"
OpName %f3x3 "f3x3"
OpName %x "x"
OpName %ai "ai"
OpName %ai4 "ai4"
OpName %ah2x4 "ah2x4"
OpName %af4 "af4"
OpName %l "l"
OpName %r "r"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %57 RelaxedPrecision
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %95 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%31 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%v2float = OpTypeVector %float 2
%55 = OpConstantComposite %v2float %float_0 %float_0
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%_arr_mat3v3float_int_1 = OpTypeArray %mat3v3float %int_1
%_ptr_Function__arr_mat3v3float_int_1 = OpTypePointer Function %_arr_mat3v3float_int_1
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%79 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%l = OpVariable %_ptr_Function_float Function
%r = OpVariable %_ptr_Function_float Function
OpStore %i %int_0
OpStore %i4 %31
%46 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%47 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%48 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%45 = OpCompositeConstruct %mat3v3float %46 %47 %48
OpStore %f3x3 %45
%52 = OpAccessChain %_ptr_Function_float %x %int_3
OpStore %52 %float_0
%56 = OpLoad %v4float %x
%57 = OpVectorShuffle %v4float %56 %55 5 4 2 3
OpStore %x %57
%61 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %61 %int_0
%65 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %65 %31
%70 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%71 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%72 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%69 = OpCompositeConstruct %mat3v3float %70 %71 %72
%73 = OpAccessChain %_ptr_Function_mat3v3float %ah2x4 %int_0
OpStore %73 %69
%77 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%78 = OpAccessChain %_ptr_Function_float %77 %int_0
OpStore %78 %float_0
%80 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%81 = OpLoad %v4float %80
%82 = OpVectorShuffle %v4float %81 %79 6 4 7 5
OpStore %80 %82
OpStore %l %float_0
%85 = OpAccessChain %_ptr_Function_int %ai %int_0
%86 = OpLoad %int %85
%87 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%88 = OpLoad %v4int %87
%89 = OpCompositeExtract %int %88 0
%90 = OpIAdd %int %86 %89
OpStore %85 %90
%91 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%92 = OpLoad %v4float %91
%93 = OpAccessChain %_ptr_Function_v3float %ah2x4 %int_0 %int_0
%95 = OpLoad %v3float %93
%96 = OpCompositeExtract %float %95 0
%97 = OpVectorTimesScalar %v4float %92 %96
OpStore %91 %97
%98 = OpAccessChain %_ptr_Function_int %i4 %int_1
%99 = OpLoad %int %98
%100 = OpLoad %int %i
%101 = OpIMul %int %99 %100
OpStore %98 %101
%102 = OpAccessChain %_ptr_Function_float %x %int_1
%103 = OpLoad %float %102
%104 = OpLoad %float %l
%105 = OpFMul %float %103 %104
OpStore %102 %105
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%108 = OpLoad %v4float %106
OpReturnValue %108
OpFunctionEnd

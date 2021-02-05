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
OpName %i4 "i4"
OpName %x "x"
OpName %ai "ai"
OpName %ai4 "ai4"
OpName %ah2x4 "ah2x4"
OpName %af4 "af4"
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
OpDecorate %37 RelaxedPrecision
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %87 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
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
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%28 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%v2float = OpTypeVector %float 2
%35 = OpConstantComposite %v2float %float_0 %float_0
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_int_1 = OpTypeArray %mat3v3float %int_1
%_ptr_Function__arr_mat3v3float_int_1 = OpTypePointer Function %_arr_mat3v3float_int_1
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%73 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
%i4 = OpVariable %_ptr_Function_v4int Function
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
OpStore %i4 %28
%32 = OpAccessChain %_ptr_Function_float %x %int_3
OpStore %32 %float_0
%36 = OpLoad %v4float %x
%37 = OpVectorShuffle %v4float %36 %35 5 4 2 3
OpStore %x %37
%42 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %42 %int_0
%47 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %47 %28
%63 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%64 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%65 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%62 = OpCompositeConstruct %mat3v3float %63 %64 %65
%66 = OpAccessChain %_ptr_Function_mat3v3float %ah2x4 %int_0
OpStore %66 %62
%71 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%72 = OpAccessChain %_ptr_Function_float %71 %int_0
OpStore %72 %float_0
%74 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%75 = OpLoad %v4float %74
%76 = OpVectorShuffle %v4float %75 %73 6 4 7 5
OpStore %74 %76
%77 = OpAccessChain %_ptr_Function_int %ai %int_0
%78 = OpLoad %int %77
%79 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%80 = OpLoad %v4int %79
%81 = OpCompositeExtract %int %80 0
%82 = OpIAdd %int %78 %81
OpStore %77 %82
%83 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%84 = OpLoad %v4float %83
%85 = OpAccessChain %_ptr_Function_v3float %ah2x4 %int_0 %int_0
%87 = OpLoad %v3float %85
%88 = OpCompositeExtract %float %87 0
%89 = OpVectorTimesScalar %v4float %84 %88
OpStore %83 %89
%90 = OpAccessChain %_ptr_Function_int %i4 %int_1
%91 = OpLoad %int %90
%92 = OpIMul %int %91 %int_0
OpStore %90 %92
%93 = OpAccessChain %_ptr_Function_float %x %int_1
%94 = OpLoad %float %93
%95 = OpFMul %float %94 %float_0
OpStore %93 %95
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%98 = OpLoad %v4float %96
OpReturnValue %98
OpFunctionEnd

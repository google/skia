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
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %80 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
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
%int_1 = OpConstant %int 1
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%v4int = OpTypeVector %int 4
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%35 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%_ptr_Function_v4int = OpTypePointer Function %v4int
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
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
%66 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%26 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %26 %int_0
%36 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %36 %35
%53 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%54 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%55 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%52 = OpCompositeConstruct %mat3v3float %53 %54 %55
%56 = OpAccessChain %_ptr_Function_mat3v3float %ah2x4 %int_0
OpStore %56 %52
%62 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%64 = OpAccessChain %_ptr_Function_float %62 %int_0
OpStore %64 %float_0
%67 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v4float %68 %66 6 4 7 5
OpStore %67 %69
%70 = OpAccessChain %_ptr_Function_int %ai %int_0
%71 = OpLoad %int %70
%72 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%73 = OpLoad %v4int %72
%74 = OpCompositeExtract %int %73 0
%75 = OpIAdd %int %71 %74
OpStore %70 %75
%76 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%77 = OpLoad %v4float %76
%78 = OpAccessChain %_ptr_Function_v3float %ah2x4 %int_0 %int_0
%80 = OpLoad %v3float %78
%81 = OpCompositeExtract %float %80 0
%82 = OpVectorTimesScalar %v4float %77 %81
OpStore %76 %82
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%85 = OpLoad %v4float %83
OpReturnValue %85
OpFunctionEnd

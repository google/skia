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
OpName %x "x"
OpName %ai "ai"
OpName %ai4 "ai4"
OpName %ah2x4 "ah2x4"
OpName %af4 "af4"
OpName %S "S"
OpMemberName %S 0 "f"
OpMemberName %S 1 "af"
OpMemberName %S 2 "h4"
OpMemberName %S 3 "ah4"
OpName %s "s"
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
OpDecorate %40 RelaxedPrecision
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %_arr_float_int_5 ArrayStride 16
OpDecorate %_arr_v4float_int_5 ArrayStride 16
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 16
OpMemberDecorate %S 2 Offset 96
OpMemberDecorate %S 2 RelaxedPrecision
OpMemberDecorate %S 3 Offset 112
OpMemberDecorate %S 3 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%v2float = OpTypeVector %float 2
%38 = OpConstantComposite %v2float %float_0 %float_0
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
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
%74 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_arr_v4float_int_5 = OpTypeArray %v4float %int_5
%S = OpTypeStruct %float %_arr_float_int_5 %v4float %_arr_v4float_int_5
%_ptr_Function_S = OpTypePointer Function %S
%86 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%90 = OpConstantComposite %v2float %float_5 %float_5
%105 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
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
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%s = OpVariable %_ptr_Function_S Function
%l = OpVariable %_ptr_Function_float Function
%r = OpVariable %_ptr_Function_float Function
OpStore %i %int_0
OpStore %i4 %31
%35 = OpAccessChain %_ptr_Function_float %x %int_3
OpStore %35 %float_0
%39 = OpLoad %v4float %x
%40 = OpVectorShuffle %v4float %39 %38 5 4 2 3
OpStore %x %40
%44 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %44 %int_0
%48 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %48 %31
%64 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%65 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%66 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%63 = OpCompositeConstruct %mat3v3float %64 %65 %66
%67 = OpAccessChain %_ptr_Function_mat3v3float %ah2x4 %int_0
OpStore %67 %63
%72 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%73 = OpAccessChain %_ptr_Function_float %72 %int_0
OpStore %73 %float_0
%75 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%76 = OpLoad %v4float %75
%77 = OpVectorShuffle %v4float %76 %74 6 4 7 5
OpStore %75 %77
%84 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %84 %float_0
%85 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
OpStore %85 %float_0
%87 = OpAccessChain %_ptr_Function_v4float %s %int_2
%88 = OpLoad %v4float %87
%89 = OpVectorShuffle %v4float %88 %86 5 6 4 3
OpStore %87 %89
%91 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_2
%92 = OpLoad %v4float %91
%93 = OpVectorShuffle %v4float %92 %90 0 4 2 5
OpStore %91 %93
OpStore %l %float_0
%96 = OpAccessChain %_ptr_Function_int %ai %int_0
%97 = OpLoad %int %96
%98 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%99 = OpLoad %v4int %98
%100 = OpCompositeExtract %int %99 0
%101 = OpIAdd %int %97 %100
OpStore %96 %101
%102 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %102 %float_1
%103 = OpAccessChain %_ptr_Function_float %s %int_1 %int_0
OpStore %103 %float_2
%104 = OpAccessChain %_ptr_Function_v4float %s %int_2
OpStore %104 %74
%106 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
OpStore %106 %105
%107 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%108 = OpLoad %v4float %107
%109 = OpAccessChain %_ptr_Function_v3float %ah2x4 %int_0 %int_0
%111 = OpLoad %v3float %109
%112 = OpCompositeExtract %float %111 0
%113 = OpVectorTimesScalar %v4float %108 %112
OpStore %107 %113
%114 = OpAccessChain %_ptr_Function_int %i4 %int_1
%115 = OpLoad %int %114
%116 = OpLoad %int %i
%117 = OpIMul %int %115 %116
OpStore %114 %117
%118 = OpAccessChain %_ptr_Function_float %x %int_1
%119 = OpLoad %float %118
%120 = OpLoad %float %l
%121 = OpFMul %float %119 %120
OpStore %118 %121
%122 = OpAccessChain %_ptr_Function_float %s %int_0
%123 = OpLoad %float %122
%124 = OpLoad %float %l
%125 = OpFMul %float %123 %124
OpStore %122 %125
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%128 = OpLoad %v4float %126
OpReturnValue %128
OpFunctionEnd

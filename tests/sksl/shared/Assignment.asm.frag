OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %globalVar "globalVar"
OpName %S "S"
OpMemberName %S 0 "f"
OpMemberName %S 1 "af"
OpMemberName %S 2 "h4"
OpMemberName %S 3 "ah4"
OpName %globalStruct "globalStruct"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %i "i"
OpName %i4 "i4"
OpName %f3x3 "f3x3"
OpName %x "x"
OpName %ai "ai"
OpName %ai4 "ai4"
OpName %ah2x4 "ah2x4"
OpName %af4 "af4"
OpName %s "s"
OpName %l "l"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %globalVar RelaxedPrecision
OpDecorate %_arr_float_int_5 ArrayStride 16
OpDecorate %_arr_v4float_int_5 ArrayStride 16
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 16
OpMemberDecorate %S 2 Offset 96
OpMemberDecorate %S 2 RelaxedPrecision
OpMemberDecorate %S 3 Offset 112
OpMemberDecorate %S 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %x RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Private_v4float = OpTypePointer Private %v4float
%globalVar = OpVariable %_ptr_Private_v4float Private
%int = OpTypeInt 32 1
%int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_arr_v4float_int_5 = OpTypeArray %v4float %int_5
%S = OpTypeStruct %float %_arr_float_int_5 %v4float %_arr_v4float_int_5
%_ptr_Private_S = OpTypePointer Private %S
%globalStruct = OpVariable %_ptr_Private_S Private
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%24 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%32 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%45 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
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
%_ptr_Function_float = OpTypePointer Function %float
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%_arr_mat3v3float_int_1 = OpTypeArray %mat3v3float %int_1
%_ptr_Function__arr_mat3v3float_int_1 = OpTypePointer Function %_arr_mat3v3float_int_1
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%90 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_S = OpTypePointer Function %S
%98 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%102 = OpConstantComposite %v2float %float_5 %float_5
%106 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Private_float = OpTypePointer Private %float
%119 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %24
%25 = OpLabel
%29 = OpVariable %_ptr_Function_v2float Function
OpStore %29 %28
%31 = OpFunctionCall %v4float %main %29
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %32
%33 = OpFunctionParameter %_ptr_Function_v2float
%34 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%s = OpVariable %_ptr_Function_S Function
%l = OpVariable %_ptr_Function_float Function
OpStore %i %int_0
OpStore %i4 %45
%59 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%60 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%61 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%62 = OpCompositeConstruct %mat3v3float %59 %60 %61
OpStore %f3x3 %62
%65 = OpAccessChain %_ptr_Function_float %x %int_3
OpStore %65 %float_0
%67 = OpLoad %v4float %x
%68 = OpVectorShuffle %v4float %67 %28 5 4 2 3
OpStore %x %68
%72 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %72 %int_0
%76 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %76 %45
%80 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%81 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%82 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%83 = OpCompositeConstruct %mat3v3float %80 %81 %82
%84 = OpAccessChain %_ptr_Function_mat3v3float %ah2x4 %int_0
OpStore %84 %83
%88 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%89 = OpAccessChain %_ptr_Function_float %88 %int_0
OpStore %89 %float_0
%91 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%92 = OpLoad %v4float %91
%93 = OpVectorShuffle %v4float %92 %90 6 4 7 5
OpStore %91 %93
%96 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %96 %float_0
%97 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
OpStore %97 %float_0
%99 = OpAccessChain %_ptr_Function_v4float %s %int_2
%100 = OpLoad %v4float %99
%101 = OpVectorShuffle %v4float %100 %98 5 6 4 3
OpStore %99 %101
%103 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_2
%104 = OpLoad %v4float %103
%105 = OpVectorShuffle %v4float %104 %102 0 4 2 5
OpStore %103 %105
OpStore %globalVar %106
%107 = OpAccessChain %_ptr_Private_float %globalStruct %int_0
OpStore %107 %float_0
OpStore %l %float_0
%110 = OpAccessChain %_ptr_Function_int %ai %int_0
%111 = OpLoad %int %110
%112 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%113 = OpLoad %v4int %112
%114 = OpCompositeExtract %int %113 0
%115 = OpIAdd %int %111 %114
OpStore %110 %115
%116 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %116 %float_1
%117 = OpAccessChain %_ptr_Function_float %s %int_1 %int_0
OpStore %117 %float_2
%118 = OpAccessChain %_ptr_Function_v4float %s %int_2
OpStore %118 %90
%120 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
OpStore %120 %119
%121 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%122 = OpLoad %v4float %121
%123 = OpAccessChain %_ptr_Function_v3float %ah2x4 %int_0 %int_0
%125 = OpLoad %v3float %123
%126 = OpCompositeExtract %float %125 0
%127 = OpVectorTimesScalar %v4float %122 %126
OpStore %121 %127
%128 = OpAccessChain %_ptr_Function_int %i4 %int_1
%129 = OpLoad %int %128
%130 = OpLoad %int %i
%131 = OpIMul %int %129 %130
OpStore %128 %131
%132 = OpAccessChain %_ptr_Function_float %x %int_1
%133 = OpLoad %float %132
%134 = OpLoad %float %l
%135 = OpFMul %float %133 %134
OpStore %132 %135
%136 = OpAccessChain %_ptr_Function_float %s %int_0
%137 = OpLoad %float %136
%138 = OpLoad %float %l
%139 = OpFMul %float %137 %138
OpStore %136 %139
%140 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%142 = OpLoad %v4float %140
OpReturnValue %142
OpFunctionEnd

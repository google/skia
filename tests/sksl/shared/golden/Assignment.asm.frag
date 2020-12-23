OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
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
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %23 RelaxedPrecision
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat2v4float_int_1 ArrayStride 32
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %_arr_float_int_5 ArrayStride 16
OpDecorate %_arr_v4float_int_5 ArrayStride 16
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 16
OpMemberDecorate %S 2 Offset 96
OpMemberDecorate %S 2 RelaxedPrecision
OpMemberDecorate %S 3 Offset 112
OpMemberDecorate %S 3 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%int_1 = OpConstant %int 1
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%v4int = OpTypeVector %int 4
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%35 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%mat2v4float = OpTypeMatrix %v4float 2
%_arr_mat2v4float_int_1 = OpTypeArray %mat2v4float %int_1
%_ptr_Function__arr_mat2v4float_int_1 = OpTypePointer Function %_arr_mat2v4float_int_1
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%float_0_0 = OpConstant %float 0
%float_1_0 = OpConstant %float 1
%67 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_arr_v4float_int_5 = OpTypeArray %v4float %int_5
%S = OpTypeStruct %float %_arr_float_int_5 %v4float %_arr_v4float_int_5
%_ptr_Function_S = OpTypePointer Function %S
%float_9 = OpConstant %float 9
%v3float = OpTypeVector %float 3
%80 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%86 = OpConstantComposite %v2float %float_5 %float_5
%float_2_0 = OpConstant %float 2
%93 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%95 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%97 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%98 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_3_0 = OpConstant %float 3
%float_4_0 = OpConstant %float 4
%float_5_0 = OpConstant %float 5
%float_6_0 = OpConstant %float 6
%float_7_0 = OpConstant %float 7
%float_8_0 = OpConstant %float 8
%float_9_0 = OpConstant %float 9
%_ptr_Function_v3float = OpTypePointer Function %v3float
%146 = OpConstantComposite %v4float %float_0_0 %float_0_0 %float_0_0 %float_0_0
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat2v4float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%s = OpVariable %_ptr_Function_S Function
%108 = OpVariable %_ptr_Function_mat3v3float Function
%16 = OpAccessChain %_ptr_Function_float %x %int_3
OpStore %16 %float_0
%22 = OpLoad %v4float %x
%23 = OpVectorShuffle %v4float %22 %20 5 4 2 3
OpStore %x %23
%29 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %29 %int_0
%38 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %38 %35
%53 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%54 = OpCompositeConstruct %v4float %float_5 %float_6 %float_7 %float_8
%52 = OpCompositeConstruct %mat2v4float %53 %54
%55 = OpAccessChain %_ptr_Function_mat2v4float %ah2x4 %int_0
OpStore %55 %52
%57 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %57 %int_0
%58 = OpAccessChain %_ptr_Function_int %ai %int_0
%59 = OpLoad %int %58
%60 = OpAccessChain %_ptr_Function_int %ai %59
OpStore %60 %int_0
%65 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%66 = OpAccessChain %_ptr_Function_float %65 %int_0
OpStore %66 %float_0_0
%69 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v4float %70 %67 6 4 7 5
OpStore %69 %71
%78 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %78 %float_0_0
%79 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
OpStore %79 %float_0_0
%83 = OpAccessChain %_ptr_Function_v4float %s %int_2
%84 = OpLoad %v4float %83
%85 = OpVectorShuffle %v4float %84 %80 5 6 4 3
OpStore %83 %85
%87 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_2
%88 = OpLoad %v4float %87
%89 = OpVectorShuffle %v4float %88 %86 0 4 2 5
OpStore %87 %89
%90 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %90 %float_1_0
%92 = OpAccessChain %_ptr_Function_float %s %int_1 %int_0
OpStore %92 %float_2_0
%94 = OpAccessChain %_ptr_Function_v4float %s %int_2
OpStore %94 %93
%96 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
OpStore %96 %95
OpStore %sk_FragColor %97
%99 = OpCompositeExtract %int %98 0
%100 = OpConvertSToF %float %99
%101 = OpCompositeExtract %int %98 1
%102 = OpConvertSToF %float %101
%103 = OpCompositeExtract %int %98 2
%104 = OpConvertSToF %float %103
%105 = OpCompositeExtract %int %98 3
%106 = OpConvertSToF %float %105
%107 = OpCompositeConstruct %v4float %100 %102 %104 %106
OpStore %sk_FragColor %107
%119 = OpCompositeConstruct %v3float %float_1_0 %float_2_0 %float_3_0
%120 = OpCompositeConstruct %v3float %float_4_0 %float_5_0 %float_6_0
%121 = OpCompositeConstruct %v3float %float_7_0 %float_8_0 %float_9_0
%118 = OpCompositeConstruct %mat3v3float %119 %120 %121
OpStore %108 %118
%122 = OpAccessChain %_ptr_Function_v3float %108 %int_0
%124 = OpLoad %v3float %122
%125 = OpVectorShuffle %v4float %124 %124 0 0 1 2
OpStore %sk_FragColor %125
%126 = OpLoad %v4float %x
OpStore %sk_FragColor %126
%128 = OpAccessChain %_ptr_Function_int %ai %int_0
%129 = OpLoad %int %128
%127 = OpConvertSToF %float %129
%130 = OpCompositeConstruct %v4float %127 %127 %127 %127
OpStore %sk_FragColor %130
%131 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%132 = OpLoad %v4int %131
%133 = OpCompositeExtract %int %132 0
%134 = OpConvertSToF %float %133
%135 = OpCompositeExtract %int %132 1
%136 = OpConvertSToF %float %135
%137 = OpCompositeExtract %int %132 2
%138 = OpConvertSToF %float %137
%139 = OpCompositeExtract %int %132 3
%140 = OpConvertSToF %float %139
%141 = OpCompositeConstruct %v4float %134 %136 %138 %140
OpStore %sk_FragColor %141
%142 = OpAccessChain %_ptr_Function_v4float %ah2x4 %int_0 %int_0
%143 = OpLoad %v4float %142
OpStore %sk_FragColor %143
%144 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%145 = OpLoad %v4float %144
OpStore %sk_FragColor %145
OpStore %sk_FragColor %146
%147 = OpAccessChain %_ptr_Function_float %s %int_0
%148 = OpLoad %float %147
%149 = OpCompositeConstruct %v4float %148 %148 %148 %148
OpStore %sk_FragColor %149
%150 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
%151 = OpLoad %float %150
%152 = OpCompositeConstruct %v4float %151 %151 %151 %151
OpStore %sk_FragColor %152
%153 = OpAccessChain %_ptr_Function_v4float %s %int_2
%154 = OpLoad %v4float %153
OpStore %sk_FragColor %154
%155 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
%156 = OpLoad %v4float %155
OpStore %sk_FragColor %156
OpReturn
OpFunctionEnd

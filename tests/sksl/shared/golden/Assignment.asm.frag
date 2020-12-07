### Compilation failed:

error: SPIR-V validation error: OpConstantComposite Constituent <id> '99[%99]'s type does not match Result Type <id> '5[%v4float]'s vector element type.
  %98 = OpConstantComposite %v4float %99 %99 %99 %99

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
OpDecorate %118 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
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
%99 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%98 = OpConstantComposite %v4float %99 %99 %99 %99
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
%138 = OpConstantComposite %v4float %float_0_0 %float_0_0 %float_0_0 %float_0_0
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat2v4float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%s = OpVariable %_ptr_Function_S Function
%100 = OpVariable %_ptr_Function_mat3v3float Function
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
OpStore %sk_FragColor %98
%111 = OpCompositeConstruct %v3float %float_1_0 %float_2_0 %float_3_0
%112 = OpCompositeConstruct %v3float %float_4_0 %float_5_0 %float_6_0
%113 = OpCompositeConstruct %v3float %float_7_0 %float_8_0 %float_9_0
%110 = OpCompositeConstruct %mat3v3float %111 %112 %113
OpStore %100 %110
%114 = OpAccessChain %_ptr_Function_v3float %100 %int_0
%116 = OpLoad %v3float %114
%117 = OpVectorShuffle %v4float %116 %116 0 0 1 2
OpStore %sk_FragColor %117
%118 = OpLoad %v4float %x
OpStore %sk_FragColor %118
%120 = OpAccessChain %_ptr_Function_int %ai %int_0
%121 = OpLoad %int %120
%119 = OpConvertSToF %float %121
%122 = OpCompositeConstruct %v4float %119 %119 %119 %119
OpStore %sk_FragColor %122
%123 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%124 = OpLoad %v4int %123
%125 = OpCompositeExtract %int %124 0
%126 = OpConvertSToF %float %125
%127 = OpCompositeExtract %int %124 1
%128 = OpConvertSToF %float %127
%129 = OpCompositeExtract %int %124 2
%130 = OpConvertSToF %float %129
%131 = OpCompositeExtract %int %124 3
%132 = OpConvertSToF %float %131
%133 = OpCompositeConstruct %v4float %126 %128 %130 %132
OpStore %sk_FragColor %133
%134 = OpAccessChain %_ptr_Function_v4float %ah2x4 %int_0 %int_0
%135 = OpLoad %v4float %134
OpStore %sk_FragColor %135
%136 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%137 = OpLoad %v4float %136
OpStore %sk_FragColor %137
OpStore %sk_FragColor %138
%139 = OpAccessChain %_ptr_Function_float %s %int_0
%140 = OpLoad %float %139
%141 = OpCompositeConstruct %v4float %140 %140 %140 %140
OpStore %sk_FragColor %141
%142 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
%143 = OpLoad %float %142
%144 = OpCompositeConstruct %v4float %143 %143 %143 %143
OpStore %sk_FragColor %144
%145 = OpAccessChain %_ptr_Function_v4float %s %int_2
%146 = OpLoad %v4float %145
OpStore %sk_FragColor %146
%147 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
%148 = OpLoad %v4float %147
OpStore %sk_FragColor %148
OpReturn
OpFunctionEnd

1 error

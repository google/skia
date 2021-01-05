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
OpDecorate %83 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
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
%66 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_arr_v4float_int_5 = OpTypeArray %v4float %int_5
%S = OpTypeStruct %float %_arr_float_int_5 %v4float %_arr_v4float_int_5
%_ptr_Function_S = OpTypePointer Function %S
%float_9 = OpConstant %float 9
%v3float = OpTypeVector %float 3
%78 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%84 = OpConstantComposite %v2float %float_5 %float_5
%90 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%92 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%94 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%95 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%136 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat2v4float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%s = OpVariable %_ptr_Function_S Function
%105 = OpVariable %_ptr_Function_mat3v3float Function
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
%64 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%65 = OpAccessChain %_ptr_Function_float %64 %int_0
OpStore %65 %float_0
%67 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v4float %68 %66 6 4 7 5
OpStore %67 %69
%76 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %76 %float_0
%77 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
OpStore %77 %float_0
%81 = OpAccessChain %_ptr_Function_v4float %s %int_2
%82 = OpLoad %v4float %81
%83 = OpVectorShuffle %v4float %82 %78 5 6 4 3
OpStore %81 %83
%85 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_2
%86 = OpLoad %v4float %85
%87 = OpVectorShuffle %v4float %86 %84 0 4 2 5
OpStore %85 %87
%88 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %88 %float_1
%89 = OpAccessChain %_ptr_Function_float %s %int_1 %int_0
OpStore %89 %float_2
%91 = OpAccessChain %_ptr_Function_v4float %s %int_2
OpStore %91 %90
%93 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
OpStore %93 %92
OpStore %sk_FragColor %94
%96 = OpCompositeExtract %int %95 0
%97 = OpConvertSToF %float %96
%98 = OpCompositeExtract %int %95 1
%99 = OpConvertSToF %float %98
%100 = OpCompositeExtract %int %95 2
%101 = OpConvertSToF %float %100
%102 = OpCompositeExtract %int %95 3
%103 = OpConvertSToF %float %102
%104 = OpCompositeConstruct %v4float %97 %99 %101 %103
OpStore %sk_FragColor %104
%109 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%110 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%111 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%108 = OpCompositeConstruct %mat3v3float %109 %110 %111
OpStore %105 %108
%112 = OpAccessChain %_ptr_Function_v3float %105 %int_0
%114 = OpLoad %v3float %112
%115 = OpVectorShuffle %v4float %114 %114 0 0 1 2
OpStore %sk_FragColor %115
%116 = OpLoad %v4float %x
OpStore %sk_FragColor %116
%118 = OpAccessChain %_ptr_Function_int %ai %int_0
%119 = OpLoad %int %118
%117 = OpConvertSToF %float %119
%120 = OpCompositeConstruct %v4float %117 %117 %117 %117
OpStore %sk_FragColor %120
%121 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%122 = OpLoad %v4int %121
%123 = OpCompositeExtract %int %122 0
%124 = OpConvertSToF %float %123
%125 = OpCompositeExtract %int %122 1
%126 = OpConvertSToF %float %125
%127 = OpCompositeExtract %int %122 2
%128 = OpConvertSToF %float %127
%129 = OpCompositeExtract %int %122 3
%130 = OpConvertSToF %float %129
%131 = OpCompositeConstruct %v4float %124 %126 %128 %130
OpStore %sk_FragColor %131
%132 = OpAccessChain %_ptr_Function_v4float %ah2x4 %int_0 %int_0
%133 = OpLoad %v4float %132
OpStore %sk_FragColor %133
%134 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%135 = OpLoad %v4float %134
OpStore %sk_FragColor %135
OpStore %sk_FragColor %136
%137 = OpAccessChain %_ptr_Function_float %s %int_0
%138 = OpLoad %float %137
%139 = OpCompositeConstruct %v4float %138 %138 %138 %138
OpStore %sk_FragColor %139
%140 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
%141 = OpLoad %float %140
%142 = OpCompositeConstruct %v4float %141 %141 %141 %141
OpStore %sk_FragColor %142
%143 = OpAccessChain %_ptr_Function_v4float %s %int_2
%144 = OpLoad %v4float %143
OpStore %sk_FragColor %144
%145 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
%146 = OpLoad %v4float %145
OpStore %sk_FragColor %146
OpReturn
OpFunctionEnd

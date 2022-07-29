OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %v3 "v3"
OpName %m1 "m1"
OpName %m2 "m2"
OpName %m3 "m3"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %s1 "s1"
OpName %s2 "s2"
OpName %s3 "s3"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %_arr_v3int_int_2 ArrayStride 16
OpDecorate %_arr_mat2v2float_int_3 ArrayStride 32
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_n4 = OpConstant %float -4
%v3int = OpTypeVector %int 3
%int_2 = OpConstant %int 2
%_arr_v3int_int_2 = OpTypeArray %v3int %int_2
%_ptr_Function__arr_v3int_int_2 = OpTypePointer Function %_arr_v3int_int_2
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%47 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%50 = OpConstantComposite %v3int %int_4 %int_5 %int_6
%int_n6 = OpConstant %int -6
%55 = OpConstantComposite %v3int %int_4 %int_5 %int_n6
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_3 = OpTypeArray %mat2v2float %int_3
%_ptr_Function__arr_mat2v2float_int_3 = OpTypePointer Function %_arr_mat2v2float_int_3
%61 = OpConstantComposite %v2float %float_1 %float_0
%62 = OpConstantComposite %v2float %float_0 %float_1
%63 = OpConstantComposite %mat2v2float %61 %62
%64 = OpConstantComposite %v2float %float_2 %float_0
%65 = OpConstantComposite %v2float %float_0 %float_2
%66 = OpConstantComposite %mat2v2float %64 %65
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%69 = OpConstantComposite %v2float %float_3 %float_4
%70 = OpConstantComposite %v2float %float_5 %float_6
%71 = OpConstantComposite %mat2v2float %69 %70
%75 = OpConstantComposite %v2float %float_2 %float_3
%76 = OpConstantComposite %v2float %float_4 %float_5
%77 = OpConstantComposite %mat2v2float %75 %76
%78 = OpConstantComposite %v2float %float_6 %float_0
%79 = OpConstantComposite %v2float %float_0 %float_6
%80 = OpConstantComposite %mat2v2float %78 %79
%S = OpTypeStruct %int %int
%_arr_S_int_3 = OpTypeArray %S %int_3
%_ptr_Function__arr_S_int_3 = OpTypePointer Function %_arr_S_int_3
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%v3bool = OpTypeVector %bool 3
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%f1 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f3 = OpVariable %_ptr_Function__arr_float_int_4 Function
%v1 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
%v2 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
%v3 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
%m1 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
%m2 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
%m3 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
%s1 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s2 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_S_int_3 Function
%177 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f1 %35
OpStore %f2 %35
%39 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_n4
OpStore %f3 %39
%51 = OpCompositeConstruct %_arr_v3int_int_2 %47 %50
OpStore %v1 %51
OpStore %v2 %51
%56 = OpCompositeConstruct %_arr_v3int_int_2 %47 %55
OpStore %v3 %56
%72 = OpCompositeConstruct %_arr_mat2v2float_int_3 %63 %66 %71
OpStore %m1 %72
OpStore %m2 %72
%81 = OpCompositeConstruct %_arr_mat2v2float_int_3 %63 %77 %80
OpStore %m3 %81
%86 = OpCompositeConstruct %S %int_1 %int_2
%87 = OpCompositeConstruct %S %int_3 %int_4
%88 = OpCompositeConstruct %S %int_5 %int_6
%89 = OpCompositeConstruct %_arr_S_int_3 %86 %87 %88
OpStore %s1 %89
%92 = OpCompositeConstruct %S %int_0 %int_0
%93 = OpCompositeConstruct %_arr_S_int_3 %86 %92 %88
OpStore %s2 %93
OpStore %s3 %89
%97 = OpLogicalAnd %bool %true %true
%98 = OpLogicalAnd %bool %true %97
%99 = OpLogicalAnd %bool %true %98
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpLogicalOr %bool %false %false
%103 = OpLogicalOr %bool %false %102
%104 = OpFUnordNotEqual %bool %float_4 %float_n4
%105 = OpLogicalOr %bool %104 %103
OpBranch %101
%101 = OpLabel
%106 = OpPhi %bool %false %25 %105 %100
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLogicalAnd %bool %true %true
OpBranch %108
%108 = OpLabel
%110 = OpPhi %bool %false %101 %109 %107
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpINotEqual %v3bool %50 %55
%115 = OpAny %bool %113
%116 = OpLogicalOr %bool %115 %false
OpBranch %112
%112 = OpLabel
%117 = OpPhi %bool %false %108 %116 %111
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpFOrdEqual %v2bool %61 %61
%122 = OpAll %bool %121
%123 = OpFOrdEqual %v2bool %62 %62
%124 = OpAll %bool %123
%125 = OpLogicalAnd %bool %122 %124
%126 = OpFOrdEqual %v2bool %64 %64
%127 = OpAll %bool %126
%128 = OpFOrdEqual %v2bool %65 %65
%129 = OpAll %bool %128
%130 = OpLogicalAnd %bool %127 %129
%131 = OpLogicalAnd %bool %130 %125
%132 = OpFOrdEqual %v2bool %69 %69
%133 = OpAll %bool %132
%134 = OpFOrdEqual %v2bool %70 %70
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %133 %135
%137 = OpLogicalAnd %bool %136 %131
OpBranch %119
%119 = OpLabel
%138 = OpPhi %bool %false %112 %137 %118
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%141 = OpFUnordNotEqual %v2bool %61 %61
%142 = OpAny %bool %141
%143 = OpFUnordNotEqual %v2bool %62 %62
%144 = OpAny %bool %143
%145 = OpLogicalOr %bool %142 %144
%146 = OpFUnordNotEqual %v2bool %64 %75
%147 = OpAny %bool %146
%148 = OpFUnordNotEqual %v2bool %65 %76
%149 = OpAny %bool %148
%150 = OpLogicalOr %bool %147 %149
%151 = OpLogicalOr %bool %150 %145
%152 = OpFUnordNotEqual %v2bool %69 %78
%153 = OpAny %bool %152
%154 = OpFUnordNotEqual %v2bool %70 %79
%155 = OpAny %bool %154
%156 = OpLogicalOr %bool %153 %155
%157 = OpLogicalOr %bool %156 %151
OpBranch %140
%140 = OpLabel
%158 = OpPhi %bool %false %119 %157 %139
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpLogicalOr %bool %false %false
%162 = OpINotEqual %bool %int_3 %int_0
%163 = OpINotEqual %bool %int_4 %int_0
%164 = OpLogicalOr %bool %163 %162
%165 = OpLogicalOr %bool %164 %161
%166 = OpLogicalOr %bool %false %false
%167 = OpLogicalOr %bool %166 %165
OpBranch %160
%160 = OpLabel
%168 = OpPhi %bool %false %140 %167 %159
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%171 = OpLogicalAnd %bool %true %true
%172 = OpLogicalAnd %bool %true %true
%173 = OpLogicalAnd %bool %172 %171
%174 = OpLogicalAnd %bool %true %true
%175 = OpLogicalAnd %bool %174 %173
OpBranch %170
%170 = OpLabel
%176 = OpPhi %bool %false %160 %175 %169
OpSelectionMerge %181 None
OpBranchConditional %176 %179 %180
%179 = OpLabel
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%184 = OpLoad %v4float %182
OpStore %177 %184
OpBranch %181
%180 = OpLabel
%185 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%186 = OpLoad %v4float %185
OpStore %177 %186
OpBranch %181
%181 = OpLabel
%187 = OpLoad %v4float %177
OpReturnValue %187
OpFunctionEnd

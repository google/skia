OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
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
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
%199 = OpVariable %_ptr_Function_v4float Function
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
%96 = OpFOrdEqual %bool %float_1 %float_1
%97 = OpFOrdEqual %bool %float_2 %float_2
%98 = OpLogicalAnd %bool %97 %96
%99 = OpFOrdEqual %bool %float_3 %float_3
%100 = OpLogicalAnd %bool %99 %98
%101 = OpFOrdEqual %bool %float_4 %float_4
%102 = OpLogicalAnd %bool %101 %100
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpFUnordNotEqual %bool %float_1 %float_1
%106 = OpFUnordNotEqual %bool %float_2 %float_2
%107 = OpLogicalOr %bool %106 %105
%108 = OpFUnordNotEqual %bool %float_3 %float_3
%109 = OpLogicalOr %bool %108 %107
%110 = OpFUnordNotEqual %bool %float_4 %float_n4
%111 = OpLogicalOr %bool %110 %109
OpBranch %104
%104 = OpLabel
%112 = OpPhi %bool %false %25 %111 %103
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpIEqual %v3bool %47 %47
%117 = OpAll %bool %115
%118 = OpIEqual %v3bool %50 %50
%119 = OpAll %bool %118
%120 = OpLogicalAnd %bool %119 %117
OpBranch %114
%114 = OpLabel
%121 = OpPhi %bool %false %104 %120 %113
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpINotEqual %v3bool %47 %47
%125 = OpAny %bool %124
%126 = OpINotEqual %v3bool %50 %55
%127 = OpAny %bool %126
%128 = OpLogicalOr %bool %127 %125
OpBranch %123
%123 = OpLabel
%129 = OpPhi %bool %false %114 %128 %122
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%133 = OpFOrdEqual %v2bool %61 %61
%134 = OpAll %bool %133
%135 = OpFOrdEqual %v2bool %62 %62
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %134 %136
%138 = OpFOrdEqual %v2bool %64 %64
%139 = OpAll %bool %138
%140 = OpFOrdEqual %v2bool %65 %65
%141 = OpAll %bool %140
%142 = OpLogicalAnd %bool %139 %141
%143 = OpLogicalAnd %bool %142 %137
%144 = OpFOrdEqual %v2bool %69 %69
%145 = OpAll %bool %144
%146 = OpFOrdEqual %v2bool %70 %70
%147 = OpAll %bool %146
%148 = OpLogicalAnd %bool %145 %147
%149 = OpLogicalAnd %bool %148 %143
OpBranch %131
%131 = OpLabel
%150 = OpPhi %bool %false %123 %149 %130
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpFUnordNotEqual %v2bool %61 %61
%154 = OpAny %bool %153
%155 = OpFUnordNotEqual %v2bool %62 %62
%156 = OpAny %bool %155
%157 = OpLogicalOr %bool %154 %156
%158 = OpFUnordNotEqual %v2bool %64 %75
%159 = OpAny %bool %158
%160 = OpFUnordNotEqual %v2bool %65 %76
%161 = OpAny %bool %160
%162 = OpLogicalOr %bool %159 %161
%163 = OpLogicalOr %bool %162 %157
%164 = OpFUnordNotEqual %v2bool %69 %78
%165 = OpAny %bool %164
%166 = OpFUnordNotEqual %v2bool %70 %79
%167 = OpAny %bool %166
%168 = OpLogicalOr %bool %165 %167
%169 = OpLogicalOr %bool %168 %163
OpBranch %152
%152 = OpLabel
%170 = OpPhi %bool %false %131 %169 %151
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpINotEqual %bool %int_1 %int_1
%174 = OpINotEqual %bool %int_2 %int_2
%175 = OpLogicalOr %bool %174 %173
%176 = OpINotEqual %bool %int_3 %int_0
%177 = OpINotEqual %bool %int_4 %int_0
%178 = OpLogicalOr %bool %177 %176
%179 = OpLogicalOr %bool %178 %175
%180 = OpINotEqual %bool %int_5 %int_5
%181 = OpINotEqual %bool %int_6 %int_6
%182 = OpLogicalOr %bool %181 %180
%183 = OpLogicalOr %bool %182 %179
OpBranch %172
%172 = OpLabel
%184 = OpPhi %bool %false %152 %183 %171
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpIEqual %bool %int_1 %int_1
%188 = OpIEqual %bool %int_2 %int_2
%189 = OpLogicalAnd %bool %188 %187
%190 = OpIEqual %bool %int_3 %int_3
%191 = OpIEqual %bool %int_4 %int_4
%192 = OpLogicalAnd %bool %191 %190
%193 = OpLogicalAnd %bool %192 %189
%194 = OpIEqual %bool %int_5 %int_5
%195 = OpIEqual %bool %int_6 %int_6
%196 = OpLogicalAnd %bool %195 %194
%197 = OpLogicalAnd %bool %196 %193
OpBranch %186
%186 = OpLabel
%198 = OpPhi %bool %false %172 %197 %185
OpSelectionMerge %203 None
OpBranchConditional %198 %201 %202
%201 = OpLabel
%204 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%206 = OpLoad %v4float %204
OpStore %199 %206
OpBranch %203
%202 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%208 = OpLoad %v4float %207
OpStore %199 %208
OpBranch %203
%203 = OpLabel
%209 = OpLoad %v4float %199
OpReturnValue %209
OpFunctionEnd

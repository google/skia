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
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
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
%48 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%51 = OpConstantComposite %v3int %int_4 %int_5 %int_6
%int_n6 = OpConstant %int -6
%57 = OpConstantComposite %v3int %int_4 %int_5 %int_n6
%S = OpTypeStruct %int %int
%_arr_S_int_3 = OpTypeArray %S %int_3
%_ptr_Function__arr_S_int_3 = OpTypePointer Function %_arr_S_int_3
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
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
%s1 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s2 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_S_int_3 Function
%213 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f1 %35
%37 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f2 %37
%40 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_n4
OpStore %f3 %40
%52 = OpCompositeConstruct %_arr_v3int_int_2 %48 %51
OpStore %v1 %52
%54 = OpCompositeConstruct %_arr_v3int_int_2 %48 %51
OpStore %v2 %54
%58 = OpCompositeConstruct %_arr_v3int_int_2 %48 %57
OpStore %v3 %58
%63 = OpCompositeConstruct %S %int_1 %int_2
%64 = OpCompositeConstruct %S %int_3 %int_4
%65 = OpCompositeConstruct %S %int_5 %int_6
%66 = OpCompositeConstruct %_arr_S_int_3 %63 %64 %65
OpStore %s1 %66
%68 = OpCompositeConstruct %S %int_1 %int_2
%70 = OpCompositeConstruct %S %int_0 %int_0
%71 = OpCompositeConstruct %S %int_5 %int_6
%72 = OpCompositeConstruct %_arr_S_int_3 %68 %70 %71
OpStore %s2 %72
%74 = OpCompositeConstruct %S %int_1 %int_2
%75 = OpCompositeConstruct %S %int_3 %int_4
%76 = OpCompositeConstruct %S %int_5 %int_6
%77 = OpCompositeConstruct %_arr_S_int_3 %74 %75 %76
OpStore %s3 %77
%79 = OpLoad %_arr_float_int_4 %f1
%80 = OpLoad %_arr_float_int_4 %f2
%81 = OpCompositeExtract %float %79 0
%82 = OpCompositeExtract %float %80 0
%83 = OpFOrdEqual %bool %81 %82
%84 = OpCompositeExtract %float %79 1
%85 = OpCompositeExtract %float %80 1
%86 = OpFOrdEqual %bool %84 %85
%87 = OpLogicalAnd %bool %86 %83
%88 = OpCompositeExtract %float %79 2
%89 = OpCompositeExtract %float %80 2
%90 = OpFOrdEqual %bool %88 %89
%91 = OpLogicalAnd %bool %90 %87
%92 = OpCompositeExtract %float %79 3
%93 = OpCompositeExtract %float %80 3
%94 = OpFOrdEqual %bool %92 %93
%95 = OpLogicalAnd %bool %94 %91
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpLoad %_arr_float_int_4 %f1
%99 = OpLoad %_arr_float_int_4 %f3
%100 = OpCompositeExtract %float %98 0
%101 = OpCompositeExtract %float %99 0
%102 = OpFOrdNotEqual %bool %100 %101
%103 = OpCompositeExtract %float %98 1
%104 = OpCompositeExtract %float %99 1
%105 = OpFOrdNotEqual %bool %103 %104
%106 = OpLogicalOr %bool %105 %102
%107 = OpCompositeExtract %float %98 2
%108 = OpCompositeExtract %float %99 2
%109 = OpFOrdNotEqual %bool %107 %108
%110 = OpLogicalOr %bool %109 %106
%111 = OpCompositeExtract %float %98 3
%112 = OpCompositeExtract %float %99 3
%113 = OpFOrdNotEqual %bool %111 %112
%114 = OpLogicalOr %bool %113 %110
OpBranch %97
%97 = OpLabel
%115 = OpPhi %bool %false %25 %114 %96
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpLoad %_arr_v3int_int_2 %v1
%119 = OpLoad %_arr_v3int_int_2 %v2
%120 = OpCompositeExtract %v3int %118 0
%121 = OpCompositeExtract %v3int %119 0
%122 = OpIEqual %v3bool %120 %121
%124 = OpAll %bool %122
%125 = OpCompositeExtract %v3int %118 1
%126 = OpCompositeExtract %v3int %119 1
%127 = OpIEqual %v3bool %125 %126
%128 = OpAll %bool %127
%129 = OpLogicalAnd %bool %128 %124
OpBranch %117
%117 = OpLabel
%130 = OpPhi %bool %false %97 %129 %116
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %_arr_v3int_int_2 %v1
%134 = OpLoad %_arr_v3int_int_2 %v3
%135 = OpCompositeExtract %v3int %133 0
%136 = OpCompositeExtract %v3int %134 0
%137 = OpINotEqual %v3bool %135 %136
%138 = OpAny %bool %137
%139 = OpCompositeExtract %v3int %133 1
%140 = OpCompositeExtract %v3int %134 1
%141 = OpINotEqual %v3bool %139 %140
%142 = OpAny %bool %141
%143 = OpLogicalOr %bool %142 %138
OpBranch %132
%132 = OpLabel
%144 = OpPhi %bool %false %117 %143 %131
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%147 = OpLoad %_arr_S_int_3 %s1
%148 = OpLoad %_arr_S_int_3 %s2
%149 = OpCompositeExtract %S %147 0
%150 = OpCompositeExtract %S %148 0
%151 = OpCompositeExtract %int %149 0
%152 = OpCompositeExtract %int %150 0
%153 = OpINotEqual %bool %151 %152
%154 = OpCompositeExtract %int %149 1
%155 = OpCompositeExtract %int %150 1
%156 = OpINotEqual %bool %154 %155
%157 = OpLogicalOr %bool %156 %153
%158 = OpCompositeExtract %S %147 1
%159 = OpCompositeExtract %S %148 1
%160 = OpCompositeExtract %int %158 0
%161 = OpCompositeExtract %int %159 0
%162 = OpINotEqual %bool %160 %161
%163 = OpCompositeExtract %int %158 1
%164 = OpCompositeExtract %int %159 1
%165 = OpINotEqual %bool %163 %164
%166 = OpLogicalOr %bool %165 %162
%167 = OpLogicalOr %bool %166 %157
%168 = OpCompositeExtract %S %147 2
%169 = OpCompositeExtract %S %148 2
%170 = OpCompositeExtract %int %168 0
%171 = OpCompositeExtract %int %169 0
%172 = OpINotEqual %bool %170 %171
%173 = OpCompositeExtract %int %168 1
%174 = OpCompositeExtract %int %169 1
%175 = OpINotEqual %bool %173 %174
%176 = OpLogicalOr %bool %175 %172
%177 = OpLogicalOr %bool %176 %167
OpBranch %146
%146 = OpLabel
%178 = OpPhi %bool %false %132 %177 %145
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%181 = OpLoad %_arr_S_int_3 %s3
%182 = OpLoad %_arr_S_int_3 %s1
%183 = OpCompositeExtract %S %181 0
%184 = OpCompositeExtract %S %182 0
%185 = OpCompositeExtract %int %183 0
%186 = OpCompositeExtract %int %184 0
%187 = OpIEqual %bool %185 %186
%188 = OpCompositeExtract %int %183 1
%189 = OpCompositeExtract %int %184 1
%190 = OpIEqual %bool %188 %189
%191 = OpLogicalAnd %bool %190 %187
%192 = OpCompositeExtract %S %181 1
%193 = OpCompositeExtract %S %182 1
%194 = OpCompositeExtract %int %192 0
%195 = OpCompositeExtract %int %193 0
%196 = OpIEqual %bool %194 %195
%197 = OpCompositeExtract %int %192 1
%198 = OpCompositeExtract %int %193 1
%199 = OpIEqual %bool %197 %198
%200 = OpLogicalAnd %bool %199 %196
%201 = OpLogicalAnd %bool %200 %191
%202 = OpCompositeExtract %S %181 2
%203 = OpCompositeExtract %S %182 2
%204 = OpCompositeExtract %int %202 0
%205 = OpCompositeExtract %int %203 0
%206 = OpIEqual %bool %204 %205
%207 = OpCompositeExtract %int %202 1
%208 = OpCompositeExtract %int %203 1
%209 = OpIEqual %bool %207 %208
%210 = OpLogicalAnd %bool %209 %206
%211 = OpLogicalAnd %bool %210 %201
OpBranch %180
%180 = OpLabel
%212 = OpPhi %bool %false %146 %211 %179
OpSelectionMerge %217 None
OpBranchConditional %212 %215 %216
%215 = OpLabel
%218 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%220 = OpLoad %v4float %218
OpStore %213 %220
OpBranch %217
%216 = OpLabel
%221 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%222 = OpLoad %v4float %221
OpStore %213 %222
OpBranch %217
%217 = OpLabel
%223 = OpLoad %v4float %213
OpReturnValue %223
OpFunctionEnd

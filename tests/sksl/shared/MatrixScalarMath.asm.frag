OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %minus "minus"
OpName %star "star"
OpName %slash "slash"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_bifffff22 "test_bifffff22"
OpName %one "one"
OpName %m2 "m2"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %_0_expected "_0_expected"
OpName %_1_one "_1_one"
OpName %_2_m2 "_2_m2"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%minus = OpVariable %_ptr_Private_int Private
%int_2 = OpConstant %int 2
%star = OpVariable %_ptr_Private_int Private
%int_3 = OpConstant %int 3
%slash = OpVariable %_ptr_Private_int Private
%int_4 = OpConstant %int 4
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%24 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%36 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%70 = OpConstantComposite %v2float %float_1 %float_1
%71 = OpConstantComposite %mat2v2float %70 %70
%float_2 = OpConstant %float 2
%87 = OpConstantComposite %v2float %float_2 %float_2
%88 = OpConstantComposite %mat2v2float %87 %87
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%133 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_0_5 = OpConstant %float 0.5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %24
%25 = OpLabel
%29 = OpVariable %_ptr_Function_v2float Function
OpStore %29 %28
%31 = OpFunctionCall %v4float %main %29
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
%test_bifffff22 = OpFunction %bool None %36
%37 = OpFunctionParameter %_ptr_Function_int
%38 = OpFunctionParameter %_ptr_Function_float
%39 = OpFunctionParameter %_ptr_Function_float
%40 = OpFunctionParameter %_ptr_Function_float
%41 = OpFunctionParameter %_ptr_Function_float
%42 = OpFunctionParameter %_ptr_Function_mat2v2float
%43 = OpLabel
%one = OpVariable %_ptr_Function_float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%45 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%48 = OpLoad %v4float %45
%49 = OpCompositeExtract %float %48 0
OpStore %one %49
%51 = OpLoad %float %38
%52 = OpFMul %float %51 %49
%53 = OpLoad %float %39
%54 = OpFMul %float %53 %49
%55 = OpLoad %float %40
%56 = OpFMul %float %55 %49
%57 = OpLoad %float %41
%58 = OpFMul %float %57 %49
%59 = OpCompositeConstruct %v2float %52 %54
%60 = OpCompositeConstruct %v2float %56 %58
%61 = OpCompositeConstruct %mat2v2float %59 %60
OpStore %m2 %61
%62 = OpLoad %int %37
OpSelectionMerge %63 None
OpSwitch %62 %63 1 %64 2 %65 3 %66 4 %67
%64 = OpLabel
%68 = OpLoad %mat2v2float %m2
%72 = OpCompositeExtract %v2float %68 0
%73 = OpFAdd %v2float %72 %70
%74 = OpCompositeExtract %v2float %68 1
%75 = OpFAdd %v2float %74 %70
%76 = OpCompositeConstruct %mat2v2float %73 %75
OpStore %m2 %76
OpBranch %63
%65 = OpLabel
%77 = OpLoad %mat2v2float %m2
%78 = OpCompositeExtract %v2float %77 0
%79 = OpFSub %v2float %78 %70
%80 = OpCompositeExtract %v2float %77 1
%81 = OpFSub %v2float %80 %70
%82 = OpCompositeConstruct %mat2v2float %79 %81
OpStore %m2 %82
OpBranch %63
%66 = OpLabel
%83 = OpLoad %mat2v2float %m2
%85 = OpMatrixTimesScalar %mat2v2float %83 %float_2
OpStore %m2 %85
OpBranch %63
%67 = OpLabel
%86 = OpLoad %mat2v2float %m2
%89 = OpCompositeExtract %v2float %86 0
%90 = OpFDiv %v2float %89 %87
%91 = OpCompositeExtract %v2float %86 1
%92 = OpFDiv %v2float %91 %87
%93 = OpCompositeConstruct %mat2v2float %90 %92
OpStore %m2 %93
OpBranch %63
%63 = OpLabel
%96 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%97 = OpLoad %v2float %96
%98 = OpCompositeExtract %float %97 0
%99 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%100 = OpLoad %v2float %99
%101 = OpCompositeExtract %float %100 0
%102 = OpFOrdEqual %bool %98 %101
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%106 = OpLoad %v2float %105
%107 = OpCompositeExtract %float %106 1
%108 = OpAccessChain %_ptr_Function_v2float %42 %int_0
%109 = OpLoad %v2float %108
%110 = OpCompositeExtract %float %109 1
%111 = OpFOrdEqual %bool %107 %110
OpBranch %104
%104 = OpLabel
%112 = OpPhi %bool %false %63 %111 %103
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%116 = OpLoad %v2float %115
%117 = OpCompositeExtract %float %116 0
%118 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%119 = OpLoad %v2float %118
%120 = OpCompositeExtract %float %119 0
%121 = OpFOrdEqual %bool %117 %120
OpBranch %114
%114 = OpLabel
%122 = OpPhi %bool %false %104 %121 %113
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%126 = OpLoad %v2float %125
%127 = OpCompositeExtract %float %126 1
%128 = OpAccessChain %_ptr_Function_v2float %42 %int_1
%129 = OpLoad %v2float %128
%130 = OpCompositeExtract %float %129 1
%131 = OpFOrdEqual %bool %127 %130
OpBranch %124
%124 = OpLabel
%132 = OpPhi %bool %false %114 %131 %123
OpReturnValue %132
OpFunctionEnd
%main = OpFunction %v4float None %133
%134 = OpFunctionParameter %_ptr_Function_v2float
%135 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%220 = OpVariable %_ptr_Function_int Function
%221 = OpVariable %_ptr_Function_float Function
%222 = OpVariable %_ptr_Function_float Function
%223 = OpVariable %_ptr_Function_float Function
%224 = OpVariable %_ptr_Function_float Function
%232 = OpVariable %_ptr_Function_mat2v2float Function
%238 = OpVariable %_ptr_Function_int Function
%239 = OpVariable %_ptr_Function_float Function
%240 = OpVariable %_ptr_Function_float Function
%241 = OpVariable %_ptr_Function_float Function
%242 = OpVariable %_ptr_Function_float Function
%250 = OpVariable %_ptr_Function_mat2v2float Function
%256 = OpVariable %_ptr_Function_int Function
%257 = OpVariable %_ptr_Function_float Function
%258 = OpVariable %_ptr_Function_float Function
%259 = OpVariable %_ptr_Function_float Function
%260 = OpVariable %_ptr_Function_float Function
%269 = OpVariable %_ptr_Function_mat2v2float Function
%272 = OpVariable %_ptr_Function_v4float Function
OpStore %minus %int_2
OpStore %star %int_3
OpStore %slash %int_4
%137 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%138 = OpLoad %v4float %137
%139 = OpCompositeExtract %float %138 1
OpStore %f1 %139
%141 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%142 = OpLoad %v4float %141
%143 = OpCompositeExtract %float %142 1
%144 = OpFMul %float %float_2 %143
OpStore %f2 %144
%147 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%148 = OpLoad %v4float %147
%149 = OpCompositeExtract %float %148 1
%150 = OpFMul %float %float_3 %149
OpStore %f3 %150
%153 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 1
%156 = OpFMul %float %float_4 %155
OpStore %f4 %156
%158 = OpFAdd %float %139 %float_1
%159 = OpFAdd %float %144 %float_1
%160 = OpFAdd %float %150 %float_1
%161 = OpFAdd %float %156 %float_1
%162 = OpCompositeConstruct %v2float %158 %159
%163 = OpCompositeConstruct %v2float %160 %161
%164 = OpCompositeConstruct %mat2v2float %162 %163
OpStore %_0_expected %164
%166 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%167 = OpLoad %v4float %166
%168 = OpCompositeExtract %float %167 0
OpStore %_1_one %168
%170 = OpFMul %float %139 %168
%171 = OpFMul %float %144 %168
%172 = OpFMul %float %150 %168
%173 = OpFMul %float %156 %168
%174 = OpCompositeConstruct %v2float %170 %171
%175 = OpCompositeConstruct %v2float %172 %173
%176 = OpCompositeConstruct %mat2v2float %174 %175
OpStore %_2_m2 %176
%177 = OpFAdd %v2float %174 %70
%178 = OpFAdd %v2float %175 %70
%179 = OpCompositeConstruct %mat2v2float %177 %178
OpStore %_2_m2 %179
%180 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%181 = OpLoad %v2float %180
%182 = OpCompositeExtract %float %181 0
%183 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%184 = OpLoad %v2float %183
%185 = OpCompositeExtract %float %184 0
%186 = OpFOrdEqual %bool %182 %185
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%190 = OpLoad %v2float %189
%191 = OpCompositeExtract %float %190 1
%192 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%193 = OpLoad %v2float %192
%194 = OpCompositeExtract %float %193 1
%195 = OpFOrdEqual %bool %191 %194
OpBranch %188
%188 = OpLabel
%196 = OpPhi %bool %false %135 %195 %187
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%199 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%200 = OpLoad %v2float %199
%201 = OpCompositeExtract %float %200 0
%202 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%203 = OpLoad %v2float %202
%204 = OpCompositeExtract %float %203 0
%205 = OpFOrdEqual %bool %201 %204
OpBranch %198
%198 = OpLabel
%206 = OpPhi %bool %false %188 %205 %197
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%209 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%210 = OpLoad %v2float %209
%211 = OpCompositeExtract %float %210 1
%212 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%213 = OpLoad %v2float %212
%214 = OpCompositeExtract %float %213 1
%215 = OpFOrdEqual %bool %211 %214
OpBranch %208
%208 = OpLabel
%216 = OpPhi %bool %false %198 %215 %207
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpLoad %int %minus
OpStore %220 %219
OpStore %221 %139
OpStore %222 %144
OpStore %223 %150
OpStore %224 %156
%225 = OpFSub %float %139 %float_1
%226 = OpFSub %float %144 %float_1
%227 = OpFSub %float %150 %float_1
%228 = OpFSub %float %156 %float_1
%229 = OpCompositeConstruct %v2float %225 %226
%230 = OpCompositeConstruct %v2float %227 %228
%231 = OpCompositeConstruct %mat2v2float %229 %230
OpStore %232 %231
%233 = OpFunctionCall %bool %test_bifffff22 %220 %221 %222 %223 %224 %232
OpBranch %218
%218 = OpLabel
%234 = OpPhi %bool %false %208 %233 %217
OpSelectionMerge %236 None
OpBranchConditional %234 %235 %236
%235 = OpLabel
%237 = OpLoad %int %star
OpStore %238 %237
OpStore %239 %139
OpStore %240 %144
OpStore %241 %150
OpStore %242 %156
%243 = OpFMul %float %139 %float_2
%244 = OpFMul %float %144 %float_2
%245 = OpFMul %float %150 %float_2
%246 = OpFMul %float %156 %float_2
%247 = OpCompositeConstruct %v2float %243 %244
%248 = OpCompositeConstruct %v2float %245 %246
%249 = OpCompositeConstruct %mat2v2float %247 %248
OpStore %250 %249
%251 = OpFunctionCall %bool %test_bifffff22 %238 %239 %240 %241 %242 %250
OpBranch %236
%236 = OpLabel
%252 = OpPhi %bool %false %218 %251 %235
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
%255 = OpLoad %int %slash
OpStore %256 %255
OpStore %257 %139
OpStore %258 %144
OpStore %259 %150
OpStore %260 %156
%262 = OpFMul %float %139 %float_0_5
%263 = OpFMul %float %144 %float_0_5
%264 = OpFMul %float %150 %float_0_5
%265 = OpFMul %float %156 %float_0_5
%266 = OpCompositeConstruct %v2float %262 %263
%267 = OpCompositeConstruct %v2float %264 %265
%268 = OpCompositeConstruct %mat2v2float %266 %267
OpStore %269 %268
%270 = OpFunctionCall %bool %test_bifffff22 %256 %257 %258 %259 %260 %269
OpBranch %254
%254 = OpLabel
%271 = OpPhi %bool %false %236 %270 %253
OpSelectionMerge %276 None
OpBranchConditional %271 %274 %275
%274 = OpLabel
%277 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%278 = OpLoad %v4float %277
OpStore %272 %278
OpBranch %276
%275 = OpLabel
%279 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%280 = OpLoad %v4float %279
OpStore %272 %280
OpBranch %276
%276 = OpLabel
%281 = OpLoad %v4float %272
OpReturnValue %281
OpFunctionEnd

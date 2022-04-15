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
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %m1 "m1"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %test_comma_b "test_comma_b"
OpName %x "x"
OpName %y "y"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m3 "_2_m3"
OpName %_3_m4 "_3_m4"
OpName %_4_m5 "_4_m5"
OpName %_5_m7 "_5_m7"
OpName %_6_m9 "_6_m9"
OpName %_7_m10 "_7_m10"
OpName %_8_m11 "_8_m11"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %m1 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %491 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%37 = OpConstantComposite %v2float %float_1 %float_2
%38 = OpConstantComposite %v2float %float_3 %float_4
%39 = OpConstantComposite %mat2v2float %37 %38
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%70 = OpConstantComposite %v2float %float_6 %float_0
%71 = OpConstantComposite %v2float %float_0 %float_6
%72 = OpConstantComposite %mat2v2float %70 %71
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%95 = OpConstantComposite %v2float %float_6 %float_12
%96 = OpConstantComposite %v2float %float_18 %float_24
%97 = OpConstantComposite %mat2v2float %95 %96
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%119 = OpConstantComposite %v2float %float_4 %float_0
%120 = OpConstantComposite %v2float %float_0 %float_4
%121 = OpConstantComposite %mat2v2float %119 %120
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%145 = OpConstantComposite %v2float %float_5 %float_2
%146 = OpConstantComposite %v2float %float_3 %float_8
%147 = OpConstantComposite %mat2v2float %145 %146
%float_7 = OpConstant %float 7
%158 = OpConstantComposite %v2float %float_5 %float_6
%159 = OpConstantComposite %v2float %float_7 %float_8
%160 = OpConstantComposite %mat2v2float %158 %159
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%178 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%179 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%180 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%181 = OpConstantComposite %mat3v3float %178 %179 %180
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%203 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%204 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%205 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%206 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%207 = OpConstantComposite %mat4v4float %203 %204 %205 %206
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%231 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%232 = OpConstantComposite %mat4v4float %231 %231 %231 %231
%252 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%253 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%254 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%255 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%256 = OpConstantComposite %mat4v4float %252 %253 %254 %255
%288 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %25
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok %true
OpStore %m1 %39
%41 = OpLoad %bool %ok
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpLoad %mat2v2float %m1
%46 = OpCompositeExtract %v2float %44 0
%47 = OpFOrdEqual %v2bool %46 %37
%48 = OpAll %bool %47
%49 = OpCompositeExtract %v2float %44 1
%50 = OpFOrdEqual %v2bool %49 %38
%51 = OpAll %bool %50
%52 = OpLogicalAnd %bool %48 %51
OpBranch %43
%43 = OpLabel
%53 = OpPhi %bool %false %26 %52 %42
OpStore %ok %53
%55 = OpLoad %mat2v2float %m1
OpStore %m3 %55
%56 = OpLoad %bool %ok
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%59 = OpLoad %mat2v2float %m3
%60 = OpCompositeExtract %v2float %59 0
%61 = OpFOrdEqual %v2bool %60 %37
%62 = OpAll %bool %61
%63 = OpCompositeExtract %v2float %59 1
%64 = OpFOrdEqual %v2bool %63 %38
%65 = OpAll %bool %64
%66 = OpLogicalAnd %bool %62 %65
OpBranch %58
%58 = OpLabel
%67 = OpPhi %bool %false %43 %66 %57
OpStore %ok %67
OpStore %m4 %72
%73 = OpLoad %bool %ok
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %mat2v2float %m4
%77 = OpCompositeExtract %v2float %76 0
%78 = OpFOrdEqual %v2bool %77 %70
%79 = OpAll %bool %78
%80 = OpCompositeExtract %v2float %76 1
%81 = OpFOrdEqual %v2bool %80 %71
%82 = OpAll %bool %81
%83 = OpLogicalAnd %bool %79 %82
OpBranch %75
%75 = OpLabel
%84 = OpPhi %bool %false %58 %83 %74
OpStore %ok %84
%85 = OpLoad %mat2v2float %m3
%86 = OpLoad %mat2v2float %m4
%87 = OpMatrixTimesMatrix %mat2v2float %85 %86
OpStore %m3 %87
%88 = OpLoad %bool %ok
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpLoad %mat2v2float %m3
%98 = OpCompositeExtract %v2float %91 0
%99 = OpFOrdEqual %v2bool %98 %95
%100 = OpAll %bool %99
%101 = OpCompositeExtract %v2float %91 1
%102 = OpFOrdEqual %v2bool %101 %96
%103 = OpAll %bool %102
%104 = OpLogicalAnd %bool %100 %103
OpBranch %90
%90 = OpLabel
%105 = OpPhi %bool %false %75 %104 %89
OpStore %ok %105
%109 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%110 = OpLoad %v2float %109
%111 = OpCompositeExtract %float %110 1
%112 = OpCompositeConstruct %v2float %111 %float_0
%113 = OpCompositeConstruct %v2float %float_0 %111
%114 = OpCompositeConstruct %mat2v2float %112 %113
OpStore %m5 %114
%115 = OpLoad %bool %ok
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpLoad %mat2v2float %m5
%122 = OpCompositeExtract %v2float %118 0
%123 = OpFOrdEqual %v2bool %122 %119
%124 = OpAll %bool %123
%125 = OpCompositeExtract %v2float %118 1
%126 = OpFOrdEqual %v2bool %125 %120
%127 = OpAll %bool %126
%128 = OpLogicalAnd %bool %124 %127
OpBranch %117
%117 = OpLabel
%129 = OpPhi %bool %false %90 %128 %116
OpStore %ok %129
%130 = OpLoad %mat2v2float %m1
%131 = OpLoad %mat2v2float %m5
%132 = OpCompositeExtract %v2float %130 0
%133 = OpCompositeExtract %v2float %131 0
%134 = OpFAdd %v2float %132 %133
%135 = OpCompositeExtract %v2float %130 1
%136 = OpCompositeExtract %v2float %131 1
%137 = OpFAdd %v2float %135 %136
%138 = OpCompositeConstruct %mat2v2float %134 %137
OpStore %m1 %138
%139 = OpLoad %bool %ok
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%142 = OpLoad %mat2v2float %m1
%148 = OpCompositeExtract %v2float %142 0
%149 = OpFOrdEqual %v2bool %148 %145
%150 = OpAll %bool %149
%151 = OpCompositeExtract %v2float %142 1
%152 = OpFOrdEqual %v2bool %151 %146
%153 = OpAll %bool %152
%154 = OpLogicalAnd %bool %150 %153
OpBranch %141
%141 = OpLabel
%155 = OpPhi %bool %false %117 %154 %140
OpStore %ok %155
OpStore %m7 %160
%161 = OpLoad %bool %ok
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpLoad %mat2v2float %m7
%165 = OpCompositeExtract %v2float %164 0
%166 = OpFOrdEqual %v2bool %165 %158
%167 = OpAll %bool %166
%168 = OpCompositeExtract %v2float %164 1
%169 = OpFOrdEqual %v2bool %168 %159
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %167 %170
OpBranch %163
%163 = OpLabel
%172 = OpPhi %bool %false %141 %171 %162
OpStore %ok %172
OpStore %m9 %181
%182 = OpLoad %bool %ok
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpLoad %mat3v3float %m9
%187 = OpCompositeExtract %v3float %185 0
%188 = OpFOrdEqual %v3bool %187 %178
%189 = OpAll %bool %188
%190 = OpCompositeExtract %v3float %185 1
%191 = OpFOrdEqual %v3bool %190 %179
%192 = OpAll %bool %191
%193 = OpLogicalAnd %bool %189 %192
%194 = OpCompositeExtract %v3float %185 2
%195 = OpFOrdEqual %v3bool %194 %180
%196 = OpAll %bool %195
%197 = OpLogicalAnd %bool %193 %196
OpBranch %184
%184 = OpLabel
%198 = OpPhi %bool %false %163 %197 %183
OpStore %ok %198
OpStore %m10 %207
%208 = OpLoad %bool %ok
OpSelectionMerge %210 None
OpBranchConditional %208 %209 %210
%209 = OpLabel
%211 = OpLoad %mat4v4float %m10
%213 = OpCompositeExtract %v4float %211 0
%214 = OpFOrdEqual %v4bool %213 %203
%215 = OpAll %bool %214
%216 = OpCompositeExtract %v4float %211 1
%217 = OpFOrdEqual %v4bool %216 %204
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %215 %218
%220 = OpCompositeExtract %v4float %211 2
%221 = OpFOrdEqual %v4bool %220 %205
%222 = OpAll %bool %221
%223 = OpLogicalAnd %bool %219 %222
%224 = OpCompositeExtract %v4float %211 3
%225 = OpFOrdEqual %v4bool %224 %206
%226 = OpAll %bool %225
%227 = OpLogicalAnd %bool %223 %226
OpBranch %210
%210 = OpLabel
%228 = OpPhi %bool %false %184 %227 %209
OpStore %ok %228
OpStore %m11 %232
%233 = OpLoad %mat4v4float %m11
%234 = OpLoad %mat4v4float %m10
%235 = OpCompositeExtract %v4float %233 0
%236 = OpCompositeExtract %v4float %234 0
%237 = OpFSub %v4float %235 %236
%238 = OpCompositeExtract %v4float %233 1
%239 = OpCompositeExtract %v4float %234 1
%240 = OpFSub %v4float %238 %239
%241 = OpCompositeExtract %v4float %233 2
%242 = OpCompositeExtract %v4float %234 2
%243 = OpFSub %v4float %241 %242
%244 = OpCompositeExtract %v4float %233 3
%245 = OpCompositeExtract %v4float %234 3
%246 = OpFSub %v4float %244 %245
%247 = OpCompositeConstruct %mat4v4float %237 %240 %243 %246
OpStore %m11 %247
%248 = OpLoad %bool %ok
OpSelectionMerge %250 None
OpBranchConditional %248 %249 %250
%249 = OpLabel
%251 = OpLoad %mat4v4float %m11
%257 = OpCompositeExtract %v4float %251 0
%258 = OpFOrdEqual %v4bool %257 %252
%259 = OpAll %bool %258
%260 = OpCompositeExtract %v4float %251 1
%261 = OpFOrdEqual %v4bool %260 %253
%262 = OpAll %bool %261
%263 = OpLogicalAnd %bool %259 %262
%264 = OpCompositeExtract %v4float %251 2
%265 = OpFOrdEqual %v4bool %264 %254
%266 = OpAll %bool %265
%267 = OpLogicalAnd %bool %263 %266
%268 = OpCompositeExtract %v4float %251 3
%269 = OpFOrdEqual %v4bool %268 %255
%270 = OpAll %bool %269
%271 = OpLogicalAnd %bool %267 %270
OpBranch %250
%250 = OpLabel
%272 = OpPhi %bool %false %210 %271 %249
OpStore %ok %272
%273 = OpLoad %bool %ok
OpReturnValue %273
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%274 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
OpStore %x %39
OpStore %y %39
%277 = OpLoad %mat2v2float %x
%278 = OpLoad %mat2v2float %y
%279 = OpCompositeExtract %v2float %277 0
%280 = OpCompositeExtract %v2float %278 0
%281 = OpFOrdEqual %v2bool %279 %280
%282 = OpAll %bool %281
%283 = OpCompositeExtract %v2float %277 1
%284 = OpCompositeExtract %v2float %278 1
%285 = OpFOrdEqual %v2bool %283 %284
%286 = OpAll %bool %285
%287 = OpLogicalAnd %bool %282 %286
OpReturnValue %287
OpFunctionEnd
%main = OpFunction %v4float None %288
%289 = OpFunctionParameter %_ptr_Function_v2float
%290 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%483 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m1 %39
%293 = OpLoad %bool %_0_ok
OpSelectionMerge %295 None
OpBranchConditional %293 %294 %295
%294 = OpLabel
%296 = OpLoad %mat2v2float %_1_m1
%297 = OpCompositeExtract %v2float %296 0
%298 = OpFOrdEqual %v2bool %297 %37
%299 = OpAll %bool %298
%300 = OpCompositeExtract %v2float %296 1
%301 = OpFOrdEqual %v2bool %300 %38
%302 = OpAll %bool %301
%303 = OpLogicalAnd %bool %299 %302
OpBranch %295
%295 = OpLabel
%304 = OpPhi %bool %false %290 %303 %294
OpStore %_0_ok %304
%306 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %306
%307 = OpLoad %bool %_0_ok
OpSelectionMerge %309 None
OpBranchConditional %307 %308 %309
%308 = OpLabel
%310 = OpLoad %mat2v2float %_2_m3
%311 = OpCompositeExtract %v2float %310 0
%312 = OpFOrdEqual %v2bool %311 %37
%313 = OpAll %bool %312
%314 = OpCompositeExtract %v2float %310 1
%315 = OpFOrdEqual %v2bool %314 %38
%316 = OpAll %bool %315
%317 = OpLogicalAnd %bool %313 %316
OpBranch %309
%309 = OpLabel
%318 = OpPhi %bool %false %295 %317 %308
OpStore %_0_ok %318
OpStore %_3_m4 %72
%320 = OpLoad %bool %_0_ok
OpSelectionMerge %322 None
OpBranchConditional %320 %321 %322
%321 = OpLabel
%323 = OpLoad %mat2v2float %_3_m4
%324 = OpCompositeExtract %v2float %323 0
%325 = OpFOrdEqual %v2bool %324 %70
%326 = OpAll %bool %325
%327 = OpCompositeExtract %v2float %323 1
%328 = OpFOrdEqual %v2bool %327 %71
%329 = OpAll %bool %328
%330 = OpLogicalAnd %bool %326 %329
OpBranch %322
%322 = OpLabel
%331 = OpPhi %bool %false %309 %330 %321
OpStore %_0_ok %331
%332 = OpLoad %mat2v2float %_2_m3
%333 = OpLoad %mat2v2float %_3_m4
%334 = OpMatrixTimesMatrix %mat2v2float %332 %333
OpStore %_2_m3 %334
%335 = OpLoad %bool %_0_ok
OpSelectionMerge %337 None
OpBranchConditional %335 %336 %337
%336 = OpLabel
%338 = OpLoad %mat2v2float %_2_m3
%339 = OpCompositeExtract %v2float %338 0
%340 = OpFOrdEqual %v2bool %339 %95
%341 = OpAll %bool %340
%342 = OpCompositeExtract %v2float %338 1
%343 = OpFOrdEqual %v2bool %342 %96
%344 = OpAll %bool %343
%345 = OpLogicalAnd %bool %341 %344
OpBranch %337
%337 = OpLabel
%346 = OpPhi %bool %false %322 %345 %336
OpStore %_0_ok %346
%348 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%349 = OpLoad %v2float %348
%350 = OpCompositeExtract %float %349 1
%351 = OpCompositeConstruct %v2float %350 %float_0
%352 = OpCompositeConstruct %v2float %float_0 %350
%353 = OpCompositeConstruct %mat2v2float %351 %352
OpStore %_4_m5 %353
%354 = OpLoad %bool %_0_ok
OpSelectionMerge %356 None
OpBranchConditional %354 %355 %356
%355 = OpLabel
%357 = OpLoad %mat2v2float %_4_m5
%358 = OpCompositeExtract %v2float %357 0
%359 = OpFOrdEqual %v2bool %358 %119
%360 = OpAll %bool %359
%361 = OpCompositeExtract %v2float %357 1
%362 = OpFOrdEqual %v2bool %361 %120
%363 = OpAll %bool %362
%364 = OpLogicalAnd %bool %360 %363
OpBranch %356
%356 = OpLabel
%365 = OpPhi %bool %false %337 %364 %355
OpStore %_0_ok %365
%366 = OpLoad %mat2v2float %_1_m1
%367 = OpLoad %mat2v2float %_4_m5
%368 = OpCompositeExtract %v2float %366 0
%369 = OpCompositeExtract %v2float %367 0
%370 = OpFAdd %v2float %368 %369
%371 = OpCompositeExtract %v2float %366 1
%372 = OpCompositeExtract %v2float %367 1
%373 = OpFAdd %v2float %371 %372
%374 = OpCompositeConstruct %mat2v2float %370 %373
OpStore %_1_m1 %374
%375 = OpLoad %bool %_0_ok
OpSelectionMerge %377 None
OpBranchConditional %375 %376 %377
%376 = OpLabel
%378 = OpLoad %mat2v2float %_1_m1
%379 = OpCompositeExtract %v2float %378 0
%380 = OpFOrdEqual %v2bool %379 %145
%381 = OpAll %bool %380
%382 = OpCompositeExtract %v2float %378 1
%383 = OpFOrdEqual %v2bool %382 %146
%384 = OpAll %bool %383
%385 = OpLogicalAnd %bool %381 %384
OpBranch %377
%377 = OpLabel
%386 = OpPhi %bool %false %356 %385 %376
OpStore %_0_ok %386
OpStore %_5_m7 %160
%388 = OpLoad %bool %_0_ok
OpSelectionMerge %390 None
OpBranchConditional %388 %389 %390
%389 = OpLabel
%391 = OpLoad %mat2v2float %_5_m7
%392 = OpCompositeExtract %v2float %391 0
%393 = OpFOrdEqual %v2bool %392 %158
%394 = OpAll %bool %393
%395 = OpCompositeExtract %v2float %391 1
%396 = OpFOrdEqual %v2bool %395 %159
%397 = OpAll %bool %396
%398 = OpLogicalAnd %bool %394 %397
OpBranch %390
%390 = OpLabel
%399 = OpPhi %bool %false %377 %398 %389
OpStore %_0_ok %399
OpStore %_6_m9 %181
%401 = OpLoad %bool %_0_ok
OpSelectionMerge %403 None
OpBranchConditional %401 %402 %403
%402 = OpLabel
%404 = OpLoad %mat3v3float %_6_m9
%405 = OpCompositeExtract %v3float %404 0
%406 = OpFOrdEqual %v3bool %405 %178
%407 = OpAll %bool %406
%408 = OpCompositeExtract %v3float %404 1
%409 = OpFOrdEqual %v3bool %408 %179
%410 = OpAll %bool %409
%411 = OpLogicalAnd %bool %407 %410
%412 = OpCompositeExtract %v3float %404 2
%413 = OpFOrdEqual %v3bool %412 %180
%414 = OpAll %bool %413
%415 = OpLogicalAnd %bool %411 %414
OpBranch %403
%403 = OpLabel
%416 = OpPhi %bool %false %390 %415 %402
OpStore %_0_ok %416
OpStore %_7_m10 %207
%418 = OpLoad %bool %_0_ok
OpSelectionMerge %420 None
OpBranchConditional %418 %419 %420
%419 = OpLabel
%421 = OpLoad %mat4v4float %_7_m10
%422 = OpCompositeExtract %v4float %421 0
%423 = OpFOrdEqual %v4bool %422 %203
%424 = OpAll %bool %423
%425 = OpCompositeExtract %v4float %421 1
%426 = OpFOrdEqual %v4bool %425 %204
%427 = OpAll %bool %426
%428 = OpLogicalAnd %bool %424 %427
%429 = OpCompositeExtract %v4float %421 2
%430 = OpFOrdEqual %v4bool %429 %205
%431 = OpAll %bool %430
%432 = OpLogicalAnd %bool %428 %431
%433 = OpCompositeExtract %v4float %421 3
%434 = OpFOrdEqual %v4bool %433 %206
%435 = OpAll %bool %434
%436 = OpLogicalAnd %bool %432 %435
OpBranch %420
%420 = OpLabel
%437 = OpPhi %bool %false %403 %436 %419
OpStore %_0_ok %437
OpStore %_8_m11 %232
%439 = OpLoad %mat4v4float %_8_m11
%440 = OpLoad %mat4v4float %_7_m10
%441 = OpCompositeExtract %v4float %439 0
%442 = OpCompositeExtract %v4float %440 0
%443 = OpFSub %v4float %441 %442
%444 = OpCompositeExtract %v4float %439 1
%445 = OpCompositeExtract %v4float %440 1
%446 = OpFSub %v4float %444 %445
%447 = OpCompositeExtract %v4float %439 2
%448 = OpCompositeExtract %v4float %440 2
%449 = OpFSub %v4float %447 %448
%450 = OpCompositeExtract %v4float %439 3
%451 = OpCompositeExtract %v4float %440 3
%452 = OpFSub %v4float %450 %451
%453 = OpCompositeConstruct %mat4v4float %443 %446 %449 %452
OpStore %_8_m11 %453
%454 = OpLoad %bool %_0_ok
OpSelectionMerge %456 None
OpBranchConditional %454 %455 %456
%455 = OpLabel
%457 = OpLoad %mat4v4float %_8_m11
%458 = OpCompositeExtract %v4float %457 0
%459 = OpFOrdEqual %v4bool %458 %252
%460 = OpAll %bool %459
%461 = OpCompositeExtract %v4float %457 1
%462 = OpFOrdEqual %v4bool %461 %253
%463 = OpAll %bool %462
%464 = OpLogicalAnd %bool %460 %463
%465 = OpCompositeExtract %v4float %457 2
%466 = OpFOrdEqual %v4bool %465 %254
%467 = OpAll %bool %466
%468 = OpLogicalAnd %bool %464 %467
%469 = OpCompositeExtract %v4float %457 3
%470 = OpFOrdEqual %v4bool %469 %255
%471 = OpAll %bool %470
%472 = OpLogicalAnd %bool %468 %471
OpBranch %456
%456 = OpLabel
%473 = OpPhi %bool %false %420 %472 %455
OpStore %_0_ok %473
%474 = OpLoad %bool %_0_ok
OpSelectionMerge %476 None
OpBranchConditional %474 %475 %476
%475 = OpLabel
%477 = OpFunctionCall %bool %test_half_b
OpBranch %476
%476 = OpLabel
%478 = OpPhi %bool %false %456 %477 %475
OpSelectionMerge %480 None
OpBranchConditional %478 %479 %480
%479 = OpLabel
%481 = OpFunctionCall %bool %test_comma_b
OpBranch %480
%480 = OpLabel
%482 = OpPhi %bool %false %476 %481 %479
OpSelectionMerge %487 None
OpBranchConditional %482 %485 %486
%485 = OpLabel
%488 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%491 = OpLoad %v4float %488
OpStore %483 %491
OpBranch %487
%486 = OpLabel
%492 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%493 = OpLoad %v4float %492
OpStore %483 %493
OpBranch %487
%487 = OpLabel
%494 = OpLoad %v4float %483
OpReturnValue %494
OpFunctionEnd

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
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %516 RelaxedPrecision
OpDecorate %541 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %560 RelaxedPrecision
OpDecorate %561 RelaxedPrecision
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
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%75 = OpConstantComposite %v2float %float_6 %float_0
%76 = OpConstantComposite %v2float %float_0 %float_6
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%102 = OpConstantComposite %v2float %float_6 %float_12
%103 = OpConstantComposite %v2float %float_18 %float_24
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%128 = OpConstantComposite %v2float %float_4 %float_0
%129 = OpConstantComposite %v2float %float_0 %float_4
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%156 = OpConstantComposite %v2float %float_5 %float_2
%157 = OpConstantComposite %v2float %float_3 %float_8
%float_7 = OpConstant %float 7
%171 = OpConstantComposite %v2float %float_5 %float_6
%172 = OpConstantComposite %v2float %float_7 %float_8
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%194 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%195 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%196 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%223 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%224 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%225 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%226 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%255 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%276 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%277 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%278 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%279 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%317 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%39 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m1 %39
%41 = OpLoad %bool %ok
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpLoad %mat2v2float %m1
%46 = OpCompositeExtract %v2float %44 0
%47 = OpCompositeExtract %v2float %39 0
%48 = OpFOrdEqual %v2bool %46 %47
%49 = OpAll %bool %48
%50 = OpCompositeExtract %v2float %44 1
%51 = OpCompositeExtract %v2float %39 1
%52 = OpFOrdEqual %v2bool %50 %51
%53 = OpAll %bool %52
%54 = OpLogicalAnd %bool %49 %53
OpBranch %43
%43 = OpLabel
%55 = OpPhi %bool %false %26 %54 %42
OpStore %ok %55
%57 = OpLoad %mat2v2float %m1
OpStore %m3 %57
%58 = OpLoad %bool %ok
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %mat2v2float %m3
%62 = OpCompositeExtract %v2float %61 0
%63 = OpCompositeExtract %v2float %39 0
%64 = OpFOrdEqual %v2bool %62 %63
%65 = OpAll %bool %64
%66 = OpCompositeExtract %v2float %61 1
%67 = OpCompositeExtract %v2float %39 1
%68 = OpFOrdEqual %v2bool %66 %67
%69 = OpAll %bool %68
%70 = OpLogicalAnd %bool %65 %69
OpBranch %60
%60 = OpLabel
%71 = OpPhi %bool %false %43 %70 %59
OpStore %ok %71
%74 = OpCompositeConstruct %mat2v2float %75 %76
OpStore %m4 %74
%77 = OpLoad %bool %ok
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpLoad %mat2v2float %m4
%81 = OpCompositeConstruct %mat2v2float %75 %76
%82 = OpCompositeExtract %v2float %80 0
%83 = OpCompositeExtract %v2float %81 0
%84 = OpFOrdEqual %v2bool %82 %83
%85 = OpAll %bool %84
%86 = OpCompositeExtract %v2float %80 1
%87 = OpCompositeExtract %v2float %81 1
%88 = OpFOrdEqual %v2bool %86 %87
%89 = OpAll %bool %88
%90 = OpLogicalAnd %bool %85 %89
OpBranch %79
%79 = OpLabel
%91 = OpPhi %bool %false %60 %90 %78
OpStore %ok %91
%92 = OpLoad %mat2v2float %m3
%93 = OpLoad %mat2v2float %m4
%94 = OpMatrixTimesMatrix %mat2v2float %92 %93
OpStore %m3 %94
%95 = OpLoad %bool %ok
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpLoad %mat2v2float %m3
%104 = OpCompositeConstruct %mat2v2float %102 %103
%105 = OpCompositeExtract %v2float %98 0
%106 = OpCompositeExtract %v2float %104 0
%107 = OpFOrdEqual %v2bool %105 %106
%108 = OpAll %bool %107
%109 = OpCompositeExtract %v2float %98 1
%110 = OpCompositeExtract %v2float %104 1
%111 = OpFOrdEqual %v2bool %109 %110
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %108 %112
OpBranch %97
%97 = OpLabel
%114 = OpPhi %bool %false %79 %113 %96
OpStore %ok %114
%118 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%119 = OpLoad %v2float %118
%120 = OpCompositeExtract %float %119 1
%122 = OpCompositeConstruct %v2float %120 %float_0
%123 = OpCompositeConstruct %v2float %float_0 %120
%121 = OpCompositeConstruct %mat2v2float %122 %123
OpStore %m5 %121
%124 = OpLoad %bool %ok
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpLoad %mat2v2float %m5
%130 = OpCompositeConstruct %mat2v2float %128 %129
%131 = OpCompositeExtract %v2float %127 0
%132 = OpCompositeExtract %v2float %130 0
%133 = OpFOrdEqual %v2bool %131 %132
%134 = OpAll %bool %133
%135 = OpCompositeExtract %v2float %127 1
%136 = OpCompositeExtract %v2float %130 1
%137 = OpFOrdEqual %v2bool %135 %136
%138 = OpAll %bool %137
%139 = OpLogicalAnd %bool %134 %138
OpBranch %126
%126 = OpLabel
%140 = OpPhi %bool %false %97 %139 %125
OpStore %ok %140
%141 = OpLoad %mat2v2float %m1
%142 = OpLoad %mat2v2float %m5
%143 = OpCompositeExtract %v2float %141 0
%144 = OpCompositeExtract %v2float %142 0
%145 = OpFAdd %v2float %143 %144
%146 = OpCompositeExtract %v2float %141 1
%147 = OpCompositeExtract %v2float %142 1
%148 = OpFAdd %v2float %146 %147
%149 = OpCompositeConstruct %mat2v2float %145 %148
OpStore %m1 %149
%150 = OpLoad %bool %ok
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpLoad %mat2v2float %m1
%158 = OpCompositeConstruct %mat2v2float %156 %157
%159 = OpCompositeExtract %v2float %153 0
%160 = OpCompositeExtract %v2float %158 0
%161 = OpFOrdEqual %v2bool %159 %160
%162 = OpAll %bool %161
%163 = OpCompositeExtract %v2float %153 1
%164 = OpCompositeExtract %v2float %158 1
%165 = OpFOrdEqual %v2bool %163 %164
%166 = OpAll %bool %165
%167 = OpLogicalAnd %bool %162 %166
OpBranch %152
%152 = OpLabel
%168 = OpPhi %bool %false %126 %167 %151
OpStore %ok %168
%173 = OpCompositeConstruct %mat2v2float %171 %172
OpStore %m7 %173
%174 = OpLoad %bool %ok
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpLoad %mat2v2float %m7
%178 = OpCompositeExtract %v2float %177 0
%179 = OpCompositeExtract %v2float %173 0
%180 = OpFOrdEqual %v2bool %178 %179
%181 = OpAll %bool %180
%182 = OpCompositeExtract %v2float %177 1
%183 = OpCompositeExtract %v2float %173 1
%184 = OpFOrdEqual %v2bool %182 %183
%185 = OpAll %bool %184
%186 = OpLogicalAnd %bool %181 %185
OpBranch %176
%176 = OpLabel
%187 = OpPhi %bool %false %152 %186 %175
OpStore %ok %187
%193 = OpCompositeConstruct %mat3v3float %194 %195 %196
OpStore %m9 %193
%197 = OpLoad %bool %ok
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%200 = OpLoad %mat3v3float %m9
%201 = OpCompositeConstruct %mat3v3float %194 %195 %196
%203 = OpCompositeExtract %v3float %200 0
%204 = OpCompositeExtract %v3float %201 0
%205 = OpFOrdEqual %v3bool %203 %204
%206 = OpAll %bool %205
%207 = OpCompositeExtract %v3float %200 1
%208 = OpCompositeExtract %v3float %201 1
%209 = OpFOrdEqual %v3bool %207 %208
%210 = OpAll %bool %209
%211 = OpLogicalAnd %bool %206 %210
%212 = OpCompositeExtract %v3float %200 2
%213 = OpCompositeExtract %v3float %201 2
%214 = OpFOrdEqual %v3bool %212 %213
%215 = OpAll %bool %214
%216 = OpLogicalAnd %bool %211 %215
OpBranch %199
%199 = OpLabel
%217 = OpPhi %bool %false %176 %216 %198
OpStore %ok %217
%222 = OpCompositeConstruct %mat4v4float %223 %224 %225 %226
OpStore %m10 %222
%227 = OpLoad %bool %ok
OpSelectionMerge %229 None
OpBranchConditional %227 %228 %229
%228 = OpLabel
%230 = OpLoad %mat4v4float %m10
%231 = OpCompositeConstruct %mat4v4float %223 %224 %225 %226
%233 = OpCompositeExtract %v4float %230 0
%234 = OpCompositeExtract %v4float %231 0
%235 = OpFOrdEqual %v4bool %233 %234
%236 = OpAll %bool %235
%237 = OpCompositeExtract %v4float %230 1
%238 = OpCompositeExtract %v4float %231 1
%239 = OpFOrdEqual %v4bool %237 %238
%240 = OpAll %bool %239
%241 = OpLogicalAnd %bool %236 %240
%242 = OpCompositeExtract %v4float %230 2
%243 = OpCompositeExtract %v4float %231 2
%244 = OpFOrdEqual %v4bool %242 %243
%245 = OpAll %bool %244
%246 = OpLogicalAnd %bool %241 %245
%247 = OpCompositeExtract %v4float %230 3
%248 = OpCompositeExtract %v4float %231 3
%249 = OpFOrdEqual %v4bool %247 %248
%250 = OpAll %bool %249
%251 = OpLogicalAnd %bool %246 %250
OpBranch %229
%229 = OpLabel
%252 = OpPhi %bool %false %199 %251 %228
OpStore %ok %252
%256 = OpCompositeConstruct %mat4v4float %255 %255 %255 %255
OpStore %m11 %256
%257 = OpLoad %mat4v4float %m11
%258 = OpLoad %mat4v4float %m10
%259 = OpCompositeExtract %v4float %257 0
%260 = OpCompositeExtract %v4float %258 0
%261 = OpFSub %v4float %259 %260
%262 = OpCompositeExtract %v4float %257 1
%263 = OpCompositeExtract %v4float %258 1
%264 = OpFSub %v4float %262 %263
%265 = OpCompositeExtract %v4float %257 2
%266 = OpCompositeExtract %v4float %258 2
%267 = OpFSub %v4float %265 %266
%268 = OpCompositeExtract %v4float %257 3
%269 = OpCompositeExtract %v4float %258 3
%270 = OpFSub %v4float %268 %269
%271 = OpCompositeConstruct %mat4v4float %261 %264 %267 %270
OpStore %m11 %271
%272 = OpLoad %bool %ok
OpSelectionMerge %274 None
OpBranchConditional %272 %273 %274
%273 = OpLabel
%275 = OpLoad %mat4v4float %m11
%280 = OpCompositeConstruct %mat4v4float %276 %277 %278 %279
%281 = OpCompositeExtract %v4float %275 0
%282 = OpCompositeExtract %v4float %280 0
%283 = OpFOrdEqual %v4bool %281 %282
%284 = OpAll %bool %283
%285 = OpCompositeExtract %v4float %275 1
%286 = OpCompositeExtract %v4float %280 1
%287 = OpFOrdEqual %v4bool %285 %286
%288 = OpAll %bool %287
%289 = OpLogicalAnd %bool %284 %288
%290 = OpCompositeExtract %v4float %275 2
%291 = OpCompositeExtract %v4float %280 2
%292 = OpFOrdEqual %v4bool %290 %291
%293 = OpAll %bool %292
%294 = OpLogicalAnd %bool %289 %293
%295 = OpCompositeExtract %v4float %275 3
%296 = OpCompositeExtract %v4float %280 3
%297 = OpFOrdEqual %v4bool %295 %296
%298 = OpAll %bool %297
%299 = OpLogicalAnd %bool %294 %298
OpBranch %274
%274 = OpLabel
%300 = OpPhi %bool %false %229 %299 %273
OpStore %ok %300
%301 = OpLoad %bool %ok
OpReturnValue %301
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%302 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
%305 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %x %305
OpStore %y %305
%306 = OpLoad %mat2v2float %x
%307 = OpLoad %mat2v2float %y
%308 = OpCompositeExtract %v2float %306 0
%309 = OpCompositeExtract %v2float %307 0
%310 = OpFOrdEqual %v2bool %308 %309
%311 = OpAll %bool %310
%312 = OpCompositeExtract %v2float %306 1
%313 = OpCompositeExtract %v2float %307 1
%314 = OpFOrdEqual %v2bool %312 %313
%315 = OpAll %bool %314
%316 = OpLogicalAnd %bool %311 %315
OpReturnValue %316
OpFunctionEnd
%main = OpFunction %v4float None %317
%318 = OpFunctionParameter %_ptr_Function_v2float
%319 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%550 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%322 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %_1_m1 %322
%323 = OpLoad %bool %_0_ok
OpSelectionMerge %325 None
OpBranchConditional %323 %324 %325
%324 = OpLabel
%326 = OpLoad %mat2v2float %_1_m1
%327 = OpCompositeExtract %v2float %326 0
%328 = OpCompositeExtract %v2float %322 0
%329 = OpFOrdEqual %v2bool %327 %328
%330 = OpAll %bool %329
%331 = OpCompositeExtract %v2float %326 1
%332 = OpCompositeExtract %v2float %322 1
%333 = OpFOrdEqual %v2bool %331 %332
%334 = OpAll %bool %333
%335 = OpLogicalAnd %bool %330 %334
OpBranch %325
%325 = OpLabel
%336 = OpPhi %bool %false %319 %335 %324
OpStore %_0_ok %336
%338 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %338
%339 = OpLoad %bool %_0_ok
OpSelectionMerge %341 None
OpBranchConditional %339 %340 %341
%340 = OpLabel
%342 = OpLoad %mat2v2float %_2_m3
%343 = OpCompositeExtract %v2float %342 0
%344 = OpCompositeExtract %v2float %322 0
%345 = OpFOrdEqual %v2bool %343 %344
%346 = OpAll %bool %345
%347 = OpCompositeExtract %v2float %342 1
%348 = OpCompositeExtract %v2float %322 1
%349 = OpFOrdEqual %v2bool %347 %348
%350 = OpAll %bool %349
%351 = OpLogicalAnd %bool %346 %350
OpBranch %341
%341 = OpLabel
%352 = OpPhi %bool %false %325 %351 %340
OpStore %_0_ok %352
%354 = OpCompositeConstruct %mat2v2float %75 %76
OpStore %_3_m4 %354
%355 = OpLoad %bool %_0_ok
OpSelectionMerge %357 None
OpBranchConditional %355 %356 %357
%356 = OpLabel
%358 = OpLoad %mat2v2float %_3_m4
%359 = OpCompositeConstruct %mat2v2float %75 %76
%360 = OpCompositeExtract %v2float %358 0
%361 = OpCompositeExtract %v2float %359 0
%362 = OpFOrdEqual %v2bool %360 %361
%363 = OpAll %bool %362
%364 = OpCompositeExtract %v2float %358 1
%365 = OpCompositeExtract %v2float %359 1
%366 = OpFOrdEqual %v2bool %364 %365
%367 = OpAll %bool %366
%368 = OpLogicalAnd %bool %363 %367
OpBranch %357
%357 = OpLabel
%369 = OpPhi %bool %false %341 %368 %356
OpStore %_0_ok %369
%370 = OpLoad %mat2v2float %_2_m3
%371 = OpLoad %mat2v2float %_3_m4
%372 = OpMatrixTimesMatrix %mat2v2float %370 %371
OpStore %_2_m3 %372
%373 = OpLoad %bool %_0_ok
OpSelectionMerge %375 None
OpBranchConditional %373 %374 %375
%374 = OpLabel
%376 = OpLoad %mat2v2float %_2_m3
%377 = OpCompositeConstruct %mat2v2float %102 %103
%378 = OpCompositeExtract %v2float %376 0
%379 = OpCompositeExtract %v2float %377 0
%380 = OpFOrdEqual %v2bool %378 %379
%381 = OpAll %bool %380
%382 = OpCompositeExtract %v2float %376 1
%383 = OpCompositeExtract %v2float %377 1
%384 = OpFOrdEqual %v2bool %382 %383
%385 = OpAll %bool %384
%386 = OpLogicalAnd %bool %381 %385
OpBranch %375
%375 = OpLabel
%387 = OpPhi %bool %false %357 %386 %374
OpStore %_0_ok %387
%389 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%390 = OpLoad %v2float %389
%391 = OpCompositeExtract %float %390 1
%393 = OpCompositeConstruct %v2float %391 %float_0
%394 = OpCompositeConstruct %v2float %float_0 %391
%392 = OpCompositeConstruct %mat2v2float %393 %394
OpStore %_4_m5 %392
%395 = OpLoad %bool %_0_ok
OpSelectionMerge %397 None
OpBranchConditional %395 %396 %397
%396 = OpLabel
%398 = OpLoad %mat2v2float %_4_m5
%399 = OpCompositeConstruct %mat2v2float %128 %129
%400 = OpCompositeExtract %v2float %398 0
%401 = OpCompositeExtract %v2float %399 0
%402 = OpFOrdEqual %v2bool %400 %401
%403 = OpAll %bool %402
%404 = OpCompositeExtract %v2float %398 1
%405 = OpCompositeExtract %v2float %399 1
%406 = OpFOrdEqual %v2bool %404 %405
%407 = OpAll %bool %406
%408 = OpLogicalAnd %bool %403 %407
OpBranch %397
%397 = OpLabel
%409 = OpPhi %bool %false %375 %408 %396
OpStore %_0_ok %409
%410 = OpLoad %mat2v2float %_1_m1
%411 = OpLoad %mat2v2float %_4_m5
%412 = OpCompositeExtract %v2float %410 0
%413 = OpCompositeExtract %v2float %411 0
%414 = OpFAdd %v2float %412 %413
%415 = OpCompositeExtract %v2float %410 1
%416 = OpCompositeExtract %v2float %411 1
%417 = OpFAdd %v2float %415 %416
%418 = OpCompositeConstruct %mat2v2float %414 %417
OpStore %_1_m1 %418
%419 = OpLoad %bool %_0_ok
OpSelectionMerge %421 None
OpBranchConditional %419 %420 %421
%420 = OpLabel
%422 = OpLoad %mat2v2float %_1_m1
%423 = OpCompositeConstruct %mat2v2float %156 %157
%424 = OpCompositeExtract %v2float %422 0
%425 = OpCompositeExtract %v2float %423 0
%426 = OpFOrdEqual %v2bool %424 %425
%427 = OpAll %bool %426
%428 = OpCompositeExtract %v2float %422 1
%429 = OpCompositeExtract %v2float %423 1
%430 = OpFOrdEqual %v2bool %428 %429
%431 = OpAll %bool %430
%432 = OpLogicalAnd %bool %427 %431
OpBranch %421
%421 = OpLabel
%433 = OpPhi %bool %false %397 %432 %420
OpStore %_0_ok %433
%435 = OpCompositeConstruct %mat2v2float %171 %172
OpStore %_5_m7 %435
%436 = OpLoad %bool %_0_ok
OpSelectionMerge %438 None
OpBranchConditional %436 %437 %438
%437 = OpLabel
%439 = OpLoad %mat2v2float %_5_m7
%440 = OpCompositeExtract %v2float %439 0
%441 = OpCompositeExtract %v2float %435 0
%442 = OpFOrdEqual %v2bool %440 %441
%443 = OpAll %bool %442
%444 = OpCompositeExtract %v2float %439 1
%445 = OpCompositeExtract %v2float %435 1
%446 = OpFOrdEqual %v2bool %444 %445
%447 = OpAll %bool %446
%448 = OpLogicalAnd %bool %443 %447
OpBranch %438
%438 = OpLabel
%449 = OpPhi %bool %false %421 %448 %437
OpStore %_0_ok %449
%451 = OpCompositeConstruct %mat3v3float %194 %195 %196
OpStore %_6_m9 %451
%452 = OpLoad %bool %_0_ok
OpSelectionMerge %454 None
OpBranchConditional %452 %453 %454
%453 = OpLabel
%455 = OpLoad %mat3v3float %_6_m9
%456 = OpCompositeConstruct %mat3v3float %194 %195 %196
%457 = OpCompositeExtract %v3float %455 0
%458 = OpCompositeExtract %v3float %456 0
%459 = OpFOrdEqual %v3bool %457 %458
%460 = OpAll %bool %459
%461 = OpCompositeExtract %v3float %455 1
%462 = OpCompositeExtract %v3float %456 1
%463 = OpFOrdEqual %v3bool %461 %462
%464 = OpAll %bool %463
%465 = OpLogicalAnd %bool %460 %464
%466 = OpCompositeExtract %v3float %455 2
%467 = OpCompositeExtract %v3float %456 2
%468 = OpFOrdEqual %v3bool %466 %467
%469 = OpAll %bool %468
%470 = OpLogicalAnd %bool %465 %469
OpBranch %454
%454 = OpLabel
%471 = OpPhi %bool %false %438 %470 %453
OpStore %_0_ok %471
%473 = OpCompositeConstruct %mat4v4float %223 %224 %225 %226
OpStore %_7_m10 %473
%474 = OpLoad %bool %_0_ok
OpSelectionMerge %476 None
OpBranchConditional %474 %475 %476
%475 = OpLabel
%477 = OpLoad %mat4v4float %_7_m10
%478 = OpCompositeConstruct %mat4v4float %223 %224 %225 %226
%479 = OpCompositeExtract %v4float %477 0
%480 = OpCompositeExtract %v4float %478 0
%481 = OpFOrdEqual %v4bool %479 %480
%482 = OpAll %bool %481
%483 = OpCompositeExtract %v4float %477 1
%484 = OpCompositeExtract %v4float %478 1
%485 = OpFOrdEqual %v4bool %483 %484
%486 = OpAll %bool %485
%487 = OpLogicalAnd %bool %482 %486
%488 = OpCompositeExtract %v4float %477 2
%489 = OpCompositeExtract %v4float %478 2
%490 = OpFOrdEqual %v4bool %488 %489
%491 = OpAll %bool %490
%492 = OpLogicalAnd %bool %487 %491
%493 = OpCompositeExtract %v4float %477 3
%494 = OpCompositeExtract %v4float %478 3
%495 = OpFOrdEqual %v4bool %493 %494
%496 = OpAll %bool %495
%497 = OpLogicalAnd %bool %492 %496
OpBranch %476
%476 = OpLabel
%498 = OpPhi %bool %false %454 %497 %475
OpStore %_0_ok %498
%500 = OpCompositeConstruct %mat4v4float %255 %255 %255 %255
OpStore %_8_m11 %500
%501 = OpLoad %mat4v4float %_8_m11
%502 = OpLoad %mat4v4float %_7_m10
%503 = OpCompositeExtract %v4float %501 0
%504 = OpCompositeExtract %v4float %502 0
%505 = OpFSub %v4float %503 %504
%506 = OpCompositeExtract %v4float %501 1
%507 = OpCompositeExtract %v4float %502 1
%508 = OpFSub %v4float %506 %507
%509 = OpCompositeExtract %v4float %501 2
%510 = OpCompositeExtract %v4float %502 2
%511 = OpFSub %v4float %509 %510
%512 = OpCompositeExtract %v4float %501 3
%513 = OpCompositeExtract %v4float %502 3
%514 = OpFSub %v4float %512 %513
%515 = OpCompositeConstruct %mat4v4float %505 %508 %511 %514
OpStore %_8_m11 %515
%516 = OpLoad %bool %_0_ok
OpSelectionMerge %518 None
OpBranchConditional %516 %517 %518
%517 = OpLabel
%519 = OpLoad %mat4v4float %_8_m11
%520 = OpCompositeConstruct %mat4v4float %276 %277 %278 %279
%521 = OpCompositeExtract %v4float %519 0
%522 = OpCompositeExtract %v4float %520 0
%523 = OpFOrdEqual %v4bool %521 %522
%524 = OpAll %bool %523
%525 = OpCompositeExtract %v4float %519 1
%526 = OpCompositeExtract %v4float %520 1
%527 = OpFOrdEqual %v4bool %525 %526
%528 = OpAll %bool %527
%529 = OpLogicalAnd %bool %524 %528
%530 = OpCompositeExtract %v4float %519 2
%531 = OpCompositeExtract %v4float %520 2
%532 = OpFOrdEqual %v4bool %530 %531
%533 = OpAll %bool %532
%534 = OpLogicalAnd %bool %529 %533
%535 = OpCompositeExtract %v4float %519 3
%536 = OpCompositeExtract %v4float %520 3
%537 = OpFOrdEqual %v4bool %535 %536
%538 = OpAll %bool %537
%539 = OpLogicalAnd %bool %534 %538
OpBranch %518
%518 = OpLabel
%540 = OpPhi %bool %false %476 %539 %517
OpStore %_0_ok %540
%541 = OpLoad %bool %_0_ok
OpSelectionMerge %543 None
OpBranchConditional %541 %542 %543
%542 = OpLabel
%544 = OpFunctionCall %bool %test_half_b
OpBranch %543
%543 = OpLabel
%545 = OpPhi %bool %false %518 %544 %542
OpSelectionMerge %547 None
OpBranchConditional %545 %546 %547
%546 = OpLabel
%548 = OpFunctionCall %bool %test_comma_b
OpBranch %547
%547 = OpLabel
%549 = OpPhi %bool %false %543 %548 %546
OpSelectionMerge %554 None
OpBranchConditional %549 %552 %553
%552 = OpLabel
%555 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%558 = OpLoad %v4float %555
OpStore %550 %558
OpBranch %554
%553 = OpLabel
%559 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%560 = OpLoad %v4float %559
OpStore %550 %560
OpBranch %554
%554 = OpLabel
%561 = OpLoad %v4float %550
OpReturnValue %561
OpFunctionEnd

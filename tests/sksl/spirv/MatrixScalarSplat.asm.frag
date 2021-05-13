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
OpName %main "main"
OpName %_0_ok "_0_ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%float_4 = OpConstant %float 4
%float_6 = OpConstant %float 6
%v3bool = OpTypeVector %bool 3
%float_n2 = OpConstant %float -2
%float_n4 = OpConstant %float -4
%float_8 = OpConstant %float 8
%float_0_25 = OpConstant %float 0.25
%float_0_5 = OpConstant %float 0.5
%mat2v2float = OpTypeMatrix %v2float 2
%v2bool = OpTypeVector %bool 2
%253 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
OpStore %ok %true
%30 = OpLoad %bool %ok
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%36 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%37 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%38 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%34 = OpCompositeConstruct %mat3v3float %36 %37 %38
%41 = OpMatrixTimesScalar %mat3v3float %34 %float_4
%44 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%45 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%46 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%43 = OpCompositeConstruct %mat3v3float %44 %45 %46
%48 = OpCompositeExtract %v3float %41 0
%49 = OpCompositeExtract %v3float %43 0
%50 = OpFOrdEqual %v3bool %48 %49
%51 = OpAll %bool %50
%52 = OpCompositeExtract %v3float %41 1
%53 = OpCompositeExtract %v3float %43 1
%54 = OpFOrdEqual %v3bool %52 %53
%55 = OpAll %bool %54
%56 = OpLogicalAnd %bool %51 %55
%57 = OpCompositeExtract %v3float %41 2
%58 = OpCompositeExtract %v3float %43 2
%59 = OpFOrdEqual %v3bool %57 %58
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %56 %60
OpBranch %32
%32 = OpLabel
%62 = OpPhi %bool %false %25 %61 %31
OpStore %ok %62
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%68 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%69 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%66 = OpCompositeConstruct %mat3v3float %67 %68 %69
%70 = OpMatrixTimesScalar %mat3v3float %66 %float_4
%74 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%75 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%76 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%73 = OpCompositeConstruct %mat3v3float %74 %75 %76
%77 = OpCompositeExtract %v3float %70 0
%78 = OpCompositeExtract %v3float %73 0
%79 = OpFOrdEqual %v3bool %77 %78
%80 = OpAll %bool %79
%81 = OpCompositeExtract %v3float %70 1
%82 = OpCompositeExtract %v3float %73 1
%83 = OpFOrdEqual %v3bool %81 %82
%84 = OpAll %bool %83
%85 = OpLogicalAnd %bool %80 %84
%86 = OpCompositeExtract %v3float %70 2
%87 = OpCompositeExtract %v3float %73 2
%88 = OpFOrdEqual %v3bool %86 %87
%89 = OpAll %bool %88
%90 = OpLogicalAnd %bool %85 %89
OpBranch %65
%65 = OpLabel
%91 = OpPhi %bool %false %32 %90 %64
OpStore %ok %91
%92 = OpLoad %bool %ok
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%97 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%98 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%95 = OpCompositeConstruct %mat3v3float %96 %97 %98
%99 = OpMatrixTimesScalar %mat3v3float %95 %float_4
%102 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%103 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%104 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%101 = OpCompositeConstruct %mat3v3float %102 %103 %104
%105 = OpCompositeExtract %v3float %99 0
%106 = OpCompositeExtract %v3float %101 0
%107 = OpFOrdEqual %v3bool %105 %106
%108 = OpAll %bool %107
%109 = OpCompositeExtract %v3float %99 1
%110 = OpCompositeExtract %v3float %101 1
%111 = OpFOrdEqual %v3bool %109 %110
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %108 %112
%114 = OpCompositeExtract %v3float %99 2
%115 = OpCompositeExtract %v3float %101 2
%116 = OpFOrdEqual %v3bool %114 %115
%117 = OpAll %bool %116
%118 = OpLogicalAnd %bool %113 %117
OpBranch %94
%94 = OpLabel
%119 = OpPhi %bool %false %65 %118 %93
OpStore %ok %119
%120 = OpLoad %bool %ok
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%125 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%126 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%123 = OpCompositeConstruct %mat3v3float %124 %125 %126
%128 = OpMatrixTimesScalar %mat3v3float %123 %float_0_25
%131 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%132 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%133 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%130 = OpCompositeConstruct %mat3v3float %131 %132 %133
%134 = OpCompositeExtract %v3float %128 0
%135 = OpCompositeExtract %v3float %130 0
%136 = OpFOrdEqual %v3bool %134 %135
%137 = OpAll %bool %136
%138 = OpCompositeExtract %v3float %128 1
%139 = OpCompositeExtract %v3float %130 1
%140 = OpFOrdEqual %v3bool %138 %139
%141 = OpAll %bool %140
%142 = OpLogicalAnd %bool %137 %141
%143 = OpCompositeExtract %v3float %128 2
%144 = OpCompositeExtract %v3float %130 2
%145 = OpFOrdEqual %v3bool %143 %144
%146 = OpAll %bool %145
%147 = OpLogicalAnd %bool %142 %146
OpBranch %122
%122 = OpLabel
%148 = OpPhi %bool %false %94 %147 %121
OpStore %ok %148
%149 = OpLoad %bool %ok
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%153 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%154 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%155 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%152 = OpCompositeConstruct %mat3v3float %153 %154 %155
%156 = OpMatrixTimesScalar %mat3v3float %152 %float_4
%158 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%159 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%160 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%157 = OpCompositeConstruct %mat3v3float %158 %159 %160
%161 = OpCompositeExtract %v3float %156 0
%162 = OpCompositeExtract %v3float %157 0
%163 = OpFOrdEqual %v3bool %161 %162
%164 = OpAll %bool %163
%165 = OpCompositeExtract %v3float %156 1
%166 = OpCompositeExtract %v3float %157 1
%167 = OpFOrdEqual %v3bool %165 %166
%168 = OpAll %bool %167
%169 = OpLogicalAnd %bool %164 %168
%170 = OpCompositeExtract %v3float %156 2
%171 = OpCompositeExtract %v3float %157 2
%172 = OpFOrdEqual %v3bool %170 %171
%173 = OpAll %bool %172
%174 = OpLogicalAnd %bool %169 %173
OpBranch %151
%151 = OpLabel
%175 = OpPhi %bool %false %122 %174 %150
OpStore %ok %175
%176 = OpLoad %bool %ok
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%180 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%181 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%182 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%179 = OpCompositeConstruct %mat3v3float %180 %181 %182
%183 = OpMatrixTimesScalar %mat3v3float %179 %float_4
%185 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%186 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%187 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%184 = OpCompositeConstruct %mat3v3float %185 %186 %187
%188 = OpCompositeExtract %v3float %183 0
%189 = OpCompositeExtract %v3float %184 0
%190 = OpFOrdEqual %v3bool %188 %189
%191 = OpAll %bool %190
%192 = OpCompositeExtract %v3float %183 1
%193 = OpCompositeExtract %v3float %184 1
%194 = OpFOrdEqual %v3bool %192 %193
%195 = OpAll %bool %194
%196 = OpLogicalAnd %bool %191 %195
%197 = OpCompositeExtract %v3float %183 2
%198 = OpCompositeExtract %v3float %184 2
%199 = OpFOrdEqual %v3bool %197 %198
%200 = OpAll %bool %199
%201 = OpLogicalAnd %bool %196 %200
OpBranch %178
%178 = OpLabel
%202 = OpPhi %bool %false %151 %201 %177
OpStore %ok %202
%203 = OpLoad %bool %ok
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%207 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%208 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%209 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%206 = OpCompositeConstruct %mat3v3float %207 %208 %209
%210 = OpMatrixTimesScalar %mat3v3float %206 %float_4
%212 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%213 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%214 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%211 = OpCompositeConstruct %mat3v3float %212 %213 %214
%215 = OpCompositeExtract %v3float %210 0
%216 = OpCompositeExtract %v3float %211 0
%217 = OpFOrdEqual %v3bool %215 %216
%218 = OpAll %bool %217
%219 = OpCompositeExtract %v3float %210 1
%220 = OpCompositeExtract %v3float %211 1
%221 = OpFOrdEqual %v3bool %219 %220
%222 = OpAll %bool %221
%223 = OpLogicalAnd %bool %218 %222
%224 = OpCompositeExtract %v3float %210 2
%225 = OpCompositeExtract %v3float %211 2
%226 = OpFOrdEqual %v3bool %224 %225
%227 = OpAll %bool %226
%228 = OpLogicalAnd %bool %223 %227
OpBranch %205
%205 = OpLabel
%229 = OpPhi %bool %false %178 %228 %204
OpStore %ok %229
%230 = OpLoad %bool %ok
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%234 = OpCompositeConstruct %v2float %float_2 %float_2
%235 = OpCompositeConstruct %v2float %float_2 %float_2
%233 = OpCompositeConstruct %mat2v2float %234 %235
%237 = OpMatrixTimesScalar %mat2v2float %233 %float_4
%239 = OpCompositeConstruct %v2float %float_2 %float_2
%240 = OpCompositeConstruct %v2float %float_2 %float_2
%238 = OpCompositeConstruct %mat2v2float %239 %240
%242 = OpCompositeExtract %v2float %237 0
%243 = OpCompositeExtract %v2float %238 0
%244 = OpFOrdEqual %v2bool %242 %243
%245 = OpAll %bool %244
%246 = OpCompositeExtract %v2float %237 1
%247 = OpCompositeExtract %v2float %238 1
%248 = OpFOrdEqual %v2bool %246 %247
%249 = OpAll %bool %248
%250 = OpLogicalAnd %bool %245 %249
OpBranch %232
%232 = OpLabel
%251 = OpPhi %bool %false %205 %250 %231
OpStore %ok %251
%252 = OpLoad %bool %ok
OpReturnValue %252
OpFunctionEnd
%main = OpFunction %v4float None %253
%254 = OpFunctionParameter %_ptr_Function_v2float
%255 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%471 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%257 = OpLoad %bool %_0_ok
OpSelectionMerge %259 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
%261 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%262 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%263 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%260 = OpCompositeConstruct %mat3v3float %261 %262 %263
%264 = OpMatrixTimesScalar %mat3v3float %260 %float_4
%266 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%267 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%268 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%265 = OpCompositeConstruct %mat3v3float %266 %267 %268
%269 = OpCompositeExtract %v3float %264 0
%270 = OpCompositeExtract %v3float %265 0
%271 = OpFOrdEqual %v3bool %269 %270
%272 = OpAll %bool %271
%273 = OpCompositeExtract %v3float %264 1
%274 = OpCompositeExtract %v3float %265 1
%275 = OpFOrdEqual %v3bool %273 %274
%276 = OpAll %bool %275
%277 = OpLogicalAnd %bool %272 %276
%278 = OpCompositeExtract %v3float %264 2
%279 = OpCompositeExtract %v3float %265 2
%280 = OpFOrdEqual %v3bool %278 %279
%281 = OpAll %bool %280
%282 = OpLogicalAnd %bool %277 %281
OpBranch %259
%259 = OpLabel
%283 = OpPhi %bool %false %255 %282 %258
OpStore %_0_ok %283
%284 = OpLoad %bool %_0_ok
OpSelectionMerge %286 None
OpBranchConditional %284 %285 %286
%285 = OpLabel
%288 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%289 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%290 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%287 = OpCompositeConstruct %mat3v3float %288 %289 %290
%291 = OpMatrixTimesScalar %mat3v3float %287 %float_4
%293 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%294 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%295 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%292 = OpCompositeConstruct %mat3v3float %293 %294 %295
%296 = OpCompositeExtract %v3float %291 0
%297 = OpCompositeExtract %v3float %292 0
%298 = OpFOrdEqual %v3bool %296 %297
%299 = OpAll %bool %298
%300 = OpCompositeExtract %v3float %291 1
%301 = OpCompositeExtract %v3float %292 1
%302 = OpFOrdEqual %v3bool %300 %301
%303 = OpAll %bool %302
%304 = OpLogicalAnd %bool %299 %303
%305 = OpCompositeExtract %v3float %291 2
%306 = OpCompositeExtract %v3float %292 2
%307 = OpFOrdEqual %v3bool %305 %306
%308 = OpAll %bool %307
%309 = OpLogicalAnd %bool %304 %308
OpBranch %286
%286 = OpLabel
%310 = OpPhi %bool %false %259 %309 %285
OpStore %_0_ok %310
%311 = OpLoad %bool %_0_ok
OpSelectionMerge %313 None
OpBranchConditional %311 %312 %313
%312 = OpLabel
%315 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%316 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%317 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%314 = OpCompositeConstruct %mat3v3float %315 %316 %317
%318 = OpMatrixTimesScalar %mat3v3float %314 %float_4
%320 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%321 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%322 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%319 = OpCompositeConstruct %mat3v3float %320 %321 %322
%323 = OpCompositeExtract %v3float %318 0
%324 = OpCompositeExtract %v3float %319 0
%325 = OpFOrdEqual %v3bool %323 %324
%326 = OpAll %bool %325
%327 = OpCompositeExtract %v3float %318 1
%328 = OpCompositeExtract %v3float %319 1
%329 = OpFOrdEqual %v3bool %327 %328
%330 = OpAll %bool %329
%331 = OpLogicalAnd %bool %326 %330
%332 = OpCompositeExtract %v3float %318 2
%333 = OpCompositeExtract %v3float %319 2
%334 = OpFOrdEqual %v3bool %332 %333
%335 = OpAll %bool %334
%336 = OpLogicalAnd %bool %331 %335
OpBranch %313
%313 = OpLabel
%337 = OpPhi %bool %false %286 %336 %312
OpStore %_0_ok %337
%338 = OpLoad %bool %_0_ok
OpSelectionMerge %340 None
OpBranchConditional %338 %339 %340
%339 = OpLabel
%342 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%343 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%344 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%341 = OpCompositeConstruct %mat3v3float %342 %343 %344
%345 = OpMatrixTimesScalar %mat3v3float %341 %float_0_25
%347 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%348 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%349 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%346 = OpCompositeConstruct %mat3v3float %347 %348 %349
%350 = OpCompositeExtract %v3float %345 0
%351 = OpCompositeExtract %v3float %346 0
%352 = OpFOrdEqual %v3bool %350 %351
%353 = OpAll %bool %352
%354 = OpCompositeExtract %v3float %345 1
%355 = OpCompositeExtract %v3float %346 1
%356 = OpFOrdEqual %v3bool %354 %355
%357 = OpAll %bool %356
%358 = OpLogicalAnd %bool %353 %357
%359 = OpCompositeExtract %v3float %345 2
%360 = OpCompositeExtract %v3float %346 2
%361 = OpFOrdEqual %v3bool %359 %360
%362 = OpAll %bool %361
%363 = OpLogicalAnd %bool %358 %362
OpBranch %340
%340 = OpLabel
%364 = OpPhi %bool %false %313 %363 %339
OpStore %_0_ok %364
%365 = OpLoad %bool %_0_ok
OpSelectionMerge %367 None
OpBranchConditional %365 %366 %367
%366 = OpLabel
%369 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%370 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%371 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%368 = OpCompositeConstruct %mat3v3float %369 %370 %371
%372 = OpMatrixTimesScalar %mat3v3float %368 %float_4
%374 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%375 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%376 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%373 = OpCompositeConstruct %mat3v3float %374 %375 %376
%377 = OpCompositeExtract %v3float %372 0
%378 = OpCompositeExtract %v3float %373 0
%379 = OpFOrdEqual %v3bool %377 %378
%380 = OpAll %bool %379
%381 = OpCompositeExtract %v3float %372 1
%382 = OpCompositeExtract %v3float %373 1
%383 = OpFOrdEqual %v3bool %381 %382
%384 = OpAll %bool %383
%385 = OpLogicalAnd %bool %380 %384
%386 = OpCompositeExtract %v3float %372 2
%387 = OpCompositeExtract %v3float %373 2
%388 = OpFOrdEqual %v3bool %386 %387
%389 = OpAll %bool %388
%390 = OpLogicalAnd %bool %385 %389
OpBranch %367
%367 = OpLabel
%391 = OpPhi %bool %false %340 %390 %366
OpStore %_0_ok %391
%392 = OpLoad %bool %_0_ok
OpSelectionMerge %394 None
OpBranchConditional %392 %393 %394
%393 = OpLabel
%396 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%397 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%398 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%395 = OpCompositeConstruct %mat3v3float %396 %397 %398
%399 = OpMatrixTimesScalar %mat3v3float %395 %float_4
%401 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%402 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%403 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%400 = OpCompositeConstruct %mat3v3float %401 %402 %403
%404 = OpCompositeExtract %v3float %399 0
%405 = OpCompositeExtract %v3float %400 0
%406 = OpFOrdEqual %v3bool %404 %405
%407 = OpAll %bool %406
%408 = OpCompositeExtract %v3float %399 1
%409 = OpCompositeExtract %v3float %400 1
%410 = OpFOrdEqual %v3bool %408 %409
%411 = OpAll %bool %410
%412 = OpLogicalAnd %bool %407 %411
%413 = OpCompositeExtract %v3float %399 2
%414 = OpCompositeExtract %v3float %400 2
%415 = OpFOrdEqual %v3bool %413 %414
%416 = OpAll %bool %415
%417 = OpLogicalAnd %bool %412 %416
OpBranch %394
%394 = OpLabel
%418 = OpPhi %bool %false %367 %417 %393
OpStore %_0_ok %418
%419 = OpLoad %bool %_0_ok
OpSelectionMerge %421 None
OpBranchConditional %419 %420 %421
%420 = OpLabel
%423 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%424 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%425 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%422 = OpCompositeConstruct %mat3v3float %423 %424 %425
%426 = OpMatrixTimesScalar %mat3v3float %422 %float_4
%428 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%429 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%430 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%427 = OpCompositeConstruct %mat3v3float %428 %429 %430
%431 = OpCompositeExtract %v3float %426 0
%432 = OpCompositeExtract %v3float %427 0
%433 = OpFOrdEqual %v3bool %431 %432
%434 = OpAll %bool %433
%435 = OpCompositeExtract %v3float %426 1
%436 = OpCompositeExtract %v3float %427 1
%437 = OpFOrdEqual %v3bool %435 %436
%438 = OpAll %bool %437
%439 = OpLogicalAnd %bool %434 %438
%440 = OpCompositeExtract %v3float %426 2
%441 = OpCompositeExtract %v3float %427 2
%442 = OpFOrdEqual %v3bool %440 %441
%443 = OpAll %bool %442
%444 = OpLogicalAnd %bool %439 %443
OpBranch %421
%421 = OpLabel
%445 = OpPhi %bool %false %394 %444 %420
OpStore %_0_ok %445
%446 = OpLoad %bool %_0_ok
OpSelectionMerge %448 None
OpBranchConditional %446 %447 %448
%447 = OpLabel
%450 = OpCompositeConstruct %v2float %float_2 %float_2
%451 = OpCompositeConstruct %v2float %float_2 %float_2
%449 = OpCompositeConstruct %mat2v2float %450 %451
%452 = OpMatrixTimesScalar %mat2v2float %449 %float_4
%454 = OpCompositeConstruct %v2float %float_2 %float_2
%455 = OpCompositeConstruct %v2float %float_2 %float_2
%453 = OpCompositeConstruct %mat2v2float %454 %455
%456 = OpCompositeExtract %v2float %452 0
%457 = OpCompositeExtract %v2float %453 0
%458 = OpFOrdEqual %v2bool %456 %457
%459 = OpAll %bool %458
%460 = OpCompositeExtract %v2float %452 1
%461 = OpCompositeExtract %v2float %453 1
%462 = OpFOrdEqual %v2bool %460 %461
%463 = OpAll %bool %462
%464 = OpLogicalAnd %bool %459 %463
OpBranch %448
%448 = OpLabel
%465 = OpPhi %bool %false %421 %464 %447
OpStore %_0_ok %465
%466 = OpLoad %bool %_0_ok
OpSelectionMerge %468 None
OpBranchConditional %466 %467 %468
%467 = OpLabel
%469 = OpFunctionCall %bool %test_half_b
OpBranch %468
%468 = OpLabel
%470 = OpPhi %bool %false %448 %469 %467
OpSelectionMerge %475 None
OpBranchConditional %470 %473 %474
%473 = OpLabel
%476 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%480 = OpLoad %v4float %476
OpStore %471 %480
OpBranch %475
%474 = OpLabel
%481 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%483 = OpLoad %v4float %481
OpStore %471 %483
OpBranch %475
%475 = OpLabel
%484 = OpLoad %v4float %471
OpReturnValue %484
OpFunctionEnd

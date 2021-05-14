### Compilation failed:

error: SPIR-V validation error: Expected floating scalar or vector type as Result Type: FDiv
  %288 = OpFDiv %mat2v4float %285 %287

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
OpName %m23 "m23"
OpName %m24 "m24"
OpName %m32 "m32"
OpName %m34 "m34"
OpName %m42 "m42"
OpName %m43 "m43"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_5_m42 "_5_m42"
OpName %_6_m43 "_6_m43"
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
OpDecorate %m23 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %547 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %578 RelaxedPrecision
OpDecorate %581 RelaxedPrecision
OpDecorate %582 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_0_75 = OpConstant %float 0.75
%308 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
OpStore %ok %true
%35 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%34 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %m23 %34
%38 = OpLoad %bool %ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%41 = OpLoad %mat2v3float %m23
%43 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%44 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%42 = OpCompositeConstruct %mat2v3float %43 %44
%46 = OpCompositeExtract %v3float %41 0
%47 = OpCompositeExtract %v3float %42 0
%48 = OpFOrdEqual %v3bool %46 %47
%49 = OpAll %bool %48
%50 = OpCompositeExtract %v3float %41 1
%51 = OpCompositeExtract %v3float %42 1
%52 = OpFOrdEqual %v3bool %50 %51
%53 = OpAll %bool %52
%54 = OpLogicalAnd %bool %49 %53
OpBranch %40
%40 = OpLabel
%55 = OpPhi %bool %false %25 %54 %39
OpStore %ok %55
%61 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%62 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%60 = OpCompositeConstruct %mat2v4float %61 %62
OpStore %m24 %60
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %mat2v4float %m24
%68 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%69 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%67 = OpCompositeConstruct %mat2v4float %68 %69
%71 = OpCompositeExtract %v4float %66 0
%72 = OpCompositeExtract %v4float %67 0
%73 = OpFOrdEqual %v4bool %71 %72
%74 = OpAll %bool %73
%75 = OpCompositeExtract %v4float %66 1
%76 = OpCompositeExtract %v4float %67 1
%77 = OpFOrdEqual %v4bool %75 %76
%78 = OpAll %bool %77
%79 = OpLogicalAnd %bool %74 %78
OpBranch %65
%65 = OpLabel
%80 = OpPhi %bool %false %40 %79 %64
OpStore %ok %80
%86 = OpCompositeConstruct %v2float %float_4 %float_0
%87 = OpCompositeConstruct %v2float %float_0 %float_4
%88 = OpCompositeConstruct %v2float %float_0 %float_0
%85 = OpCompositeConstruct %mat3v2float %86 %87 %88
OpStore %m32 %85
%89 = OpLoad %bool %ok
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %mat3v2float %m32
%94 = OpCompositeConstruct %v2float %float_4 %float_0
%95 = OpCompositeConstruct %v2float %float_0 %float_4
%96 = OpCompositeConstruct %v2float %float_0 %float_0
%93 = OpCompositeConstruct %mat3v2float %94 %95 %96
%98 = OpCompositeExtract %v2float %92 0
%99 = OpCompositeExtract %v2float %93 0
%100 = OpFOrdEqual %v2bool %98 %99
%101 = OpAll %bool %100
%102 = OpCompositeExtract %v2float %92 1
%103 = OpCompositeExtract %v2float %93 1
%104 = OpFOrdEqual %v2bool %102 %103
%105 = OpAll %bool %104
%106 = OpLogicalAnd %bool %101 %105
%107 = OpCompositeExtract %v2float %92 2
%108 = OpCompositeExtract %v2float %93 2
%109 = OpFOrdEqual %v2bool %107 %108
%110 = OpAll %bool %109
%111 = OpLogicalAnd %bool %106 %110
OpBranch %91
%91 = OpLabel
%112 = OpPhi %bool %false %65 %111 %90
OpStore %ok %112
%118 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%119 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%120 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%117 = OpCompositeConstruct %mat3v4float %118 %119 %120
OpStore %m34 %117
%121 = OpLoad %bool %ok
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpLoad %mat3v4float %m34
%126 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%127 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%128 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%125 = OpCompositeConstruct %mat3v4float %126 %127 %128
%129 = OpCompositeExtract %v4float %124 0
%130 = OpCompositeExtract %v4float %125 0
%131 = OpFOrdEqual %v4bool %129 %130
%132 = OpAll %bool %131
%133 = OpCompositeExtract %v4float %124 1
%134 = OpCompositeExtract %v4float %125 1
%135 = OpFOrdEqual %v4bool %133 %134
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %132 %136
%138 = OpCompositeExtract %v4float %124 2
%139 = OpCompositeExtract %v4float %125 2
%140 = OpFOrdEqual %v4bool %138 %139
%141 = OpAll %bool %140
%142 = OpLogicalAnd %bool %137 %141
OpBranch %123
%123 = OpLabel
%143 = OpPhi %bool %false %91 %142 %122
OpStore %ok %143
%149 = OpCompositeConstruct %v2float %float_6 %float_0
%150 = OpCompositeConstruct %v2float %float_0 %float_6
%151 = OpCompositeConstruct %v2float %float_0 %float_0
%152 = OpCompositeConstruct %v2float %float_0 %float_0
%148 = OpCompositeConstruct %mat4v2float %149 %150 %151 %152
OpStore %m42 %148
%153 = OpLoad %bool %ok
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%156 = OpLoad %mat4v2float %m42
%158 = OpCompositeConstruct %v2float %float_6 %float_0
%159 = OpCompositeConstruct %v2float %float_0 %float_6
%160 = OpCompositeConstruct %v2float %float_0 %float_0
%161 = OpCompositeConstruct %v2float %float_0 %float_0
%157 = OpCompositeConstruct %mat4v2float %158 %159 %160 %161
%162 = OpCompositeExtract %v2float %156 0
%163 = OpCompositeExtract %v2float %157 0
%164 = OpFOrdEqual %v2bool %162 %163
%165 = OpAll %bool %164
%166 = OpCompositeExtract %v2float %156 1
%167 = OpCompositeExtract %v2float %157 1
%168 = OpFOrdEqual %v2bool %166 %167
%169 = OpAll %bool %168
%170 = OpLogicalAnd %bool %165 %169
%171 = OpCompositeExtract %v2float %156 2
%172 = OpCompositeExtract %v2float %157 2
%173 = OpFOrdEqual %v2bool %171 %172
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %170 %174
%176 = OpCompositeExtract %v2float %156 3
%177 = OpCompositeExtract %v2float %157 3
%178 = OpFOrdEqual %v2bool %176 %177
%179 = OpAll %bool %178
%180 = OpLogicalAnd %bool %175 %179
OpBranch %155
%155 = OpLabel
%181 = OpPhi %bool %false %123 %180 %154
OpStore %ok %181
%187 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%188 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%189 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%190 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%186 = OpCompositeConstruct %mat4v3float %187 %188 %189 %190
OpStore %m43 %186
%191 = OpLoad %bool %ok
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
%194 = OpLoad %mat4v3float %m43
%196 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%197 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%198 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%199 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%195 = OpCompositeConstruct %mat4v3float %196 %197 %198 %199
%200 = OpCompositeExtract %v3float %194 0
%201 = OpCompositeExtract %v3float %195 0
%202 = OpFOrdEqual %v3bool %200 %201
%203 = OpAll %bool %202
%204 = OpCompositeExtract %v3float %194 1
%205 = OpCompositeExtract %v3float %195 1
%206 = OpFOrdEqual %v3bool %204 %205
%207 = OpAll %bool %206
%208 = OpLogicalAnd %bool %203 %207
%209 = OpCompositeExtract %v3float %194 2
%210 = OpCompositeExtract %v3float %195 2
%211 = OpFOrdEqual %v3bool %209 %210
%212 = OpAll %bool %211
%213 = OpLogicalAnd %bool %208 %212
%214 = OpCompositeExtract %v3float %194 3
%215 = OpCompositeExtract %v3float %195 3
%216 = OpFOrdEqual %v3bool %214 %215
%217 = OpAll %bool %216
%218 = OpLogicalAnd %bool %213 %217
OpBranch %193
%193 = OpLabel
%219 = OpPhi %bool %false %155 %218 %192
OpStore %ok %219
%220 = OpLoad %mat2v3float %m23
%222 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%223 = OpCompositeConstruct %mat2v3float %222 %222
%224 = OpCompositeExtract %v3float %220 0
%225 = OpCompositeExtract %v3float %223 0
%226 = OpFAdd %v3float %224 %225
%227 = OpCompositeExtract %v3float %220 1
%228 = OpCompositeExtract %v3float %223 1
%229 = OpFAdd %v3float %227 %228
%230 = OpCompositeConstruct %mat2v3float %226 %229
OpStore %m23 %230
%231 = OpLoad %bool %ok
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%234 = OpLoad %mat2v3float %m23
%236 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%237 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%235 = OpCompositeConstruct %mat2v3float %236 %237
%238 = OpCompositeExtract %v3float %234 0
%239 = OpCompositeExtract %v3float %235 0
%240 = OpFOrdEqual %v3bool %238 %239
%241 = OpAll %bool %240
%242 = OpCompositeExtract %v3float %234 1
%243 = OpCompositeExtract %v3float %235 1
%244 = OpFOrdEqual %v3bool %242 %243
%245 = OpAll %bool %244
%246 = OpLogicalAnd %bool %241 %245
OpBranch %233
%233 = OpLabel
%247 = OpPhi %bool %false %193 %246 %232
OpStore %ok %247
%248 = OpLoad %mat3v2float %m32
%249 = OpCompositeConstruct %v2float %float_2 %float_2
%250 = OpCompositeConstruct %mat3v2float %249 %249 %249
%251 = OpCompositeExtract %v2float %248 0
%252 = OpCompositeExtract %v2float %250 0
%253 = OpFSub %v2float %251 %252
%254 = OpCompositeExtract %v2float %248 1
%255 = OpCompositeExtract %v2float %250 1
%256 = OpFSub %v2float %254 %255
%257 = OpCompositeExtract %v2float %248 2
%258 = OpCompositeExtract %v2float %250 2
%259 = OpFSub %v2float %257 %258
%260 = OpCompositeConstruct %mat3v2float %253 %256 %259
OpStore %m32 %260
%261 = OpLoad %bool %ok
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
%264 = OpLoad %mat3v2float %m32
%267 = OpCompositeConstruct %v2float %float_2 %float_n2
%268 = OpCompositeConstruct %v2float %float_n2 %float_2
%269 = OpCompositeConstruct %v2float %float_n2 %float_n2
%266 = OpCompositeConstruct %mat3v2float %267 %268 %269
%270 = OpCompositeExtract %v2float %264 0
%271 = OpCompositeExtract %v2float %266 0
%272 = OpFOrdEqual %v2bool %270 %271
%273 = OpAll %bool %272
%274 = OpCompositeExtract %v2float %264 1
%275 = OpCompositeExtract %v2float %266 1
%276 = OpFOrdEqual %v2bool %274 %275
%277 = OpAll %bool %276
%278 = OpLogicalAnd %bool %273 %277
%279 = OpCompositeExtract %v2float %264 2
%280 = OpCompositeExtract %v2float %266 2
%281 = OpFOrdEqual %v2bool %279 %280
%282 = OpAll %bool %281
%283 = OpLogicalAnd %bool %278 %282
OpBranch %263
%263 = OpLabel
%284 = OpPhi %bool %false %233 %283 %262
OpStore %ok %284
%285 = OpLoad %mat2v4float %m24
%286 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%287 = OpCompositeConstruct %mat2v4float %286 %286
%288 = OpFDiv %mat2v4float %285 %287
OpStore %m24 %288
%289 = OpLoad %bool %ok
OpSelectionMerge %291 None
OpBranchConditional %289 %290 %291
%290 = OpLabel
%292 = OpLoad %mat2v4float %m24
%295 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%296 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%294 = OpCompositeConstruct %mat2v4float %295 %296
%297 = OpCompositeExtract %v4float %292 0
%298 = OpCompositeExtract %v4float %294 0
%299 = OpFOrdEqual %v4bool %297 %298
%300 = OpAll %bool %299
%301 = OpCompositeExtract %v4float %292 1
%302 = OpCompositeExtract %v4float %294 1
%303 = OpFOrdEqual %v4bool %301 %302
%304 = OpAll %bool %303
%305 = OpLogicalAnd %bool %300 %304
OpBranch %291
%291 = OpLabel
%306 = OpPhi %bool %false %263 %305 %290
OpStore %ok %306
%307 = OpLoad %bool %ok
OpReturnValue %307
OpFunctionEnd
%main = OpFunction %v4float None %308
%309 = OpFunctionParameter %_ptr_Function_v2float
%310 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%569 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%314 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%315 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%313 = OpCompositeConstruct %mat2v3float %314 %315
OpStore %_1_m23 %313
%316 = OpLoad %bool %_0_ok
OpSelectionMerge %318 None
OpBranchConditional %316 %317 %318
%317 = OpLabel
%319 = OpLoad %mat2v3float %_1_m23
%321 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%322 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%320 = OpCompositeConstruct %mat2v3float %321 %322
%323 = OpCompositeExtract %v3float %319 0
%324 = OpCompositeExtract %v3float %320 0
%325 = OpFOrdEqual %v3bool %323 %324
%326 = OpAll %bool %325
%327 = OpCompositeExtract %v3float %319 1
%328 = OpCompositeExtract %v3float %320 1
%329 = OpFOrdEqual %v3bool %327 %328
%330 = OpAll %bool %329
%331 = OpLogicalAnd %bool %326 %330
OpBranch %318
%318 = OpLabel
%332 = OpPhi %bool %false %310 %331 %317
OpStore %_0_ok %332
%335 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%336 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%334 = OpCompositeConstruct %mat2v4float %335 %336
OpStore %_2_m24 %334
%337 = OpLoad %bool %_0_ok
OpSelectionMerge %339 None
OpBranchConditional %337 %338 %339
%338 = OpLabel
%340 = OpLoad %mat2v4float %_2_m24
%342 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%343 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%341 = OpCompositeConstruct %mat2v4float %342 %343
%344 = OpCompositeExtract %v4float %340 0
%345 = OpCompositeExtract %v4float %341 0
%346 = OpFOrdEqual %v4bool %344 %345
%347 = OpAll %bool %346
%348 = OpCompositeExtract %v4float %340 1
%349 = OpCompositeExtract %v4float %341 1
%350 = OpFOrdEqual %v4bool %348 %349
%351 = OpAll %bool %350
%352 = OpLogicalAnd %bool %347 %351
OpBranch %339
%339 = OpLabel
%353 = OpPhi %bool %false %318 %352 %338
OpStore %_0_ok %353
%356 = OpCompositeConstruct %v2float %float_4 %float_0
%357 = OpCompositeConstruct %v2float %float_0 %float_4
%358 = OpCompositeConstruct %v2float %float_0 %float_0
%355 = OpCompositeConstruct %mat3v2float %356 %357 %358
OpStore %_3_m32 %355
%359 = OpLoad %bool %_0_ok
OpSelectionMerge %361 None
OpBranchConditional %359 %360 %361
%360 = OpLabel
%362 = OpLoad %mat3v2float %_3_m32
%364 = OpCompositeConstruct %v2float %float_4 %float_0
%365 = OpCompositeConstruct %v2float %float_0 %float_4
%366 = OpCompositeConstruct %v2float %float_0 %float_0
%363 = OpCompositeConstruct %mat3v2float %364 %365 %366
%367 = OpCompositeExtract %v2float %362 0
%368 = OpCompositeExtract %v2float %363 0
%369 = OpFOrdEqual %v2bool %367 %368
%370 = OpAll %bool %369
%371 = OpCompositeExtract %v2float %362 1
%372 = OpCompositeExtract %v2float %363 1
%373 = OpFOrdEqual %v2bool %371 %372
%374 = OpAll %bool %373
%375 = OpLogicalAnd %bool %370 %374
%376 = OpCompositeExtract %v2float %362 2
%377 = OpCompositeExtract %v2float %363 2
%378 = OpFOrdEqual %v2bool %376 %377
%379 = OpAll %bool %378
%380 = OpLogicalAnd %bool %375 %379
OpBranch %361
%361 = OpLabel
%381 = OpPhi %bool %false %339 %380 %360
OpStore %_0_ok %381
%384 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%385 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%386 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%383 = OpCompositeConstruct %mat3v4float %384 %385 %386
OpStore %_4_m34 %383
%387 = OpLoad %bool %_0_ok
OpSelectionMerge %389 None
OpBranchConditional %387 %388 %389
%388 = OpLabel
%390 = OpLoad %mat3v4float %_4_m34
%392 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%393 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%394 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%391 = OpCompositeConstruct %mat3v4float %392 %393 %394
%395 = OpCompositeExtract %v4float %390 0
%396 = OpCompositeExtract %v4float %391 0
%397 = OpFOrdEqual %v4bool %395 %396
%398 = OpAll %bool %397
%399 = OpCompositeExtract %v4float %390 1
%400 = OpCompositeExtract %v4float %391 1
%401 = OpFOrdEqual %v4bool %399 %400
%402 = OpAll %bool %401
%403 = OpLogicalAnd %bool %398 %402
%404 = OpCompositeExtract %v4float %390 2
%405 = OpCompositeExtract %v4float %391 2
%406 = OpFOrdEqual %v4bool %404 %405
%407 = OpAll %bool %406
%408 = OpLogicalAnd %bool %403 %407
OpBranch %389
%389 = OpLabel
%409 = OpPhi %bool %false %361 %408 %388
OpStore %_0_ok %409
%412 = OpCompositeConstruct %v2float %float_6 %float_0
%413 = OpCompositeConstruct %v2float %float_0 %float_6
%414 = OpCompositeConstruct %v2float %float_0 %float_0
%415 = OpCompositeConstruct %v2float %float_0 %float_0
%411 = OpCompositeConstruct %mat4v2float %412 %413 %414 %415
OpStore %_5_m42 %411
%416 = OpLoad %bool %_0_ok
OpSelectionMerge %418 None
OpBranchConditional %416 %417 %418
%417 = OpLabel
%419 = OpLoad %mat4v2float %_5_m42
%421 = OpCompositeConstruct %v2float %float_6 %float_0
%422 = OpCompositeConstruct %v2float %float_0 %float_6
%423 = OpCompositeConstruct %v2float %float_0 %float_0
%424 = OpCompositeConstruct %v2float %float_0 %float_0
%420 = OpCompositeConstruct %mat4v2float %421 %422 %423 %424
%425 = OpCompositeExtract %v2float %419 0
%426 = OpCompositeExtract %v2float %420 0
%427 = OpFOrdEqual %v2bool %425 %426
%428 = OpAll %bool %427
%429 = OpCompositeExtract %v2float %419 1
%430 = OpCompositeExtract %v2float %420 1
%431 = OpFOrdEqual %v2bool %429 %430
%432 = OpAll %bool %431
%433 = OpLogicalAnd %bool %428 %432
%434 = OpCompositeExtract %v2float %419 2
%435 = OpCompositeExtract %v2float %420 2
%436 = OpFOrdEqual %v2bool %434 %435
%437 = OpAll %bool %436
%438 = OpLogicalAnd %bool %433 %437
%439 = OpCompositeExtract %v2float %419 3
%440 = OpCompositeExtract %v2float %420 3
%441 = OpFOrdEqual %v2bool %439 %440
%442 = OpAll %bool %441
%443 = OpLogicalAnd %bool %438 %442
OpBranch %418
%418 = OpLabel
%444 = OpPhi %bool %false %389 %443 %417
OpStore %_0_ok %444
%447 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%448 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%449 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%450 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%446 = OpCompositeConstruct %mat4v3float %447 %448 %449 %450
OpStore %_6_m43 %446
%451 = OpLoad %bool %_0_ok
OpSelectionMerge %453 None
OpBranchConditional %451 %452 %453
%452 = OpLabel
%454 = OpLoad %mat4v3float %_6_m43
%456 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%457 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%458 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%459 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%455 = OpCompositeConstruct %mat4v3float %456 %457 %458 %459
%460 = OpCompositeExtract %v3float %454 0
%461 = OpCompositeExtract %v3float %455 0
%462 = OpFOrdEqual %v3bool %460 %461
%463 = OpAll %bool %462
%464 = OpCompositeExtract %v3float %454 1
%465 = OpCompositeExtract %v3float %455 1
%466 = OpFOrdEqual %v3bool %464 %465
%467 = OpAll %bool %466
%468 = OpLogicalAnd %bool %463 %467
%469 = OpCompositeExtract %v3float %454 2
%470 = OpCompositeExtract %v3float %455 2
%471 = OpFOrdEqual %v3bool %469 %470
%472 = OpAll %bool %471
%473 = OpLogicalAnd %bool %468 %472
%474 = OpCompositeExtract %v3float %454 3
%475 = OpCompositeExtract %v3float %455 3
%476 = OpFOrdEqual %v3bool %474 %475
%477 = OpAll %bool %476
%478 = OpLogicalAnd %bool %473 %477
OpBranch %453
%453 = OpLabel
%479 = OpPhi %bool %false %418 %478 %452
OpStore %_0_ok %479
%480 = OpLoad %mat2v3float %_1_m23
%481 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%482 = OpCompositeConstruct %mat2v3float %481 %481
%483 = OpCompositeExtract %v3float %480 0
%484 = OpCompositeExtract %v3float %482 0
%485 = OpFAdd %v3float %483 %484
%486 = OpCompositeExtract %v3float %480 1
%487 = OpCompositeExtract %v3float %482 1
%488 = OpFAdd %v3float %486 %487
%489 = OpCompositeConstruct %mat2v3float %485 %488
OpStore %_1_m23 %489
%490 = OpLoad %bool %_0_ok
OpSelectionMerge %492 None
OpBranchConditional %490 %491 %492
%491 = OpLabel
%493 = OpLoad %mat2v3float %_1_m23
%495 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%496 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%494 = OpCompositeConstruct %mat2v3float %495 %496
%497 = OpCompositeExtract %v3float %493 0
%498 = OpCompositeExtract %v3float %494 0
%499 = OpFOrdEqual %v3bool %497 %498
%500 = OpAll %bool %499
%501 = OpCompositeExtract %v3float %493 1
%502 = OpCompositeExtract %v3float %494 1
%503 = OpFOrdEqual %v3bool %501 %502
%504 = OpAll %bool %503
%505 = OpLogicalAnd %bool %500 %504
OpBranch %492
%492 = OpLabel
%506 = OpPhi %bool %false %453 %505 %491
OpStore %_0_ok %506
%507 = OpLoad %mat3v2float %_3_m32
%508 = OpCompositeConstruct %v2float %float_2 %float_2
%509 = OpCompositeConstruct %mat3v2float %508 %508 %508
%510 = OpCompositeExtract %v2float %507 0
%511 = OpCompositeExtract %v2float %509 0
%512 = OpFSub %v2float %510 %511
%513 = OpCompositeExtract %v2float %507 1
%514 = OpCompositeExtract %v2float %509 1
%515 = OpFSub %v2float %513 %514
%516 = OpCompositeExtract %v2float %507 2
%517 = OpCompositeExtract %v2float %509 2
%518 = OpFSub %v2float %516 %517
%519 = OpCompositeConstruct %mat3v2float %512 %515 %518
OpStore %_3_m32 %519
%520 = OpLoad %bool %_0_ok
OpSelectionMerge %522 None
OpBranchConditional %520 %521 %522
%521 = OpLabel
%523 = OpLoad %mat3v2float %_3_m32
%525 = OpCompositeConstruct %v2float %float_2 %float_n2
%526 = OpCompositeConstruct %v2float %float_n2 %float_2
%527 = OpCompositeConstruct %v2float %float_n2 %float_n2
%524 = OpCompositeConstruct %mat3v2float %525 %526 %527
%528 = OpCompositeExtract %v2float %523 0
%529 = OpCompositeExtract %v2float %524 0
%530 = OpFOrdEqual %v2bool %528 %529
%531 = OpAll %bool %530
%532 = OpCompositeExtract %v2float %523 1
%533 = OpCompositeExtract %v2float %524 1
%534 = OpFOrdEqual %v2bool %532 %533
%535 = OpAll %bool %534
%536 = OpLogicalAnd %bool %531 %535
%537 = OpCompositeExtract %v2float %523 2
%538 = OpCompositeExtract %v2float %524 2
%539 = OpFOrdEqual %v2bool %537 %538
%540 = OpAll %bool %539
%541 = OpLogicalAnd %bool %536 %540
OpBranch %522
%522 = OpLabel
%542 = OpPhi %bool %false %492 %541 %521
OpStore %_0_ok %542
%543 = OpLoad %mat2v4float %_2_m24
%544 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%545 = OpCompositeConstruct %mat2v4float %544 %544
%546 = OpFDiv %mat2v4float %543 %545
OpStore %_2_m24 %546
%547 = OpLoad %bool %_0_ok
OpSelectionMerge %549 None
OpBranchConditional %547 %548 %549
%548 = OpLabel
%550 = OpLoad %mat2v4float %_2_m24
%552 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%553 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%551 = OpCompositeConstruct %mat2v4float %552 %553
%554 = OpCompositeExtract %v4float %550 0
%555 = OpCompositeExtract %v4float %551 0
%556 = OpFOrdEqual %v4bool %554 %555
%557 = OpAll %bool %556
%558 = OpCompositeExtract %v4float %550 1
%559 = OpCompositeExtract %v4float %551 1
%560 = OpFOrdEqual %v4bool %558 %559
%561 = OpAll %bool %560
%562 = OpLogicalAnd %bool %557 %561
OpBranch %549
%549 = OpLabel
%563 = OpPhi %bool %false %522 %562 %548
OpStore %_0_ok %563
%564 = OpLoad %bool %_0_ok
OpSelectionMerge %566 None
OpBranchConditional %564 %565 %566
%565 = OpLabel
%567 = OpFunctionCall %bool %test_half_b
OpBranch %566
%566 = OpLabel
%568 = OpPhi %bool %false %549 %567 %565
OpSelectionMerge %573 None
OpBranchConditional %568 %571 %572
%571 = OpLabel
%574 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%578 = OpLoad %v4float %574
OpStore %569 %578
OpBranch %573
%572 = OpLabel
%579 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%581 = OpLoad %v4float %579
OpStore %569 %581
OpBranch %573
%573 = OpLabel
%582 = OpLoad %v4float %569
OpReturnValue %582
OpFunctionEnd

1 error

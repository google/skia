OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorWhite"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %result "result"
OpName %h "h"
OpName %h2 "h2"
OpName %h3 "h3"
OpName %h4 "h4"
OpName %h2x2 "h2x2"
OpName %h3x3 "h3x3"
OpName %h4x4 "h4x4"
OpName %i "i"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %i4 "i4"
OpName %f "f"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %f2x2 "f2x2"
OpName %f3x3 "f3x3"
OpName %f4x4 "f4x4"
OpName %b "b"
OpName %b2 "b2"
OpName %b3 "b3"
OpName %b4 "b4"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %28 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%int_1 = OpConstant %int 1
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Function_bool = OpTypePointer Function %bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%true = OpConstantTrue %bool
%float_1 = OpConstant %float 1
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%h = OpVariable %_ptr_Function_float Function
%h2 = OpVariable %_ptr_Function_v2float Function
%h3 = OpVariable %_ptr_Function_v3float Function
%h4 = OpVariable %_ptr_Function_v4float Function
%h2x2 = OpVariable %_ptr_Function_mat2v2float Function
%h3x3 = OpVariable %_ptr_Function_mat3v3float Function
%h4x4 = OpVariable %_ptr_Function_mat4v4float Function
%i = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_v2int Function
%i3 = OpVariable %_ptr_Function_v3int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_v2float Function
%f3 = OpVariable %_ptr_Function_v3float Function
%f4 = OpVariable %_ptr_Function_v4float Function
%f2x2 = OpVariable %_ptr_Function_mat2v2float Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%f4x4 = OpVariable %_ptr_Function_mat4v4float Function
%b = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_v2bool Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
%ok = OpVariable %_ptr_Function_bool Function
%350 = OpVariable %_ptr_Function_v4float Function
%24 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%28 = OpLoad %v4float %24
%29 = OpCompositeExtract %float %28 0
OpStore %h %29
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%35 = OpLoad %v4float %34
%36 = OpCompositeExtract %float %35 1
%37 = OpCompositeConstruct %v2float %36 %36
OpStore %h2 %37
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%42 = OpLoad %v4float %41
%43 = OpCompositeExtract %float %42 2
%44 = OpCompositeConstruct %v3float %43 %43 %43
OpStore %h3 %44
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%47 = OpLoad %v4float %46
%48 = OpCompositeExtract %float %47 3
%49 = OpCompositeConstruct %v4float %48 %48 %48 %48
OpStore %h4 %49
%50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%51 = OpLoad %v4float %50
%52 = OpCompositeExtract %float %51 0
%53 = OpAccessChain %_ptr_Function_float %h3 %int_1
OpStore %53 %52
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%56 = OpLoad %v4float %55
%57 = OpCompositeExtract %float %56 1
%58 = OpCompositeConstruct %v2float %57 %57
%59 = OpLoad %v3float %h3
%60 = OpVectorShuffle %v3float %59 %58 3 1 4
OpStore %h3 %60
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%62 = OpLoad %v4float %61
%63 = OpCompositeExtract %float %62 3
%64 = OpCompositeConstruct %v4float %63 %63 %63 %63
%65 = OpLoad %v4float %h4
%66 = OpVectorShuffle %v4float %65 %64 6 7 4 5
OpStore %h4 %66
%70 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%71 = OpLoad %v4float %70
%72 = OpCompositeExtract %float %71 0
%75 = OpCompositeConstruct %v2float %72 %float_0
%76 = OpCompositeConstruct %v2float %float_0 %72
%73 = OpCompositeConstruct %mat2v2float %75 %76
OpStore %h2x2 %73
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%81 = OpLoad %v4float %80
%82 = OpCompositeExtract %float %81 1
%84 = OpCompositeConstruct %v3float %82 %float_0 %float_0
%85 = OpCompositeConstruct %v3float %float_0 %82 %float_0
%86 = OpCompositeConstruct %v3float %float_0 %float_0 %82
%83 = OpCompositeConstruct %mat3v3float %84 %85 %86
OpStore %h3x3 %83
%90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%91 = OpLoad %v4float %90
%92 = OpCompositeExtract %float %91 2
%94 = OpCompositeConstruct %v4float %92 %float_0 %float_0 %float_0
%95 = OpCompositeConstruct %v4float %float_0 %92 %float_0 %float_0
%96 = OpCompositeConstruct %v4float %float_0 %float_0 %92 %float_0
%97 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %92
%93 = OpCompositeConstruct %mat4v4float %94 %95 %96 %97
OpStore %h4x4 %93
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%99 = OpLoad %v4float %98
%100 = OpCompositeExtract %float %99 2
%101 = OpCompositeConstruct %v3float %100 %100 %100
%102 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
OpStore %102 %101
%103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%104 = OpLoad %v4float %103
%105 = OpCompositeExtract %float %104 0
%107 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%108 = OpAccessChain %_ptr_Function_float %107 %int_3
OpStore %108 %105
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%110 = OpLoad %v4float %109
%111 = OpCompositeExtract %float %110 0
%113 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%114 = OpAccessChain %_ptr_Function_float %113 %int_0
OpStore %114 %111
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 0
%120 = OpConvertFToS %int %119
OpStore %i %120
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%125 = OpLoad %v4float %124
%126 = OpCompositeExtract %float %125 1
%127 = OpConvertFToS %int %126
%128 = OpCompositeConstruct %v2int %127 %127
OpStore %i2 %128
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%133 = OpLoad %v4float %132
%134 = OpCompositeExtract %float %133 2
%135 = OpConvertFToS %int %134
%136 = OpCompositeConstruct %v3int %135 %135 %135
OpStore %i3 %136
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%141 = OpLoad %v4float %140
%142 = OpCompositeExtract %float %141 3
%143 = OpConvertFToS %int %142
%144 = OpCompositeConstruct %v4int %143 %143 %143 %143
OpStore %i4 %144
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%146 = OpLoad %v4float %145
%147 = OpCompositeExtract %float %146 2
%148 = OpConvertFToS %int %147
%149 = OpCompositeConstruct %v3int %148 %148 %148
%150 = OpLoad %v4int %i4
%151 = OpVectorShuffle %v4int %150 %149 4 5 6 3
OpStore %i4 %151
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 0
%155 = OpConvertFToS %int %154
%156 = OpAccessChain %_ptr_Function_int %i2 %int_1
OpStore %156 %155
%158 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%159 = OpLoad %v4float %158
%160 = OpCompositeExtract %float %159 0
OpStore %f %160
%162 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%163 = OpLoad %v4float %162
%164 = OpCompositeExtract %float %163 1
%165 = OpCompositeConstruct %v2float %164 %164
OpStore %f2 %165
%167 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%168 = OpLoad %v4float %167
%169 = OpCompositeExtract %float %168 2
%170 = OpCompositeConstruct %v3float %169 %169 %169
OpStore %f3 %170
%172 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%173 = OpLoad %v4float %172
%174 = OpCompositeExtract %float %173 3
%175 = OpCompositeConstruct %v4float %174 %174 %174 %174
OpStore %f4 %175
%176 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%177 = OpLoad %v4float %176
%178 = OpCompositeExtract %float %177 1
%179 = OpCompositeConstruct %v2float %178 %178
%180 = OpLoad %v3float %f3
%181 = OpVectorShuffle %v3float %180 %179 3 4 2
OpStore %f3 %181
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%183 = OpLoad %v4float %182
%184 = OpCompositeExtract %float %183 0
%185 = OpAccessChain %_ptr_Function_float %f2 %int_0
OpStore %185 %184
%187 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%188 = OpLoad %v4float %187
%189 = OpCompositeExtract %float %188 0
%191 = OpCompositeConstruct %v2float %189 %float_0
%192 = OpCompositeConstruct %v2float %float_0 %189
%190 = OpCompositeConstruct %mat2v2float %191 %192
OpStore %f2x2 %190
%194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%195 = OpLoad %v4float %194
%196 = OpCompositeExtract %float %195 1
%198 = OpCompositeConstruct %v3float %196 %float_0 %float_0
%199 = OpCompositeConstruct %v3float %float_0 %196 %float_0
%200 = OpCompositeConstruct %v3float %float_0 %float_0 %196
%197 = OpCompositeConstruct %mat3v3float %198 %199 %200
OpStore %f3x3 %197
%202 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%203 = OpLoad %v4float %202
%204 = OpCompositeExtract %float %203 2
%206 = OpCompositeConstruct %v4float %204 %float_0 %float_0 %float_0
%207 = OpCompositeConstruct %v4float %float_0 %204 %float_0 %float_0
%208 = OpCompositeConstruct %v4float %float_0 %float_0 %204 %float_0
%209 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %204
%205 = OpCompositeConstruct %mat4v4float %206 %207 %208 %209
OpStore %f4x4 %205
%210 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%211 = OpLoad %v4float %210
%212 = OpCompositeExtract %float %211 0
%213 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%214 = OpAccessChain %_ptr_Function_float %213 %int_0
OpStore %214 %212
%217 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%218 = OpLoad %v4float %217
%219 = OpCompositeExtract %float %218 0
%220 = OpFUnordNotEqual %bool %219 %float_0
OpStore %b %220
%224 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%225 = OpLoad %v4float %224
%226 = OpCompositeExtract %float %225 1
%227 = OpFUnordNotEqual %bool %226 %float_0
%228 = OpCompositeConstruct %v2bool %227 %227
OpStore %b2 %228
%232 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%233 = OpLoad %v4float %232
%234 = OpCompositeExtract %float %233 2
%235 = OpFUnordNotEqual %bool %234 %float_0
%236 = OpCompositeConstruct %v3bool %235 %235 %235
OpStore %b3 %236
%240 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%241 = OpLoad %v4float %240
%242 = OpCompositeExtract %float %241 3
%243 = OpFUnordNotEqual %bool %242 %float_0
%244 = OpCompositeConstruct %v4bool %243 %243 %243 %243
OpStore %b4 %244
%245 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%246 = OpLoad %v4float %245
%247 = OpCompositeExtract %float %246 1
%248 = OpFUnordNotEqual %bool %247 %float_0
%249 = OpCompositeConstruct %v2bool %248 %248
%250 = OpLoad %v4bool %b4
%251 = OpVectorShuffle %v4bool %250 %249 4 1 2 5
OpStore %b4 %251
%252 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%253 = OpLoad %v4float %252
%254 = OpCompositeExtract %float %253 0
%255 = OpFUnordNotEqual %bool %254 %float_0
%256 = OpAccessChain %_ptr_Function_bool %b3 %int_2
OpStore %256 %255
OpStore %ok %true
%259 = OpLoad %bool %ok
OpSelectionMerge %261 None
OpBranchConditional %259 %260 %261
%260 = OpLabel
%263 = OpLoad %float %h
%264 = OpLoad %v2float %h2
%265 = OpCompositeExtract %float %264 0
%266 = OpFMul %float %263 %265
%267 = OpLoad %v3float %h3
%268 = OpCompositeExtract %float %267 0
%269 = OpFMul %float %266 %268
%270 = OpLoad %v4float %h4
%271 = OpCompositeExtract %float %270 0
%272 = OpFMul %float %269 %271
%273 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%274 = OpLoad %v2float %273
%275 = OpCompositeExtract %float %274 0
%276 = OpFMul %float %272 %275
%277 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%278 = OpLoad %v3float %277
%279 = OpCompositeExtract %float %278 0
%280 = OpFMul %float %276 %279
%281 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%282 = OpLoad %v4float %281
%283 = OpCompositeExtract %float %282 0
%284 = OpFMul %float %280 %283
%285 = OpFOrdEqual %bool %float_1 %284
OpBranch %261
%261 = OpLabel
%286 = OpPhi %bool %false %19 %285 %260
OpStore %ok %286
%287 = OpLoad %bool %ok
OpSelectionMerge %289 None
OpBranchConditional %287 %288 %289
%288 = OpLabel
%290 = OpLoad %float %f
%291 = OpLoad %v2float %f2
%292 = OpCompositeExtract %float %291 0
%293 = OpFMul %float %290 %292
%294 = OpLoad %v3float %f3
%295 = OpCompositeExtract %float %294 0
%296 = OpFMul %float %293 %295
%297 = OpLoad %v4float %f4
%298 = OpCompositeExtract %float %297 0
%299 = OpFMul %float %296 %298
%300 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%301 = OpLoad %v2float %300
%302 = OpCompositeExtract %float %301 0
%303 = OpFMul %float %299 %302
%304 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%305 = OpLoad %v3float %304
%306 = OpCompositeExtract %float %305 0
%307 = OpFMul %float %303 %306
%308 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
%309 = OpLoad %v4float %308
%310 = OpCompositeExtract %float %309 0
%311 = OpFMul %float %307 %310
%312 = OpFOrdEqual %bool %float_1 %311
OpBranch %289
%289 = OpLabel
%313 = OpPhi %bool %false %261 %312 %288
OpStore %ok %313
%314 = OpLoad %bool %ok
OpSelectionMerge %316 None
OpBranchConditional %314 %315 %316
%315 = OpLabel
%317 = OpLoad %int %i
%318 = OpLoad %v2int %i2
%319 = OpCompositeExtract %int %318 0
%320 = OpIMul %int %317 %319
%321 = OpLoad %v3int %i3
%322 = OpCompositeExtract %int %321 0
%323 = OpIMul %int %320 %322
%324 = OpLoad %v4int %i4
%325 = OpCompositeExtract %int %324 0
%326 = OpIMul %int %323 %325
%327 = OpIEqual %bool %int_1 %326
OpBranch %316
%316 = OpLabel
%328 = OpPhi %bool %false %289 %327 %315
OpStore %ok %328
%329 = OpLoad %bool %ok
OpSelectionMerge %331 None
OpBranchConditional %329 %330 %331
%330 = OpLabel
%332 = OpLoad %bool %b
OpSelectionMerge %334 None
OpBranchConditional %332 %333 %334
%333 = OpLabel
%335 = OpLoad %v2bool %b2
%336 = OpCompositeExtract %bool %335 0
OpBranch %334
%334 = OpLabel
%337 = OpPhi %bool %false %330 %336 %333
OpSelectionMerge %339 None
OpBranchConditional %337 %338 %339
%338 = OpLabel
%340 = OpLoad %v3bool %b3
%341 = OpCompositeExtract %bool %340 0
OpBranch %339
%339 = OpLabel
%342 = OpPhi %bool %false %334 %341 %338
OpSelectionMerge %344 None
OpBranchConditional %342 %343 %344
%343 = OpLabel
%345 = OpLoad %v4bool %b4
%346 = OpCompositeExtract %bool %345 0
OpBranch %344
%344 = OpLabel
%347 = OpPhi %bool %false %339 %346 %343
OpBranch %331
%331 = OpLabel
%348 = OpPhi %bool %false %316 %347 %344
OpStore %ok %348
%349 = OpLoad %bool %ok
OpSelectionMerge %353 None
OpBranchConditional %349 %351 %352
%351 = OpLabel
%354 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%355 = OpLoad %v4float %354
OpStore %350 %355
OpBranch %353
%352 = OpLabel
%356 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%357 = OpLoad %v4float %356
OpStore %350 %357
OpBranch %353
%353 = OpLabel
%358 = OpLoad %v4float %350
OpReturnValue %358
OpFunctionEnd

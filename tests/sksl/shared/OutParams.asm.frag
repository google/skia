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
OpDecorate %26 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%349 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
OpStore %h %27
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%33 = OpLoad %v4float %32
%34 = OpCompositeExtract %float %33 1
%35 = OpCompositeConstruct %v2float %34 %34
OpStore %h2 %35
%39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%40 = OpLoad %v4float %39
%41 = OpCompositeExtract %float %40 2
%42 = OpCompositeConstruct %v3float %41 %41 %41
OpStore %h3 %42
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%46 = OpLoad %v4float %45
%47 = OpCompositeExtract %float %46 3
%48 = OpCompositeConstruct %v4float %47 %47 %47 %47
OpStore %h4 %48
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%50 = OpLoad %v4float %49
%51 = OpCompositeExtract %float %50 0
%52 = OpAccessChain %_ptr_Function_float %h3 %int_1
OpStore %52 %51
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%55 = OpLoad %v4float %54
%56 = OpCompositeExtract %float %55 1
%57 = OpCompositeConstruct %v2float %56 %56
%58 = OpLoad %v3float %h3
%59 = OpVectorShuffle %v3float %58 %57 3 1 4
OpStore %h3 %59
%60 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%61 = OpLoad %v4float %60
%62 = OpCompositeExtract %float %61 3
%63 = OpCompositeConstruct %v4float %62 %62 %62 %62
%64 = OpLoad %v4float %h4
%65 = OpVectorShuffle %v4float %64 %63 6 7 4 5
OpStore %h4 %65
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%70 = OpLoad %v4float %69
%71 = OpCompositeExtract %float %70 0
%74 = OpCompositeConstruct %v2float %71 %float_0
%75 = OpCompositeConstruct %v2float %float_0 %71
%72 = OpCompositeConstruct %mat2v2float %74 %75
OpStore %h2x2 %72
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%80 = OpLoad %v4float %79
%81 = OpCompositeExtract %float %80 1
%83 = OpCompositeConstruct %v3float %81 %float_0 %float_0
%84 = OpCompositeConstruct %v3float %float_0 %81 %float_0
%85 = OpCompositeConstruct %v3float %float_0 %float_0 %81
%82 = OpCompositeConstruct %mat3v3float %83 %84 %85
OpStore %h3x3 %82
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%90 = OpLoad %v4float %89
%91 = OpCompositeExtract %float %90 2
%93 = OpCompositeConstruct %v4float %91 %float_0 %float_0 %float_0
%94 = OpCompositeConstruct %v4float %float_0 %91 %float_0 %float_0
%95 = OpCompositeConstruct %v4float %float_0 %float_0 %91 %float_0
%96 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %91
%92 = OpCompositeConstruct %mat4v4float %93 %94 %95 %96
OpStore %h4x4 %92
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%98 = OpLoad %v4float %97
%99 = OpCompositeExtract %float %98 2
%100 = OpCompositeConstruct %v3float %99 %99 %99
%101 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
OpStore %101 %100
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%103 = OpLoad %v4float %102
%104 = OpCompositeExtract %float %103 0
%106 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%107 = OpAccessChain %_ptr_Function_float %106 %int_3
OpStore %107 %104
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%109 = OpLoad %v4float %108
%110 = OpCompositeExtract %float %109 0
%112 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%113 = OpAccessChain %_ptr_Function_float %112 %int_0
OpStore %113 %110
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%117 = OpLoad %v4float %116
%118 = OpCompositeExtract %float %117 0
%119 = OpConvertFToS %int %118
OpStore %i %119
%123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%124 = OpLoad %v4float %123
%125 = OpCompositeExtract %float %124 1
%126 = OpConvertFToS %int %125
%127 = OpCompositeConstruct %v2int %126 %126
OpStore %i2 %127
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%132 = OpLoad %v4float %131
%133 = OpCompositeExtract %float %132 2
%134 = OpConvertFToS %int %133
%135 = OpCompositeConstruct %v3int %134 %134 %134
OpStore %i3 %135
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%140 = OpLoad %v4float %139
%141 = OpCompositeExtract %float %140 3
%142 = OpConvertFToS %int %141
%143 = OpCompositeConstruct %v4int %142 %142 %142 %142
OpStore %i4 %143
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%145 = OpLoad %v4float %144
%146 = OpCompositeExtract %float %145 2
%147 = OpConvertFToS %int %146
%148 = OpCompositeConstruct %v3int %147 %147 %147
%149 = OpLoad %v4int %i4
%150 = OpVectorShuffle %v4int %149 %148 4 5 6 3
OpStore %i4 %150
%151 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%152 = OpLoad %v4float %151
%153 = OpCompositeExtract %float %152 0
%154 = OpConvertFToS %int %153
%155 = OpAccessChain %_ptr_Function_int %i2 %int_1
OpStore %155 %154
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%158 = OpLoad %v4float %157
%159 = OpCompositeExtract %float %158 0
OpStore %f %159
%161 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%162 = OpLoad %v4float %161
%163 = OpCompositeExtract %float %162 1
%164 = OpCompositeConstruct %v2float %163 %163
OpStore %f2 %164
%166 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%167 = OpLoad %v4float %166
%168 = OpCompositeExtract %float %167 2
%169 = OpCompositeConstruct %v3float %168 %168 %168
OpStore %f3 %169
%171 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%172 = OpLoad %v4float %171
%173 = OpCompositeExtract %float %172 3
%174 = OpCompositeConstruct %v4float %173 %173 %173 %173
OpStore %f4 %174
%175 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%176 = OpLoad %v4float %175
%177 = OpCompositeExtract %float %176 1
%178 = OpCompositeConstruct %v2float %177 %177
%179 = OpLoad %v3float %f3
%180 = OpVectorShuffle %v3float %179 %178 3 4 2
OpStore %f3 %180
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%182 = OpLoad %v4float %181
%183 = OpCompositeExtract %float %182 0
%184 = OpAccessChain %_ptr_Function_float %f2 %int_0
OpStore %184 %183
%186 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%187 = OpLoad %v4float %186
%188 = OpCompositeExtract %float %187 0
%190 = OpCompositeConstruct %v2float %188 %float_0
%191 = OpCompositeConstruct %v2float %float_0 %188
%189 = OpCompositeConstruct %mat2v2float %190 %191
OpStore %f2x2 %189
%193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%194 = OpLoad %v4float %193
%195 = OpCompositeExtract %float %194 1
%197 = OpCompositeConstruct %v3float %195 %float_0 %float_0
%198 = OpCompositeConstruct %v3float %float_0 %195 %float_0
%199 = OpCompositeConstruct %v3float %float_0 %float_0 %195
%196 = OpCompositeConstruct %mat3v3float %197 %198 %199
OpStore %f3x3 %196
%201 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%202 = OpLoad %v4float %201
%203 = OpCompositeExtract %float %202 2
%205 = OpCompositeConstruct %v4float %203 %float_0 %float_0 %float_0
%206 = OpCompositeConstruct %v4float %float_0 %203 %float_0 %float_0
%207 = OpCompositeConstruct %v4float %float_0 %float_0 %203 %float_0
%208 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %203
%204 = OpCompositeConstruct %mat4v4float %205 %206 %207 %208
OpStore %f4x4 %204
%209 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%210 = OpLoad %v4float %209
%211 = OpCompositeExtract %float %210 0
%212 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%213 = OpAccessChain %_ptr_Function_float %212 %int_0
OpStore %213 %211
%216 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%217 = OpLoad %v4float %216
%218 = OpCompositeExtract %float %217 0
%219 = OpFUnordNotEqual %bool %218 %float_0
OpStore %b %219
%223 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%224 = OpLoad %v4float %223
%225 = OpCompositeExtract %float %224 1
%226 = OpFUnordNotEqual %bool %225 %float_0
%227 = OpCompositeConstruct %v2bool %226 %226
OpStore %b2 %227
%231 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%232 = OpLoad %v4float %231
%233 = OpCompositeExtract %float %232 2
%234 = OpFUnordNotEqual %bool %233 %float_0
%235 = OpCompositeConstruct %v3bool %234 %234 %234
OpStore %b3 %235
%239 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%240 = OpLoad %v4float %239
%241 = OpCompositeExtract %float %240 3
%242 = OpFUnordNotEqual %bool %241 %float_0
%243 = OpCompositeConstruct %v4bool %242 %242 %242 %242
OpStore %b4 %243
%244 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%245 = OpLoad %v4float %244
%246 = OpCompositeExtract %float %245 1
%247 = OpFUnordNotEqual %bool %246 %float_0
%248 = OpCompositeConstruct %v2bool %247 %247
%249 = OpLoad %v4bool %b4
%250 = OpVectorShuffle %v4bool %249 %248 4 1 2 5
OpStore %b4 %250
%251 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%252 = OpLoad %v4float %251
%253 = OpCompositeExtract %float %252 0
%254 = OpFUnordNotEqual %bool %253 %float_0
%255 = OpAccessChain %_ptr_Function_bool %b3 %int_2
OpStore %255 %254
OpStore %ok %true
%258 = OpLoad %bool %ok
OpSelectionMerge %260 None
OpBranchConditional %258 %259 %260
%259 = OpLabel
%262 = OpLoad %float %h
%263 = OpLoad %v2float %h2
%264 = OpCompositeExtract %float %263 0
%265 = OpFMul %float %262 %264
%266 = OpLoad %v3float %h3
%267 = OpCompositeExtract %float %266 0
%268 = OpFMul %float %265 %267
%269 = OpLoad %v4float %h4
%270 = OpCompositeExtract %float %269 0
%271 = OpFMul %float %268 %270
%272 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%273 = OpLoad %v2float %272
%274 = OpCompositeExtract %float %273 0
%275 = OpFMul %float %271 %274
%276 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%277 = OpLoad %v3float %276
%278 = OpCompositeExtract %float %277 0
%279 = OpFMul %float %275 %278
%280 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%281 = OpLoad %v4float %280
%282 = OpCompositeExtract %float %281 0
%283 = OpFMul %float %279 %282
%284 = OpFOrdEqual %bool %float_1 %283
OpBranch %260
%260 = OpLabel
%285 = OpPhi %bool %false %19 %284 %259
OpStore %ok %285
%286 = OpLoad %bool %ok
OpSelectionMerge %288 None
OpBranchConditional %286 %287 %288
%287 = OpLabel
%289 = OpLoad %float %f
%290 = OpLoad %v2float %f2
%291 = OpCompositeExtract %float %290 0
%292 = OpFMul %float %289 %291
%293 = OpLoad %v3float %f3
%294 = OpCompositeExtract %float %293 0
%295 = OpFMul %float %292 %294
%296 = OpLoad %v4float %f4
%297 = OpCompositeExtract %float %296 0
%298 = OpFMul %float %295 %297
%299 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%300 = OpLoad %v2float %299
%301 = OpCompositeExtract %float %300 0
%302 = OpFMul %float %298 %301
%303 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%304 = OpLoad %v3float %303
%305 = OpCompositeExtract %float %304 0
%306 = OpFMul %float %302 %305
%307 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
%308 = OpLoad %v4float %307
%309 = OpCompositeExtract %float %308 0
%310 = OpFMul %float %306 %309
%311 = OpFOrdEqual %bool %float_1 %310
OpBranch %288
%288 = OpLabel
%312 = OpPhi %bool %false %260 %311 %287
OpStore %ok %312
%313 = OpLoad %bool %ok
OpSelectionMerge %315 None
OpBranchConditional %313 %314 %315
%314 = OpLabel
%316 = OpLoad %int %i
%317 = OpLoad %v2int %i2
%318 = OpCompositeExtract %int %317 0
%319 = OpIMul %int %316 %318
%320 = OpLoad %v3int %i3
%321 = OpCompositeExtract %int %320 0
%322 = OpIMul %int %319 %321
%323 = OpLoad %v4int %i4
%324 = OpCompositeExtract %int %323 0
%325 = OpIMul %int %322 %324
%326 = OpIEqual %bool %int_1 %325
OpBranch %315
%315 = OpLabel
%327 = OpPhi %bool %false %288 %326 %314
OpStore %ok %327
%328 = OpLoad %bool %ok
OpSelectionMerge %330 None
OpBranchConditional %328 %329 %330
%329 = OpLabel
%331 = OpLoad %bool %b
OpSelectionMerge %333 None
OpBranchConditional %331 %332 %333
%332 = OpLabel
%334 = OpLoad %v2bool %b2
%335 = OpCompositeExtract %bool %334 0
OpBranch %333
%333 = OpLabel
%336 = OpPhi %bool %false %329 %335 %332
OpSelectionMerge %338 None
OpBranchConditional %336 %337 %338
%337 = OpLabel
%339 = OpLoad %v3bool %b3
%340 = OpCompositeExtract %bool %339 0
OpBranch %338
%338 = OpLabel
%341 = OpPhi %bool %false %333 %340 %337
OpSelectionMerge %343 None
OpBranchConditional %341 %342 %343
%342 = OpLabel
%344 = OpLoad %v4bool %b4
%345 = OpCompositeExtract %bool %344 0
OpBranch %343
%343 = OpLabel
%346 = OpPhi %bool %false %338 %345 %342
OpBranch %330
%330 = OpLabel
%347 = OpPhi %bool %false %315 %346 %343
OpStore %ok %347
%348 = OpLoad %bool %ok
OpSelectionMerge %352 None
OpBranchConditional %348 %350 %351
%350 = OpLabel
%353 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%354 = OpLoad %v4float %353
OpStore %349 %354
OpBranch %352
%351 = OpLabel
%355 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%356 = OpLoad %v4float %355
OpStore %349 %356
OpBranch %352
%352 = OpLabel
%357 = OpLoad %v4float %349
OpReturnValue %357
OpFunctionEnd

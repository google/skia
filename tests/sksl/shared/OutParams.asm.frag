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
OpDecorate %32 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
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
%false = OpConstantFalse %bool
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
%345 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
OpStore %h %27
%31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%32 = OpLoad %v4float %31
%33 = OpCompositeExtract %float %32 1
%34 = OpCompositeConstruct %v2float %33 %33
OpStore %h2 %34
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%39 = OpLoad %v4float %38
%40 = OpCompositeExtract %float %39 2
%41 = OpCompositeConstruct %v3float %40 %40 %40
OpStore %h3 %41
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%45 = OpLoad %v4float %44
%46 = OpCompositeExtract %float %45 3
%47 = OpCompositeConstruct %v4float %46 %46 %46 %46
OpStore %h4 %47
%48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%49 = OpLoad %v4float %48
%50 = OpCompositeExtract %float %49 0
%51 = OpAccessChain %_ptr_Function_float %h3 %int_1
OpStore %51 %50
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%54 = OpLoad %v4float %53
%55 = OpCompositeExtract %float %54 1
%56 = OpCompositeConstruct %v2float %55 %55
%57 = OpLoad %v3float %h3
%58 = OpVectorShuffle %v3float %57 %56 3 1 4
OpStore %h3 %58
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%60 = OpLoad %v4float %59
%61 = OpCompositeExtract %float %60 3
%62 = OpCompositeConstruct %v4float %61 %61 %61 %61
%63 = OpLoad %v4float %h4
%64 = OpVectorShuffle %v4float %63 %62 6 7 4 5
OpStore %h4 %64
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%69 = OpLoad %v4float %68
%70 = OpCompositeExtract %float %69 0
%73 = OpCompositeConstruct %v2float %70 %float_0
%74 = OpCompositeConstruct %v2float %float_0 %70
%71 = OpCompositeConstruct %mat2v2float %73 %74
OpStore %h2x2 %71
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%79 = OpLoad %v4float %78
%80 = OpCompositeExtract %float %79 1
%82 = OpCompositeConstruct %v3float %80 %float_0 %float_0
%83 = OpCompositeConstruct %v3float %float_0 %80 %float_0
%84 = OpCompositeConstruct %v3float %float_0 %float_0 %80
%81 = OpCompositeConstruct %mat3v3float %82 %83 %84
OpStore %h3x3 %81
%88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%89 = OpLoad %v4float %88
%90 = OpCompositeExtract %float %89 2
%92 = OpCompositeConstruct %v4float %90 %float_0 %float_0 %float_0
%93 = OpCompositeConstruct %v4float %float_0 %90 %float_0 %float_0
%94 = OpCompositeConstruct %v4float %float_0 %float_0 %90 %float_0
%95 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %90
%91 = OpCompositeConstruct %mat4v4float %92 %93 %94 %95
OpStore %h4x4 %91
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%97 = OpLoad %v4float %96
%98 = OpCompositeExtract %float %97 2
%99 = OpCompositeConstruct %v3float %98 %98 %98
%100 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
OpStore %100 %99
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%102 = OpLoad %v4float %101
%103 = OpCompositeExtract %float %102 0
%105 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%106 = OpAccessChain %_ptr_Function_float %105 %int_3
OpStore %106 %103
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%108 = OpLoad %v4float %107
%109 = OpCompositeExtract %float %108 0
%111 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%112 = OpAccessChain %_ptr_Function_float %111 %int_0
OpStore %112 %109
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%116 = OpLoad %v4float %115
%117 = OpCompositeExtract %float %116 0
%118 = OpConvertFToS %int %117
OpStore %i %118
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%123 = OpLoad %v4float %122
%124 = OpCompositeExtract %float %123 1
%125 = OpConvertFToS %int %124
%126 = OpCompositeConstruct %v2int %125 %125
OpStore %i2 %126
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%131 = OpLoad %v4float %130
%132 = OpCompositeExtract %float %131 2
%133 = OpConvertFToS %int %132
%134 = OpCompositeConstruct %v3int %133 %133 %133
OpStore %i3 %134
%138 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%139 = OpLoad %v4float %138
%140 = OpCompositeExtract %float %139 3
%141 = OpConvertFToS %int %140
%142 = OpCompositeConstruct %v4int %141 %141 %141 %141
OpStore %i4 %142
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%144 = OpLoad %v4float %143
%145 = OpCompositeExtract %float %144 2
%146 = OpConvertFToS %int %145
%147 = OpCompositeConstruct %v3int %146 %146 %146
%148 = OpLoad %v4int %i4
%149 = OpVectorShuffle %v4int %148 %147 4 5 6 3
OpStore %i4 %149
%150 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%151 = OpLoad %v4float %150
%152 = OpCompositeExtract %float %151 0
%153 = OpConvertFToS %int %152
%154 = OpAccessChain %_ptr_Function_int %i2 %int_1
OpStore %154 %153
%156 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%157 = OpLoad %v4float %156
%158 = OpCompositeExtract %float %157 0
OpStore %f %158
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%161 = OpLoad %v4float %160
%162 = OpCompositeExtract %float %161 1
%163 = OpCompositeConstruct %v2float %162 %162
OpStore %f2 %163
%165 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%166 = OpLoad %v4float %165
%167 = OpCompositeExtract %float %166 2
%168 = OpCompositeConstruct %v3float %167 %167 %167
OpStore %f3 %168
%170 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%171 = OpLoad %v4float %170
%172 = OpCompositeExtract %float %171 3
%173 = OpCompositeConstruct %v4float %172 %172 %172 %172
OpStore %f4 %173
%174 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%175 = OpLoad %v4float %174
%176 = OpCompositeExtract %float %175 1
%177 = OpCompositeConstruct %v2float %176 %176
%178 = OpLoad %v3float %f3
%179 = OpVectorShuffle %v3float %178 %177 3 4 2
OpStore %f3 %179
%180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%181 = OpLoad %v4float %180
%182 = OpCompositeExtract %float %181 0
%183 = OpAccessChain %_ptr_Function_float %f2 %int_0
OpStore %183 %182
%185 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%186 = OpLoad %v4float %185
%187 = OpCompositeExtract %float %186 0
%189 = OpCompositeConstruct %v2float %187 %float_0
%190 = OpCompositeConstruct %v2float %float_0 %187
%188 = OpCompositeConstruct %mat2v2float %189 %190
OpStore %f2x2 %188
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%193 = OpLoad %v4float %192
%194 = OpCompositeExtract %float %193 1
%196 = OpCompositeConstruct %v3float %194 %float_0 %float_0
%197 = OpCompositeConstruct %v3float %float_0 %194 %float_0
%198 = OpCompositeConstruct %v3float %float_0 %float_0 %194
%195 = OpCompositeConstruct %mat3v3float %196 %197 %198
OpStore %f3x3 %195
%200 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%201 = OpLoad %v4float %200
%202 = OpCompositeExtract %float %201 2
%204 = OpCompositeConstruct %v4float %202 %float_0 %float_0 %float_0
%205 = OpCompositeConstruct %v4float %float_0 %202 %float_0 %float_0
%206 = OpCompositeConstruct %v4float %float_0 %float_0 %202 %float_0
%207 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %202
%203 = OpCompositeConstruct %mat4v4float %204 %205 %206 %207
OpStore %f4x4 %203
%208 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%209 = OpLoad %v4float %208
%210 = OpCompositeExtract %float %209 0
%211 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%212 = OpAccessChain %_ptr_Function_float %211 %int_0
OpStore %212 %210
%215 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%216 = OpLoad %v4float %215
%217 = OpCompositeExtract %float %216 0
%218 = OpFUnordNotEqual %bool %217 %float_0
OpStore %b %218
%222 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%223 = OpLoad %v4float %222
%224 = OpCompositeExtract %float %223 1
%225 = OpFUnordNotEqual %bool %224 %float_0
%226 = OpCompositeConstruct %v2bool %225 %225
OpStore %b2 %226
%230 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%231 = OpLoad %v4float %230
%232 = OpCompositeExtract %float %231 2
%233 = OpFUnordNotEqual %bool %232 %float_0
%234 = OpCompositeConstruct %v3bool %233 %233 %233
OpStore %b3 %234
%238 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%239 = OpLoad %v4float %238
%240 = OpCompositeExtract %float %239 3
%241 = OpFUnordNotEqual %bool %240 %float_0
%242 = OpCompositeConstruct %v4bool %241 %241 %241 %241
OpStore %b4 %242
%243 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%244 = OpLoad %v4float %243
%245 = OpCompositeExtract %float %244 1
%246 = OpFUnordNotEqual %bool %245 %float_0
%247 = OpCompositeConstruct %v2bool %246 %246
%248 = OpLoad %v4bool %b4
%249 = OpVectorShuffle %v4bool %248 %247 4 1 2 5
OpStore %b4 %249
%250 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%251 = OpLoad %v4float %250
%252 = OpCompositeExtract %float %251 0
%253 = OpFUnordNotEqual %bool %252 %float_0
%254 = OpAccessChain %_ptr_Function_bool %b3 %int_2
OpStore %254 %253
OpStore %ok %true
%258 = OpLoad %float %h
%259 = OpLoad %v2float %h2
%260 = OpCompositeExtract %float %259 0
%261 = OpFMul %float %258 %260
%262 = OpLoad %v3float %h3
%263 = OpCompositeExtract %float %262 0
%264 = OpFMul %float %261 %263
%265 = OpLoad %v4float %h4
%266 = OpCompositeExtract %float %265 0
%267 = OpFMul %float %264 %266
%268 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%269 = OpLoad %v2float %268
%270 = OpCompositeExtract %float %269 0
%271 = OpFMul %float %267 %270
%272 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%273 = OpLoad %v3float %272
%274 = OpCompositeExtract %float %273 0
%275 = OpFMul %float %271 %274
%276 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%277 = OpLoad %v4float %276
%278 = OpCompositeExtract %float %277 0
%279 = OpFMul %float %275 %278
%280 = OpFOrdEqual %bool %float_1 %279
OpStore %ok %280
%282 = OpLoad %bool %ok
OpSelectionMerge %284 None
OpBranchConditional %282 %283 %284
%283 = OpLabel
%285 = OpLoad %float %f
%286 = OpLoad %v2float %f2
%287 = OpCompositeExtract %float %286 0
%288 = OpFMul %float %285 %287
%289 = OpLoad %v3float %f3
%290 = OpCompositeExtract %float %289 0
%291 = OpFMul %float %288 %290
%292 = OpLoad %v4float %f4
%293 = OpCompositeExtract %float %292 0
%294 = OpFMul %float %291 %293
%295 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%296 = OpLoad %v2float %295
%297 = OpCompositeExtract %float %296 0
%298 = OpFMul %float %294 %297
%299 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%300 = OpLoad %v3float %299
%301 = OpCompositeExtract %float %300 0
%302 = OpFMul %float %298 %301
%303 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
%304 = OpLoad %v4float %303
%305 = OpCompositeExtract %float %304 0
%306 = OpFMul %float %302 %305
%307 = OpFOrdEqual %bool %float_1 %306
OpBranch %284
%284 = OpLabel
%308 = OpPhi %bool %false %19 %307 %283
OpStore %ok %308
%309 = OpLoad %bool %ok
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%312 = OpLoad %int %i
%313 = OpLoad %v2int %i2
%314 = OpCompositeExtract %int %313 0
%315 = OpIMul %int %312 %314
%316 = OpLoad %v3int %i3
%317 = OpCompositeExtract %int %316 0
%318 = OpIMul %int %315 %317
%319 = OpLoad %v4int %i4
%320 = OpCompositeExtract %int %319 0
%321 = OpIMul %int %318 %320
%322 = OpIEqual %bool %int_1 %321
OpBranch %311
%311 = OpLabel
%323 = OpPhi %bool %false %284 %322 %310
OpStore %ok %323
%324 = OpLoad %bool %ok
OpSelectionMerge %326 None
OpBranchConditional %324 %325 %326
%325 = OpLabel
%327 = OpLoad %bool %b
OpSelectionMerge %329 None
OpBranchConditional %327 %328 %329
%328 = OpLabel
%330 = OpLoad %v2bool %b2
%331 = OpCompositeExtract %bool %330 0
OpBranch %329
%329 = OpLabel
%332 = OpPhi %bool %false %325 %331 %328
OpSelectionMerge %334 None
OpBranchConditional %332 %333 %334
%333 = OpLabel
%335 = OpLoad %v3bool %b3
%336 = OpCompositeExtract %bool %335 0
OpBranch %334
%334 = OpLabel
%337 = OpPhi %bool %false %329 %336 %333
OpSelectionMerge %339 None
OpBranchConditional %337 %338 %339
%338 = OpLabel
%340 = OpLoad %v4bool %b4
%341 = OpCompositeExtract %bool %340 0
OpBranch %339
%339 = OpLabel
%342 = OpPhi %bool %false %334 %341 %338
OpBranch %326
%326 = OpLabel
%343 = OpPhi %bool %false %311 %342 %339
OpStore %ok %343
%344 = OpLoad %bool %ok
OpSelectionMerge %348 None
OpBranchConditional %344 %346 %347
%346 = OpLabel
%349 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%350 = OpLoad %v4float %349
OpStore %345 %350
OpBranch %348
%347 = OpLabel
%351 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%352 = OpLoad %v4float %351
OpStore %345 %352
OpBranch %348
%348 = OpLabel
%353 = OpLoad %v4float %345
OpReturnValue %353
OpFunctionEnd

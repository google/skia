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
OpName %out_half "out_half"
OpName %out_half2 "out_half2"
OpName %out_half3 "out_half3"
OpName %out_half4 "out_half4"
OpName %out_half2x2 "out_half2x2"
OpName %out_half3x3 "out_half3x3"
OpName %out_half4x4 "out_half4x4"
OpName %out_int "out_int"
OpName %out_int2 "out_int2"
OpName %out_int3 "out_int3"
OpName %out_int4 "out_int4"
OpName %out_float "out_float"
OpName %out_float2 "out_float2"
OpName %out_float3 "out_float3"
OpName %out_float4 "out_float4"
OpName %out_float2x2 "out_float2x2"
OpName %out_float3x3 "out_float3x3"
OpName %out_float4x4 "out_float4x4"
OpName %out_bool "out_bool"
OpName %out_bool2 "out_bool2"
OpName %out_bool3 "out_bool3"
OpName %out_bool4 "out_bool4"
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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %32 Binding 0
OpDecorate %32 DescriptorSet 0
OpDecorate %48 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%32 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%37 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%40 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%51 = OpTypeFunction %void %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%60 = OpTypeFunction %void %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%68 = OpTypeFunction %void %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%77 = OpTypeFunction %void %_ptr_Function_mat2v2float
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%89 = OpTypeFunction %void %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%101 = OpTypeFunction %void %_ptr_Function_mat4v4float
%_ptr_Function_int = OpTypePointer Function %int
%113 = OpTypeFunction %void %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%122 = OpTypeFunction %void %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%132 = OpTypeFunction %void %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%142 = OpTypeFunction %void %_ptr_Function_v4int
%_ptr_Function_bool = OpTypePointer Function %bool
%201 = OpTypeFunction %void %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%210 = OpTypeFunction %void %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%220 = OpTypeFunction %void %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%230 = OpTypeFunction %void %_ptr_Function_v4bool
%239 = OpTypeFunction %v4float
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_entrypoint = OpFunction %void None %37
%38 = OpLabel
%39 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %39
OpReturn
OpFunctionEnd
%out_half = OpFunction %void None %40
%42 = OpFunctionParameter %_ptr_Function_float
%43 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%48 = OpLoad %v4float %44
%49 = OpCompositeExtract %float %48 0
OpStore %42 %49
OpReturn
OpFunctionEnd
%out_half2 = OpFunction %void None %51
%53 = OpFunctionParameter %_ptr_Function_v2float
%54 = OpLabel
%55 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%56 = OpLoad %v4float %55
%57 = OpCompositeExtract %float %56 1
%58 = OpCompositeConstruct %v2float %57 %57
OpStore %53 %58
OpReturn
OpFunctionEnd
%out_half3 = OpFunction %void None %60
%62 = OpFunctionParameter %_ptr_Function_v3float
%63 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%65 = OpLoad %v4float %64
%66 = OpCompositeExtract %float %65 2
%67 = OpCompositeConstruct %v3float %66 %66 %66
OpStore %62 %67
OpReturn
OpFunctionEnd
%out_half4 = OpFunction %void None %68
%70 = OpFunctionParameter %_ptr_Function_v4float
%71 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%73 = OpLoad %v4float %72
%74 = OpCompositeExtract %float %73 3
%75 = OpCompositeConstruct %v4float %74 %74 %74 %74
OpStore %70 %75
OpReturn
OpFunctionEnd
%out_half2x2 = OpFunction %void None %77
%79 = OpFunctionParameter %_ptr_Function_mat2v2float
%80 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%82 = OpLoad %v4float %81
%83 = OpCompositeExtract %float %82 0
%86 = OpCompositeConstruct %v2float %83 %float_0
%87 = OpCompositeConstruct %v2float %float_0 %83
%84 = OpCompositeConstruct %mat2v2float %86 %87
OpStore %79 %84
OpReturn
OpFunctionEnd
%out_half3x3 = OpFunction %void None %89
%91 = OpFunctionParameter %_ptr_Function_mat3v3float
%92 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%94 = OpLoad %v4float %93
%95 = OpCompositeExtract %float %94 1
%97 = OpCompositeConstruct %v3float %95 %float_0 %float_0
%98 = OpCompositeConstruct %v3float %float_0 %95 %float_0
%99 = OpCompositeConstruct %v3float %float_0 %float_0 %95
%96 = OpCompositeConstruct %mat3v3float %97 %98 %99
OpStore %91 %96
OpReturn
OpFunctionEnd
%out_half4x4 = OpFunction %void None %101
%103 = OpFunctionParameter %_ptr_Function_mat4v4float
%104 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%106 = OpLoad %v4float %105
%107 = OpCompositeExtract %float %106 2
%109 = OpCompositeConstruct %v4float %107 %float_0 %float_0 %float_0
%110 = OpCompositeConstruct %v4float %float_0 %107 %float_0 %float_0
%111 = OpCompositeConstruct %v4float %float_0 %float_0 %107 %float_0
%112 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %107
%108 = OpCompositeConstruct %mat4v4float %109 %110 %111 %112
OpStore %103 %108
OpReturn
OpFunctionEnd
%out_int = OpFunction %void None %113
%115 = OpFunctionParameter %_ptr_Function_int
%116 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 0
%120 = OpConvertFToS %int %119
OpStore %115 %120
OpReturn
OpFunctionEnd
%out_int2 = OpFunction %void None %122
%124 = OpFunctionParameter %_ptr_Function_v2int
%125 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%127 = OpLoad %v4float %126
%128 = OpCompositeExtract %float %127 1
%129 = OpConvertFToS %int %128
%130 = OpCompositeConstruct %v2int %129 %129
OpStore %124 %130
OpReturn
OpFunctionEnd
%out_int3 = OpFunction %void None %132
%134 = OpFunctionParameter %_ptr_Function_v3int
%135 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%137 = OpLoad %v4float %136
%138 = OpCompositeExtract %float %137 2
%139 = OpConvertFToS %int %138
%140 = OpCompositeConstruct %v3int %139 %139 %139
OpStore %134 %140
OpReturn
OpFunctionEnd
%out_int4 = OpFunction %void None %142
%144 = OpFunctionParameter %_ptr_Function_v4int
%145 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%147 = OpLoad %v4float %146
%148 = OpCompositeExtract %float %147 3
%149 = OpConvertFToS %int %148
%150 = OpCompositeConstruct %v4int %149 %149 %149 %149
OpStore %144 %150
OpReturn
OpFunctionEnd
%out_float = OpFunction %void None %40
%151 = OpFunctionParameter %_ptr_Function_float
%152 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 0
OpStore %151 %155
OpReturn
OpFunctionEnd
%out_float2 = OpFunction %void None %51
%156 = OpFunctionParameter %_ptr_Function_v2float
%157 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%159 = OpLoad %v4float %158
%160 = OpCompositeExtract %float %159 1
%161 = OpCompositeConstruct %v2float %160 %160
OpStore %156 %161
OpReturn
OpFunctionEnd
%out_float3 = OpFunction %void None %60
%162 = OpFunctionParameter %_ptr_Function_v3float
%163 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%165 = OpLoad %v4float %164
%166 = OpCompositeExtract %float %165 2
%167 = OpCompositeConstruct %v3float %166 %166 %166
OpStore %162 %167
OpReturn
OpFunctionEnd
%out_float4 = OpFunction %void None %68
%168 = OpFunctionParameter %_ptr_Function_v4float
%169 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%171 = OpLoad %v4float %170
%172 = OpCompositeExtract %float %171 3
%173 = OpCompositeConstruct %v4float %172 %172 %172 %172
OpStore %168 %173
OpReturn
OpFunctionEnd
%out_float2x2 = OpFunction %void None %77
%174 = OpFunctionParameter %_ptr_Function_mat2v2float
%175 = OpLabel
%176 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%177 = OpLoad %v4float %176
%178 = OpCompositeExtract %float %177 0
%180 = OpCompositeConstruct %v2float %178 %float_0
%181 = OpCompositeConstruct %v2float %float_0 %178
%179 = OpCompositeConstruct %mat2v2float %180 %181
OpStore %174 %179
OpReturn
OpFunctionEnd
%out_float3x3 = OpFunction %void None %89
%182 = OpFunctionParameter %_ptr_Function_mat3v3float
%183 = OpLabel
%184 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%185 = OpLoad %v4float %184
%186 = OpCompositeExtract %float %185 1
%188 = OpCompositeConstruct %v3float %186 %float_0 %float_0
%189 = OpCompositeConstruct %v3float %float_0 %186 %float_0
%190 = OpCompositeConstruct %v3float %float_0 %float_0 %186
%187 = OpCompositeConstruct %mat3v3float %188 %189 %190
OpStore %182 %187
OpReturn
OpFunctionEnd
%out_float4x4 = OpFunction %void None %101
%191 = OpFunctionParameter %_ptr_Function_mat4v4float
%192 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%194 = OpLoad %v4float %193
%195 = OpCompositeExtract %float %194 2
%197 = OpCompositeConstruct %v4float %195 %float_0 %float_0 %float_0
%198 = OpCompositeConstruct %v4float %float_0 %195 %float_0 %float_0
%199 = OpCompositeConstruct %v4float %float_0 %float_0 %195 %float_0
%200 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %195
%196 = OpCompositeConstruct %mat4v4float %197 %198 %199 %200
OpStore %191 %196
OpReturn
OpFunctionEnd
%out_bool = OpFunction %void None %201
%203 = OpFunctionParameter %_ptr_Function_bool
%204 = OpLabel
%205 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%206 = OpLoad %v4float %205
%207 = OpCompositeExtract %float %206 0
%208 = OpFUnordNotEqual %bool %207 %float_0
OpStore %203 %208
OpReturn
OpFunctionEnd
%out_bool2 = OpFunction %void None %210
%212 = OpFunctionParameter %_ptr_Function_v2bool
%213 = OpLabel
%214 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%215 = OpLoad %v4float %214
%216 = OpCompositeExtract %float %215 1
%217 = OpFUnordNotEqual %bool %216 %float_0
%218 = OpCompositeConstruct %v2bool %217 %217
OpStore %212 %218
OpReturn
OpFunctionEnd
%out_bool3 = OpFunction %void None %220
%222 = OpFunctionParameter %_ptr_Function_v3bool
%223 = OpLabel
%224 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%225 = OpLoad %v4float %224
%226 = OpCompositeExtract %float %225 2
%227 = OpFUnordNotEqual %bool %226 %float_0
%228 = OpCompositeConstruct %v3bool %227 %227 %227
OpStore %222 %228
OpReturn
OpFunctionEnd
%out_bool4 = OpFunction %void None %230
%232 = OpFunctionParameter %_ptr_Function_v4bool
%233 = OpLabel
%234 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%235 = OpLoad %v4float %234
%236 = OpCompositeExtract %float %235 3
%237 = OpFUnordNotEqual %bool %236 %float_0
%238 = OpCompositeConstruct %v4bool %237 %237 %237 %237
OpStore %232 %238
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %239
%240 = OpLabel
%h = OpVariable %_ptr_Function_float Function
%h2 = OpVariable %_ptr_Function_v2float Function
%h3 = OpVariable %_ptr_Function_v3float Function
%h4 = OpVariable %_ptr_Function_v4float Function
%252 = OpVariable %_ptr_Function_float Function
%257 = OpVariable %_ptr_Function_v2float Function
%264 = OpVariable %_ptr_Function_v4float Function
%h2x2 = OpVariable %_ptr_Function_mat2v2float Function
%h3x3 = OpVariable %_ptr_Function_mat3v3float Function
%h4x4 = OpVariable %_ptr_Function_mat4v4float Function
%277 = OpVariable %_ptr_Function_v3float Function
%284 = OpVariable %_ptr_Function_float Function
%291 = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_v2int Function
%i3 = OpVariable %_ptr_Function_v3int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%304 = OpVariable %_ptr_Function_v3int Function
%311 = OpVariable %_ptr_Function_int Function
%f = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_v2float Function
%f3 = OpVariable %_ptr_Function_v3float Function
%f4 = OpVariable %_ptr_Function_v4float Function
%324 = OpVariable %_ptr_Function_v2float Function
%331 = OpVariable %_ptr_Function_float Function
%f2x2 = OpVariable %_ptr_Function_mat2v2float Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%f4x4 = OpVariable %_ptr_Function_mat4v4float Function
%343 = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_v2bool Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
%356 = OpVariable %_ptr_Function_v2bool Function
%363 = OpVariable %_ptr_Function_bool Function
%ok = OpVariable %_ptr_Function_bool Function
%460 = OpVariable %_ptr_Function_v4float Function
%242 = OpFunctionCall %void %out_half %h
%244 = OpFunctionCall %void %out_half2 %h2
%246 = OpFunctionCall %void %out_half3 %h3
%248 = OpFunctionCall %void %out_half4 %h4
%249 = OpAccessChain %_ptr_Function_float %h3 %int_1
%251 = OpLoad %float %249
OpStore %252 %251
%253 = OpFunctionCall %void %out_half %252
%254 = OpLoad %float %252
OpStore %249 %254
%255 = OpLoad %v3float %h3
%256 = OpVectorShuffle %v2float %255 %255 0 2
OpStore %257 %256
%258 = OpFunctionCall %void %out_half2 %257
%259 = OpLoad %v2float %257
%260 = OpLoad %v3float %h3
%261 = OpVectorShuffle %v3float %260 %259 3 1 4
OpStore %h3 %261
%262 = OpLoad %v4float %h4
%263 = OpVectorShuffle %v4float %262 %262 2 3 0 1
OpStore %264 %263
%265 = OpFunctionCall %void %out_half4 %264
%266 = OpLoad %v4float %264
%267 = OpLoad %v4float %h4
%268 = OpVectorShuffle %v4float %267 %266 6 7 4 5
OpStore %h4 %268
%270 = OpFunctionCall %void %out_half2x2 %h2x2
%272 = OpFunctionCall %void %out_half3x3 %h3x3
%274 = OpFunctionCall %void %out_half4x4 %h4x4
%275 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
%276 = OpLoad %v3float %275
OpStore %277 %276
%278 = OpFunctionCall %void %out_half3 %277
%279 = OpLoad %v3float %277
OpStore %275 %279
%281 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%282 = OpAccessChain %_ptr_Function_float %281 %int_3
%283 = OpLoad %float %282
OpStore %284 %283
%285 = OpFunctionCall %void %out_half %284
%286 = OpLoad %float %284
OpStore %282 %286
%288 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%289 = OpAccessChain %_ptr_Function_float %288 %int_0
%290 = OpLoad %float %289
OpStore %291 %290
%292 = OpFunctionCall %void %out_half %291
%293 = OpLoad %float %291
OpStore %289 %293
%295 = OpFunctionCall %void %out_int %i
%297 = OpFunctionCall %void %out_int2 %i2
%299 = OpFunctionCall %void %out_int3 %i3
%301 = OpFunctionCall %void %out_int4 %i4
%302 = OpLoad %v4int %i4
%303 = OpVectorShuffle %v3int %302 %302 0 1 2
OpStore %304 %303
%305 = OpFunctionCall %void %out_int3 %304
%306 = OpLoad %v3int %304
%307 = OpLoad %v4int %i4
%308 = OpVectorShuffle %v4int %307 %306 4 5 6 3
OpStore %i4 %308
%309 = OpAccessChain %_ptr_Function_int %i2 %int_1
%310 = OpLoad %int %309
OpStore %311 %310
%312 = OpFunctionCall %void %out_int %311
%313 = OpLoad %int %311
OpStore %309 %313
%315 = OpFunctionCall %void %out_float %f
%317 = OpFunctionCall %void %out_float2 %f2
%319 = OpFunctionCall %void %out_float3 %f3
%321 = OpFunctionCall %void %out_float4 %f4
%322 = OpLoad %v3float %f3
%323 = OpVectorShuffle %v2float %322 %322 0 1
OpStore %324 %323
%325 = OpFunctionCall %void %out_float2 %324
%326 = OpLoad %v2float %324
%327 = OpLoad %v3float %f3
%328 = OpVectorShuffle %v3float %327 %326 3 4 2
OpStore %f3 %328
%329 = OpAccessChain %_ptr_Function_float %f2 %int_0
%330 = OpLoad %float %329
OpStore %331 %330
%332 = OpFunctionCall %void %out_float %331
%333 = OpLoad %float %331
OpStore %329 %333
%335 = OpFunctionCall %void %out_float2x2 %f2x2
%337 = OpFunctionCall %void %out_float3x3 %f3x3
%339 = OpFunctionCall %void %out_float4x4 %f4x4
%340 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%341 = OpAccessChain %_ptr_Function_float %340 %int_0
%342 = OpLoad %float %341
OpStore %343 %342
%344 = OpFunctionCall %void %out_float %343
%345 = OpLoad %float %343
OpStore %341 %345
%347 = OpFunctionCall %void %out_bool %b
%349 = OpFunctionCall %void %out_bool2 %b2
%351 = OpFunctionCall %void %out_bool3 %b3
%353 = OpFunctionCall %void %out_bool4 %b4
%354 = OpLoad %v4bool %b4
%355 = OpVectorShuffle %v2bool %354 %354 0 3
OpStore %356 %355
%357 = OpFunctionCall %void %out_bool2 %356
%358 = OpLoad %v2bool %356
%359 = OpLoad %v4bool %b4
%360 = OpVectorShuffle %v4bool %359 %358 4 1 2 5
OpStore %b4 %360
%361 = OpAccessChain %_ptr_Function_bool %b3 %int_2
%362 = OpLoad %bool %361
OpStore %363 %362
%364 = OpFunctionCall %void %out_bool %363
%365 = OpLoad %bool %363
OpStore %361 %365
OpStore %ok %true
%369 = OpLoad %bool %ok
OpSelectionMerge %371 None
OpBranchConditional %369 %370 %371
%370 = OpLabel
%373 = OpLoad %float %h
%374 = OpLoad %v2float %h2
%375 = OpCompositeExtract %float %374 0
%376 = OpFMul %float %373 %375
%377 = OpLoad %v3float %h3
%378 = OpCompositeExtract %float %377 0
%379 = OpFMul %float %376 %378
%380 = OpLoad %v4float %h4
%381 = OpCompositeExtract %float %380 0
%382 = OpFMul %float %379 %381
%383 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%384 = OpLoad %v2float %383
%385 = OpCompositeExtract %float %384 0
%386 = OpFMul %float %382 %385
%387 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%388 = OpLoad %v3float %387
%389 = OpCompositeExtract %float %388 0
%390 = OpFMul %float %386 %389
%391 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%392 = OpLoad %v4float %391
%393 = OpCompositeExtract %float %392 0
%394 = OpFMul %float %390 %393
%395 = OpFOrdEqual %bool %float_1 %394
OpBranch %371
%371 = OpLabel
%396 = OpPhi %bool %false %240 %395 %370
OpStore %ok %396
%397 = OpLoad %bool %ok
OpSelectionMerge %399 None
OpBranchConditional %397 %398 %399
%398 = OpLabel
%400 = OpLoad %float %f
%401 = OpLoad %v2float %f2
%402 = OpCompositeExtract %float %401 0
%403 = OpFMul %float %400 %402
%404 = OpLoad %v3float %f3
%405 = OpCompositeExtract %float %404 0
%406 = OpFMul %float %403 %405
%407 = OpLoad %v4float %f4
%408 = OpCompositeExtract %float %407 0
%409 = OpFMul %float %406 %408
%410 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%411 = OpLoad %v2float %410
%412 = OpCompositeExtract %float %411 0
%413 = OpFMul %float %409 %412
%414 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%415 = OpLoad %v3float %414
%416 = OpCompositeExtract %float %415 0
%417 = OpFMul %float %413 %416
%418 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
%419 = OpLoad %v4float %418
%420 = OpCompositeExtract %float %419 0
%421 = OpFMul %float %417 %420
%422 = OpFOrdEqual %bool %float_1 %421
OpBranch %399
%399 = OpLabel
%423 = OpPhi %bool %false %371 %422 %398
OpStore %ok %423
%424 = OpLoad %bool %ok
OpSelectionMerge %426 None
OpBranchConditional %424 %425 %426
%425 = OpLabel
%427 = OpLoad %int %i
%428 = OpLoad %v2int %i2
%429 = OpCompositeExtract %int %428 0
%430 = OpIMul %int %427 %429
%431 = OpLoad %v3int %i3
%432 = OpCompositeExtract %int %431 0
%433 = OpIMul %int %430 %432
%434 = OpLoad %v4int %i4
%435 = OpCompositeExtract %int %434 0
%436 = OpIMul %int %433 %435
%437 = OpIEqual %bool %int_1 %436
OpBranch %426
%426 = OpLabel
%438 = OpPhi %bool %false %399 %437 %425
OpStore %ok %438
%439 = OpLoad %bool %ok
OpSelectionMerge %441 None
OpBranchConditional %439 %440 %441
%440 = OpLabel
%442 = OpLoad %bool %b
OpSelectionMerge %444 None
OpBranchConditional %442 %443 %444
%443 = OpLabel
%445 = OpLoad %v2bool %b2
%446 = OpCompositeExtract %bool %445 0
OpBranch %444
%444 = OpLabel
%447 = OpPhi %bool %false %440 %446 %443
OpSelectionMerge %449 None
OpBranchConditional %447 %448 %449
%448 = OpLabel
%450 = OpLoad %v3bool %b3
%451 = OpCompositeExtract %bool %450 0
OpBranch %449
%449 = OpLabel
%452 = OpPhi %bool %false %444 %451 %448
OpSelectionMerge %454 None
OpBranchConditional %452 %453 %454
%453 = OpLabel
%455 = OpLoad %v4bool %b4
%456 = OpCompositeExtract %bool %455 0
OpBranch %454
%454 = OpLabel
%457 = OpPhi %bool %false %449 %456 %453
OpBranch %441
%441 = OpLabel
%458 = OpPhi %bool %false %426 %457 %454
OpStore %ok %458
%459 = OpLoad %bool %ok
OpSelectionMerge %463 None
OpBranchConditional %459 %461 %462
%461 = OpLabel
%464 = OpAccessChain %_ptr_Uniform_v4float %32 %int_0
%465 = OpLoad %v4float %464
OpStore %460 %465
OpBranch %463
%462 = OpLabel
%466 = OpAccessChain %_ptr_Uniform_v4float %32 %int_1
%467 = OpLoad %v4float %466
OpStore %460 %467
OpBranch %463
%463 = OpLabel
%468 = OpLoad %v4float %460
OpReturnValue %468
OpFunctionEnd

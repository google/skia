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
OpMemberName %_UniformBuffer 2 "colorWhite"
OpName %_entrypoint_v "_entrypoint_v"
OpName %out_half_vh "out_half_vh"
OpName %out_half2_vh2 "out_half2_vh2"
OpName %out_half3_vh3 "out_half3_vh3"
OpName %out_half4_vh4 "out_half4_vh4"
OpName %out_half2x2_vh22 "out_half2x2_vh22"
OpName %out_half3x3_vh33 "out_half3x3_vh33"
OpName %out_half4x4_vh44 "out_half4x4_vh44"
OpName %out_int_vi "out_int_vi"
OpName %out_int2_vi2 "out_int2_vi2"
OpName %out_int3_vi3 "out_int3_vi3"
OpName %out_int4_vi4 "out_int4_vi4"
OpName %out_float_vf "out_float_vf"
OpName %out_float2_vf2 "out_float2_vf2"
OpName %out_float3_vf3 "out_float3_vf3"
OpName %out_float4_vf4 "out_float4_vf4"
OpName %out_float2x2_vf22 "out_float2x2_vf22"
OpName %out_float3x3_vf33 "out_float3x3_vf33"
OpName %out_float4x4_vf44 "out_float4x4_vf44"
OpName %out_bool_vb "out_bool_vb"
OpName %out_bool2_vb2 "out_bool2_vb2"
OpName %out_bool3_vb3 "out_bool3_vb3"
OpName %out_bool4_vb4 "out_bool4_vb4"
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
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %h RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %h2 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %h3 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %h4 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %h2x2 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %h3x3 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %h4x4 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%41 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%45 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%55 = OpTypeFunction %void %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%63 = OpTypeFunction %void %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%71 = OpTypeFunction %void %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%80 = OpTypeFunction %void %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%91 = OpTypeFunction %void %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%103 = OpTypeFunction %void %_ptr_Function_mat4v4float
%_ptr_Function_int = OpTypePointer Function %int
%115 = OpTypeFunction %void %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%124 = OpTypeFunction %void %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%134 = OpTypeFunction %void %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%144 = OpTypeFunction %void %_ptr_Function_v4int
%_ptr_Function_bool = OpTypePointer Function %bool
%203 = OpTypeFunction %void %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%212 = OpTypeFunction %void %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%222 = OpTypeFunction %void %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%232 = OpTypeFunction %void %_ptr_Function_v4bool
%241 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %37
%38 = OpLabel
%42 = OpVariable %_ptr_Function_v2float Function
OpStore %42 %41
%44 = OpFunctionCall %v4float %main %42
OpStore %sk_FragColor %44
OpReturn
OpFunctionEnd
%out_half_vh = OpFunction %void None %45
%47 = OpFunctionParameter %_ptr_Function_float
%48 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%53 = OpLoad %v4float %49
%54 = OpCompositeExtract %float %53 0
OpStore %47 %54
OpReturn
OpFunctionEnd
%out_half2_vh2 = OpFunction %void None %55
%56 = OpFunctionParameter %_ptr_Function_v2float
%57 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%59 = OpLoad %v4float %58
%60 = OpCompositeExtract %float %59 1
%61 = OpCompositeConstruct %v2float %60 %60
OpStore %56 %61
OpReturn
OpFunctionEnd
%out_half3_vh3 = OpFunction %void None %63
%65 = OpFunctionParameter %_ptr_Function_v3float
%66 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%68 = OpLoad %v4float %67
%69 = OpCompositeExtract %float %68 2
%70 = OpCompositeConstruct %v3float %69 %69 %69
OpStore %65 %70
OpReturn
OpFunctionEnd
%out_half4_vh4 = OpFunction %void None %71
%73 = OpFunctionParameter %_ptr_Function_v4float
%74 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%76 = OpLoad %v4float %75
%77 = OpCompositeExtract %float %76 3
%78 = OpCompositeConstruct %v4float %77 %77 %77 %77
OpStore %73 %78
OpReturn
OpFunctionEnd
%out_half2x2_vh22 = OpFunction %void None %80
%82 = OpFunctionParameter %_ptr_Function_mat2v2float
%83 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%88 = OpCompositeConstruct %v2float %86 %float_0
%89 = OpCompositeConstruct %v2float %float_0 %86
%87 = OpCompositeConstruct %mat2v2float %88 %89
OpStore %82 %87
OpReturn
OpFunctionEnd
%out_half3x3_vh33 = OpFunction %void None %91
%93 = OpFunctionParameter %_ptr_Function_mat3v3float
%94 = OpLabel
%95 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%96 = OpLoad %v4float %95
%97 = OpCompositeExtract %float %96 1
%99 = OpCompositeConstruct %v3float %97 %float_0 %float_0
%100 = OpCompositeConstruct %v3float %float_0 %97 %float_0
%101 = OpCompositeConstruct %v3float %float_0 %float_0 %97
%98 = OpCompositeConstruct %mat3v3float %99 %100 %101
OpStore %93 %98
OpReturn
OpFunctionEnd
%out_half4x4_vh44 = OpFunction %void None %103
%105 = OpFunctionParameter %_ptr_Function_mat4v4float
%106 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%108 = OpLoad %v4float %107
%109 = OpCompositeExtract %float %108 2
%111 = OpCompositeConstruct %v4float %109 %float_0 %float_0 %float_0
%112 = OpCompositeConstruct %v4float %float_0 %109 %float_0 %float_0
%113 = OpCompositeConstruct %v4float %float_0 %float_0 %109 %float_0
%114 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %109
%110 = OpCompositeConstruct %mat4v4float %111 %112 %113 %114
OpStore %105 %110
OpReturn
OpFunctionEnd
%out_int_vi = OpFunction %void None %115
%117 = OpFunctionParameter %_ptr_Function_int
%118 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%120 = OpLoad %v4float %119
%121 = OpCompositeExtract %float %120 0
%122 = OpConvertFToS %int %121
OpStore %117 %122
OpReturn
OpFunctionEnd
%out_int2_vi2 = OpFunction %void None %124
%126 = OpFunctionParameter %_ptr_Function_v2int
%127 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%129 = OpLoad %v4float %128
%130 = OpCompositeExtract %float %129 1
%131 = OpConvertFToS %int %130
%132 = OpCompositeConstruct %v2int %131 %131
OpStore %126 %132
OpReturn
OpFunctionEnd
%out_int3_vi3 = OpFunction %void None %134
%136 = OpFunctionParameter %_ptr_Function_v3int
%137 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%139 = OpLoad %v4float %138
%140 = OpCompositeExtract %float %139 2
%141 = OpConvertFToS %int %140
%142 = OpCompositeConstruct %v3int %141 %141 %141
OpStore %136 %142
OpReturn
OpFunctionEnd
%out_int4_vi4 = OpFunction %void None %144
%146 = OpFunctionParameter %_ptr_Function_v4int
%147 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%149 = OpLoad %v4float %148
%150 = OpCompositeExtract %float %149 3
%151 = OpConvertFToS %int %150
%152 = OpCompositeConstruct %v4int %151 %151 %151 %151
OpStore %146 %152
OpReturn
OpFunctionEnd
%out_float_vf = OpFunction %void None %45
%153 = OpFunctionParameter %_ptr_Function_float
%154 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%156 = OpLoad %v4float %155
%157 = OpCompositeExtract %float %156 0
OpStore %153 %157
OpReturn
OpFunctionEnd
%out_float2_vf2 = OpFunction %void None %55
%158 = OpFunctionParameter %_ptr_Function_v2float
%159 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%161 = OpLoad %v4float %160
%162 = OpCompositeExtract %float %161 1
%163 = OpCompositeConstruct %v2float %162 %162
OpStore %158 %163
OpReturn
OpFunctionEnd
%out_float3_vf3 = OpFunction %void None %63
%164 = OpFunctionParameter %_ptr_Function_v3float
%165 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%167 = OpLoad %v4float %166
%168 = OpCompositeExtract %float %167 2
%169 = OpCompositeConstruct %v3float %168 %168 %168
OpStore %164 %169
OpReturn
OpFunctionEnd
%out_float4_vf4 = OpFunction %void None %71
%170 = OpFunctionParameter %_ptr_Function_v4float
%171 = OpLabel
%172 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%173 = OpLoad %v4float %172
%174 = OpCompositeExtract %float %173 3
%175 = OpCompositeConstruct %v4float %174 %174 %174 %174
OpStore %170 %175
OpReturn
OpFunctionEnd
%out_float2x2_vf22 = OpFunction %void None %80
%176 = OpFunctionParameter %_ptr_Function_mat2v2float
%177 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%179 = OpLoad %v4float %178
%180 = OpCompositeExtract %float %179 0
%182 = OpCompositeConstruct %v2float %180 %float_0
%183 = OpCompositeConstruct %v2float %float_0 %180
%181 = OpCompositeConstruct %mat2v2float %182 %183
OpStore %176 %181
OpReturn
OpFunctionEnd
%out_float3x3_vf33 = OpFunction %void None %91
%184 = OpFunctionParameter %_ptr_Function_mat3v3float
%185 = OpLabel
%186 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%187 = OpLoad %v4float %186
%188 = OpCompositeExtract %float %187 1
%190 = OpCompositeConstruct %v3float %188 %float_0 %float_0
%191 = OpCompositeConstruct %v3float %float_0 %188 %float_0
%192 = OpCompositeConstruct %v3float %float_0 %float_0 %188
%189 = OpCompositeConstruct %mat3v3float %190 %191 %192
OpStore %184 %189
OpReturn
OpFunctionEnd
%out_float4x4_vf44 = OpFunction %void None %103
%193 = OpFunctionParameter %_ptr_Function_mat4v4float
%194 = OpLabel
%195 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%196 = OpLoad %v4float %195
%197 = OpCompositeExtract %float %196 2
%199 = OpCompositeConstruct %v4float %197 %float_0 %float_0 %float_0
%200 = OpCompositeConstruct %v4float %float_0 %197 %float_0 %float_0
%201 = OpCompositeConstruct %v4float %float_0 %float_0 %197 %float_0
%202 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %197
%198 = OpCompositeConstruct %mat4v4float %199 %200 %201 %202
OpStore %193 %198
OpReturn
OpFunctionEnd
%out_bool_vb = OpFunction %void None %203
%205 = OpFunctionParameter %_ptr_Function_bool
%206 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%208 = OpLoad %v4float %207
%209 = OpCompositeExtract %float %208 0
%210 = OpFUnordNotEqual %bool %209 %float_0
OpStore %205 %210
OpReturn
OpFunctionEnd
%out_bool2_vb2 = OpFunction %void None %212
%214 = OpFunctionParameter %_ptr_Function_v2bool
%215 = OpLabel
%216 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%217 = OpLoad %v4float %216
%218 = OpCompositeExtract %float %217 1
%219 = OpFUnordNotEqual %bool %218 %float_0
%220 = OpCompositeConstruct %v2bool %219 %219
OpStore %214 %220
OpReturn
OpFunctionEnd
%out_bool3_vb3 = OpFunction %void None %222
%224 = OpFunctionParameter %_ptr_Function_v3bool
%225 = OpLabel
%226 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%227 = OpLoad %v4float %226
%228 = OpCompositeExtract %float %227 2
%229 = OpFUnordNotEqual %bool %228 %float_0
%230 = OpCompositeConstruct %v3bool %229 %229 %229
OpStore %224 %230
OpReturn
OpFunctionEnd
%out_bool4_vb4 = OpFunction %void None %232
%234 = OpFunctionParameter %_ptr_Function_v4bool
%235 = OpLabel
%236 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%237 = OpLoad %v4float %236
%238 = OpCompositeExtract %float %237 3
%239 = OpFUnordNotEqual %bool %238 %float_0
%240 = OpCompositeConstruct %v4bool %239 %239 %239 %239
OpStore %234 %240
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %241
%242 = OpFunctionParameter %_ptr_Function_v2float
%243 = OpLabel
%h = OpVariable %_ptr_Function_float Function
%245 = OpVariable %_ptr_Function_float Function
%h2 = OpVariable %_ptr_Function_v2float Function
%249 = OpVariable %_ptr_Function_v2float Function
%h3 = OpVariable %_ptr_Function_v3float Function
%253 = OpVariable %_ptr_Function_v3float Function
%h4 = OpVariable %_ptr_Function_v4float Function
%257 = OpVariable %_ptr_Function_v4float Function
%262 = OpVariable %_ptr_Function_float Function
%265 = OpVariable %_ptr_Function_v2float Function
%270 = OpVariable %_ptr_Function_v4float Function
%h2x2 = OpVariable %_ptr_Function_mat2v2float Function
%276 = OpVariable %_ptr_Function_mat2v2float Function
%h3x3 = OpVariable %_ptr_Function_mat3v3float Function
%280 = OpVariable %_ptr_Function_mat3v3float Function
%h4x4 = OpVariable %_ptr_Function_mat4v4float Function
%284 = OpVariable %_ptr_Function_mat4v4float Function
%288 = OpVariable %_ptr_Function_v3float Function
%294 = OpVariable %_ptr_Function_float Function
%300 = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%304 = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_v2int Function
%308 = OpVariable %_ptr_Function_v2int Function
%i3 = OpVariable %_ptr_Function_v3int Function
%312 = OpVariable %_ptr_Function_v3int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%316 = OpVariable %_ptr_Function_v4int Function
%319 = OpVariable %_ptr_Function_v3int Function
%325 = OpVariable %_ptr_Function_int Function
%f = OpVariable %_ptr_Function_float Function
%329 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_v2float Function
%333 = OpVariable %_ptr_Function_v2float Function
%f3 = OpVariable %_ptr_Function_v3float Function
%337 = OpVariable %_ptr_Function_v3float Function
%f4 = OpVariable %_ptr_Function_v4float Function
%341 = OpVariable %_ptr_Function_v4float Function
%344 = OpVariable %_ptr_Function_v2float Function
%350 = OpVariable %_ptr_Function_float Function
%f2x2 = OpVariable %_ptr_Function_mat2v2float Function
%354 = OpVariable %_ptr_Function_mat2v2float Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%358 = OpVariable %_ptr_Function_mat3v3float Function
%f4x4 = OpVariable %_ptr_Function_mat4v4float Function
%362 = OpVariable %_ptr_Function_mat4v4float Function
%367 = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_bool Function
%371 = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_v2bool Function
%375 = OpVariable %_ptr_Function_v2bool Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%379 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
%383 = OpVariable %_ptr_Function_v4bool Function
%386 = OpVariable %_ptr_Function_v2bool Function
%392 = OpVariable %_ptr_Function_bool Function
%ok = OpVariable %_ptr_Function_bool Function
%489 = OpVariable %_ptr_Function_v4float Function
%246 = OpFunctionCall %void %out_half_vh %245
%247 = OpLoad %float %245
OpStore %h %247
%250 = OpFunctionCall %void %out_half2_vh2 %249
%251 = OpLoad %v2float %249
OpStore %h2 %251
%254 = OpFunctionCall %void %out_half3_vh3 %253
%255 = OpLoad %v3float %253
OpStore %h3 %255
%258 = OpFunctionCall %void %out_half4_vh4 %257
%259 = OpLoad %v4float %257
OpStore %h4 %259
%260 = OpAccessChain %_ptr_Function_float %h3 %int_1
%263 = OpFunctionCall %void %out_half_vh %262
%264 = OpLoad %float %262
OpStore %260 %264
%266 = OpFunctionCall %void %out_half2_vh2 %265
%267 = OpLoad %v2float %265
%268 = OpLoad %v3float %h3
%269 = OpVectorShuffle %v3float %268 %267 3 1 4
OpStore %h3 %269
%271 = OpFunctionCall %void %out_half4_vh4 %270
%272 = OpLoad %v4float %270
%273 = OpLoad %v4float %h4
%274 = OpVectorShuffle %v4float %273 %272 6 7 4 5
OpStore %h4 %274
%277 = OpFunctionCall %void %out_half2x2_vh22 %276
%278 = OpLoad %mat2v2float %276
OpStore %h2x2 %278
%281 = OpFunctionCall %void %out_half3x3_vh33 %280
%282 = OpLoad %mat3v3float %280
OpStore %h3x3 %282
%285 = OpFunctionCall %void %out_half4x4_vh44 %284
%286 = OpLoad %mat4v4float %284
OpStore %h4x4 %286
%287 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
%289 = OpFunctionCall %void %out_half3_vh3 %288
%290 = OpLoad %v3float %288
OpStore %287 %290
%292 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%293 = OpAccessChain %_ptr_Function_float %292 %int_3
%295 = OpFunctionCall %void %out_half_vh %294
%296 = OpLoad %float %294
OpStore %293 %296
%298 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%299 = OpAccessChain %_ptr_Function_float %298 %int_0
%301 = OpFunctionCall %void %out_half_vh %300
%302 = OpLoad %float %300
OpStore %299 %302
%305 = OpFunctionCall %void %out_int_vi %304
%306 = OpLoad %int %304
OpStore %i %306
%309 = OpFunctionCall %void %out_int2_vi2 %308
%310 = OpLoad %v2int %308
OpStore %i2 %310
%313 = OpFunctionCall %void %out_int3_vi3 %312
%314 = OpLoad %v3int %312
OpStore %i3 %314
%317 = OpFunctionCall %void %out_int4_vi4 %316
%318 = OpLoad %v4int %316
OpStore %i4 %318
%320 = OpFunctionCall %void %out_int3_vi3 %319
%321 = OpLoad %v3int %319
%322 = OpLoad %v4int %i4
%323 = OpVectorShuffle %v4int %322 %321 4 5 6 3
OpStore %i4 %323
%324 = OpAccessChain %_ptr_Function_int %i2 %int_1
%326 = OpFunctionCall %void %out_int_vi %325
%327 = OpLoad %int %325
OpStore %324 %327
%330 = OpFunctionCall %void %out_float_vf %329
%331 = OpLoad %float %329
OpStore %f %331
%334 = OpFunctionCall %void %out_float2_vf2 %333
%335 = OpLoad %v2float %333
OpStore %f2 %335
%338 = OpFunctionCall %void %out_float3_vf3 %337
%339 = OpLoad %v3float %337
OpStore %f3 %339
%342 = OpFunctionCall %void %out_float4_vf4 %341
%343 = OpLoad %v4float %341
OpStore %f4 %343
%345 = OpFunctionCall %void %out_float2_vf2 %344
%346 = OpLoad %v2float %344
%347 = OpLoad %v3float %f3
%348 = OpVectorShuffle %v3float %347 %346 3 4 2
OpStore %f3 %348
%349 = OpAccessChain %_ptr_Function_float %f2 %int_0
%351 = OpFunctionCall %void %out_float_vf %350
%352 = OpLoad %float %350
OpStore %349 %352
%355 = OpFunctionCall %void %out_float2x2_vf22 %354
%356 = OpLoad %mat2v2float %354
OpStore %f2x2 %356
%359 = OpFunctionCall %void %out_float3x3_vf33 %358
%360 = OpLoad %mat3v3float %358
OpStore %f3x3 %360
%363 = OpFunctionCall %void %out_float4x4_vf44 %362
%364 = OpLoad %mat4v4float %362
OpStore %f4x4 %364
%365 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%366 = OpAccessChain %_ptr_Function_float %365 %int_0
%368 = OpFunctionCall %void %out_float_vf %367
%369 = OpLoad %float %367
OpStore %366 %369
%372 = OpFunctionCall %void %out_bool_vb %371
%373 = OpLoad %bool %371
OpStore %b %373
%376 = OpFunctionCall %void %out_bool2_vb2 %375
%377 = OpLoad %v2bool %375
OpStore %b2 %377
%380 = OpFunctionCall %void %out_bool3_vb3 %379
%381 = OpLoad %v3bool %379
OpStore %b3 %381
%384 = OpFunctionCall %void %out_bool4_vb4 %383
%385 = OpLoad %v4bool %383
OpStore %b4 %385
%387 = OpFunctionCall %void %out_bool2_vb2 %386
%388 = OpLoad %v2bool %386
%389 = OpLoad %v4bool %b4
%390 = OpVectorShuffle %v4bool %389 %388 4 1 2 5
OpStore %b4 %390
%391 = OpAccessChain %_ptr_Function_bool %b3 %int_2
%393 = OpFunctionCall %void %out_bool_vb %392
%394 = OpLoad %bool %392
OpStore %391 %394
OpStore %ok %true
%398 = OpLoad %bool %ok
OpSelectionMerge %400 None
OpBranchConditional %398 %399 %400
%399 = OpLabel
%402 = OpLoad %float %h
%403 = OpLoad %v2float %h2
%404 = OpCompositeExtract %float %403 0
%405 = OpFMul %float %402 %404
%406 = OpLoad %v3float %h3
%407 = OpCompositeExtract %float %406 0
%408 = OpFMul %float %405 %407
%409 = OpLoad %v4float %h4
%410 = OpCompositeExtract %float %409 0
%411 = OpFMul %float %408 %410
%412 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%413 = OpLoad %v2float %412
%414 = OpCompositeExtract %float %413 0
%415 = OpFMul %float %411 %414
%416 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%417 = OpLoad %v3float %416
%418 = OpCompositeExtract %float %417 0
%419 = OpFMul %float %415 %418
%420 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%421 = OpLoad %v4float %420
%422 = OpCompositeExtract %float %421 0
%423 = OpFMul %float %419 %422
%424 = OpFOrdEqual %bool %float_1 %423
OpBranch %400
%400 = OpLabel
%425 = OpPhi %bool %false %243 %424 %399
OpStore %ok %425
%426 = OpLoad %bool %ok
OpSelectionMerge %428 None
OpBranchConditional %426 %427 %428
%427 = OpLabel
%429 = OpLoad %float %f
%430 = OpLoad %v2float %f2
%431 = OpCompositeExtract %float %430 0
%432 = OpFMul %float %429 %431
%433 = OpLoad %v3float %f3
%434 = OpCompositeExtract %float %433 0
%435 = OpFMul %float %432 %434
%436 = OpLoad %v4float %f4
%437 = OpCompositeExtract %float %436 0
%438 = OpFMul %float %435 %437
%439 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%440 = OpLoad %v2float %439
%441 = OpCompositeExtract %float %440 0
%442 = OpFMul %float %438 %441
%443 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%444 = OpLoad %v3float %443
%445 = OpCompositeExtract %float %444 0
%446 = OpFMul %float %442 %445
%447 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
%448 = OpLoad %v4float %447
%449 = OpCompositeExtract %float %448 0
%450 = OpFMul %float %446 %449
%451 = OpFOrdEqual %bool %float_1 %450
OpBranch %428
%428 = OpLabel
%452 = OpPhi %bool %false %400 %451 %427
OpStore %ok %452
%453 = OpLoad %bool %ok
OpSelectionMerge %455 None
OpBranchConditional %453 %454 %455
%454 = OpLabel
%456 = OpLoad %int %i
%457 = OpLoad %v2int %i2
%458 = OpCompositeExtract %int %457 0
%459 = OpIMul %int %456 %458
%460 = OpLoad %v3int %i3
%461 = OpCompositeExtract %int %460 0
%462 = OpIMul %int %459 %461
%463 = OpLoad %v4int %i4
%464 = OpCompositeExtract %int %463 0
%465 = OpIMul %int %462 %464
%466 = OpIEqual %bool %int_1 %465
OpBranch %455
%455 = OpLabel
%467 = OpPhi %bool %false %428 %466 %454
OpStore %ok %467
%468 = OpLoad %bool %ok
OpSelectionMerge %470 None
OpBranchConditional %468 %469 %470
%469 = OpLabel
%471 = OpLoad %bool %b
OpSelectionMerge %473 None
OpBranchConditional %471 %472 %473
%472 = OpLabel
%474 = OpLoad %v2bool %b2
%475 = OpCompositeExtract %bool %474 0
OpBranch %473
%473 = OpLabel
%476 = OpPhi %bool %false %469 %475 %472
OpSelectionMerge %478 None
OpBranchConditional %476 %477 %478
%477 = OpLabel
%479 = OpLoad %v3bool %b3
%480 = OpCompositeExtract %bool %479 0
OpBranch %478
%478 = OpLabel
%481 = OpPhi %bool %false %473 %480 %477
OpSelectionMerge %483 None
OpBranchConditional %481 %482 %483
%482 = OpLabel
%484 = OpLoad %v4bool %b4
%485 = OpCompositeExtract %bool %484 0
OpBranch %483
%483 = OpLabel
%486 = OpPhi %bool %false %478 %485 %482
OpBranch %470
%470 = OpLabel
%487 = OpPhi %bool %false %455 %486 %483
OpStore %ok %487
%488 = OpLoad %bool %ok
OpSelectionMerge %492 None
OpBranchConditional %488 %490 %491
%490 = OpLabel
%493 = OpAccessChain %_ptr_Uniform_v4float %32 %int_0
%494 = OpLoad %v4float %493
OpStore %489 %494
OpBranch %492
%491 = OpLabel
%495 = OpAccessChain %_ptr_Uniform_v4float %32 %int_1
%496 = OpLoad %v4float %495
OpStore %489 %496
OpBranch %492
%492 = OpLabel
%497 = OpLoad %v4float %489
OpReturnValue %497
OpFunctionEnd

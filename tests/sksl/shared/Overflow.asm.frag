OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %huge "huge"
OpName %hugeI "hugeI"
OpName %hugeU "hugeU"
OpName %hugeS "hugeS"
OpName %hugeUS "hugeUS"
OpName %hugeNI "hugeNI"
OpName %hugeNS "hugeNS"
OpName %i4 "i4"
OpName %hugeIvec "hugeIvec"
OpName %u4 "u4"
OpName %hugeUvec "hugeUvec"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %huge RelaxedPrecision
OpDecorate %hugeS RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %hugeUS RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %hugeNS RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_9_00000076e_35 = OpConstant %float 9.00000076e+35
%float_1e_09 = OpConstant %float 1e+09
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1073741824 = OpConstant %int 1073741824
%int_2 = OpConstant %int 2
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_2147483648 = OpConstant %uint 2147483648
%uint_2 = OpConstant %uint 2
%int_16384 = OpConstant %int 16384
%uint_32768 = OpConstant %uint 32768
%int_n2147483648 = OpConstant %int -2147483648
%int_n32768 = OpConstant %int -32768
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%168 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%170 = OpConstantComposite %v4int %int_1073741824 %int_1073741824 %int_1073741824 %int_1073741824
%v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%204 = OpConstantComposite %v4uint %uint_2 %uint_2 %uint_2 %uint_2
%206 = OpConstantComposite %v4uint %uint_2147483648 %uint_2147483648 %uint_2147483648 %uint_2147483648
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%huge = OpVariable %_ptr_Function_float Function
%hugeI = OpVariable %_ptr_Function_int Function
%hugeU = OpVariable %_ptr_Function_uint Function
%hugeS = OpVariable %_ptr_Function_int Function
%hugeUS = OpVariable %_ptr_Function_uint Function
%hugeNI = OpVariable %_ptr_Function_int Function
%hugeNS = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%hugeIvec = OpVariable %_ptr_Function_v4int Function
%u4 = OpVariable %_ptr_Function_v4uint Function
%hugeUvec = OpVariable %_ptr_Function_v4uint Function
%30 = OpFMul %float %float_9_00000076e_35 %float_1e_09
%31 = OpFMul %float %30 %float_1e_09
%32 = OpFMul %float %31 %float_1e_09
%33 = OpFMul %float %32 %float_1e_09
%34 = OpFMul %float %33 %float_1e_09
%35 = OpFMul %float %34 %float_1e_09
%36 = OpFMul %float %35 %float_1e_09
%37 = OpFMul %float %36 %float_1e_09
%38 = OpFMul %float %37 %float_1e_09
%39 = OpFMul %float %38 %float_1e_09
OpStore %huge %39
%45 = OpIMul %int %int_1073741824 %int_2
%46 = OpIMul %int %45 %int_2
%47 = OpIMul %int %46 %int_2
%48 = OpIMul %int %47 %int_2
%49 = OpIMul %int %48 %int_2
%50 = OpIMul %int %49 %int_2
%51 = OpIMul %int %50 %int_2
%52 = OpIMul %int %51 %int_2
%53 = OpIMul %int %52 %int_2
%54 = OpIMul %int %53 %int_2
%55 = OpIMul %int %54 %int_2
%56 = OpIMul %int %55 %int_2
%57 = OpIMul %int %56 %int_2
%58 = OpIMul %int %57 %int_2
%59 = OpIMul %int %58 %int_2
%60 = OpIMul %int %59 %int_2
%61 = OpIMul %int %60 %int_2
%62 = OpIMul %int %61 %int_2
%63 = OpIMul %int %62 %int_2
%64 = OpIMul %int %63 %int_2
OpStore %hugeI %64
%70 = OpIMul %uint %uint_2147483648 %uint_2
%71 = OpIMul %uint %70 %uint_2
%72 = OpIMul %uint %71 %uint_2
%73 = OpIMul %uint %72 %uint_2
%74 = OpIMul %uint %73 %uint_2
%75 = OpIMul %uint %74 %uint_2
%76 = OpIMul %uint %75 %uint_2
%77 = OpIMul %uint %76 %uint_2
%78 = OpIMul %uint %77 %uint_2
%79 = OpIMul %uint %78 %uint_2
%80 = OpIMul %uint %79 %uint_2
%81 = OpIMul %uint %80 %uint_2
%82 = OpIMul %uint %81 %uint_2
%83 = OpIMul %uint %82 %uint_2
%84 = OpIMul %uint %83 %uint_2
%85 = OpIMul %uint %84 %uint_2
%86 = OpIMul %uint %85 %uint_2
%87 = OpIMul %uint %86 %uint_2
%88 = OpIMul %uint %87 %uint_2
OpStore %hugeU %88
%91 = OpIMul %int %int_16384 %int_2
%92 = OpIMul %int %91 %int_2
%93 = OpIMul %int %92 %int_2
%94 = OpIMul %int %93 %int_2
%95 = OpIMul %int %94 %int_2
%96 = OpIMul %int %95 %int_2
%97 = OpIMul %int %96 %int_2
%98 = OpIMul %int %97 %int_2
%99 = OpIMul %int %98 %int_2
%100 = OpIMul %int %99 %int_2
%101 = OpIMul %int %100 %int_2
%102 = OpIMul %int %101 %int_2
%103 = OpIMul %int %102 %int_2
%104 = OpIMul %int %103 %int_2
%105 = OpIMul %int %104 %int_2
%106 = OpIMul %int %105 %int_2
%107 = OpIMul %int %106 %int_2
OpStore %hugeS %107
%110 = OpIMul %uint %uint_32768 %uint_2
%111 = OpIMul %uint %110 %uint_2
%112 = OpIMul %uint %111 %uint_2
%113 = OpIMul %uint %112 %uint_2
%114 = OpIMul %uint %113 %uint_2
%115 = OpIMul %uint %114 %uint_2
%116 = OpIMul %uint %115 %uint_2
%117 = OpIMul %uint %116 %uint_2
%118 = OpIMul %uint %117 %uint_2
%119 = OpIMul %uint %118 %uint_2
%120 = OpIMul %uint %119 %uint_2
%121 = OpIMul %uint %120 %uint_2
%122 = OpIMul %uint %121 %uint_2
%123 = OpIMul %uint %122 %uint_2
%124 = OpIMul %uint %123 %uint_2
%125 = OpIMul %uint %124 %uint_2
OpStore %hugeUS %125
%128 = OpIMul %int %int_n2147483648 %int_2
%129 = OpIMul %int %128 %int_2
%130 = OpIMul %int %129 %int_2
%131 = OpIMul %int %130 %int_2
%132 = OpIMul %int %131 %int_2
%133 = OpIMul %int %132 %int_2
%134 = OpIMul %int %133 %int_2
%135 = OpIMul %int %134 %int_2
%136 = OpIMul %int %135 %int_2
%137 = OpIMul %int %136 %int_2
%138 = OpIMul %int %137 %int_2
%139 = OpIMul %int %138 %int_2
%140 = OpIMul %int %139 %int_2
%141 = OpIMul %int %140 %int_2
%142 = OpIMul %int %141 %int_2
%143 = OpIMul %int %142 %int_2
%144 = OpIMul %int %143 %int_2
%145 = OpIMul %int %144 %int_2
%146 = OpIMul %int %145 %int_2
OpStore %hugeNI %146
%149 = OpIMul %int %int_n32768 %int_2
%150 = OpIMul %int %149 %int_2
%151 = OpIMul %int %150 %int_2
%152 = OpIMul %int %151 %int_2
%153 = OpIMul %int %152 %int_2
%154 = OpIMul %int %153 %int_2
%155 = OpIMul %int %154 %int_2
%156 = OpIMul %int %155 %int_2
%157 = OpIMul %int %156 %int_2
%158 = OpIMul %int %157 %int_2
%159 = OpIMul %int %158 %int_2
%160 = OpIMul %int %159 %int_2
%161 = OpIMul %int %160 %int_2
%162 = OpIMul %int %161 %int_2
%163 = OpIMul %int %162 %int_2
%164 = OpIMul %int %163 %int_2
OpStore %hugeNS %164
OpStore %i4 %168
%171 = OpLoad %v4int %i4
%172 = OpIMul %v4int %170 %171
%173 = OpLoad %v4int %i4
%174 = OpIMul %v4int %172 %173
%175 = OpLoad %v4int %i4
%176 = OpIMul %v4int %174 %175
%177 = OpLoad %v4int %i4
%178 = OpIMul %v4int %176 %177
%179 = OpLoad %v4int %i4
%180 = OpIMul %v4int %178 %179
%181 = OpLoad %v4int %i4
%182 = OpIMul %v4int %180 %181
%183 = OpLoad %v4int %i4
%184 = OpIMul %v4int %182 %183
%185 = OpLoad %v4int %i4
%186 = OpIMul %v4int %184 %185
%187 = OpLoad %v4int %i4
%188 = OpIMul %v4int %186 %187
%189 = OpLoad %v4int %i4
%190 = OpIMul %v4int %188 %189
%191 = OpLoad %v4int %i4
%192 = OpIMul %v4int %190 %191
%193 = OpLoad %v4int %i4
%194 = OpIMul %v4int %192 %193
%195 = OpLoad %v4int %i4
%196 = OpIMul %v4int %194 %195
%197 = OpLoad %v4int %i4
%198 = OpIMul %v4int %196 %197
%199 = OpLoad %v4int %i4
%200 = OpIMul %v4int %198 %199
OpStore %hugeIvec %200
OpStore %u4 %204
%207 = OpLoad %v4uint %u4
%208 = OpIMul %v4uint %206 %207
%209 = OpLoad %v4uint %u4
%210 = OpIMul %v4uint %208 %209
%211 = OpLoad %v4uint %u4
%212 = OpIMul %v4uint %210 %211
%213 = OpLoad %v4uint %u4
%214 = OpIMul %v4uint %212 %213
%215 = OpLoad %v4uint %u4
%216 = OpIMul %v4uint %214 %215
%217 = OpLoad %v4uint %u4
%218 = OpIMul %v4uint %216 %217
%219 = OpLoad %v4uint %u4
%220 = OpIMul %v4uint %218 %219
%221 = OpLoad %v4uint %u4
%222 = OpIMul %v4uint %220 %221
%223 = OpLoad %v4uint %u4
%224 = OpIMul %v4uint %222 %223
%225 = OpLoad %v4uint %u4
%226 = OpIMul %v4uint %224 %225
%227 = OpLoad %v4uint %u4
%228 = OpIMul %v4uint %226 %227
%229 = OpLoad %v4uint %u4
%230 = OpIMul %v4uint %228 %229
%231 = OpLoad %v4uint %u4
%232 = OpIMul %v4uint %230 %231
%233 = OpLoad %v4uint %u4
%234 = OpIMul %v4uint %232 %233
OpStore %hugeUvec %234
%235 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%238 = OpLoad %v4float %235
%240 = OpLoad %float %huge
%239 = OpExtInst %float %1 FClamp %240 %float_0 %float_1
%242 = OpVectorTimesScalar %v4float %238 %239
%244 = OpLoad %int %hugeI
%245 = OpConvertSToF %float %244
%243 = OpExtInst %float %1 FClamp %245 %float_0 %float_1
%246 = OpVectorTimesScalar %v4float %242 %243
%248 = OpLoad %uint %hugeU
%249 = OpConvertUToF %float %248
%247 = OpExtInst %float %1 FClamp %249 %float_0 %float_1
%250 = OpVectorTimesScalar %v4float %246 %247
%252 = OpLoad %int %hugeS
%253 = OpConvertSToF %float %252
%251 = OpExtInst %float %1 FClamp %253 %float_0 %float_1
%254 = OpVectorTimesScalar %v4float %250 %251
%256 = OpLoad %uint %hugeUS
%257 = OpConvertUToF %float %256
%255 = OpExtInst %float %1 FClamp %257 %float_0 %float_1
%258 = OpVectorTimesScalar %v4float %254 %255
%260 = OpLoad %int %hugeNI
%261 = OpConvertSToF %float %260
%259 = OpExtInst %float %1 FClamp %261 %float_0 %float_1
%262 = OpVectorTimesScalar %v4float %258 %259
%264 = OpLoad %int %hugeNS
%265 = OpConvertSToF %float %264
%263 = OpExtInst %float %1 FClamp %265 %float_0 %float_1
%266 = OpVectorTimesScalar %v4float %262 %263
%268 = OpLoad %v4int %hugeIvec
%269 = OpCompositeExtract %int %268 0
%270 = OpConvertSToF %float %269
%271 = OpCompositeExtract %int %268 1
%272 = OpConvertSToF %float %271
%273 = OpCompositeExtract %int %268 2
%274 = OpConvertSToF %float %273
%275 = OpCompositeExtract %int %268 3
%276 = OpConvertSToF %float %275
%277 = OpCompositeConstruct %v4float %270 %272 %274 %276
%278 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%279 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%267 = OpExtInst %v4float %1 FClamp %277 %278 %279
%280 = OpFMul %v4float %266 %267
%282 = OpLoad %v4uint %hugeUvec
%283 = OpCompositeExtract %uint %282 0
%284 = OpConvertUToF %float %283
%285 = OpCompositeExtract %uint %282 1
%286 = OpConvertUToF %float %285
%287 = OpCompositeExtract %uint %282 2
%288 = OpConvertUToF %float %287
%289 = OpCompositeExtract %uint %282 3
%290 = OpConvertUToF %float %289
%291 = OpCompositeConstruct %v4float %284 %286 %288 %290
%292 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%293 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%281 = OpExtInst %v4float %1 FClamp %291 %292 %293
%294 = OpFMul %v4float %280 %281
OpReturnValue %294
OpFunctionEnd

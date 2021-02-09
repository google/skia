OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_blend_color_luminance "_blend_color_luminance"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_blend_color_saturation "_blend_color_saturation"
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %_blend_set_color_saturation "_blend_set_color_saturation"
OpName %sat "sat"
OpName %blend_hue "blend_hue"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%20 = OpTypeFunction %float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%28 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%_ptr_Function_float = OpTypePointer Function %float
%30 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%142 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%170 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%172 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%274 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%330 = OpTypeFunction %void
%_blend_color_luminance = OpFunction %float None %20
%22 = OpFunctionParameter %_ptr_Function_v3float
%23 = OpLabel
%29 = OpLoad %v3float %22
%24 = OpDot %float %28 %29
OpReturnValue %24
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %30
%32 = OpFunctionParameter %_ptr_Function_v3float
%33 = OpFunctionParameter %_ptr_Function_float
%34 = OpFunctionParameter %_ptr_Function_v3float
%35 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%101 = OpVariable %_ptr_Function_v3float Function
%38 = OpLoad %v3float %34
%37 = OpDot %float %28 %38
OpStore %lum %37
%40 = OpLoad %float %lum
%42 = OpLoad %v3float %32
%41 = OpDot %float %28 %42
%43 = OpFSub %float %40 %41
%44 = OpLoad %v3float %32
%45 = OpCompositeConstruct %v3float %43 %43 %43
%46 = OpFAdd %v3float %45 %44
OpStore %result %46
%50 = OpLoad %v3float %result
%51 = OpCompositeExtract %float %50 0
%52 = OpLoad %v3float %result
%53 = OpCompositeExtract %float %52 1
%49 = OpExtInst %float %1 FMin %51 %53
%54 = OpLoad %v3float %result
%55 = OpCompositeExtract %float %54 2
%48 = OpExtInst %float %1 FMin %49 %55
OpStore %minComp %48
%59 = OpLoad %v3float %result
%60 = OpCompositeExtract %float %59 0
%61 = OpLoad %v3float %result
%62 = OpCompositeExtract %float %61 1
%58 = OpExtInst %float %1 FMax %60 %62
%63 = OpLoad %v3float %result
%64 = OpCompositeExtract %float %63 2
%57 = OpExtInst %float %1 FMax %58 %64
OpStore %maxComp %57
%66 = OpLoad %float %minComp
%68 = OpFOrdLessThan %bool %66 %float_0
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %float %lum
%72 = OpLoad %float %minComp
%73 = OpFOrdNotEqual %bool %71 %72
OpBranch %70
%70 = OpLabel
%74 = OpPhi %bool %false %35 %73 %69
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %float %lum
%78 = OpLoad %v3float %result
%79 = OpLoad %float %lum
%80 = OpCompositeConstruct %v3float %79 %79 %79
%81 = OpFSub %v3float %78 %80
%82 = OpLoad %float %lum
%83 = OpVectorTimesScalar %v3float %81 %82
%84 = OpLoad %float %lum
%85 = OpLoad %float %minComp
%86 = OpFSub %float %84 %85
%88 = OpFDiv %float %float_1 %86
%89 = OpVectorTimesScalar %v3float %83 %88
%90 = OpCompositeConstruct %v3float %77 %77 %77
%91 = OpFAdd %v3float %90 %89
OpStore %result %91
OpBranch %76
%76 = OpLabel
%92 = OpLoad %float %maxComp
%93 = OpLoad %float %33
%94 = OpFOrdGreaterThan %bool %92 %93
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpLoad %float %maxComp
%98 = OpLoad %float %lum
%99 = OpFOrdNotEqual %bool %97 %98
OpBranch %96
%96 = OpLabel
%100 = OpPhi %bool %false %76 %99 %95
OpSelectionMerge %104 None
OpBranchConditional %100 %102 %103
%102 = OpLabel
%105 = OpLoad %float %lum
%106 = OpLoad %v3float %result
%107 = OpLoad %float %lum
%108 = OpCompositeConstruct %v3float %107 %107 %107
%109 = OpFSub %v3float %106 %108
%110 = OpLoad %float %33
%111 = OpLoad %float %lum
%112 = OpFSub %float %110 %111
%113 = OpVectorTimesScalar %v3float %109 %112
%114 = OpLoad %float %maxComp
%115 = OpLoad %float %lum
%116 = OpFSub %float %114 %115
%117 = OpFDiv %float %float_1 %116
%118 = OpVectorTimesScalar %v3float %113 %117
%119 = OpCompositeConstruct %v3float %105 %105 %105
%120 = OpFAdd %v3float %119 %118
OpStore %101 %120
OpBranch %104
%103 = OpLabel
%121 = OpLoad %v3float %result
OpStore %101 %121
OpBranch %104
%104 = OpLabel
%122 = OpLoad %v3float %101
OpReturnValue %122
OpFunctionEnd
%_blend_color_saturation = OpFunction %float None %20
%123 = OpFunctionParameter %_ptr_Function_v3float
%124 = OpLabel
%127 = OpLoad %v3float %123
%128 = OpCompositeExtract %float %127 0
%129 = OpLoad %v3float %123
%130 = OpCompositeExtract %float %129 1
%126 = OpExtInst %float %1 FMax %128 %130
%131 = OpLoad %v3float %123
%132 = OpCompositeExtract %float %131 2
%125 = OpExtInst %float %1 FMax %126 %132
%135 = OpLoad %v3float %123
%136 = OpCompositeExtract %float %135 0
%137 = OpLoad %v3float %123
%138 = OpCompositeExtract %float %137 1
%134 = OpExtInst %float %1 FMin %136 %138
%139 = OpLoad %v3float %123
%140 = OpCompositeExtract %float %139 2
%133 = OpExtInst %float %1 FMin %134 %140
%141 = OpFSub %float %125 %133
OpReturnValue %141
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %142
%143 = OpFunctionParameter %_ptr_Function_v3float
%144 = OpFunctionParameter %_ptr_Function_float
%145 = OpLabel
%151 = OpVariable %_ptr_Function_v3float Function
%146 = OpLoad %v3float %143
%147 = OpCompositeExtract %float %146 0
%148 = OpLoad %v3float %143
%149 = OpCompositeExtract %float %148 2
%150 = OpFOrdLessThan %bool %147 %149
OpSelectionMerge %154 None
OpBranchConditional %150 %152 %153
%152 = OpLabel
%155 = OpLoad %float %144
%156 = OpLoad %v3float %143
%157 = OpCompositeExtract %float %156 1
%158 = OpLoad %v3float %143
%159 = OpCompositeExtract %float %158 0
%160 = OpFSub %float %157 %159
%161 = OpFMul %float %155 %160
%162 = OpLoad %v3float %143
%163 = OpCompositeExtract %float %162 2
%164 = OpLoad %v3float %143
%165 = OpCompositeExtract %float %164 0
%166 = OpFSub %float %163 %165
%167 = OpFDiv %float %161 %166
%168 = OpLoad %float %144
%169 = OpCompositeConstruct %v3float %float_0 %167 %168
OpStore %151 %169
OpBranch %154
%153 = OpLabel
OpStore %151 %170
OpBranch %154
%154 = OpLabel
%171 = OpLoad %v3float %151
OpReturnValue %171
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %172
%173 = OpFunctionParameter %_ptr_Function_v3float
%174 = OpFunctionParameter %_ptr_Function_v3float
%175 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%211 = OpVariable %_ptr_Function_v3float Function
%213 = OpVariable %_ptr_Function_float Function
%225 = OpVariable %_ptr_Function_v3float Function
%227 = OpVariable %_ptr_Function_float Function
%232 = OpVariable %_ptr_Function_v3float Function
%234 = OpVariable %_ptr_Function_float Function
%247 = OpVariable %_ptr_Function_v3float Function
%249 = OpVariable %_ptr_Function_float Function
%262 = OpVariable %_ptr_Function_v3float Function
%264 = OpVariable %_ptr_Function_float Function
%269 = OpVariable %_ptr_Function_v3float Function
%271 = OpVariable %_ptr_Function_float Function
%179 = OpLoad %v3float %174
%180 = OpCompositeExtract %float %179 0
%181 = OpLoad %v3float %174
%182 = OpCompositeExtract %float %181 1
%178 = OpExtInst %float %1 FMax %180 %182
%183 = OpLoad %v3float %174
%184 = OpCompositeExtract %float %183 2
%177 = OpExtInst %float %1 FMax %178 %184
%187 = OpLoad %v3float %174
%188 = OpCompositeExtract %float %187 0
%189 = OpLoad %v3float %174
%190 = OpCompositeExtract %float %189 1
%186 = OpExtInst %float %1 FMin %188 %190
%191 = OpLoad %v3float %174
%192 = OpCompositeExtract %float %191 2
%185 = OpExtInst %float %1 FMin %186 %192
%193 = OpFSub %float %177 %185
OpStore %sat %193
%194 = OpLoad %v3float %173
%195 = OpCompositeExtract %float %194 0
%196 = OpLoad %v3float %173
%197 = OpCompositeExtract %float %196 1
%198 = OpFOrdLessThanEqual %bool %195 %197
OpSelectionMerge %201 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%202 = OpLoad %v3float %173
%203 = OpCompositeExtract %float %202 1
%204 = OpLoad %v3float %173
%205 = OpCompositeExtract %float %204 2
%206 = OpFOrdLessThanEqual %bool %203 %205
OpSelectionMerge %209 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%210 = OpLoad %v3float %173
OpStore %211 %210
%212 = OpLoad %float %sat
OpStore %213 %212
%214 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %211 %213
OpReturnValue %214
%208 = OpLabel
%215 = OpLoad %v3float %173
%216 = OpCompositeExtract %float %215 0
%217 = OpLoad %v3float %173
%218 = OpCompositeExtract %float %217 2
%219 = OpFOrdLessThanEqual %bool %216 %218
OpSelectionMerge %222 None
OpBranchConditional %219 %220 %221
%220 = OpLabel
%223 = OpLoad %v3float %173
%224 = OpVectorShuffle %v3float %223 %223 0 2 1
OpStore %225 %224
%226 = OpLoad %float %sat
OpStore %227 %226
%228 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %225 %227
%229 = OpVectorShuffle %v3float %228 %228 0 2 1
OpReturnValue %229
%221 = OpLabel
%230 = OpLoad %v3float %173
%231 = OpVectorShuffle %v3float %230 %230 2 0 1
OpStore %232 %231
%233 = OpLoad %float %sat
OpStore %234 %233
%235 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %232 %234
%236 = OpVectorShuffle %v3float %235 %235 1 2 0
OpReturnValue %236
%222 = OpLabel
OpBranch %209
%209 = OpLabel
OpBranch %201
%200 = OpLabel
%237 = OpLoad %v3float %173
%238 = OpCompositeExtract %float %237 0
%239 = OpLoad %v3float %173
%240 = OpCompositeExtract %float %239 2
%241 = OpFOrdLessThanEqual %bool %238 %240
OpSelectionMerge %244 None
OpBranchConditional %241 %242 %243
%242 = OpLabel
%245 = OpLoad %v3float %173
%246 = OpVectorShuffle %v3float %245 %245 1 0 2
OpStore %247 %246
%248 = OpLoad %float %sat
OpStore %249 %248
%250 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %247 %249
%251 = OpVectorShuffle %v3float %250 %250 1 0 2
OpReturnValue %251
%243 = OpLabel
%252 = OpLoad %v3float %173
%253 = OpCompositeExtract %float %252 1
%254 = OpLoad %v3float %173
%255 = OpCompositeExtract %float %254 2
%256 = OpFOrdLessThanEqual %bool %253 %255
OpSelectionMerge %259 None
OpBranchConditional %256 %257 %258
%257 = OpLabel
%260 = OpLoad %v3float %173
%261 = OpVectorShuffle %v3float %260 %260 1 2 0
OpStore %262 %261
%263 = OpLoad %float %sat
OpStore %264 %263
%265 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %262 %264
%266 = OpVectorShuffle %v3float %265 %265 2 0 1
OpReturnValue %266
%258 = OpLabel
%267 = OpLoad %v3float %173
%268 = OpVectorShuffle %v3float %267 %267 2 1 0
OpStore %269 %268
%270 = OpLoad %float %sat
OpStore %271 %270
%272 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %269 %271
%273 = OpVectorShuffle %v3float %272 %272 2 1 0
OpReturnValue %273
%259 = OpLabel
OpBranch %244
%244 = OpLabel
OpBranch %201
%201 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_hue = OpFunction %v4float None %274
%276 = OpFunctionParameter %_ptr_Function_v4float
%277 = OpFunctionParameter %_ptr_Function_v4float
%278 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%298 = OpVariable %_ptr_Function_v3float Function
%300 = OpVariable %_ptr_Function_v3float Function
%302 = OpVariable %_ptr_Function_v3float Function
%304 = OpVariable %_ptr_Function_float Function
%306 = OpVariable %_ptr_Function_v3float Function
%280 = OpLoad %v4float %277
%281 = OpCompositeExtract %float %280 3
%282 = OpLoad %v4float %276
%283 = OpCompositeExtract %float %282 3
%284 = OpFMul %float %281 %283
OpStore %alpha %284
%286 = OpLoad %v4float %276
%287 = OpVectorShuffle %v3float %286 %286 0 1 2
%288 = OpLoad %v4float %277
%289 = OpCompositeExtract %float %288 3
%290 = OpVectorTimesScalar %v3float %287 %289
OpStore %sda %290
%292 = OpLoad %v4float %277
%293 = OpVectorShuffle %v3float %292 %292 0 1 2
%294 = OpLoad %v4float %276
%295 = OpCompositeExtract %float %294 3
%296 = OpVectorTimesScalar %v3float %293 %295
OpStore %dsa %296
%297 = OpLoad %v3float %sda
OpStore %298 %297
%299 = OpLoad %v3float %dsa
OpStore %300 %299
%301 = OpFunctionCall %v3float %_blend_set_color_saturation %298 %300
OpStore %302 %301
%303 = OpLoad %float %alpha
OpStore %304 %303
%305 = OpLoad %v3float %dsa
OpStore %306 %305
%307 = OpFunctionCall %v3float %_blend_set_color_luminance %302 %304 %306
%308 = OpLoad %v4float %277
%309 = OpVectorShuffle %v3float %308 %308 0 1 2
%310 = OpFAdd %v3float %307 %309
%311 = OpLoad %v3float %dsa
%312 = OpFSub %v3float %310 %311
%313 = OpLoad %v4float %276
%314 = OpVectorShuffle %v3float %313 %313 0 1 2
%315 = OpFAdd %v3float %312 %314
%316 = OpLoad %v3float %sda
%317 = OpFSub %v3float %315 %316
%318 = OpCompositeExtract %float %317 0
%319 = OpCompositeExtract %float %317 1
%320 = OpCompositeExtract %float %317 2
%321 = OpLoad %v4float %276
%322 = OpCompositeExtract %float %321 3
%323 = OpLoad %v4float %277
%324 = OpCompositeExtract %float %323 3
%325 = OpFAdd %float %322 %324
%326 = OpLoad %float %alpha
%327 = OpFSub %float %325 %326
%328 = OpCompositeConstruct %v4float %318 %319 %320 %327
OpReturnValue %328
OpFunctionEnd
%main = OpFunction %void None %330
%331 = OpLabel
%333 = OpVariable %_ptr_Function_v4float Function
%335 = OpVariable %_ptr_Function_v4float Function
%332 = OpLoad %v4float %src
OpStore %333 %332
%334 = OpLoad %v4float %dst
OpStore %335 %334
%336 = OpFunctionCall %v4float %blend_hue %333 %335
OpStore %sk_FragColor %336
OpReturn
OpFunctionEnd

### Compilation failed:

error: SPIR-V validation error: Variable must be decorated with a location
  %src = OpVariable %_ptr_Input_v4float Input

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_guarded_divide "_guarded_divide"
OpName %_guarded_divide_0 "_guarded_divide"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
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
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%19 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%28 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_1 = OpConstant %float 1
%38 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%48 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%161 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%162 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%264 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%320 = OpTypeFunction %void
%_guarded_divide = OpFunction %float None %19
%21 = OpFunctionParameter %_ptr_Function_float
%22 = OpFunctionParameter %_ptr_Function_float
%23 = OpLabel
%24 = OpLoad %float %21
%25 = OpLoad %float %22
%26 = OpFDiv %float %24 %25
OpReturnValue %26
OpFunctionEnd
%_guarded_divide_0 = OpFunction %v3float None %28
%30 = OpFunctionParameter %_ptr_Function_v3float
%31 = OpFunctionParameter %_ptr_Function_float
%32 = OpLabel
%33 = OpLoad %v3float %30
%34 = OpLoad %float %31
%36 = OpFDiv %float %float_1 %34
%37 = OpVectorTimesScalar %v3float %33 %36
OpReturnValue %37
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %38
%39 = OpFunctionParameter %_ptr_Function_v3float
%40 = OpFunctionParameter %_ptr_Function_float
%41 = OpFunctionParameter %_ptr_Function_v3float
%42 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%94 = OpVariable %_ptr_Function_float Function
%98 = OpVariable %_ptr_Function_float Function
%124 = OpVariable %_ptr_Function_v3float Function
%128 = OpVariable %_ptr_Function_float Function
%49 = OpLoad %v3float %41
%44 = OpDot %float %48 %49
OpStore %lum %44
%51 = OpLoad %float %lum
%53 = OpLoad %v3float %39
%52 = OpDot %float %48 %53
%54 = OpFSub %float %51 %52
%55 = OpLoad %v3float %39
%56 = OpCompositeConstruct %v3float %54 %54 %54
%57 = OpFAdd %v3float %56 %55
OpStore %result %57
%61 = OpLoad %v3float %result
%62 = OpCompositeExtract %float %61 0
%63 = OpLoad %v3float %result
%64 = OpCompositeExtract %float %63 1
%60 = OpExtInst %float %1 FMin %62 %64
%65 = OpLoad %v3float %result
%66 = OpCompositeExtract %float %65 2
%59 = OpExtInst %float %1 FMin %60 %66
OpStore %minComp %59
%70 = OpLoad %v3float %result
%71 = OpCompositeExtract %float %70 0
%72 = OpLoad %v3float %result
%73 = OpCompositeExtract %float %72 1
%69 = OpExtInst %float %1 FMax %71 %73
%74 = OpLoad %v3float %result
%75 = OpCompositeExtract %float %74 2
%68 = OpExtInst %float %1 FMax %69 %75
OpStore %maxComp %68
%77 = OpLoad %float %minComp
%79 = OpFOrdLessThan %bool %77 %float_0
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %float %lum
%83 = OpLoad %float %minComp
%84 = OpFOrdNotEqual %bool %82 %83
OpBranch %81
%81 = OpLabel
%85 = OpPhi %bool %false %42 %84 %80
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %float %lum
%89 = OpLoad %v3float %result
%90 = OpLoad %float %lum
%91 = OpCompositeConstruct %v3float %90 %90 %90
%92 = OpFSub %v3float %89 %91
%93 = OpLoad %float %lum
OpStore %94 %93
%95 = OpLoad %float %lum
%96 = OpLoad %float %minComp
%97 = OpFSub %float %95 %96
OpStore %98 %97
%99 = OpFunctionCall %float %_guarded_divide %94 %98
%100 = OpVectorTimesScalar %v3float %92 %99
%101 = OpCompositeConstruct %v3float %88 %88 %88
%102 = OpFAdd %v3float %101 %100
OpStore %result %102
OpBranch %87
%87 = OpLabel
%103 = OpLoad %float %maxComp
%104 = OpLoad %float %40
%105 = OpFOrdGreaterThan %bool %103 %104
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %float %maxComp
%109 = OpLoad %float %lum
%110 = OpFOrdNotEqual %bool %108 %109
OpBranch %107
%107 = OpLabel
%111 = OpPhi %bool %false %87 %110 %106
OpSelectionMerge %114 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpLoad %float %lum
%116 = OpLoad %v3float %result
%117 = OpLoad %float %lum
%118 = OpCompositeConstruct %v3float %117 %117 %117
%119 = OpFSub %v3float %116 %118
%120 = OpLoad %float %40
%121 = OpLoad %float %lum
%122 = OpFSub %float %120 %121
%123 = OpVectorTimesScalar %v3float %119 %122
OpStore %124 %123
%125 = OpLoad %float %maxComp
%126 = OpLoad %float %lum
%127 = OpFSub %float %125 %126
OpStore %128 %127
%129 = OpFunctionCall %v3float %_guarded_divide_0 %124 %128
%130 = OpCompositeConstruct %v3float %115 %115 %115
%131 = OpFAdd %v3float %130 %129
OpReturnValue %131
%113 = OpLabel
%132 = OpLoad %v3float %result
OpReturnValue %132
%114 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %28
%133 = OpFunctionParameter %_ptr_Function_v3float
%134 = OpFunctionParameter %_ptr_Function_float
%135 = OpLabel
%151 = OpVariable %_ptr_Function_float Function
%157 = OpVariable %_ptr_Function_float Function
%136 = OpLoad %v3float %133
%137 = OpCompositeExtract %float %136 0
%138 = OpLoad %v3float %133
%139 = OpCompositeExtract %float %138 2
%140 = OpFOrdLessThan %bool %137 %139
OpSelectionMerge %143 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%144 = OpLoad %float %134
%145 = OpLoad %v3float %133
%146 = OpCompositeExtract %float %145 1
%147 = OpLoad %v3float %133
%148 = OpCompositeExtract %float %147 0
%149 = OpFSub %float %146 %148
%150 = OpFMul %float %144 %149
OpStore %151 %150
%152 = OpLoad %v3float %133
%153 = OpCompositeExtract %float %152 2
%154 = OpLoad %v3float %133
%155 = OpCompositeExtract %float %154 0
%156 = OpFSub %float %153 %155
OpStore %157 %156
%158 = OpFunctionCall %float %_guarded_divide %151 %157
%159 = OpLoad %float %134
%160 = OpCompositeConstruct %v3float %float_0 %158 %159
OpReturnValue %160
%142 = OpLabel
OpReturnValue %161
%143 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %162
%163 = OpFunctionParameter %_ptr_Function_v3float
%164 = OpFunctionParameter %_ptr_Function_v3float
%165 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%201 = OpVariable %_ptr_Function_v3float Function
%203 = OpVariable %_ptr_Function_float Function
%215 = OpVariable %_ptr_Function_v3float Function
%217 = OpVariable %_ptr_Function_float Function
%222 = OpVariable %_ptr_Function_v3float Function
%224 = OpVariable %_ptr_Function_float Function
%237 = OpVariable %_ptr_Function_v3float Function
%239 = OpVariable %_ptr_Function_float Function
%252 = OpVariable %_ptr_Function_v3float Function
%254 = OpVariable %_ptr_Function_float Function
%259 = OpVariable %_ptr_Function_v3float Function
%261 = OpVariable %_ptr_Function_float Function
%169 = OpLoad %v3float %164
%170 = OpCompositeExtract %float %169 0
%171 = OpLoad %v3float %164
%172 = OpCompositeExtract %float %171 1
%168 = OpExtInst %float %1 FMax %170 %172
%173 = OpLoad %v3float %164
%174 = OpCompositeExtract %float %173 2
%167 = OpExtInst %float %1 FMax %168 %174
%177 = OpLoad %v3float %164
%178 = OpCompositeExtract %float %177 0
%179 = OpLoad %v3float %164
%180 = OpCompositeExtract %float %179 1
%176 = OpExtInst %float %1 FMin %178 %180
%181 = OpLoad %v3float %164
%182 = OpCompositeExtract %float %181 2
%175 = OpExtInst %float %1 FMin %176 %182
%183 = OpFSub %float %167 %175
OpStore %sat %183
%184 = OpLoad %v3float %163
%185 = OpCompositeExtract %float %184 0
%186 = OpLoad %v3float %163
%187 = OpCompositeExtract %float %186 1
%188 = OpFOrdLessThanEqual %bool %185 %187
OpSelectionMerge %191 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%192 = OpLoad %v3float %163
%193 = OpCompositeExtract %float %192 1
%194 = OpLoad %v3float %163
%195 = OpCompositeExtract %float %194 2
%196 = OpFOrdLessThanEqual %bool %193 %195
OpSelectionMerge %199 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%200 = OpLoad %v3float %163
OpStore %201 %200
%202 = OpLoad %float %sat
OpStore %203 %202
%204 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %201 %203
OpReturnValue %204
%198 = OpLabel
%205 = OpLoad %v3float %163
%206 = OpCompositeExtract %float %205 0
%207 = OpLoad %v3float %163
%208 = OpCompositeExtract %float %207 2
%209 = OpFOrdLessThanEqual %bool %206 %208
OpSelectionMerge %212 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%213 = OpLoad %v3float %163
%214 = OpVectorShuffle %v3float %213 %213 0 2 1
OpStore %215 %214
%216 = OpLoad %float %sat
OpStore %217 %216
%218 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %215 %217
%219 = OpVectorShuffle %v3float %218 %218 0 2 1
OpReturnValue %219
%211 = OpLabel
%220 = OpLoad %v3float %163
%221 = OpVectorShuffle %v3float %220 %220 2 0 1
OpStore %222 %221
%223 = OpLoad %float %sat
OpStore %224 %223
%225 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %222 %224
%226 = OpVectorShuffle %v3float %225 %225 1 2 0
OpReturnValue %226
%212 = OpLabel
OpBranch %199
%199 = OpLabel
OpBranch %191
%190 = OpLabel
%227 = OpLoad %v3float %163
%228 = OpCompositeExtract %float %227 0
%229 = OpLoad %v3float %163
%230 = OpCompositeExtract %float %229 2
%231 = OpFOrdLessThanEqual %bool %228 %230
OpSelectionMerge %234 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%235 = OpLoad %v3float %163
%236 = OpVectorShuffle %v3float %235 %235 1 0 2
OpStore %237 %236
%238 = OpLoad %float %sat
OpStore %239 %238
%240 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %237 %239
%241 = OpVectorShuffle %v3float %240 %240 1 0 2
OpReturnValue %241
%233 = OpLabel
%242 = OpLoad %v3float %163
%243 = OpCompositeExtract %float %242 1
%244 = OpLoad %v3float %163
%245 = OpCompositeExtract %float %244 2
%246 = OpFOrdLessThanEqual %bool %243 %245
OpSelectionMerge %249 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
%250 = OpLoad %v3float %163
%251 = OpVectorShuffle %v3float %250 %250 1 2 0
OpStore %252 %251
%253 = OpLoad %float %sat
OpStore %254 %253
%255 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %252 %254
%256 = OpVectorShuffle %v3float %255 %255 2 0 1
OpReturnValue %256
%248 = OpLabel
%257 = OpLoad %v3float %163
%258 = OpVectorShuffle %v3float %257 %257 2 1 0
OpStore %259 %258
%260 = OpLoad %float %sat
OpStore %261 %260
%262 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %259 %261
%263 = OpVectorShuffle %v3float %262 %262 2 1 0
OpReturnValue %263
%249 = OpLabel
OpBranch %234
%234 = OpLabel
OpBranch %191
%191 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_hue = OpFunction %v4float None %264
%266 = OpFunctionParameter %_ptr_Function_v4float
%267 = OpFunctionParameter %_ptr_Function_v4float
%268 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%288 = OpVariable %_ptr_Function_v3float Function
%290 = OpVariable %_ptr_Function_v3float Function
%292 = OpVariable %_ptr_Function_v3float Function
%294 = OpVariable %_ptr_Function_float Function
%296 = OpVariable %_ptr_Function_v3float Function
%270 = OpLoad %v4float %267
%271 = OpCompositeExtract %float %270 3
%272 = OpLoad %v4float %266
%273 = OpCompositeExtract %float %272 3
%274 = OpFMul %float %271 %273
OpStore %alpha %274
%276 = OpLoad %v4float %266
%277 = OpVectorShuffle %v3float %276 %276 0 1 2
%278 = OpLoad %v4float %267
%279 = OpCompositeExtract %float %278 3
%280 = OpVectorTimesScalar %v3float %277 %279
OpStore %sda %280
%282 = OpLoad %v4float %267
%283 = OpVectorShuffle %v3float %282 %282 0 1 2
%284 = OpLoad %v4float %266
%285 = OpCompositeExtract %float %284 3
%286 = OpVectorTimesScalar %v3float %283 %285
OpStore %dsa %286
%287 = OpLoad %v3float %sda
OpStore %288 %287
%289 = OpLoad %v3float %dsa
OpStore %290 %289
%291 = OpFunctionCall %v3float %_blend_set_color_saturation %288 %290
OpStore %292 %291
%293 = OpLoad %float %alpha
OpStore %294 %293
%295 = OpLoad %v3float %dsa
OpStore %296 %295
%297 = OpFunctionCall %v3float %_blend_set_color_luminance %292 %294 %296
%298 = OpLoad %v4float %267
%299 = OpVectorShuffle %v3float %298 %298 0 1 2
%300 = OpFAdd %v3float %297 %299
%301 = OpLoad %v3float %dsa
%302 = OpFSub %v3float %300 %301
%303 = OpLoad %v4float %266
%304 = OpVectorShuffle %v3float %303 %303 0 1 2
%305 = OpFAdd %v3float %302 %304
%306 = OpLoad %v3float %sda
%307 = OpFSub %v3float %305 %306
%308 = OpCompositeExtract %float %307 0
%309 = OpCompositeExtract %float %307 1
%310 = OpCompositeExtract %float %307 2
%311 = OpLoad %v4float %266
%312 = OpCompositeExtract %float %311 3
%313 = OpLoad %v4float %267
%314 = OpCompositeExtract %float %313 3
%315 = OpFAdd %float %312 %314
%316 = OpLoad %float %alpha
%317 = OpFSub %float %315 %316
%318 = OpCompositeConstruct %v4float %308 %309 %310 %317
OpReturnValue %318
OpFunctionEnd
%main = OpFunction %void None %320
%321 = OpLabel
%323 = OpVariable %_ptr_Function_v4float Function
%325 = OpVariable %_ptr_Function_v4float Function
%322 = OpLoad %v4float %src
OpStore %323 %322
%324 = OpLoad %v4float %dst
OpStore %325 %324
%326 = OpFunctionCall %v4float %blend_hue %323 %325
OpStore %sk_FragColor %326
OpReturn
OpFunctionEnd

1 error

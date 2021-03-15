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
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_4_d "_4_d"
OpName %_5_n "_5_n"
OpName %_6_d "_6_d"
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %_7_n "_7_n"
OpName %_8_d "_8_d"
OpName %_blend_set_color_saturation "_blend_set_color_saturation"
OpName %sat "sat"
OpName %blend_saturation "blend_saturation"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%18 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%30 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%119 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%150 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%151 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%253 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%309 = OpTypeFunction %void
%_blend_set_color_luminance = OpFunction %v3float None %18
%21 = OpFunctionParameter %_ptr_Function_v3float
%22 = OpFunctionParameter %_ptr_Function_float
%23 = OpFunctionParameter %_ptr_Function_v3float
%24 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%_4_d = OpVariable %_ptr_Function_float Function
%_5_n = OpVariable %_ptr_Function_v3float Function
%_6_d = OpVariable %_ptr_Function_float Function
%31 = OpLoad %v3float %23
%26 = OpDot %float %30 %31
OpStore %lum %26
%33 = OpLoad %float %lum
%35 = OpLoad %v3float %21
%34 = OpDot %float %30 %35
%36 = OpFSub %float %33 %34
%37 = OpLoad %v3float %21
%38 = OpCompositeConstruct %v3float %36 %36 %36
%39 = OpFAdd %v3float %38 %37
OpStore %result %39
%43 = OpLoad %v3float %result
%44 = OpCompositeExtract %float %43 0
%45 = OpLoad %v3float %result
%46 = OpCompositeExtract %float %45 1
%42 = OpExtInst %float %1 FMin %44 %46
%47 = OpLoad %v3float %result
%48 = OpCompositeExtract %float %47 2
%41 = OpExtInst %float %1 FMin %42 %48
OpStore %minComp %41
%52 = OpLoad %v3float %result
%53 = OpCompositeExtract %float %52 0
%54 = OpLoad %v3float %result
%55 = OpCompositeExtract %float %54 1
%51 = OpExtInst %float %1 FMax %53 %55
%56 = OpLoad %v3float %result
%57 = OpCompositeExtract %float %56 2
%50 = OpExtInst %float %1 FMax %51 %57
OpStore %maxComp %50
%59 = OpLoad %float %minComp
%61 = OpFOrdLessThan %bool %59 %float_0
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpLoad %float %lum
%65 = OpLoad %float %minComp
%66 = OpFOrdNotEqual %bool %64 %65
OpBranch %63
%63 = OpLabel
%67 = OpPhi %bool %false %24 %66 %62
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpLoad %float %lum
%72 = OpLoad %float %minComp
%73 = OpFSub %float %71 %72
OpStore %_4_d %73
%74 = OpLoad %float %lum
%75 = OpLoad %v3float %result
%76 = OpLoad %float %lum
%77 = OpCompositeConstruct %v3float %76 %76 %76
%78 = OpFSub %v3float %75 %77
%79 = OpLoad %float %lum
%80 = OpLoad %float %_4_d
%81 = OpFDiv %float %79 %80
%82 = OpVectorTimesScalar %v3float %78 %81
%83 = OpCompositeConstruct %v3float %74 %74 %74
%84 = OpFAdd %v3float %83 %82
OpStore %result %84
OpBranch %69
%69 = OpLabel
%85 = OpLoad %float %maxComp
%86 = OpLoad %float %22
%87 = OpFOrdGreaterThan %bool %85 %86
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %float %maxComp
%91 = OpLoad %float %lum
%92 = OpFOrdNotEqual %bool %90 %91
OpBranch %89
%89 = OpLabel
%93 = OpPhi %bool %false %69 %92 %88
OpSelectionMerge %96 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%98 = OpLoad %v3float %result
%99 = OpLoad %float %lum
%100 = OpCompositeConstruct %v3float %99 %99 %99
%101 = OpFSub %v3float %98 %100
%102 = OpLoad %float %22
%103 = OpLoad %float %lum
%104 = OpFSub %float %102 %103
%105 = OpVectorTimesScalar %v3float %101 %104
OpStore %_5_n %105
%107 = OpLoad %float %maxComp
%108 = OpLoad %float %lum
%109 = OpFSub %float %107 %108
OpStore %_6_d %109
%110 = OpLoad %float %lum
%111 = OpLoad %v3float %_5_n
%112 = OpLoad %float %_6_d
%114 = OpFDiv %float %float_1 %112
%115 = OpVectorTimesScalar %v3float %111 %114
%116 = OpCompositeConstruct %v3float %110 %110 %110
%117 = OpFAdd %v3float %116 %115
OpReturnValue %117
%95 = OpLabel
%118 = OpLoad %v3float %result
OpReturnValue %118
%96 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %119
%120 = OpFunctionParameter %_ptr_Function_v3float
%121 = OpFunctionParameter %_ptr_Function_float
%122 = OpLabel
%_7_n = OpVariable %_ptr_Function_float Function
%_8_d = OpVariable %_ptr_Function_float Function
%123 = OpLoad %v3float %120
%124 = OpCompositeExtract %float %123 0
%125 = OpLoad %v3float %120
%126 = OpCompositeExtract %float %125 2
%127 = OpFOrdLessThan %bool %124 %126
OpSelectionMerge %130 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%132 = OpLoad %float %121
%133 = OpLoad %v3float %120
%134 = OpCompositeExtract %float %133 1
%135 = OpLoad %v3float %120
%136 = OpCompositeExtract %float %135 0
%137 = OpFSub %float %134 %136
%138 = OpFMul %float %132 %137
OpStore %_7_n %138
%140 = OpLoad %v3float %120
%141 = OpCompositeExtract %float %140 2
%142 = OpLoad %v3float %120
%143 = OpCompositeExtract %float %142 0
%144 = OpFSub %float %141 %143
OpStore %_8_d %144
%145 = OpLoad %float %_7_n
%146 = OpLoad %float %_8_d
%147 = OpFDiv %float %145 %146
%148 = OpLoad %float %121
%149 = OpCompositeConstruct %v3float %float_0 %147 %148
OpReturnValue %149
%129 = OpLabel
OpReturnValue %150
%130 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %151
%152 = OpFunctionParameter %_ptr_Function_v3float
%153 = OpFunctionParameter %_ptr_Function_v3float
%154 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%190 = OpVariable %_ptr_Function_v3float Function
%192 = OpVariable %_ptr_Function_float Function
%204 = OpVariable %_ptr_Function_v3float Function
%206 = OpVariable %_ptr_Function_float Function
%211 = OpVariable %_ptr_Function_v3float Function
%213 = OpVariable %_ptr_Function_float Function
%226 = OpVariable %_ptr_Function_v3float Function
%228 = OpVariable %_ptr_Function_float Function
%241 = OpVariable %_ptr_Function_v3float Function
%243 = OpVariable %_ptr_Function_float Function
%248 = OpVariable %_ptr_Function_v3float Function
%250 = OpVariable %_ptr_Function_float Function
%158 = OpLoad %v3float %153
%159 = OpCompositeExtract %float %158 0
%160 = OpLoad %v3float %153
%161 = OpCompositeExtract %float %160 1
%157 = OpExtInst %float %1 FMax %159 %161
%162 = OpLoad %v3float %153
%163 = OpCompositeExtract %float %162 2
%156 = OpExtInst %float %1 FMax %157 %163
%166 = OpLoad %v3float %153
%167 = OpCompositeExtract %float %166 0
%168 = OpLoad %v3float %153
%169 = OpCompositeExtract %float %168 1
%165 = OpExtInst %float %1 FMin %167 %169
%170 = OpLoad %v3float %153
%171 = OpCompositeExtract %float %170 2
%164 = OpExtInst %float %1 FMin %165 %171
%172 = OpFSub %float %156 %164
OpStore %sat %172
%173 = OpLoad %v3float %152
%174 = OpCompositeExtract %float %173 0
%175 = OpLoad %v3float %152
%176 = OpCompositeExtract %float %175 1
%177 = OpFOrdLessThanEqual %bool %174 %176
OpSelectionMerge %180 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%181 = OpLoad %v3float %152
%182 = OpCompositeExtract %float %181 1
%183 = OpLoad %v3float %152
%184 = OpCompositeExtract %float %183 2
%185 = OpFOrdLessThanEqual %bool %182 %184
OpSelectionMerge %188 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%189 = OpLoad %v3float %152
OpStore %190 %189
%191 = OpLoad %float %sat
OpStore %192 %191
%193 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %190 %192
OpReturnValue %193
%187 = OpLabel
%194 = OpLoad %v3float %152
%195 = OpCompositeExtract %float %194 0
%196 = OpLoad %v3float %152
%197 = OpCompositeExtract %float %196 2
%198 = OpFOrdLessThanEqual %bool %195 %197
OpSelectionMerge %201 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%202 = OpLoad %v3float %152
%203 = OpVectorShuffle %v3float %202 %202 0 2 1
OpStore %204 %203
%205 = OpLoad %float %sat
OpStore %206 %205
%207 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %204 %206
%208 = OpVectorShuffle %v3float %207 %207 0 2 1
OpReturnValue %208
%200 = OpLabel
%209 = OpLoad %v3float %152
%210 = OpVectorShuffle %v3float %209 %209 2 0 1
OpStore %211 %210
%212 = OpLoad %float %sat
OpStore %213 %212
%214 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %211 %213
%215 = OpVectorShuffle %v3float %214 %214 1 2 0
OpReturnValue %215
%201 = OpLabel
OpBranch %188
%188 = OpLabel
OpBranch %180
%179 = OpLabel
%216 = OpLoad %v3float %152
%217 = OpCompositeExtract %float %216 0
%218 = OpLoad %v3float %152
%219 = OpCompositeExtract %float %218 2
%220 = OpFOrdLessThanEqual %bool %217 %219
OpSelectionMerge %223 None
OpBranchConditional %220 %221 %222
%221 = OpLabel
%224 = OpLoad %v3float %152
%225 = OpVectorShuffle %v3float %224 %224 1 0 2
OpStore %226 %225
%227 = OpLoad %float %sat
OpStore %228 %227
%229 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %226 %228
%230 = OpVectorShuffle %v3float %229 %229 1 0 2
OpReturnValue %230
%222 = OpLabel
%231 = OpLoad %v3float %152
%232 = OpCompositeExtract %float %231 1
%233 = OpLoad %v3float %152
%234 = OpCompositeExtract %float %233 2
%235 = OpFOrdLessThanEqual %bool %232 %234
OpSelectionMerge %238 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
%239 = OpLoad %v3float %152
%240 = OpVectorShuffle %v3float %239 %239 1 2 0
OpStore %241 %240
%242 = OpLoad %float %sat
OpStore %243 %242
%244 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %241 %243
%245 = OpVectorShuffle %v3float %244 %244 2 0 1
OpReturnValue %245
%237 = OpLabel
%246 = OpLoad %v3float %152
%247 = OpVectorShuffle %v3float %246 %246 2 1 0
OpStore %248 %247
%249 = OpLoad %float %sat
OpStore %250 %249
%251 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %248 %250
%252 = OpVectorShuffle %v3float %251 %251 2 1 0
OpReturnValue %252
%238 = OpLabel
OpBranch %223
%223 = OpLabel
OpBranch %180
%180 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_saturation = OpFunction %v4float None %253
%255 = OpFunctionParameter %_ptr_Function_v4float
%256 = OpFunctionParameter %_ptr_Function_v4float
%257 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%277 = OpVariable %_ptr_Function_v3float Function
%279 = OpVariable %_ptr_Function_v3float Function
%281 = OpVariable %_ptr_Function_v3float Function
%283 = OpVariable %_ptr_Function_float Function
%285 = OpVariable %_ptr_Function_v3float Function
%259 = OpLoad %v4float %256
%260 = OpCompositeExtract %float %259 3
%261 = OpLoad %v4float %255
%262 = OpCompositeExtract %float %261 3
%263 = OpFMul %float %260 %262
OpStore %alpha %263
%265 = OpLoad %v4float %255
%266 = OpVectorShuffle %v3float %265 %265 0 1 2
%267 = OpLoad %v4float %256
%268 = OpCompositeExtract %float %267 3
%269 = OpVectorTimesScalar %v3float %266 %268
OpStore %sda %269
%271 = OpLoad %v4float %256
%272 = OpVectorShuffle %v3float %271 %271 0 1 2
%273 = OpLoad %v4float %255
%274 = OpCompositeExtract %float %273 3
%275 = OpVectorTimesScalar %v3float %272 %274
OpStore %dsa %275
%276 = OpLoad %v3float %dsa
OpStore %277 %276
%278 = OpLoad %v3float %sda
OpStore %279 %278
%280 = OpFunctionCall %v3float %_blend_set_color_saturation %277 %279
OpStore %281 %280
%282 = OpLoad %float %alpha
OpStore %283 %282
%284 = OpLoad %v3float %dsa
OpStore %285 %284
%286 = OpFunctionCall %v3float %_blend_set_color_luminance %281 %283 %285
%287 = OpLoad %v4float %256
%288 = OpVectorShuffle %v3float %287 %287 0 1 2
%289 = OpFAdd %v3float %286 %288
%290 = OpLoad %v3float %dsa
%291 = OpFSub %v3float %289 %290
%292 = OpLoad %v4float %255
%293 = OpVectorShuffle %v3float %292 %292 0 1 2
%294 = OpFAdd %v3float %291 %293
%295 = OpLoad %v3float %sda
%296 = OpFSub %v3float %294 %295
%297 = OpCompositeExtract %float %296 0
%298 = OpCompositeExtract %float %296 1
%299 = OpCompositeExtract %float %296 2
%300 = OpLoad %v4float %255
%301 = OpCompositeExtract %float %300 3
%302 = OpLoad %v4float %256
%303 = OpCompositeExtract %float %302 3
%304 = OpFAdd %float %301 %303
%305 = OpLoad %float %alpha
%306 = OpFSub %float %304 %305
%307 = OpCompositeConstruct %v4float %297 %298 %299 %306
OpReturnValue %307
OpFunctionEnd
%main = OpFunction %void None %309
%310 = OpLabel
%312 = OpVariable %_ptr_Function_v4float Function
%314 = OpVariable %_ptr_Function_v4float Function
%311 = OpLoad %v4float %src
OpStore %312 %311
%313 = OpLoad %v4float %dst
OpStore %314 %313
%315 = OpFunctionCall %v4float %blend_saturation %312 %314
OpStore %sk_FragColor %315
OpReturn
OpFunctionEnd

1 error

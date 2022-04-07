OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_set_color_saturation_helper_Qh3h3h "blend_set_color_saturation_helper_Qh3h3h"
OpName %blend_hslc_h4h4h4hb "blend_hslc_h4h4h4hb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_2_blend_set_color_saturation "_2_blend_set_color_saturation"
OpName %_3_sat "_3_sat"
OpName %_4_lum "_4_lum"
OpName %_5_result "_5_result"
OpName %_6_minComp "_6_minComp"
OpName %_7_maxComp "_7_maxComp"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %_2_blend_set_color_saturation RelaxedPrecision
OpDecorate %_3_sat RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %_4_lum RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %_5_result RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %_6_minComp RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %_7_maxComp RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_float = OpTypePointer Function %float
%16 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_0 = OpConstant %float 0
%46 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%47 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_float %_ptr_Function_bool
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%194 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%297 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_set_color_saturation_helper_Qh3h3h = OpFunction %v3float None %16
%19 = OpFunctionParameter %_ptr_Function_v3float
%20 = OpFunctionParameter %_ptr_Function_float
%21 = OpLabel
%22 = OpLoad %v3float %19
%23 = OpCompositeExtract %float %22 0
%24 = OpLoad %v3float %19
%25 = OpCompositeExtract %float %24 2
%26 = OpFOrdLessThan %bool %23 %25
OpSelectionMerge %29 None
OpBranchConditional %26 %27 %28
%27 = OpLabel
%31 = OpLoad %float %20
%32 = OpLoad %v3float %19
%33 = OpCompositeExtract %float %32 1
%34 = OpLoad %v3float %19
%35 = OpCompositeExtract %float %34 0
%36 = OpFSub %float %33 %35
%37 = OpFMul %float %31 %36
%38 = OpLoad %v3float %19
%39 = OpCompositeExtract %float %38 2
%40 = OpLoad %v3float %19
%41 = OpCompositeExtract %float %40 0
%42 = OpFSub %float %39 %41
%43 = OpFDiv %float %37 %42
%44 = OpLoad %float %20
%45 = OpCompositeConstruct %v3float %float_0 %43 %44
OpReturnValue %45
%28 = OpLabel
OpReturnValue %46
%29 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_hslc_h4h4h4hb = OpFunction %v4float None %47
%50 = OpFunctionParameter %_ptr_Function_v4float
%51 = OpFunctionParameter %_ptr_Function_v4float
%52 = OpFunctionParameter %_ptr_Function_float
%53 = OpFunctionParameter %_ptr_Function_bool
%54 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%_2_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_3_sat = OpVariable %_ptr_Function_float Function
%124 = OpVariable %_ptr_Function_v3float Function
%126 = OpVariable %_ptr_Function_float Function
%138 = OpVariable %_ptr_Function_v3float Function
%140 = OpVariable %_ptr_Function_float Function
%145 = OpVariable %_ptr_Function_v3float Function
%147 = OpVariable %_ptr_Function_float Function
%160 = OpVariable %_ptr_Function_v3float Function
%162 = OpVariable %_ptr_Function_float Function
%175 = OpVariable %_ptr_Function_v3float Function
%177 = OpVariable %_ptr_Function_float Function
%182 = OpVariable %_ptr_Function_v3float Function
%184 = OpVariable %_ptr_Function_float Function
%_4_lum = OpVariable %_ptr_Function_float Function
%_5_result = OpVariable %_ptr_Function_v3float Function
%_6_minComp = OpVariable %_ptr_Function_float Function
%_7_maxComp = OpVariable %_ptr_Function_float Function
%56 = OpLoad %v4float %51
%57 = OpCompositeExtract %float %56 3
%58 = OpLoad %v4float %50
%59 = OpCompositeExtract %float %58 3
%60 = OpFMul %float %57 %59
OpStore %alpha %60
%62 = OpLoad %v4float %50
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%64 = OpLoad %v4float %51
%65 = OpCompositeExtract %float %64 3
%66 = OpVectorTimesScalar %v3float %63 %65
OpStore %sda %66
%68 = OpLoad %v4float %51
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%70 = OpLoad %v4float %50
%71 = OpCompositeExtract %float %70 3
%72 = OpVectorTimesScalar %v3float %69 %71
OpStore %dsa %72
%75 = OpLoad %v3float %sda
%76 = OpLoad %v3float %dsa
%77 = OpLoad %float %52
%78 = OpCompositeConstruct %v3float %77 %77 %77
%74 = OpExtInst %v3float %1 FMix %75 %76 %78
OpStore %l %74
%81 = OpLoad %v3float %dsa
%82 = OpLoad %v3float %sda
%83 = OpLoad %float %52
%84 = OpCompositeConstruct %v3float %83 %83 %83
%80 = OpExtInst %v3float %1 FMix %81 %82 %84
OpStore %r %80
%85 = OpLoad %bool %53
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%92 = OpLoad %v3float %r
%93 = OpCompositeExtract %float %92 0
%94 = OpLoad %v3float %r
%95 = OpCompositeExtract %float %94 1
%91 = OpExtInst %float %1 FMax %93 %95
%96 = OpLoad %v3float %r
%97 = OpCompositeExtract %float %96 2
%90 = OpExtInst %float %1 FMax %91 %97
%100 = OpLoad %v3float %r
%101 = OpCompositeExtract %float %100 0
%102 = OpLoad %v3float %r
%103 = OpCompositeExtract %float %102 1
%99 = OpExtInst %float %1 FMin %101 %103
%104 = OpLoad %v3float %r
%105 = OpCompositeExtract %float %104 2
%98 = OpExtInst %float %1 FMin %99 %105
%106 = OpFSub %float %90 %98
OpStore %_3_sat %106
%107 = OpLoad %v3float %l
%108 = OpCompositeExtract %float %107 0
%109 = OpLoad %v3float %l
%110 = OpCompositeExtract %float %109 1
%111 = OpFOrdLessThanEqual %bool %108 %110
OpSelectionMerge %114 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpLoad %v3float %l
%116 = OpCompositeExtract %float %115 1
%117 = OpLoad %v3float %l
%118 = OpCompositeExtract %float %117 2
%119 = OpFOrdLessThanEqual %bool %116 %118
OpSelectionMerge %122 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpLoad %v3float %l
OpStore %124 %123
%125 = OpLoad %float %_3_sat
OpStore %126 %125
%127 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %124 %126
OpStore %_2_blend_set_color_saturation %127
OpBranch %122
%121 = OpLabel
%128 = OpLoad %v3float %l
%129 = OpCompositeExtract %float %128 0
%130 = OpLoad %v3float %l
%131 = OpCompositeExtract %float %130 2
%132 = OpFOrdLessThanEqual %bool %129 %131
OpSelectionMerge %135 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%136 = OpLoad %v3float %l
%137 = OpVectorShuffle %v3float %136 %136 0 2 1
OpStore %138 %137
%139 = OpLoad %float %_3_sat
OpStore %140 %139
%141 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %138 %140
%142 = OpVectorShuffle %v3float %141 %141 0 2 1
OpStore %_2_blend_set_color_saturation %142
OpBranch %135
%134 = OpLabel
%143 = OpLoad %v3float %l
%144 = OpVectorShuffle %v3float %143 %143 2 0 1
OpStore %145 %144
%146 = OpLoad %float %_3_sat
OpStore %147 %146
%148 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %145 %147
%149 = OpVectorShuffle %v3float %148 %148 1 2 0
OpStore %_2_blend_set_color_saturation %149
OpBranch %135
%135 = OpLabel
OpBranch %122
%122 = OpLabel
OpBranch %114
%113 = OpLabel
%150 = OpLoad %v3float %l
%151 = OpCompositeExtract %float %150 0
%152 = OpLoad %v3float %l
%153 = OpCompositeExtract %float %152 2
%154 = OpFOrdLessThanEqual %bool %151 %153
OpSelectionMerge %157 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%158 = OpLoad %v3float %l
%159 = OpVectorShuffle %v3float %158 %158 1 0 2
OpStore %160 %159
%161 = OpLoad %float %_3_sat
OpStore %162 %161
%163 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %160 %162
%164 = OpVectorShuffle %v3float %163 %163 1 0 2
OpStore %_2_blend_set_color_saturation %164
OpBranch %157
%156 = OpLabel
%165 = OpLoad %v3float %l
%166 = OpCompositeExtract %float %165 1
%167 = OpLoad %v3float %l
%168 = OpCompositeExtract %float %167 2
%169 = OpFOrdLessThanEqual %bool %166 %168
OpSelectionMerge %172 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%173 = OpLoad %v3float %l
%174 = OpVectorShuffle %v3float %173 %173 1 2 0
OpStore %175 %174
%176 = OpLoad %float %_3_sat
OpStore %177 %176
%178 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %175 %177
%179 = OpVectorShuffle %v3float %178 %178 2 0 1
OpStore %_2_blend_set_color_saturation %179
OpBranch %172
%171 = OpLabel
%180 = OpLoad %v3float %l
%181 = OpVectorShuffle %v3float %180 %180 2 1 0
OpStore %182 %181
%183 = OpLoad %float %_3_sat
OpStore %184 %183
%185 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %182 %184
%186 = OpVectorShuffle %v3float %185 %185 2 1 0
OpStore %_2_blend_set_color_saturation %186
OpBranch %172
%172 = OpLabel
OpBranch %157
%157 = OpLabel
OpBranch %114
%114 = OpLabel
%187 = OpLoad %v3float %_2_blend_set_color_saturation
OpStore %l %187
%188 = OpLoad %v3float %dsa
OpStore %r %188
OpBranch %87
%87 = OpLabel
%195 = OpLoad %v3float %r
%190 = OpDot %float %194 %195
OpStore %_4_lum %190
%197 = OpLoad %float %_4_lum
%199 = OpLoad %v3float %l
%198 = OpDot %float %194 %199
%200 = OpFSub %float %197 %198
%201 = OpLoad %v3float %l
%202 = OpCompositeConstruct %v3float %200 %200 %200
%203 = OpFAdd %v3float %202 %201
OpStore %_5_result %203
%207 = OpLoad %v3float %_5_result
%208 = OpCompositeExtract %float %207 0
%209 = OpLoad %v3float %_5_result
%210 = OpCompositeExtract %float %209 1
%206 = OpExtInst %float %1 FMin %208 %210
%211 = OpLoad %v3float %_5_result
%212 = OpCompositeExtract %float %211 2
%205 = OpExtInst %float %1 FMin %206 %212
OpStore %_6_minComp %205
%216 = OpLoad %v3float %_5_result
%217 = OpCompositeExtract %float %216 0
%218 = OpLoad %v3float %_5_result
%219 = OpCompositeExtract %float %218 1
%215 = OpExtInst %float %1 FMax %217 %219
%220 = OpLoad %v3float %_5_result
%221 = OpCompositeExtract %float %220 2
%214 = OpExtInst %float %1 FMax %215 %221
OpStore %_7_maxComp %214
%223 = OpLoad %float %_6_minComp
%224 = OpFOrdLessThan %bool %223 %float_0
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
%227 = OpLoad %float %_4_lum
%228 = OpLoad %float %_6_minComp
%229 = OpFUnordNotEqual %bool %227 %228
OpBranch %226
%226 = OpLabel
%230 = OpPhi %bool %false %87 %229 %225
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %float %_4_lum
%234 = OpLoad %v3float %_5_result
%235 = OpLoad %float %_4_lum
%236 = OpCompositeConstruct %v3float %235 %235 %235
%237 = OpFSub %v3float %234 %236
%238 = OpLoad %float %_4_lum
%239 = OpLoad %float %_4_lum
%240 = OpLoad %float %_6_minComp
%241 = OpFSub %float %239 %240
%242 = OpFDiv %float %238 %241
%243 = OpVectorTimesScalar %v3float %237 %242
%244 = OpCompositeConstruct %v3float %233 %233 %233
%245 = OpFAdd %v3float %244 %243
OpStore %_5_result %245
OpBranch %232
%232 = OpLabel
%246 = OpLoad %float %_7_maxComp
%247 = OpLoad %float %alpha
%248 = OpFOrdGreaterThan %bool %246 %247
OpSelectionMerge %250 None
OpBranchConditional %248 %249 %250
%249 = OpLabel
%251 = OpLoad %float %_7_maxComp
%252 = OpLoad %float %_4_lum
%253 = OpFUnordNotEqual %bool %251 %252
OpBranch %250
%250 = OpLabel
%254 = OpPhi %bool %false %232 %253 %249
OpSelectionMerge %256 None
OpBranchConditional %254 %255 %256
%255 = OpLabel
%257 = OpLoad %float %_4_lum
%258 = OpLoad %v3float %_5_result
%259 = OpLoad %float %_4_lum
%260 = OpCompositeConstruct %v3float %259 %259 %259
%261 = OpFSub %v3float %258 %260
%262 = OpLoad %float %alpha
%263 = OpLoad %float %_4_lum
%264 = OpFSub %float %262 %263
%265 = OpVectorTimesScalar %v3float %261 %264
%266 = OpLoad %float %_7_maxComp
%267 = OpLoad %float %_4_lum
%268 = OpFSub %float %266 %267
%270 = OpFDiv %float %float_1 %268
%271 = OpVectorTimesScalar %v3float %265 %270
%272 = OpCompositeConstruct %v3float %257 %257 %257
%273 = OpFAdd %v3float %272 %271
OpStore %_5_result %273
OpBranch %256
%256 = OpLabel
%274 = OpLoad %v3float %_5_result
%275 = OpLoad %v4float %51
%276 = OpVectorShuffle %v3float %275 %275 0 1 2
%277 = OpFAdd %v3float %274 %276
%278 = OpLoad %v3float %dsa
%279 = OpFSub %v3float %277 %278
%280 = OpLoad %v4float %50
%281 = OpVectorShuffle %v3float %280 %280 0 1 2
%282 = OpFAdd %v3float %279 %281
%283 = OpLoad %v3float %sda
%284 = OpFSub %v3float %282 %283
%285 = OpCompositeExtract %float %284 0
%286 = OpCompositeExtract %float %284 1
%287 = OpCompositeExtract %float %284 2
%288 = OpLoad %v4float %50
%289 = OpCompositeExtract %float %288 3
%290 = OpLoad %v4float %51
%291 = OpCompositeExtract %float %290 3
%292 = OpFAdd %float %289 %291
%293 = OpLoad %float %alpha
%294 = OpFSub %float %292 %293
%295 = OpCompositeConstruct %v4float %285 %286 %287 %294
OpReturnValue %295
OpFunctionEnd
%main = OpFunction %void None %297
%298 = OpLabel
%304 = OpVariable %_ptr_Function_v4float Function
%308 = OpVariable %_ptr_Function_v4float Function
%309 = OpVariable %_ptr_Function_float Function
%310 = OpVariable %_ptr_Function_bool Function
%299 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%303 = OpLoad %v4float %299
OpStore %304 %303
%305 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%307 = OpLoad %v4float %305
OpStore %308 %307
OpStore %309 %float_1
OpStore %310 %false
%311 = OpFunctionCall %v4float %blend_hslc_h4h4h4hb %304 %308 %309 %310
OpStore %sk_FragColor %311
OpReturn
OpFunctionEnd

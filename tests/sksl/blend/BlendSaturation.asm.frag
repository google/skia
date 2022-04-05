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
OpName %delta "delta"
OpName %blend_hslc_h4h4h4hb "blend_hslc_h4h4h4hb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_0_blend_set_color_saturation "_0_blend_set_color_saturation"
OpName %_1_sat "_1_sat"
OpName %_2_blend_set_color_luminance "_2_blend_set_color_luminance"
OpName %_3_lum "_3_lum"
OpName %_4_result "_4_result"
OpName %_5_minComp "_5_minComp"
OpName %_6_maxComp "_6_maxComp"
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
OpDecorate %delta RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %_0_blend_set_color_saturation RelaxedPrecision
OpDecorate %_1_sat RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %_2_blend_set_color_luminance RelaxedPrecision
OpDecorate %_3_lum RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %_4_result RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %_5_minComp RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %_6_maxComp RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
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
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
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
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_9_99999975en06 = OpConstant %float 9.99999975e-06
%float_0 = OpConstant %float 0
%48 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%50 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_float %_ptr_Function_bool
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%198 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%303 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%true = OpConstantTrue %bool
%blend_set_color_saturation_helper_Qh3h3h = OpFunction %v3float None %16
%19 = OpFunctionParameter %_ptr_Function_v3float
%20 = OpFunctionParameter %_ptr_Function_float
%21 = OpLabel
%delta = OpVariable %_ptr_Function_v2float Function
%34 = OpVariable %_ptr_Function_v3float Function
%25 = OpLoad %v3float %19
%26 = OpVectorShuffle %v2float %25 %25 1 2
%27 = OpLoad %v3float %19
%28 = OpVectorShuffle %v2float %27 %27 0 0
%29 = OpFSub %v2float %26 %28
OpStore %delta %29
%30 = OpLoad %v2float %delta
%31 = OpCompositeExtract %float %30 1
%33 = OpFOrdGreaterThanEqual %bool %31 %float_9_99999975en06
OpSelectionMerge %37 None
OpBranchConditional %33 %35 %36
%35 = OpLabel
%39 = OpLoad %v2float %delta
%40 = OpCompositeExtract %float %39 0
%41 = OpLoad %v2float %delta
%42 = OpCompositeExtract %float %41 1
%43 = OpFDiv %float %40 %42
%44 = OpLoad %float %20
%45 = OpFMul %float %43 %44
%46 = OpLoad %float %20
%47 = OpCompositeConstruct %v3float %float_0 %45 %46
OpStore %34 %47
OpBranch %37
%36 = OpLabel
OpStore %34 %48
OpBranch %37
%37 = OpLabel
%49 = OpLoad %v3float %34
OpReturnValue %49
OpFunctionEnd
%blend_hslc_h4h4h4hb = OpFunction %v4float None %50
%53 = OpFunctionParameter %_ptr_Function_v4float
%54 = OpFunctionParameter %_ptr_Function_v4float
%55 = OpFunctionParameter %_ptr_Function_float
%56 = OpFunctionParameter %_ptr_Function_bool
%57 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%_0_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_1_sat = OpVariable %_ptr_Function_float Function
%127 = OpVariable %_ptr_Function_v3float Function
%129 = OpVariable %_ptr_Function_float Function
%141 = OpVariable %_ptr_Function_v3float Function
%143 = OpVariable %_ptr_Function_float Function
%148 = OpVariable %_ptr_Function_v3float Function
%150 = OpVariable %_ptr_Function_float Function
%163 = OpVariable %_ptr_Function_v3float Function
%165 = OpVariable %_ptr_Function_float Function
%178 = OpVariable %_ptr_Function_v3float Function
%180 = OpVariable %_ptr_Function_float Function
%185 = OpVariable %_ptr_Function_v3float Function
%187 = OpVariable %_ptr_Function_float Function
%_2_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_3_lum = OpVariable %_ptr_Function_float Function
%_4_result = OpVariable %_ptr_Function_v3float Function
%_5_minComp = OpVariable %_ptr_Function_float Function
%_6_maxComp = OpVariable %_ptr_Function_float Function
%59 = OpLoad %v4float %54
%60 = OpCompositeExtract %float %59 3
%61 = OpLoad %v4float %53
%62 = OpCompositeExtract %float %61 3
%63 = OpFMul %float %60 %62
OpStore %alpha %63
%65 = OpLoad %v4float %53
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%67 = OpLoad %v4float %54
%68 = OpCompositeExtract %float %67 3
%69 = OpVectorTimesScalar %v3float %66 %68
OpStore %sda %69
%71 = OpLoad %v4float %54
%72 = OpVectorShuffle %v3float %71 %71 0 1 2
%73 = OpLoad %v4float %53
%74 = OpCompositeExtract %float %73 3
%75 = OpVectorTimesScalar %v3float %72 %74
OpStore %dsa %75
%78 = OpLoad %v3float %sda
%79 = OpLoad %v3float %dsa
%80 = OpLoad %float %55
%81 = OpCompositeConstruct %v3float %80 %80 %80
%77 = OpExtInst %v3float %1 FMix %78 %79 %81
OpStore %l %77
%84 = OpLoad %v3float %dsa
%85 = OpLoad %v3float %sda
%86 = OpLoad %float %55
%87 = OpCompositeConstruct %v3float %86 %86 %86
%83 = OpExtInst %v3float %1 FMix %84 %85 %87
OpStore %r %83
%88 = OpLoad %bool %56
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%95 = OpLoad %v3float %r
%96 = OpCompositeExtract %float %95 0
%97 = OpLoad %v3float %r
%98 = OpCompositeExtract %float %97 1
%94 = OpExtInst %float %1 FMax %96 %98
%99 = OpLoad %v3float %r
%100 = OpCompositeExtract %float %99 2
%93 = OpExtInst %float %1 FMax %94 %100
%103 = OpLoad %v3float %r
%104 = OpCompositeExtract %float %103 0
%105 = OpLoad %v3float %r
%106 = OpCompositeExtract %float %105 1
%102 = OpExtInst %float %1 FMin %104 %106
%107 = OpLoad %v3float %r
%108 = OpCompositeExtract %float %107 2
%101 = OpExtInst %float %1 FMin %102 %108
%109 = OpFSub %float %93 %101
OpStore %_1_sat %109
%110 = OpLoad %v3float %l
%111 = OpCompositeExtract %float %110 0
%112 = OpLoad %v3float %l
%113 = OpCompositeExtract %float %112 1
%114 = OpFOrdLessThanEqual %bool %111 %113
OpSelectionMerge %117 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpLoad %v3float %l
%119 = OpCompositeExtract %float %118 1
%120 = OpLoad %v3float %l
%121 = OpCompositeExtract %float %120 2
%122 = OpFOrdLessThanEqual %bool %119 %121
OpSelectionMerge %125 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpLoad %v3float %l
OpStore %127 %126
%128 = OpLoad %float %_1_sat
OpStore %129 %128
%130 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %127 %129
OpStore %_0_blend_set_color_saturation %130
OpBranch %125
%124 = OpLabel
%131 = OpLoad %v3float %l
%132 = OpCompositeExtract %float %131 0
%133 = OpLoad %v3float %l
%134 = OpCompositeExtract %float %133 2
%135 = OpFOrdLessThanEqual %bool %132 %134
OpSelectionMerge %138 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%139 = OpLoad %v3float %l
%140 = OpVectorShuffle %v3float %139 %139 0 2 1
OpStore %141 %140
%142 = OpLoad %float %_1_sat
OpStore %143 %142
%144 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %141 %143
%145 = OpVectorShuffle %v3float %144 %144 0 2 1
OpStore %_0_blend_set_color_saturation %145
OpBranch %138
%137 = OpLabel
%146 = OpLoad %v3float %l
%147 = OpVectorShuffle %v3float %146 %146 2 0 1
OpStore %148 %147
%149 = OpLoad %float %_1_sat
OpStore %150 %149
%151 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %148 %150
%152 = OpVectorShuffle %v3float %151 %151 1 2 0
OpStore %_0_blend_set_color_saturation %152
OpBranch %138
%138 = OpLabel
OpBranch %125
%125 = OpLabel
OpBranch %117
%116 = OpLabel
%153 = OpLoad %v3float %l
%154 = OpCompositeExtract %float %153 0
%155 = OpLoad %v3float %l
%156 = OpCompositeExtract %float %155 2
%157 = OpFOrdLessThanEqual %bool %154 %156
OpSelectionMerge %160 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%161 = OpLoad %v3float %l
%162 = OpVectorShuffle %v3float %161 %161 1 0 2
OpStore %163 %162
%164 = OpLoad %float %_1_sat
OpStore %165 %164
%166 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %163 %165
%167 = OpVectorShuffle %v3float %166 %166 1 0 2
OpStore %_0_blend_set_color_saturation %167
OpBranch %160
%159 = OpLabel
%168 = OpLoad %v3float %l
%169 = OpCompositeExtract %float %168 1
%170 = OpLoad %v3float %l
%171 = OpCompositeExtract %float %170 2
%172 = OpFOrdLessThanEqual %bool %169 %171
OpSelectionMerge %175 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%176 = OpLoad %v3float %l
%177 = OpVectorShuffle %v3float %176 %176 1 2 0
OpStore %178 %177
%179 = OpLoad %float %_1_sat
OpStore %180 %179
%181 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %178 %180
%182 = OpVectorShuffle %v3float %181 %181 2 0 1
OpStore %_0_blend_set_color_saturation %182
OpBranch %175
%174 = OpLabel
%183 = OpLoad %v3float %l
%184 = OpVectorShuffle %v3float %183 %183 2 1 0
OpStore %185 %184
%186 = OpLoad %float %_1_sat
OpStore %187 %186
%188 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %185 %187
%189 = OpVectorShuffle %v3float %188 %188 2 1 0
OpStore %_0_blend_set_color_saturation %189
OpBranch %175
%175 = OpLabel
OpBranch %160
%160 = OpLabel
OpBranch %117
%117 = OpLabel
%190 = OpLoad %v3float %_0_blend_set_color_saturation
OpStore %l %190
%191 = OpLoad %v3float %dsa
OpStore %r %191
OpBranch %90
%90 = OpLabel
%199 = OpLoad %v3float %r
%194 = OpDot %float %198 %199
OpStore %_3_lum %194
%201 = OpLoad %float %_3_lum
%203 = OpLoad %v3float %l
%202 = OpDot %float %198 %203
%204 = OpFSub %float %201 %202
%205 = OpLoad %v3float %l
%206 = OpCompositeConstruct %v3float %204 %204 %204
%207 = OpFAdd %v3float %206 %205
OpStore %_4_result %207
%211 = OpLoad %v3float %_4_result
%212 = OpCompositeExtract %float %211 0
%213 = OpLoad %v3float %_4_result
%214 = OpCompositeExtract %float %213 1
%210 = OpExtInst %float %1 FMin %212 %214
%215 = OpLoad %v3float %_4_result
%216 = OpCompositeExtract %float %215 2
%209 = OpExtInst %float %1 FMin %210 %216
OpStore %_5_minComp %209
%220 = OpLoad %v3float %_4_result
%221 = OpCompositeExtract %float %220 0
%222 = OpLoad %v3float %_4_result
%223 = OpCompositeExtract %float %222 1
%219 = OpExtInst %float %1 FMax %221 %223
%224 = OpLoad %v3float %_4_result
%225 = OpCompositeExtract %float %224 2
%218 = OpExtInst %float %1 FMax %219 %225
OpStore %_6_maxComp %218
%227 = OpLoad %float %_5_minComp
%228 = OpFOrdLessThan %bool %227 %float_0
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%231 = OpLoad %float %_3_lum
%232 = OpLoad %float %_5_minComp
%233 = OpFUnordNotEqual %bool %231 %232
OpBranch %230
%230 = OpLabel
%234 = OpPhi %bool %false %90 %233 %229
OpSelectionMerge %236 None
OpBranchConditional %234 %235 %236
%235 = OpLabel
%237 = OpLoad %float %_3_lum
%238 = OpLoad %v3float %_4_result
%239 = OpLoad %float %_3_lum
%240 = OpCompositeConstruct %v3float %239 %239 %239
%241 = OpFSub %v3float %238 %240
%242 = OpLoad %float %_3_lum
%243 = OpLoad %float %_3_lum
%244 = OpLoad %float %_5_minComp
%245 = OpFSub %float %243 %244
%246 = OpFDiv %float %242 %245
%247 = OpVectorTimesScalar %v3float %241 %246
%248 = OpCompositeConstruct %v3float %237 %237 %237
%249 = OpFAdd %v3float %248 %247
OpStore %_4_result %249
OpBranch %236
%236 = OpLabel
%250 = OpLoad %float %_6_maxComp
%251 = OpLoad %float %alpha
%252 = OpFOrdGreaterThan %bool %250 %251
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
%255 = OpLoad %float %_6_maxComp
%256 = OpLoad %float %_3_lum
%257 = OpFUnordNotEqual %bool %255 %256
OpBranch %254
%254 = OpLabel
%258 = OpPhi %bool %false %236 %257 %253
OpSelectionMerge %261 None
OpBranchConditional %258 %259 %260
%259 = OpLabel
%262 = OpLoad %float %_3_lum
%263 = OpLoad %v3float %_4_result
%264 = OpLoad %float %_3_lum
%265 = OpCompositeConstruct %v3float %264 %264 %264
%266 = OpFSub %v3float %263 %265
%267 = OpLoad %float %alpha
%268 = OpLoad %float %_3_lum
%269 = OpFSub %float %267 %268
%270 = OpVectorTimesScalar %v3float %266 %269
%271 = OpLoad %float %_6_maxComp
%272 = OpLoad %float %_3_lum
%273 = OpFSub %float %271 %272
%275 = OpFDiv %float %float_1 %273
%276 = OpVectorTimesScalar %v3float %270 %275
%277 = OpCompositeConstruct %v3float %262 %262 %262
%278 = OpFAdd %v3float %277 %276
OpStore %_2_blend_set_color_luminance %278
OpBranch %261
%260 = OpLabel
%279 = OpLoad %v3float %_4_result
OpStore %_2_blend_set_color_luminance %279
OpBranch %261
%261 = OpLabel
%280 = OpLoad %v3float %_2_blend_set_color_luminance
%281 = OpLoad %v4float %54
%282 = OpVectorShuffle %v3float %281 %281 0 1 2
%283 = OpFAdd %v3float %280 %282
%284 = OpLoad %v3float %dsa
%285 = OpFSub %v3float %283 %284
%286 = OpLoad %v4float %53
%287 = OpVectorShuffle %v3float %286 %286 0 1 2
%288 = OpFAdd %v3float %285 %287
%289 = OpLoad %v3float %sda
%290 = OpFSub %v3float %288 %289
%291 = OpCompositeExtract %float %290 0
%292 = OpCompositeExtract %float %290 1
%293 = OpCompositeExtract %float %290 2
%294 = OpLoad %v4float %53
%295 = OpCompositeExtract %float %294 3
%296 = OpLoad %v4float %54
%297 = OpCompositeExtract %float %296 3
%298 = OpFAdd %float %295 %297
%299 = OpLoad %float %alpha
%300 = OpFSub %float %298 %299
%301 = OpCompositeConstruct %v4float %291 %292 %293 %300
OpReturnValue %301
OpFunctionEnd
%main = OpFunction %void None %303
%304 = OpLabel
%310 = OpVariable %_ptr_Function_v4float Function
%314 = OpVariable %_ptr_Function_v4float Function
%315 = OpVariable %_ptr_Function_float Function
%317 = OpVariable %_ptr_Function_bool Function
%305 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%309 = OpLoad %v4float %305
OpStore %310 %309
%311 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%313 = OpLoad %v4float %311
OpStore %314 %313
OpStore %315 %float_1
OpStore %317 %true
%318 = OpFunctionCall %v4float %blend_hslc_h4h4h4hb %310 %314 %315 %317
OpStore %sk_FragColor %318
OpReturn
OpFunctionEnd

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
OpName %blend_set_color_saturation_helper_Qh3h3h3 "blend_set_color_saturation_helper_Qh3h3h3"
OpName %sat "sat"
OpName %blend_hslc_h4h4h4bb "blend_hslc_h4h4h4bb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_2_blend_set_color_saturation "_2_blend_set_color_saturation"
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
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %sat RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %_2_blend_set_color_saturation RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %_3_lum RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %_4_result RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %_5_minComp RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %_6_maxComp RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
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
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
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
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
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
%16 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%v2float = OpTypeVector %float 2
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%66 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%67 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_bool %_ptr_Function_bool
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%202 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%305 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%true = OpConstantTrue %bool
%blend_set_color_saturation_helper_Qh3h3h3 = OpFunction %v3float None %16
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_v3float
%20 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%21 = OpLoad %v3float %18
%22 = OpCompositeExtract %float %21 2
%23 = OpLoad %v3float %18
%24 = OpCompositeExtract %float %23 0
%25 = OpFOrdGreaterThan %bool %22 %24
OpSelectionMerge %28 None
OpBranchConditional %25 %26 %27
%26 = OpLabel
%29 = OpLoad %v3float %18
%30 = OpVectorShuffle %v2float %29 %29 1 2
%32 = OpLoad %v3float %18
%33 = OpVectorShuffle %v2float %32 %32 0 0
%34 = OpFSub %v2float %30 %33
%35 = OpLoad %v3float %18
%36 = OpVectorShuffle %v3float %35 %34 0 3 4
OpStore %18 %36
%41 = OpLoad %v3float %19
%42 = OpCompositeExtract %float %41 0
%43 = OpLoad %v3float %19
%44 = OpCompositeExtract %float %43 1
%40 = OpExtInst %float %1 FMax %42 %44
%45 = OpLoad %v3float %19
%46 = OpCompositeExtract %float %45 2
%39 = OpExtInst %float %1 FMax %40 %46
%49 = OpLoad %v3float %19
%50 = OpCompositeExtract %float %49 0
%51 = OpLoad %v3float %19
%52 = OpCompositeExtract %float %51 1
%48 = OpExtInst %float %1 FMin %50 %52
%53 = OpLoad %v3float %19
%54 = OpCompositeExtract %float %53 2
%47 = OpExtInst %float %1 FMin %48 %54
%55 = OpFSub %float %39 %47
OpStore %sat %55
%57 = OpLoad %float %sat
%58 = OpLoad %v3float %18
%59 = OpCompositeExtract %float %58 1
%60 = OpLoad %v3float %18
%61 = OpCompositeExtract %float %60 2
%62 = OpFDiv %float %59 %61
%63 = OpFMul %float %57 %62
%64 = OpLoad %float %sat
%65 = OpCompositeConstruct %v3float %float_0 %63 %64
OpReturnValue %65
%27 = OpLabel
OpReturnValue %66
%28 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_hslc_h4h4h4bb = OpFunction %v4float None %67
%70 = OpFunctionParameter %_ptr_Function_v4float
%71 = OpFunctionParameter %_ptr_Function_v4float
%72 = OpFunctionParameter %_ptr_Function_bool
%73 = OpFunctionParameter %_ptr_Function_bool
%74 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%95 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%104 = OpVariable %_ptr_Function_v3float Function
%_2_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%132 = OpVariable %_ptr_Function_v3float Function
%134 = OpVariable %_ptr_Function_v3float Function
%146 = OpVariable %_ptr_Function_v3float Function
%148 = OpVariable %_ptr_Function_v3float Function
%153 = OpVariable %_ptr_Function_v3float Function
%155 = OpVariable %_ptr_Function_v3float Function
%168 = OpVariable %_ptr_Function_v3float Function
%170 = OpVariable %_ptr_Function_v3float Function
%183 = OpVariable %_ptr_Function_v3float Function
%185 = OpVariable %_ptr_Function_v3float Function
%190 = OpVariable %_ptr_Function_v3float Function
%192 = OpVariable %_ptr_Function_v3float Function
%_3_lum = OpVariable %_ptr_Function_float Function
%_4_result = OpVariable %_ptr_Function_v3float Function
%_5_minComp = OpVariable %_ptr_Function_float Function
%_6_maxComp = OpVariable %_ptr_Function_float Function
%76 = OpLoad %v4float %71
%77 = OpCompositeExtract %float %76 3
%78 = OpLoad %v4float %70
%79 = OpCompositeExtract %float %78 3
%80 = OpFMul %float %77 %79
OpStore %alpha %80
%82 = OpLoad %v4float %70
%83 = OpVectorShuffle %v3float %82 %82 0 1 2
%84 = OpLoad %v4float %71
%85 = OpCompositeExtract %float %84 3
%86 = OpVectorTimesScalar %v3float %83 %85
OpStore %sda %86
%88 = OpLoad %v4float %71
%89 = OpVectorShuffle %v3float %88 %88 0 1 2
%90 = OpLoad %v4float %70
%91 = OpCompositeExtract %float %90 3
%92 = OpVectorTimesScalar %v3float %89 %91
OpStore %dsa %92
%94 = OpLoad %bool %72
OpSelectionMerge %98 None
OpBranchConditional %94 %96 %97
%96 = OpLabel
%99 = OpLoad %v3float %dsa
OpStore %95 %99
OpBranch %98
%97 = OpLabel
%100 = OpLoad %v3float %sda
OpStore %95 %100
OpBranch %98
%98 = OpLabel
%101 = OpLoad %v3float %95
OpStore %l %101
%103 = OpLoad %bool %72
OpSelectionMerge %107 None
OpBranchConditional %103 %105 %106
%105 = OpLabel
%108 = OpLoad %v3float %sda
OpStore %104 %108
OpBranch %107
%106 = OpLabel
%109 = OpLoad %v3float %dsa
OpStore %104 %109
OpBranch %107
%107 = OpLabel
%110 = OpLoad %v3float %104
OpStore %r %110
%111 = OpLoad %bool %73
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpLoad %v3float %l
%116 = OpCompositeExtract %float %115 0
%117 = OpLoad %v3float %l
%118 = OpCompositeExtract %float %117 1
%119 = OpFOrdLessThanEqual %bool %116 %118
OpSelectionMerge %122 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpLoad %v3float %l
%124 = OpCompositeExtract %float %123 1
%125 = OpLoad %v3float %l
%126 = OpCompositeExtract %float %125 2
%127 = OpFOrdLessThanEqual %bool %124 %126
OpSelectionMerge %130 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%131 = OpLoad %v3float %l
OpStore %132 %131
%133 = OpLoad %v3float %r
OpStore %134 %133
%135 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %132 %134
OpStore %_2_blend_set_color_saturation %135
OpBranch %130
%129 = OpLabel
%136 = OpLoad %v3float %l
%137 = OpCompositeExtract %float %136 0
%138 = OpLoad %v3float %l
%139 = OpCompositeExtract %float %138 2
%140 = OpFOrdLessThanEqual %bool %137 %139
OpSelectionMerge %143 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%144 = OpLoad %v3float %l
%145 = OpVectorShuffle %v3float %144 %144 0 2 1
OpStore %146 %145
%147 = OpLoad %v3float %r
OpStore %148 %147
%149 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %146 %148
%150 = OpVectorShuffle %v3float %149 %149 0 2 1
OpStore %_2_blend_set_color_saturation %150
OpBranch %143
%142 = OpLabel
%151 = OpLoad %v3float %l
%152 = OpVectorShuffle %v3float %151 %151 2 0 1
OpStore %153 %152
%154 = OpLoad %v3float %r
OpStore %155 %154
%156 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %153 %155
%157 = OpVectorShuffle %v3float %156 %156 1 2 0
OpStore %_2_blend_set_color_saturation %157
OpBranch %143
%143 = OpLabel
OpBranch %130
%130 = OpLabel
OpBranch %122
%121 = OpLabel
%158 = OpLoad %v3float %l
%159 = OpCompositeExtract %float %158 0
%160 = OpLoad %v3float %l
%161 = OpCompositeExtract %float %160 2
%162 = OpFOrdLessThanEqual %bool %159 %161
OpSelectionMerge %165 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpLoad %v3float %l
%167 = OpVectorShuffle %v3float %166 %166 1 0 2
OpStore %168 %167
%169 = OpLoad %v3float %r
OpStore %170 %169
%171 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %168 %170
%172 = OpVectorShuffle %v3float %171 %171 1 0 2
OpStore %_2_blend_set_color_saturation %172
OpBranch %165
%164 = OpLabel
%173 = OpLoad %v3float %l
%174 = OpCompositeExtract %float %173 1
%175 = OpLoad %v3float %l
%176 = OpCompositeExtract %float %175 2
%177 = OpFOrdLessThanEqual %bool %174 %176
OpSelectionMerge %180 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%181 = OpLoad %v3float %l
%182 = OpVectorShuffle %v3float %181 %181 1 2 0
OpStore %183 %182
%184 = OpLoad %v3float %r
OpStore %185 %184
%186 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %183 %185
%187 = OpVectorShuffle %v3float %186 %186 2 0 1
OpStore %_2_blend_set_color_saturation %187
OpBranch %180
%179 = OpLabel
%188 = OpLoad %v3float %l
%189 = OpVectorShuffle %v3float %188 %188 2 1 0
OpStore %190 %189
%191 = OpLoad %v3float %r
OpStore %192 %191
%193 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %190 %192
%194 = OpVectorShuffle %v3float %193 %193 2 1 0
OpStore %_2_blend_set_color_saturation %194
OpBranch %180
%180 = OpLabel
OpBranch %165
%165 = OpLabel
OpBranch %122
%122 = OpLabel
%195 = OpLoad %v3float %_2_blend_set_color_saturation
OpStore %l %195
%196 = OpLoad %v3float %dsa
OpStore %r %196
OpBranch %113
%113 = OpLabel
%203 = OpLoad %v3float %r
%198 = OpDot %float %202 %203
OpStore %_3_lum %198
%205 = OpLoad %float %_3_lum
%207 = OpLoad %v3float %l
%206 = OpDot %float %202 %207
%208 = OpFSub %float %205 %206
%209 = OpLoad %v3float %l
%210 = OpCompositeConstruct %v3float %208 %208 %208
%211 = OpFAdd %v3float %210 %209
OpStore %_4_result %211
%215 = OpLoad %v3float %_4_result
%216 = OpCompositeExtract %float %215 0
%217 = OpLoad %v3float %_4_result
%218 = OpCompositeExtract %float %217 1
%214 = OpExtInst %float %1 FMin %216 %218
%219 = OpLoad %v3float %_4_result
%220 = OpCompositeExtract %float %219 2
%213 = OpExtInst %float %1 FMin %214 %220
OpStore %_5_minComp %213
%224 = OpLoad %v3float %_4_result
%225 = OpCompositeExtract %float %224 0
%226 = OpLoad %v3float %_4_result
%227 = OpCompositeExtract %float %226 1
%223 = OpExtInst %float %1 FMax %225 %227
%228 = OpLoad %v3float %_4_result
%229 = OpCompositeExtract %float %228 2
%222 = OpExtInst %float %1 FMax %223 %229
OpStore %_6_maxComp %222
%231 = OpLoad %float %_5_minComp
%232 = OpFOrdLessThan %bool %231 %float_0
OpSelectionMerge %234 None
OpBranchConditional %232 %233 %234
%233 = OpLabel
%235 = OpLoad %float %_3_lum
%236 = OpLoad %float %_5_minComp
%237 = OpFUnordNotEqual %bool %235 %236
OpBranch %234
%234 = OpLabel
%238 = OpPhi %bool %false %113 %237 %233
OpSelectionMerge %240 None
OpBranchConditional %238 %239 %240
%239 = OpLabel
%241 = OpLoad %float %_3_lum
%242 = OpLoad %v3float %_4_result
%243 = OpLoad %float %_3_lum
%244 = OpCompositeConstruct %v3float %243 %243 %243
%245 = OpFSub %v3float %242 %244
%246 = OpLoad %float %_3_lum
%247 = OpLoad %float %_3_lum
%248 = OpLoad %float %_5_minComp
%249 = OpFSub %float %247 %248
%250 = OpFDiv %float %246 %249
%251 = OpVectorTimesScalar %v3float %245 %250
%252 = OpCompositeConstruct %v3float %241 %241 %241
%253 = OpFAdd %v3float %252 %251
OpStore %_4_result %253
OpBranch %240
%240 = OpLabel
%254 = OpLoad %float %_6_maxComp
%255 = OpLoad %float %alpha
%256 = OpFOrdGreaterThan %bool %254 %255
OpSelectionMerge %258 None
OpBranchConditional %256 %257 %258
%257 = OpLabel
%259 = OpLoad %float %_6_maxComp
%260 = OpLoad %float %_3_lum
%261 = OpFUnordNotEqual %bool %259 %260
OpBranch %258
%258 = OpLabel
%262 = OpPhi %bool %false %240 %261 %257
OpSelectionMerge %264 None
OpBranchConditional %262 %263 %264
%263 = OpLabel
%265 = OpLoad %float %_3_lum
%266 = OpLoad %v3float %_4_result
%267 = OpLoad %float %_3_lum
%268 = OpCompositeConstruct %v3float %267 %267 %267
%269 = OpFSub %v3float %266 %268
%270 = OpLoad %float %alpha
%271 = OpLoad %float %_3_lum
%272 = OpFSub %float %270 %271
%273 = OpVectorTimesScalar %v3float %269 %272
%274 = OpLoad %float %_6_maxComp
%275 = OpLoad %float %_3_lum
%276 = OpFSub %float %274 %275
%278 = OpFDiv %float %float_1 %276
%279 = OpVectorTimesScalar %v3float %273 %278
%280 = OpCompositeConstruct %v3float %265 %265 %265
%281 = OpFAdd %v3float %280 %279
OpStore %_4_result %281
OpBranch %264
%264 = OpLabel
%282 = OpLoad %v3float %_4_result
%283 = OpLoad %v4float %71
%284 = OpVectorShuffle %v3float %283 %283 0 1 2
%285 = OpFAdd %v3float %282 %284
%286 = OpLoad %v3float %dsa
%287 = OpFSub %v3float %285 %286
%288 = OpLoad %v4float %70
%289 = OpVectorShuffle %v3float %288 %288 0 1 2
%290 = OpFAdd %v3float %287 %289
%291 = OpLoad %v3float %sda
%292 = OpFSub %v3float %290 %291
%293 = OpCompositeExtract %float %292 0
%294 = OpCompositeExtract %float %292 1
%295 = OpCompositeExtract %float %292 2
%296 = OpLoad %v4float %70
%297 = OpCompositeExtract %float %296 3
%298 = OpLoad %v4float %71
%299 = OpCompositeExtract %float %298 3
%300 = OpFAdd %float %297 %299
%301 = OpLoad %float %alpha
%302 = OpFSub %float %300 %301
%303 = OpCompositeConstruct %v4float %293 %294 %295 %302
OpReturnValue %303
OpFunctionEnd
%main = OpFunction %void None %305
%306 = OpLabel
%312 = OpVariable %_ptr_Function_v4float Function
%316 = OpVariable %_ptr_Function_v4float Function
%318 = OpVariable %_ptr_Function_bool Function
%319 = OpVariable %_ptr_Function_bool Function
%307 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%311 = OpLoad %v4float %307
OpStore %312 %311
%313 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%315 = OpLoad %v4float %313
OpStore %316 %315
OpStore %318 %true
OpStore %319 %false
%320 = OpFunctionCall %v4float %blend_hslc_h4h4h4bb %312 %316 %318 %319
OpStore %sk_FragColor %320
OpReturn
OpFunctionEnd

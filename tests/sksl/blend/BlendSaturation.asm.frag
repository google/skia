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
OpName %blend_set_color_luminance_Qh3h3hh3 "blend_set_color_luminance_Qh3h3hh3"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %blend_set_color_saturation_helper_Qh3h3h "blend_set_color_saturation_helper_Qh3h3h"
OpName %delta "delta"
OpName %blend_set_color_saturation_Qh3h3h3 "blend_set_color_saturation_Qh3h3h3"
OpName %sat "sat"
OpName %main "main"
OpName %_0_alpha "_0_alpha"
OpName %_1_sda "_1_sda"
OpName %_2_dsa "_2_dsa"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %lum RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
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
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %sat RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %_0_alpha RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %_1_sda RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %_2_dsa RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_float = OpTypePointer Function %float
%17 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%29 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%112 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_9_99999975en06 = OpConstant %float 9.99999975e-06
%141 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%143 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%void = OpTypeVoid
%246 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%blend_set_color_luminance_Qh3h3hh3 = OpFunction %v3float None %17
%20 = OpFunctionParameter %_ptr_Function_v3float
%21 = OpFunctionParameter %_ptr_Function_float
%22 = OpFunctionParameter %_ptr_Function_v3float
%23 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%30 = OpLoad %v3float %22
%25 = OpDot %float %29 %30
OpStore %lum %25
%32 = OpLoad %float %lum
%34 = OpLoad %v3float %20
%33 = OpDot %float %29 %34
%35 = OpFSub %float %32 %33
%36 = OpLoad %v3float %20
%37 = OpCompositeConstruct %v3float %35 %35 %35
%38 = OpFAdd %v3float %37 %36
OpStore %result %38
%42 = OpLoad %v3float %result
%43 = OpCompositeExtract %float %42 0
%44 = OpLoad %v3float %result
%45 = OpCompositeExtract %float %44 1
%41 = OpExtInst %float %1 FMin %43 %45
%46 = OpLoad %v3float %result
%47 = OpCompositeExtract %float %46 2
%40 = OpExtInst %float %1 FMin %41 %47
OpStore %minComp %40
%51 = OpLoad %v3float %result
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v3float %result
%54 = OpCompositeExtract %float %53 1
%50 = OpExtInst %float %1 FMax %52 %54
%55 = OpLoad %v3float %result
%56 = OpCompositeExtract %float %55 2
%49 = OpExtInst %float %1 FMax %50 %56
OpStore %maxComp %49
%58 = OpLoad %float %minComp
%60 = OpFOrdLessThan %bool %58 %float_0
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpLoad %float %lum
%64 = OpLoad %float %minComp
%65 = OpFUnordNotEqual %bool %63 %64
OpBranch %62
%62 = OpLabel
%66 = OpPhi %bool %false %23 %65 %61
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%69 = OpLoad %float %lum
%70 = OpLoad %v3float %result
%71 = OpLoad %float %lum
%72 = OpCompositeConstruct %v3float %71 %71 %71
%73 = OpFSub %v3float %70 %72
%74 = OpLoad %float %lum
%75 = OpLoad %float %lum
%76 = OpLoad %float %minComp
%77 = OpFSub %float %75 %76
%78 = OpFDiv %float %74 %77
%79 = OpVectorTimesScalar %v3float %73 %78
%80 = OpCompositeConstruct %v3float %69 %69 %69
%81 = OpFAdd %v3float %80 %79
OpStore %result %81
OpBranch %68
%68 = OpLabel
%82 = OpLoad %float %maxComp
%83 = OpLoad %float %21
%84 = OpFOrdGreaterThan %bool %82 %83
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %float %maxComp
%88 = OpLoad %float %lum
%89 = OpFUnordNotEqual %bool %87 %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %68 %89 %85
OpSelectionMerge %93 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpLoad %float %lum
%95 = OpLoad %v3float %result
%96 = OpLoad %float %lum
%97 = OpCompositeConstruct %v3float %96 %96 %96
%98 = OpFSub %v3float %95 %97
%99 = OpLoad %float %21
%100 = OpLoad %float %lum
%101 = OpFSub %float %99 %100
%102 = OpVectorTimesScalar %v3float %98 %101
%103 = OpLoad %float %maxComp
%104 = OpLoad %float %lum
%105 = OpFSub %float %103 %104
%107 = OpFDiv %float %float_1 %105
%108 = OpVectorTimesScalar %v3float %102 %107
%109 = OpCompositeConstruct %v3float %94 %94 %94
%110 = OpFAdd %v3float %109 %108
OpReturnValue %110
%92 = OpLabel
%111 = OpLoad %v3float %result
OpReturnValue %111
%93 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_set_color_saturation_helper_Qh3h3h = OpFunction %v3float None %112
%113 = OpFunctionParameter %_ptr_Function_v3float
%114 = OpFunctionParameter %_ptr_Function_float
%115 = OpLabel
%delta = OpVariable %_ptr_Function_v2float Function
%128 = OpVariable %_ptr_Function_v3float Function
%119 = OpLoad %v3float %113
%120 = OpVectorShuffle %v2float %119 %119 1 2
%121 = OpLoad %v3float %113
%122 = OpVectorShuffle %v2float %121 %121 0 0
%123 = OpFSub %v2float %120 %122
OpStore %delta %123
%124 = OpLoad %v2float %delta
%125 = OpCompositeExtract %float %124 1
%127 = OpFOrdGreaterThanEqual %bool %125 %float_9_99999975en06
OpSelectionMerge %131 None
OpBranchConditional %127 %129 %130
%129 = OpLabel
%132 = OpLoad %v2float %delta
%133 = OpCompositeExtract %float %132 0
%134 = OpLoad %v2float %delta
%135 = OpCompositeExtract %float %134 1
%136 = OpFDiv %float %133 %135
%137 = OpLoad %float %114
%138 = OpFMul %float %136 %137
%139 = OpLoad %float %114
%140 = OpCompositeConstruct %v3float %float_0 %138 %139
OpStore %128 %140
OpBranch %131
%130 = OpLabel
OpStore %128 %141
OpBranch %131
%131 = OpLabel
%142 = OpLoad %v3float %128
OpReturnValue %142
OpFunctionEnd
%blend_set_color_saturation_Qh3h3h3 = OpFunction %v3float None %143
%144 = OpFunctionParameter %_ptr_Function_v3float
%145 = OpFunctionParameter %_ptr_Function_v3float
%146 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%182 = OpVariable %_ptr_Function_v3float Function
%184 = OpVariable %_ptr_Function_float Function
%196 = OpVariable %_ptr_Function_v3float Function
%198 = OpVariable %_ptr_Function_float Function
%203 = OpVariable %_ptr_Function_v3float Function
%205 = OpVariable %_ptr_Function_float Function
%218 = OpVariable %_ptr_Function_v3float Function
%220 = OpVariable %_ptr_Function_float Function
%233 = OpVariable %_ptr_Function_v3float Function
%235 = OpVariable %_ptr_Function_float Function
%240 = OpVariable %_ptr_Function_v3float Function
%242 = OpVariable %_ptr_Function_float Function
%150 = OpLoad %v3float %145
%151 = OpCompositeExtract %float %150 0
%152 = OpLoad %v3float %145
%153 = OpCompositeExtract %float %152 1
%149 = OpExtInst %float %1 FMax %151 %153
%154 = OpLoad %v3float %145
%155 = OpCompositeExtract %float %154 2
%148 = OpExtInst %float %1 FMax %149 %155
%158 = OpLoad %v3float %145
%159 = OpCompositeExtract %float %158 0
%160 = OpLoad %v3float %145
%161 = OpCompositeExtract %float %160 1
%157 = OpExtInst %float %1 FMin %159 %161
%162 = OpLoad %v3float %145
%163 = OpCompositeExtract %float %162 2
%156 = OpExtInst %float %1 FMin %157 %163
%164 = OpFSub %float %148 %156
OpStore %sat %164
%165 = OpLoad %v3float %144
%166 = OpCompositeExtract %float %165 0
%167 = OpLoad %v3float %144
%168 = OpCompositeExtract %float %167 1
%169 = OpFOrdLessThanEqual %bool %166 %168
OpSelectionMerge %172 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%173 = OpLoad %v3float %144
%174 = OpCompositeExtract %float %173 1
%175 = OpLoad %v3float %144
%176 = OpCompositeExtract %float %175 2
%177 = OpFOrdLessThanEqual %bool %174 %176
OpSelectionMerge %180 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%181 = OpLoad %v3float %144
OpStore %182 %181
%183 = OpLoad %float %sat
OpStore %184 %183
%185 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %182 %184
OpReturnValue %185
%179 = OpLabel
%186 = OpLoad %v3float %144
%187 = OpCompositeExtract %float %186 0
%188 = OpLoad %v3float %144
%189 = OpCompositeExtract %float %188 2
%190 = OpFOrdLessThanEqual %bool %187 %189
OpSelectionMerge %193 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%194 = OpLoad %v3float %144
%195 = OpVectorShuffle %v3float %194 %194 0 2 1
OpStore %196 %195
%197 = OpLoad %float %sat
OpStore %198 %197
%199 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %196 %198
%200 = OpVectorShuffle %v3float %199 %199 0 2 1
OpReturnValue %200
%192 = OpLabel
%201 = OpLoad %v3float %144
%202 = OpVectorShuffle %v3float %201 %201 2 0 1
OpStore %203 %202
%204 = OpLoad %float %sat
OpStore %205 %204
%206 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %203 %205
%207 = OpVectorShuffle %v3float %206 %206 1 2 0
OpReturnValue %207
%193 = OpLabel
OpBranch %180
%180 = OpLabel
OpBranch %172
%171 = OpLabel
%208 = OpLoad %v3float %144
%209 = OpCompositeExtract %float %208 0
%210 = OpLoad %v3float %144
%211 = OpCompositeExtract %float %210 2
%212 = OpFOrdLessThanEqual %bool %209 %211
OpSelectionMerge %215 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%216 = OpLoad %v3float %144
%217 = OpVectorShuffle %v3float %216 %216 1 0 2
OpStore %218 %217
%219 = OpLoad %float %sat
OpStore %220 %219
%221 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %218 %220
%222 = OpVectorShuffle %v3float %221 %221 1 0 2
OpReturnValue %222
%214 = OpLabel
%223 = OpLoad %v3float %144
%224 = OpCompositeExtract %float %223 1
%225 = OpLoad %v3float %144
%226 = OpCompositeExtract %float %225 2
%227 = OpFOrdLessThanEqual %bool %224 %226
OpSelectionMerge %230 None
OpBranchConditional %227 %228 %229
%228 = OpLabel
%231 = OpLoad %v3float %144
%232 = OpVectorShuffle %v3float %231 %231 1 2 0
OpStore %233 %232
%234 = OpLoad %float %sat
OpStore %235 %234
%236 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %233 %235
%237 = OpVectorShuffle %v3float %236 %236 2 0 1
OpReturnValue %237
%229 = OpLabel
%238 = OpLoad %v3float %144
%239 = OpVectorShuffle %v3float %238 %238 2 1 0
OpStore %240 %239
%241 = OpLoad %float %sat
OpStore %242 %241
%243 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %240 %242
%244 = OpVectorShuffle %v3float %243 %243 2 1 0
OpReturnValue %244
%230 = OpLabel
OpBranch %215
%215 = OpLabel
OpBranch %172
%172 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %246
%247 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%277 = OpVariable %_ptr_Function_v3float Function
%279 = OpVariable %_ptr_Function_v3float Function
%281 = OpVariable %_ptr_Function_v3float Function
%283 = OpVariable %_ptr_Function_float Function
%285 = OpVariable %_ptr_Function_v3float Function
%249 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%253 = OpLoad %v4float %249
%254 = OpCompositeExtract %float %253 3
%255 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%257 = OpLoad %v4float %255
%258 = OpCompositeExtract %float %257 3
%259 = OpFMul %float %254 %258
OpStore %_0_alpha %259
%261 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%262 = OpLoad %v4float %261
%263 = OpVectorShuffle %v3float %262 %262 0 1 2
%264 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%265 = OpLoad %v4float %264
%266 = OpCompositeExtract %float %265 3
%267 = OpVectorTimesScalar %v3float %263 %266
OpStore %_1_sda %267
%269 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%270 = OpLoad %v4float %269
%271 = OpVectorShuffle %v3float %270 %270 0 1 2
%272 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%273 = OpLoad %v4float %272
%274 = OpCompositeExtract %float %273 3
%275 = OpVectorTimesScalar %v3float %271 %274
OpStore %_2_dsa %275
%276 = OpLoad %v3float %_2_dsa
OpStore %277 %276
%278 = OpLoad %v3float %_1_sda
OpStore %279 %278
%280 = OpFunctionCall %v3float %blend_set_color_saturation_Qh3h3h3 %277 %279
OpStore %281 %280
%282 = OpLoad %float %_0_alpha
OpStore %283 %282
%284 = OpLoad %v3float %_2_dsa
OpStore %285 %284
%286 = OpFunctionCall %v3float %blend_set_color_luminance_Qh3h3hh3 %281 %283 %285
%287 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%288 = OpLoad %v4float %287
%289 = OpVectorShuffle %v3float %288 %288 0 1 2
%290 = OpFAdd %v3float %286 %289
%291 = OpLoad %v3float %_2_dsa
%292 = OpFSub %v3float %290 %291
%293 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%294 = OpLoad %v4float %293
%295 = OpVectorShuffle %v3float %294 %294 0 1 2
%296 = OpFAdd %v3float %292 %295
%297 = OpLoad %v3float %_1_sda
%298 = OpFSub %v3float %296 %297
%299 = OpCompositeExtract %float %298 0
%300 = OpCompositeExtract %float %298 1
%301 = OpCompositeExtract %float %298 2
%302 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%303 = OpLoad %v4float %302
%304 = OpCompositeExtract %float %303 3
%305 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%306 = OpLoad %v4float %305
%307 = OpCompositeExtract %float %306 3
%308 = OpFAdd %float %304 %307
%309 = OpLoad %float %_0_alpha
%310 = OpFSub %float %308 %309
%311 = OpCompositeConstruct %v4float %299 %300 %301 %310
OpStore %sk_FragColor %311
OpReturn
OpFunctionEnd

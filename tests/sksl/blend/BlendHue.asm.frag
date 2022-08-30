OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %_GuardedDivideEpsilon "$GuardedDivideEpsilon"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_color_luminance_Qhh3 "blend_color_luminance_Qhh3"
OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
OpName %guarded_divide_Qh3h3h "guarded_divide_Qh3h3h"
OpName %blend_set_color_luminance_Qh3h3hh3 "blend_set_color_luminance_Qh3h3hh3"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %blend_set_color_saturation_Qh3h3h3 "blend_set_color_saturation_Qh3h3h3"
OpName %mn "mn"
OpName %mx "mx"
OpName %blend_hslc_h4h4h4h2 "blend_hslc_h4h4h4h2"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %blend_hue_h4h4h4 "blend_hue_h4h4h4"
OpName %main "main"
OpDecorate %_GuardedDivideEpsilon RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %24 Binding 0
OpDecorate %24 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %lum RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %mn RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %mx RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
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
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_GuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
%bool = OpTypeBool
%false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
%float_0 = OpConstant %float 0
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%24 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%29 = OpTypeFunction %float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%36 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%_ptr_Function_float = OpTypePointer Function %float
%58 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%67 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_1 = OpConstant %float 1
%78 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%139 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%176 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%181 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_v2float
%254 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%262 = OpConstantComposite %v2float %float_0 %float_1
%void = OpTypeVoid
%266 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_color_luminance_Qhh3 = OpFunction %float None %29
%30 = OpFunctionParameter %_ptr_Function_v3float
%31 = OpLabel
%37 = OpLoad %v3float %30
%32 = OpDot %float %36 %37
OpReturnValue %32
OpFunctionEnd
%blend_color_saturation_Qhh3 = OpFunction %float None %29
%38 = OpFunctionParameter %_ptr_Function_v3float
%39 = OpLabel
%42 = OpLoad %v3float %38
%43 = OpCompositeExtract %float %42 0
%44 = OpLoad %v3float %38
%45 = OpCompositeExtract %float %44 1
%41 = OpExtInst %float %1 FMax %43 %45
%46 = OpLoad %v3float %38
%47 = OpCompositeExtract %float %46 2
%40 = OpExtInst %float %1 FMax %41 %47
%50 = OpLoad %v3float %38
%51 = OpCompositeExtract %float %50 0
%52 = OpLoad %v3float %38
%53 = OpCompositeExtract %float %52 1
%49 = OpExtInst %float %1 FMin %51 %53
%54 = OpLoad %v3float %38
%55 = OpCompositeExtract %float %54 2
%48 = OpExtInst %float %1 FMin %49 %55
%56 = OpFSub %float %40 %48
OpReturnValue %56
OpFunctionEnd
%guarded_divide_Qhhh = OpFunction %float None %58
%59 = OpFunctionParameter %_ptr_Function_float
%60 = OpFunctionParameter %_ptr_Function_float
%61 = OpLabel
%62 = OpLoad %float %59
%63 = OpLoad %float %60
%64 = OpLoad %float %_GuardedDivideEpsilon
%65 = OpFAdd %float %63 %64
%66 = OpFDiv %float %62 %65
OpReturnValue %66
OpFunctionEnd
%guarded_divide_Qh3h3h = OpFunction %v3float None %67
%68 = OpFunctionParameter %_ptr_Function_v3float
%69 = OpFunctionParameter %_ptr_Function_float
%70 = OpLabel
%71 = OpLoad %v3float %68
%72 = OpLoad %float %69
%73 = OpLoad %float %_GuardedDivideEpsilon
%74 = OpFAdd %float %72 %73
%76 = OpFDiv %float %float_1 %74
%77 = OpVectorTimesScalar %v3float %71 %76
OpReturnValue %77
OpFunctionEnd
%blend_set_color_luminance_Qh3h3hh3 = OpFunction %v3float None %78
%79 = OpFunctionParameter %_ptr_Function_v3float
%80 = OpFunctionParameter %_ptr_Function_float
%81 = OpFunctionParameter %_ptr_Function_v3float
%82 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%85 = OpVariable %_ptr_Function_v3float Function
%result = OpVariable %_ptr_Function_v3float Function
%89 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%113 = OpVariable %_ptr_Function_float Function
%115 = OpVariable %_ptr_Function_float Function
%133 = OpVariable %_ptr_Function_v3float Function
%135 = OpVariable %_ptr_Function_float Function
%84 = OpLoad %v3float %81
OpStore %85 %84
%86 = OpFunctionCall %float %blend_color_luminance_Qhh3 %85
OpStore %lum %86
%88 = OpLoad %v3float %79
OpStore %89 %88
%90 = OpFunctionCall %float %blend_color_luminance_Qhh3 %89
%91 = OpFSub %float %86 %90
%92 = OpLoad %v3float %79
%93 = OpCompositeConstruct %v3float %91 %91 %91
%94 = OpFAdd %v3float %93 %92
OpStore %result %94
%98 = OpCompositeExtract %float %94 0
%99 = OpCompositeExtract %float %94 1
%97 = OpExtInst %float %1 FMin %98 %99
%100 = OpCompositeExtract %float %94 2
%96 = OpExtInst %float %1 FMin %97 %100
OpStore %minComp %96
%103 = OpExtInst %float %1 FMax %98 %99
%102 = OpExtInst %float %1 FMax %103 %100
OpStore %maxComp %102
%104 = OpFOrdLessThan %bool %96 %float_0
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpFUnordNotEqual %bool %86 %96
OpBranch %106
%106 = OpLabel
%108 = OpPhi %bool %false %82 %107 %105
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpCompositeConstruct %v3float %86 %86 %86
%112 = OpFSub %v3float %94 %111
OpStore %113 %86
%114 = OpFSub %float %86 %96
OpStore %115 %114
%116 = OpFunctionCall %float %guarded_divide_Qhhh %113 %115
%117 = OpVectorTimesScalar %v3float %112 %116
%118 = OpFAdd %v3float %111 %117
OpStore %result %118
OpBranch %110
%110 = OpLabel
%119 = OpLoad %float %80
%120 = OpFOrdGreaterThan %bool %102 %119
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%123 = OpFUnordNotEqual %bool %102 %86
OpBranch %122
%122 = OpLabel
%124 = OpPhi %bool %false %110 %123 %121
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpLoad %v3float %result
%128 = OpCompositeConstruct %v3float %86 %86 %86
%129 = OpFSub %v3float %127 %128
%130 = OpLoad %float %80
%131 = OpFSub %float %130 %86
%132 = OpVectorTimesScalar %v3float %129 %131
OpStore %133 %132
%134 = OpFSub %float %102 %86
OpStore %135 %134
%136 = OpFunctionCall %v3float %guarded_divide_Qh3h3h %133 %135
%137 = OpFAdd %v3float %128 %136
OpStore %result %137
OpBranch %126
%126 = OpLabel
%138 = OpLoad %v3float %result
OpReturnValue %138
OpFunctionEnd
%blend_set_color_saturation_Qh3h3h3 = OpFunction %v3float None %139
%140 = OpFunctionParameter %_ptr_Function_v3float
%141 = OpFunctionParameter %_ptr_Function_v3float
%142 = OpLabel
%mn = OpVariable %_ptr_Function_float Function
%mx = OpVariable %_ptr_Function_float Function
%162 = OpVariable %_ptr_Function_v3float Function
%170 = OpVariable %_ptr_Function_v3float Function
%146 = OpLoad %v3float %140
%147 = OpCompositeExtract %float %146 0
%148 = OpLoad %v3float %140
%149 = OpCompositeExtract %float %148 1
%145 = OpExtInst %float %1 FMin %147 %149
%150 = OpLoad %v3float %140
%151 = OpCompositeExtract %float %150 2
%144 = OpExtInst %float %1 FMin %145 %151
OpStore %mn %144
%155 = OpLoad %v3float %140
%156 = OpCompositeExtract %float %155 0
%157 = OpLoad %v3float %140
%158 = OpCompositeExtract %float %157 1
%154 = OpExtInst %float %1 FMax %156 %158
%159 = OpLoad %v3float %140
%160 = OpCompositeExtract %float %159 2
%153 = OpExtInst %float %1 FMax %154 %160
OpStore %mx %153
%161 = OpFOrdGreaterThan %bool %153 %144
OpSelectionMerge %165 None
OpBranchConditional %161 %163 %164
%163 = OpLabel
%166 = OpLoad %v3float %140
%167 = OpCompositeConstruct %v3float %144 %144 %144
%168 = OpFSub %v3float %166 %167
%169 = OpLoad %v3float %141
OpStore %170 %169
%171 = OpFunctionCall %float %blend_color_saturation_Qhh3 %170
%172 = OpVectorTimesScalar %v3float %168 %171
%173 = OpFSub %float %153 %144
%174 = OpFDiv %float %float_1 %173
%175 = OpVectorTimesScalar %v3float %172 %174
OpStore %162 %175
OpBranch %165
%164 = OpLabel
OpStore %162 %176
OpBranch %165
%165 = OpLabel
%177 = OpLoad %v3float %162
OpReturnValue %177
OpFunctionEnd
%blend_hslc_h4h4h4h2 = OpFunction %v4float None %181
%182 = OpFunctionParameter %_ptr_Function_v4float
%183 = OpFunctionParameter %_ptr_Function_v4float
%184 = OpFunctionParameter %_ptr_Function_v2float
%185 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%208 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%217 = OpVariable %_ptr_Function_v3float Function
%227 = OpVariable %_ptr_Function_v3float Function
%228 = OpVariable %_ptr_Function_v3float Function
%231 = OpVariable %_ptr_Function_v3float Function
%232 = OpVariable %_ptr_Function_float Function
%234 = OpVariable %_ptr_Function_v3float Function
%187 = OpLoad %v4float %183
%188 = OpCompositeExtract %float %187 3
%189 = OpLoad %v4float %182
%190 = OpCompositeExtract %float %189 3
%191 = OpFMul %float %188 %190
OpStore %alpha %191
%193 = OpLoad %v4float %182
%194 = OpVectorShuffle %v3float %193 %193 0 1 2
%195 = OpLoad %v4float %183
%196 = OpCompositeExtract %float %195 3
%197 = OpVectorTimesScalar %v3float %194 %196
OpStore %sda %197
%199 = OpLoad %v4float %183
%200 = OpVectorShuffle %v3float %199 %199 0 1 2
%201 = OpLoad %v4float %182
%202 = OpCompositeExtract %float %201 3
%203 = OpVectorTimesScalar %v3float %200 %202
OpStore %dsa %203
%205 = OpLoad %v2float %184
%206 = OpCompositeExtract %float %205 0
%207 = OpFUnordNotEqual %bool %206 %float_0
OpSelectionMerge %211 None
OpBranchConditional %207 %209 %210
%209 = OpLabel
OpStore %208 %203
OpBranch %211
%210 = OpLabel
OpStore %208 %197
OpBranch %211
%211 = OpLabel
%212 = OpLoad %v3float %208
OpStore %l %212
%214 = OpLoad %v2float %184
%215 = OpCompositeExtract %float %214 0
%216 = OpFUnordNotEqual %bool %215 %float_0
OpSelectionMerge %220 None
OpBranchConditional %216 %218 %219
%218 = OpLabel
OpStore %217 %197
OpBranch %220
%219 = OpLabel
OpStore %217 %203
OpBranch %220
%220 = OpLabel
%221 = OpLoad %v3float %217
OpStore %r %221
%222 = OpLoad %v2float %184
%223 = OpCompositeExtract %float %222 1
%224 = OpFUnordNotEqual %bool %223 %float_0
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
OpStore %227 %212
OpStore %228 %221
%229 = OpFunctionCall %v3float %blend_set_color_saturation_Qh3h3h3 %227 %228
OpStore %l %229
OpStore %r %203
OpBranch %226
%226 = OpLabel
%230 = OpLoad %v3float %l
OpStore %231 %230
OpStore %232 %191
%233 = OpLoad %v3float %r
OpStore %234 %233
%235 = OpFunctionCall %v3float %blend_set_color_luminance_Qh3h3hh3 %231 %232 %234
%236 = OpLoad %v4float %183
%237 = OpVectorShuffle %v3float %236 %236 0 1 2
%238 = OpFAdd %v3float %235 %237
%239 = OpFSub %v3float %238 %203
%240 = OpLoad %v4float %182
%241 = OpVectorShuffle %v3float %240 %240 0 1 2
%242 = OpFAdd %v3float %239 %241
%243 = OpFSub %v3float %242 %197
%244 = OpCompositeExtract %float %243 0
%245 = OpCompositeExtract %float %243 1
%246 = OpCompositeExtract %float %243 2
%247 = OpLoad %v4float %182
%248 = OpCompositeExtract %float %247 3
%249 = OpLoad %v4float %183
%250 = OpCompositeExtract %float %249 3
%251 = OpFAdd %float %248 %250
%252 = OpFSub %float %251 %191
%253 = OpCompositeConstruct %v4float %244 %245 %246 %252
OpReturnValue %253
OpFunctionEnd
%blend_hue_h4h4h4 = OpFunction %v4float None %254
%255 = OpFunctionParameter %_ptr_Function_v4float
%256 = OpFunctionParameter %_ptr_Function_v4float
%257 = OpLabel
%259 = OpVariable %_ptr_Function_v4float Function
%261 = OpVariable %_ptr_Function_v4float Function
%263 = OpVariable %_ptr_Function_v2float Function
%258 = OpLoad %v4float %255
OpStore %259 %258
%260 = OpLoad %v4float %256
OpStore %261 %260
OpStore %263 %262
%264 = OpFunctionCall %v4float %blend_hslc_h4h4h4h2 %259 %261 %263
OpReturnValue %264
OpFunctionEnd
%main = OpFunction %void None %266
%267 = OpLabel
%273 = OpVariable %_ptr_Function_v4float Function
%277 = OpVariable %_ptr_Function_v4float Function
%16 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_GuardedDivideEpsilon %16
%268 = OpAccessChain %_ptr_Uniform_v4float %24 %int_0
%272 = OpLoad %v4float %268
OpStore %273 %272
%274 = OpAccessChain %_ptr_Uniform_v4float %24 %int_1
%276 = OpLoad %v4float %274
OpStore %277 %276
%278 = OpFunctionCall %v4float %blend_hue_h4h4h4 %273 %277
OpStore %sk_FragColor %278
OpReturn
OpFunctionEnd

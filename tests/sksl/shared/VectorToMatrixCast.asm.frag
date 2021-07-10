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
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %30 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
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
OpDecorate %278 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%mat2v2float = OpTypeMatrix %v2float 2
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%v2bool = OpTypeVector %bool 2
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v4int = OpTypeVector %int 4
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
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
%ok = OpVariable %_ptr_Function_bool Function
%279 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%30 = OpLoad %bool %ok
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%40 = OpCompositeExtract %float %37 2
%41 = OpCompositeExtract %float %37 3
%42 = OpCompositeConstruct %v2float %38 %39
%43 = OpCompositeConstruct %v2float %40 %41
%44 = OpCompositeConstruct %mat2v2float %42 %43
%49 = OpCompositeConstruct %v2float %float_n1_25 %float_0
%50 = OpCompositeConstruct %v2float %float_0_75 %float_2_25
%51 = OpCompositeConstruct %mat2v2float %49 %50
%53 = OpCompositeExtract %v2float %44 0
%54 = OpCompositeExtract %v2float %51 0
%55 = OpFOrdEqual %v2bool %53 %54
%56 = OpAll %bool %55
%57 = OpCompositeExtract %v2float %44 1
%58 = OpCompositeExtract %v2float %51 1
%59 = OpFOrdEqual %v2bool %57 %58
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %56 %60
OpBranch %32
%32 = OpLabel
%62 = OpPhi %bool %false %25 %61 %31
OpStore %ok %62
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%67 = OpLoad %v4float %66
%68 = OpCompositeExtract %float %67 0
%69 = OpCompositeExtract %float %67 1
%70 = OpCompositeExtract %float %67 2
%71 = OpCompositeExtract %float %67 3
%72 = OpCompositeConstruct %v2float %68 %69
%73 = OpCompositeConstruct %v2float %70 %71
%74 = OpCompositeConstruct %mat2v2float %72 %73
%75 = OpCompositeConstruct %v2float %float_n1_25 %float_0
%76 = OpCompositeConstruct %v2float %float_0_75 %float_2_25
%77 = OpCompositeConstruct %mat2v2float %75 %76
%78 = OpCompositeExtract %v2float %74 0
%79 = OpCompositeExtract %v2float %77 0
%80 = OpFOrdEqual %v2bool %78 %79
%81 = OpAll %bool %80
%82 = OpCompositeExtract %v2float %74 1
%83 = OpCompositeExtract %v2float %77 1
%84 = OpFOrdEqual %v2bool %82 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %81 %85
OpBranch %65
%65 = OpLabel
%87 = OpPhi %bool %false %32 %86 %64
OpStore %ok %87
%88 = OpLoad %bool %ok
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%93 = OpLoad %v4float %91
%94 = OpCompositeExtract %float %93 0
%95 = OpCompositeExtract %float %93 1
%96 = OpCompositeExtract %float %93 2
%97 = OpCompositeExtract %float %93 3
%98 = OpCompositeConstruct %v2float %94 %95
%99 = OpCompositeConstruct %v2float %96 %97
%100 = OpCompositeConstruct %mat2v2float %98 %99
%102 = OpCompositeConstruct %v2float %float_0 %float_1
%103 = OpCompositeConstruct %v2float %float_0 %float_1
%104 = OpCompositeConstruct %mat2v2float %102 %103
%105 = OpCompositeExtract %v2float %100 0
%106 = OpCompositeExtract %v2float %104 0
%107 = OpFOrdEqual %v2bool %105 %106
%108 = OpAll %bool %107
%109 = OpCompositeExtract %v2float %100 1
%110 = OpCompositeExtract %v2float %104 1
%111 = OpFOrdEqual %v2bool %109 %110
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %108 %112
OpBranch %90
%90 = OpLabel
%114 = OpPhi %bool %false %65 %113 %89
OpStore %ok %114
%115 = OpLoad %bool %ok
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%119 = OpLoad %v4float %118
%120 = OpCompositeExtract %float %119 0
%121 = OpCompositeExtract %float %119 1
%122 = OpCompositeExtract %float %119 2
%123 = OpCompositeExtract %float %119 3
%124 = OpCompositeConstruct %v2float %120 %121
%125 = OpCompositeConstruct %v2float %122 %123
%126 = OpCompositeConstruct %mat2v2float %124 %125
%127 = OpCompositeConstruct %v2float %float_0 %float_1
%128 = OpCompositeConstruct %v2float %float_0 %float_1
%129 = OpCompositeConstruct %mat2v2float %127 %128
%130 = OpCompositeExtract %v2float %126 0
%131 = OpCompositeExtract %v2float %129 0
%132 = OpFOrdEqual %v2bool %130 %131
%133 = OpAll %bool %132
%134 = OpCompositeExtract %v2float %126 1
%135 = OpCompositeExtract %v2float %129 1
%136 = OpFOrdEqual %v2bool %134 %135
%137 = OpAll %bool %136
%138 = OpLogicalAnd %bool %133 %137
OpBranch %117
%117 = OpLabel
%139 = OpPhi %bool %false %90 %138 %116
OpStore %ok %139
%140 = OpLoad %bool %ok
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%144 = OpLoad %v4float %143
%145 = OpCompositeExtract %float %144 0
%146 = OpConvertFToS %int %145
%147 = OpCompositeExtract %float %144 1
%148 = OpConvertFToS %int %147
%149 = OpCompositeExtract %float %144 2
%150 = OpConvertFToS %int %149
%151 = OpCompositeExtract %float %144 3
%152 = OpConvertFToS %int %151
%153 = OpCompositeConstruct %v4int %146 %148 %150 %152
%155 = OpCompositeExtract %int %153 0
%156 = OpConvertSToF %float %155
%157 = OpCompositeExtract %int %153 1
%158 = OpConvertSToF %float %157
%159 = OpCompositeExtract %int %153 2
%160 = OpConvertSToF %float %159
%161 = OpCompositeExtract %int %153 3
%162 = OpConvertSToF %float %161
%163 = OpCompositeConstruct %v4float %156 %158 %160 %162
%164 = OpCompositeExtract %float %163 0
%165 = OpCompositeExtract %float %163 1
%166 = OpCompositeExtract %float %163 2
%167 = OpCompositeExtract %float %163 3
%168 = OpCompositeConstruct %v2float %164 %165
%169 = OpCompositeConstruct %v2float %166 %167
%170 = OpCompositeConstruct %mat2v2float %168 %169
%171 = OpCompositeConstruct %v2float %float_0 %float_1
%172 = OpCompositeConstruct %v2float %float_0 %float_1
%173 = OpCompositeConstruct %mat2v2float %171 %172
%174 = OpCompositeExtract %v2float %170 0
%175 = OpCompositeExtract %v2float %173 0
%176 = OpFOrdEqual %v2bool %174 %175
%177 = OpAll %bool %176
%178 = OpCompositeExtract %v2float %170 1
%179 = OpCompositeExtract %v2float %173 1
%180 = OpFOrdEqual %v2bool %178 %179
%181 = OpAll %bool %180
%182 = OpLogicalAnd %bool %177 %181
OpBranch %142
%142 = OpLabel
%183 = OpPhi %bool %false %117 %182 %141
OpStore %ok %183
%184 = OpLoad %bool %ok
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%188 = OpLoad %v4float %187
%189 = OpCompositeExtract %float %188 0
%190 = OpCompositeExtract %float %188 1
%191 = OpCompositeExtract %float %188 2
%192 = OpCompositeExtract %float %188 3
%193 = OpCompositeConstruct %v2float %189 %190
%194 = OpCompositeConstruct %v2float %191 %192
%195 = OpCompositeConstruct %mat2v2float %193 %194
%196 = OpCompositeConstruct %v2float %float_0 %float_1
%197 = OpCompositeConstruct %v2float %float_0 %float_1
%198 = OpCompositeConstruct %mat2v2float %196 %197
%199 = OpCompositeExtract %v2float %195 0
%200 = OpCompositeExtract %v2float %198 0
%201 = OpFOrdEqual %v2bool %199 %200
%202 = OpAll %bool %201
%203 = OpCompositeExtract %v2float %195 1
%204 = OpCompositeExtract %v2float %198 1
%205 = OpFOrdEqual %v2bool %203 %204
%206 = OpAll %bool %205
%207 = OpLogicalAnd %bool %202 %206
OpBranch %186
%186 = OpLabel
%208 = OpPhi %bool %false %142 %207 %185
OpStore %ok %208
%209 = OpLoad %bool %ok
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%212 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%213 = OpLoad %v4float %212
%214 = OpCompositeExtract %float %213 0
%215 = OpCompositeExtract %float %213 1
%216 = OpCompositeExtract %float %213 2
%217 = OpCompositeExtract %float %213 3
%218 = OpCompositeConstruct %v2float %214 %215
%219 = OpCompositeConstruct %v2float %216 %217
%220 = OpCompositeConstruct %mat2v2float %218 %219
%221 = OpCompositeConstruct %v2float %float_0 %float_1
%222 = OpCompositeConstruct %v2float %float_0 %float_1
%223 = OpCompositeConstruct %mat2v2float %221 %222
%224 = OpCompositeExtract %v2float %220 0
%225 = OpCompositeExtract %v2float %223 0
%226 = OpFOrdEqual %v2bool %224 %225
%227 = OpAll %bool %226
%228 = OpCompositeExtract %v2float %220 1
%229 = OpCompositeExtract %v2float %223 1
%230 = OpFOrdEqual %v2bool %228 %229
%231 = OpAll %bool %230
%232 = OpLogicalAnd %bool %227 %231
OpBranch %211
%211 = OpLabel
%233 = OpPhi %bool %false %186 %232 %210
OpStore %ok %233
%234 = OpLoad %bool %ok
OpSelectionMerge %236 None
OpBranchConditional %234 %235 %236
%235 = OpLabel
%237 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%238 = OpLoad %v4float %237
%239 = OpCompositeExtract %float %238 0
%240 = OpFUnordNotEqual %bool %239 %float_0
%241 = OpCompositeExtract %float %238 1
%242 = OpFUnordNotEqual %bool %241 %float_0
%243 = OpCompositeExtract %float %238 2
%244 = OpFUnordNotEqual %bool %243 %float_0
%245 = OpCompositeExtract %float %238 3
%246 = OpFUnordNotEqual %bool %245 %float_0
%247 = OpCompositeConstruct %v4bool %240 %242 %244 %246
%249 = OpCompositeExtract %bool %247 0
%250 = OpSelect %float %249 %float_1 %float_0
%251 = OpCompositeExtract %bool %247 1
%252 = OpSelect %float %251 %float_1 %float_0
%253 = OpCompositeExtract %bool %247 2
%254 = OpSelect %float %253 %float_1 %float_0
%255 = OpCompositeExtract %bool %247 3
%256 = OpSelect %float %255 %float_1 %float_0
%257 = OpCompositeConstruct %v4float %250 %252 %254 %256
%258 = OpCompositeExtract %float %257 0
%259 = OpCompositeExtract %float %257 1
%260 = OpCompositeExtract %float %257 2
%261 = OpCompositeExtract %float %257 3
%262 = OpCompositeConstruct %v2float %258 %259
%263 = OpCompositeConstruct %v2float %260 %261
%264 = OpCompositeConstruct %mat2v2float %262 %263
%265 = OpCompositeConstruct %v2float %float_0 %float_1
%266 = OpCompositeConstruct %v2float %float_0 %float_1
%267 = OpCompositeConstruct %mat2v2float %265 %266
%268 = OpCompositeExtract %v2float %264 0
%269 = OpCompositeExtract %v2float %267 0
%270 = OpFOrdEqual %v2bool %268 %269
%271 = OpAll %bool %270
%272 = OpCompositeExtract %v2float %264 1
%273 = OpCompositeExtract %v2float %267 1
%274 = OpFOrdEqual %v2bool %272 %273
%275 = OpAll %bool %274
%276 = OpLogicalAnd %bool %271 %275
OpBranch %236
%236 = OpLabel
%277 = OpPhi %bool %false %211 %276 %235
OpStore %ok %277
%278 = OpLoad %bool %ok
OpSelectionMerge %283 None
OpBranchConditional %278 %281 %282
%281 = OpLabel
%284 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%285 = OpLoad %v4float %284
OpStore %279 %285
OpBranch %283
%282 = OpLabel
%286 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%288 = OpLoad %v4float %286
OpStore %279 %288
OpBranch %283
%283 = OpLabel
%289 = OpLoad %v4float %279
OpReturnValue %289
OpFunctionEnd

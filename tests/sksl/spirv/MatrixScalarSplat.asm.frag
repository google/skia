### Compilation failed:

error: SPIR-V validation error: Expected floating scalar or vector type as Result Type: FDiv
  %283 = OpFDiv %mat2v2float %282 %277

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
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
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
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
OpDecorate %536 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %572 RelaxedPrecision
OpDecorate %575 RelaxedPrecision
OpDecorate %576 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%float_4 = OpConstant %float 4
%float_6 = OpConstant %float 6
%v3bool = OpTypeVector %bool 3
%float_n2 = OpConstant %float -2
%float_n4 = OpConstant %float -4
%float_8 = OpConstant %float 8
%float_0_25 = OpConstant %float 0.25
%float_0_5 = OpConstant %float 0.5
%mat2v2float = OpTypeMatrix %v2float 2
%v2bool = OpTypeVector %bool 2
%299 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
OpStore %ok %true
%30 = OpLoad %bool %ok
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%36 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%37 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%38 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%34 = OpCompositeConstruct %mat3v3float %36 %37 %38
%41 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%42 = OpCompositeConstruct %mat3v3float %41 %41 %41
%43 = OpCompositeExtract %v3float %34 0
%44 = OpCompositeExtract %v3float %42 0
%45 = OpFAdd %v3float %43 %44
%46 = OpCompositeExtract %v3float %34 1
%47 = OpCompositeExtract %v3float %42 1
%48 = OpFAdd %v3float %46 %47
%49 = OpCompositeExtract %v3float %34 2
%50 = OpCompositeExtract %v3float %42 2
%51 = OpFAdd %v3float %49 %50
%52 = OpCompositeConstruct %mat3v3float %45 %48 %51
%55 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%56 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%57 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%54 = OpCompositeConstruct %mat3v3float %55 %56 %57
%59 = OpCompositeExtract %v3float %52 0
%60 = OpCompositeExtract %v3float %54 0
%61 = OpFOrdEqual %v3bool %59 %60
%62 = OpAll %bool %61
%63 = OpCompositeExtract %v3float %52 1
%64 = OpCompositeExtract %v3float %54 1
%65 = OpFOrdEqual %v3bool %63 %64
%66 = OpAll %bool %65
%67 = OpLogicalAnd %bool %62 %66
%68 = OpCompositeExtract %v3float %52 2
%69 = OpCompositeExtract %v3float %54 2
%70 = OpFOrdEqual %v3bool %68 %69
%71 = OpAll %bool %70
%72 = OpLogicalAnd %bool %67 %71
OpBranch %32
%32 = OpLabel
%73 = OpPhi %bool %false %25 %72 %31
OpStore %ok %73
%74 = OpLoad %bool %ok
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%79 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%80 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%77 = OpCompositeConstruct %mat3v3float %78 %79 %80
%81 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%82 = OpCompositeConstruct %mat3v3float %81 %81 %81
%83 = OpCompositeExtract %v3float %77 0
%84 = OpCompositeExtract %v3float %82 0
%85 = OpFSub %v3float %83 %84
%86 = OpCompositeExtract %v3float %77 1
%87 = OpCompositeExtract %v3float %82 1
%88 = OpFSub %v3float %86 %87
%89 = OpCompositeExtract %v3float %77 2
%90 = OpCompositeExtract %v3float %82 2
%91 = OpFSub %v3float %89 %90
%92 = OpCompositeConstruct %mat3v3float %85 %88 %91
%96 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%97 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%98 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%95 = OpCompositeConstruct %mat3v3float %96 %97 %98
%99 = OpCompositeExtract %v3float %92 0
%100 = OpCompositeExtract %v3float %95 0
%101 = OpFOrdEqual %v3bool %99 %100
%102 = OpAll %bool %101
%103 = OpCompositeExtract %v3float %92 1
%104 = OpCompositeExtract %v3float %95 1
%105 = OpFOrdEqual %v3bool %103 %104
%106 = OpAll %bool %105
%107 = OpLogicalAnd %bool %102 %106
%108 = OpCompositeExtract %v3float %92 2
%109 = OpCompositeExtract %v3float %95 2
%110 = OpFOrdEqual %v3bool %108 %109
%111 = OpAll %bool %110
%112 = OpLogicalAnd %bool %107 %111
OpBranch %76
%76 = OpLabel
%113 = OpPhi %bool %false %32 %112 %75
OpStore %ok %113
%114 = OpLoad %bool %ok
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%119 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%120 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%117 = OpCompositeConstruct %mat3v3float %118 %119 %120
%121 = OpMatrixTimesScalar %mat3v3float %117 %float_4
%124 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%125 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%126 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%123 = OpCompositeConstruct %mat3v3float %124 %125 %126
%127 = OpCompositeExtract %v3float %121 0
%128 = OpCompositeExtract %v3float %123 0
%129 = OpFOrdEqual %v3bool %127 %128
%130 = OpAll %bool %129
%131 = OpCompositeExtract %v3float %121 1
%132 = OpCompositeExtract %v3float %123 1
%133 = OpFOrdEqual %v3bool %131 %132
%134 = OpAll %bool %133
%135 = OpLogicalAnd %bool %130 %134
%136 = OpCompositeExtract %v3float %121 2
%137 = OpCompositeExtract %v3float %123 2
%138 = OpFOrdEqual %v3bool %136 %137
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %135 %139
OpBranch %116
%116 = OpLabel
%141 = OpPhi %bool %false %76 %140 %115
OpStore %ok %141
%142 = OpLoad %bool %ok
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%146 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%147 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%148 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%145 = OpCompositeConstruct %mat3v3float %146 %147 %148
%150 = OpMatrixTimesScalar %mat3v3float %145 %float_0_25
%153 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%154 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%155 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%152 = OpCompositeConstruct %mat3v3float %153 %154 %155
%156 = OpCompositeExtract %v3float %150 0
%157 = OpCompositeExtract %v3float %152 0
%158 = OpFOrdEqual %v3bool %156 %157
%159 = OpAll %bool %158
%160 = OpCompositeExtract %v3float %150 1
%161 = OpCompositeExtract %v3float %152 1
%162 = OpFOrdEqual %v3bool %160 %161
%163 = OpAll %bool %162
%164 = OpLogicalAnd %bool %159 %163
%165 = OpCompositeExtract %v3float %150 2
%166 = OpCompositeExtract %v3float %152 2
%167 = OpFOrdEqual %v3bool %165 %166
%168 = OpAll %bool %167
%169 = OpLogicalAnd %bool %164 %168
OpBranch %144
%144 = OpLabel
%170 = OpPhi %bool %false %116 %169 %143
OpStore %ok %170
%171 = OpLoad %bool %ok
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%175 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%176 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%177 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%174 = OpCompositeConstruct %mat3v3float %175 %176 %177
%178 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%179 = OpCompositeConstruct %mat3v3float %178 %178 %178
%180 = OpCompositeExtract %v3float %179 0
%181 = OpCompositeExtract %v3float %174 0
%182 = OpFAdd %v3float %180 %181
%183 = OpCompositeExtract %v3float %179 1
%184 = OpCompositeExtract %v3float %174 1
%185 = OpFAdd %v3float %183 %184
%186 = OpCompositeExtract %v3float %179 2
%187 = OpCompositeExtract %v3float %174 2
%188 = OpFAdd %v3float %186 %187
%189 = OpCompositeConstruct %mat3v3float %182 %185 %188
%191 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%192 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%193 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%190 = OpCompositeConstruct %mat3v3float %191 %192 %193
%194 = OpCompositeExtract %v3float %189 0
%195 = OpCompositeExtract %v3float %190 0
%196 = OpFOrdEqual %v3bool %194 %195
%197 = OpAll %bool %196
%198 = OpCompositeExtract %v3float %189 1
%199 = OpCompositeExtract %v3float %190 1
%200 = OpFOrdEqual %v3bool %198 %199
%201 = OpAll %bool %200
%202 = OpLogicalAnd %bool %197 %201
%203 = OpCompositeExtract %v3float %189 2
%204 = OpCompositeExtract %v3float %190 2
%205 = OpFOrdEqual %v3bool %203 %204
%206 = OpAll %bool %205
%207 = OpLogicalAnd %bool %202 %206
OpBranch %173
%173 = OpLabel
%208 = OpPhi %bool %false %144 %207 %172
OpStore %ok %208
%209 = OpLoad %bool %ok
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%213 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%214 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%215 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%212 = OpCompositeConstruct %mat3v3float %213 %214 %215
%216 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%217 = OpCompositeConstruct %mat3v3float %216 %216 %216
%218 = OpCompositeExtract %v3float %217 0
%219 = OpCompositeExtract %v3float %212 0
%220 = OpFSub %v3float %218 %219
%221 = OpCompositeExtract %v3float %217 1
%222 = OpCompositeExtract %v3float %212 1
%223 = OpFSub %v3float %221 %222
%224 = OpCompositeExtract %v3float %217 2
%225 = OpCompositeExtract %v3float %212 2
%226 = OpFSub %v3float %224 %225
%227 = OpCompositeConstruct %mat3v3float %220 %223 %226
%229 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%230 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%231 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%228 = OpCompositeConstruct %mat3v3float %229 %230 %231
%232 = OpCompositeExtract %v3float %227 0
%233 = OpCompositeExtract %v3float %228 0
%234 = OpFOrdEqual %v3bool %232 %233
%235 = OpAll %bool %234
%236 = OpCompositeExtract %v3float %227 1
%237 = OpCompositeExtract %v3float %228 1
%238 = OpFOrdEqual %v3bool %236 %237
%239 = OpAll %bool %238
%240 = OpLogicalAnd %bool %235 %239
%241 = OpCompositeExtract %v3float %227 2
%242 = OpCompositeExtract %v3float %228 2
%243 = OpFOrdEqual %v3bool %241 %242
%244 = OpAll %bool %243
%245 = OpLogicalAnd %bool %240 %244
OpBranch %211
%211 = OpLabel
%246 = OpPhi %bool %false %173 %245 %210
OpStore %ok %246
%247 = OpLoad %bool %ok
OpSelectionMerge %249 None
OpBranchConditional %247 %248 %249
%248 = OpLabel
%251 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%252 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%253 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%250 = OpCompositeConstruct %mat3v3float %251 %252 %253
%254 = OpMatrixTimesScalar %mat3v3float %250 %float_4
%256 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%257 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%258 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%255 = OpCompositeConstruct %mat3v3float %256 %257 %258
%259 = OpCompositeExtract %v3float %254 0
%260 = OpCompositeExtract %v3float %255 0
%261 = OpFOrdEqual %v3bool %259 %260
%262 = OpAll %bool %261
%263 = OpCompositeExtract %v3float %254 1
%264 = OpCompositeExtract %v3float %255 1
%265 = OpFOrdEqual %v3bool %263 %264
%266 = OpAll %bool %265
%267 = OpLogicalAnd %bool %262 %266
%268 = OpCompositeExtract %v3float %254 2
%269 = OpCompositeExtract %v3float %255 2
%270 = OpFOrdEqual %v3bool %268 %269
%271 = OpAll %bool %270
%272 = OpLogicalAnd %bool %267 %271
OpBranch %249
%249 = OpLabel
%273 = OpPhi %bool %false %211 %272 %248
OpStore %ok %273
%274 = OpLoad %bool %ok
OpSelectionMerge %276 None
OpBranchConditional %274 %275 %276
%275 = OpLabel
%278 = OpCompositeConstruct %v2float %float_2 %float_2
%279 = OpCompositeConstruct %v2float %float_2 %float_2
%277 = OpCompositeConstruct %mat2v2float %278 %279
%281 = OpCompositeConstruct %v2float %float_4 %float_4
%282 = OpCompositeConstruct %mat2v2float %281 %281
%283 = OpFDiv %mat2v2float %282 %277
%285 = OpCompositeConstruct %v2float %float_2 %float_2
%286 = OpCompositeConstruct %v2float %float_2 %float_2
%284 = OpCompositeConstruct %mat2v2float %285 %286
%288 = OpCompositeExtract %v2float %283 0
%289 = OpCompositeExtract %v2float %284 0
%290 = OpFOrdEqual %v2bool %288 %289
%291 = OpAll %bool %290
%292 = OpCompositeExtract %v2float %283 1
%293 = OpCompositeExtract %v2float %284 1
%294 = OpFOrdEqual %v2bool %292 %293
%295 = OpAll %bool %294
%296 = OpLogicalAnd %bool %291 %295
OpBranch %276
%276 = OpLabel
%297 = OpPhi %bool %false %249 %296 %275
OpStore %ok %297
%298 = OpLoad %bool %ok
OpReturnValue %298
OpFunctionEnd
%main = OpFunction %v4float None %299
%300 = OpFunctionParameter %_ptr_Function_v2float
%301 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%563 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%303 = OpLoad %bool %_0_ok
OpSelectionMerge %305 None
OpBranchConditional %303 %304 %305
%304 = OpLabel
%307 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%308 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%309 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%306 = OpCompositeConstruct %mat3v3float %307 %308 %309
%310 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%311 = OpCompositeConstruct %mat3v3float %310 %310 %310
%312 = OpCompositeExtract %v3float %306 0
%313 = OpCompositeExtract %v3float %311 0
%314 = OpFAdd %v3float %312 %313
%315 = OpCompositeExtract %v3float %306 1
%316 = OpCompositeExtract %v3float %311 1
%317 = OpFAdd %v3float %315 %316
%318 = OpCompositeExtract %v3float %306 2
%319 = OpCompositeExtract %v3float %311 2
%320 = OpFAdd %v3float %318 %319
%321 = OpCompositeConstruct %mat3v3float %314 %317 %320
%323 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%324 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%325 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%322 = OpCompositeConstruct %mat3v3float %323 %324 %325
%326 = OpCompositeExtract %v3float %321 0
%327 = OpCompositeExtract %v3float %322 0
%328 = OpFOrdEqual %v3bool %326 %327
%329 = OpAll %bool %328
%330 = OpCompositeExtract %v3float %321 1
%331 = OpCompositeExtract %v3float %322 1
%332 = OpFOrdEqual %v3bool %330 %331
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %329 %333
%335 = OpCompositeExtract %v3float %321 2
%336 = OpCompositeExtract %v3float %322 2
%337 = OpFOrdEqual %v3bool %335 %336
%338 = OpAll %bool %337
%339 = OpLogicalAnd %bool %334 %338
OpBranch %305
%305 = OpLabel
%340 = OpPhi %bool %false %301 %339 %304
OpStore %_0_ok %340
%341 = OpLoad %bool %_0_ok
OpSelectionMerge %343 None
OpBranchConditional %341 %342 %343
%342 = OpLabel
%345 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%346 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%347 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%344 = OpCompositeConstruct %mat3v3float %345 %346 %347
%348 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%349 = OpCompositeConstruct %mat3v3float %348 %348 %348
%350 = OpCompositeExtract %v3float %344 0
%351 = OpCompositeExtract %v3float %349 0
%352 = OpFSub %v3float %350 %351
%353 = OpCompositeExtract %v3float %344 1
%354 = OpCompositeExtract %v3float %349 1
%355 = OpFSub %v3float %353 %354
%356 = OpCompositeExtract %v3float %344 2
%357 = OpCompositeExtract %v3float %349 2
%358 = OpFSub %v3float %356 %357
%359 = OpCompositeConstruct %mat3v3float %352 %355 %358
%361 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%362 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%363 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%360 = OpCompositeConstruct %mat3v3float %361 %362 %363
%364 = OpCompositeExtract %v3float %359 0
%365 = OpCompositeExtract %v3float %360 0
%366 = OpFOrdEqual %v3bool %364 %365
%367 = OpAll %bool %366
%368 = OpCompositeExtract %v3float %359 1
%369 = OpCompositeExtract %v3float %360 1
%370 = OpFOrdEqual %v3bool %368 %369
%371 = OpAll %bool %370
%372 = OpLogicalAnd %bool %367 %371
%373 = OpCompositeExtract %v3float %359 2
%374 = OpCompositeExtract %v3float %360 2
%375 = OpFOrdEqual %v3bool %373 %374
%376 = OpAll %bool %375
%377 = OpLogicalAnd %bool %372 %376
OpBranch %343
%343 = OpLabel
%378 = OpPhi %bool %false %305 %377 %342
OpStore %_0_ok %378
%379 = OpLoad %bool %_0_ok
OpSelectionMerge %381 None
OpBranchConditional %379 %380 %381
%380 = OpLabel
%383 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%384 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%385 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%382 = OpCompositeConstruct %mat3v3float %383 %384 %385
%386 = OpMatrixTimesScalar %mat3v3float %382 %float_4
%388 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%389 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%390 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%387 = OpCompositeConstruct %mat3v3float %388 %389 %390
%391 = OpCompositeExtract %v3float %386 0
%392 = OpCompositeExtract %v3float %387 0
%393 = OpFOrdEqual %v3bool %391 %392
%394 = OpAll %bool %393
%395 = OpCompositeExtract %v3float %386 1
%396 = OpCompositeExtract %v3float %387 1
%397 = OpFOrdEqual %v3bool %395 %396
%398 = OpAll %bool %397
%399 = OpLogicalAnd %bool %394 %398
%400 = OpCompositeExtract %v3float %386 2
%401 = OpCompositeExtract %v3float %387 2
%402 = OpFOrdEqual %v3bool %400 %401
%403 = OpAll %bool %402
%404 = OpLogicalAnd %bool %399 %403
OpBranch %381
%381 = OpLabel
%405 = OpPhi %bool %false %343 %404 %380
OpStore %_0_ok %405
%406 = OpLoad %bool %_0_ok
OpSelectionMerge %408 None
OpBranchConditional %406 %407 %408
%407 = OpLabel
%410 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%411 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%412 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%409 = OpCompositeConstruct %mat3v3float %410 %411 %412
%413 = OpMatrixTimesScalar %mat3v3float %409 %float_0_25
%415 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%416 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%417 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%414 = OpCompositeConstruct %mat3v3float %415 %416 %417
%418 = OpCompositeExtract %v3float %413 0
%419 = OpCompositeExtract %v3float %414 0
%420 = OpFOrdEqual %v3bool %418 %419
%421 = OpAll %bool %420
%422 = OpCompositeExtract %v3float %413 1
%423 = OpCompositeExtract %v3float %414 1
%424 = OpFOrdEqual %v3bool %422 %423
%425 = OpAll %bool %424
%426 = OpLogicalAnd %bool %421 %425
%427 = OpCompositeExtract %v3float %413 2
%428 = OpCompositeExtract %v3float %414 2
%429 = OpFOrdEqual %v3bool %427 %428
%430 = OpAll %bool %429
%431 = OpLogicalAnd %bool %426 %430
OpBranch %408
%408 = OpLabel
%432 = OpPhi %bool %false %381 %431 %407
OpStore %_0_ok %432
%433 = OpLoad %bool %_0_ok
OpSelectionMerge %435 None
OpBranchConditional %433 %434 %435
%434 = OpLabel
%437 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%438 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%439 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%436 = OpCompositeConstruct %mat3v3float %437 %438 %439
%440 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%441 = OpCompositeConstruct %mat3v3float %440 %440 %440
%442 = OpCompositeExtract %v3float %441 0
%443 = OpCompositeExtract %v3float %436 0
%444 = OpFAdd %v3float %442 %443
%445 = OpCompositeExtract %v3float %441 1
%446 = OpCompositeExtract %v3float %436 1
%447 = OpFAdd %v3float %445 %446
%448 = OpCompositeExtract %v3float %441 2
%449 = OpCompositeExtract %v3float %436 2
%450 = OpFAdd %v3float %448 %449
%451 = OpCompositeConstruct %mat3v3float %444 %447 %450
%453 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%454 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%455 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%452 = OpCompositeConstruct %mat3v3float %453 %454 %455
%456 = OpCompositeExtract %v3float %451 0
%457 = OpCompositeExtract %v3float %452 0
%458 = OpFOrdEqual %v3bool %456 %457
%459 = OpAll %bool %458
%460 = OpCompositeExtract %v3float %451 1
%461 = OpCompositeExtract %v3float %452 1
%462 = OpFOrdEqual %v3bool %460 %461
%463 = OpAll %bool %462
%464 = OpLogicalAnd %bool %459 %463
%465 = OpCompositeExtract %v3float %451 2
%466 = OpCompositeExtract %v3float %452 2
%467 = OpFOrdEqual %v3bool %465 %466
%468 = OpAll %bool %467
%469 = OpLogicalAnd %bool %464 %468
OpBranch %435
%435 = OpLabel
%470 = OpPhi %bool %false %408 %469 %434
OpStore %_0_ok %470
%471 = OpLoad %bool %_0_ok
OpSelectionMerge %473 None
OpBranchConditional %471 %472 %473
%472 = OpLabel
%475 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%476 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%477 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%474 = OpCompositeConstruct %mat3v3float %475 %476 %477
%478 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%479 = OpCompositeConstruct %mat3v3float %478 %478 %478
%480 = OpCompositeExtract %v3float %479 0
%481 = OpCompositeExtract %v3float %474 0
%482 = OpFSub %v3float %480 %481
%483 = OpCompositeExtract %v3float %479 1
%484 = OpCompositeExtract %v3float %474 1
%485 = OpFSub %v3float %483 %484
%486 = OpCompositeExtract %v3float %479 2
%487 = OpCompositeExtract %v3float %474 2
%488 = OpFSub %v3float %486 %487
%489 = OpCompositeConstruct %mat3v3float %482 %485 %488
%491 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%492 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%493 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%490 = OpCompositeConstruct %mat3v3float %491 %492 %493
%494 = OpCompositeExtract %v3float %489 0
%495 = OpCompositeExtract %v3float %490 0
%496 = OpFOrdEqual %v3bool %494 %495
%497 = OpAll %bool %496
%498 = OpCompositeExtract %v3float %489 1
%499 = OpCompositeExtract %v3float %490 1
%500 = OpFOrdEqual %v3bool %498 %499
%501 = OpAll %bool %500
%502 = OpLogicalAnd %bool %497 %501
%503 = OpCompositeExtract %v3float %489 2
%504 = OpCompositeExtract %v3float %490 2
%505 = OpFOrdEqual %v3bool %503 %504
%506 = OpAll %bool %505
%507 = OpLogicalAnd %bool %502 %506
OpBranch %473
%473 = OpLabel
%508 = OpPhi %bool %false %435 %507 %472
OpStore %_0_ok %508
%509 = OpLoad %bool %_0_ok
OpSelectionMerge %511 None
OpBranchConditional %509 %510 %511
%510 = OpLabel
%513 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%514 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%515 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%512 = OpCompositeConstruct %mat3v3float %513 %514 %515
%516 = OpMatrixTimesScalar %mat3v3float %512 %float_4
%518 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%519 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%520 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%517 = OpCompositeConstruct %mat3v3float %518 %519 %520
%521 = OpCompositeExtract %v3float %516 0
%522 = OpCompositeExtract %v3float %517 0
%523 = OpFOrdEqual %v3bool %521 %522
%524 = OpAll %bool %523
%525 = OpCompositeExtract %v3float %516 1
%526 = OpCompositeExtract %v3float %517 1
%527 = OpFOrdEqual %v3bool %525 %526
%528 = OpAll %bool %527
%529 = OpLogicalAnd %bool %524 %528
%530 = OpCompositeExtract %v3float %516 2
%531 = OpCompositeExtract %v3float %517 2
%532 = OpFOrdEqual %v3bool %530 %531
%533 = OpAll %bool %532
%534 = OpLogicalAnd %bool %529 %533
OpBranch %511
%511 = OpLabel
%535 = OpPhi %bool %false %473 %534 %510
OpStore %_0_ok %535
%536 = OpLoad %bool %_0_ok
OpSelectionMerge %538 None
OpBranchConditional %536 %537 %538
%537 = OpLabel
%540 = OpCompositeConstruct %v2float %float_2 %float_2
%541 = OpCompositeConstruct %v2float %float_2 %float_2
%539 = OpCompositeConstruct %mat2v2float %540 %541
%542 = OpCompositeConstruct %v2float %float_4 %float_4
%543 = OpCompositeConstruct %mat2v2float %542 %542
%544 = OpFDiv %mat2v2float %543 %539
%546 = OpCompositeConstruct %v2float %float_2 %float_2
%547 = OpCompositeConstruct %v2float %float_2 %float_2
%545 = OpCompositeConstruct %mat2v2float %546 %547
%548 = OpCompositeExtract %v2float %544 0
%549 = OpCompositeExtract %v2float %545 0
%550 = OpFOrdEqual %v2bool %548 %549
%551 = OpAll %bool %550
%552 = OpCompositeExtract %v2float %544 1
%553 = OpCompositeExtract %v2float %545 1
%554 = OpFOrdEqual %v2bool %552 %553
%555 = OpAll %bool %554
%556 = OpLogicalAnd %bool %551 %555
OpBranch %538
%538 = OpLabel
%557 = OpPhi %bool %false %511 %556 %537
OpStore %_0_ok %557
%558 = OpLoad %bool %_0_ok
OpSelectionMerge %560 None
OpBranchConditional %558 %559 %560
%559 = OpLabel
%561 = OpFunctionCall %bool %test_half_b
OpBranch %560
%560 = OpLabel
%562 = OpPhi %bool %false %538 %561 %559
OpSelectionMerge %567 None
OpBranchConditional %562 %565 %566
%565 = OpLabel
%568 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%572 = OpLoad %v4float %568
OpStore %563 %572
OpBranch %567
%566 = OpLabel
%573 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%575 = OpLoad %v4float %573
OpStore %563 %575
OpBranch %567
%567 = OpLabel
%576 = OpLoad %v4float %563
OpReturnValue %576
OpFunctionEnd

1 error

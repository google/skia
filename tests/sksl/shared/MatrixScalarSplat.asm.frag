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
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %542 RelaxedPrecision
OpDecorate %570 RelaxedPrecision
OpDecorate %584 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %588 RelaxedPrecision
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
%305 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%54 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%55 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%56 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%57 = OpCompositeConstruct %mat3v3float %54 %55 %56
%59 = OpCompositeExtract %v3float %52 0
%60 = OpCompositeExtract %v3float %57 0
%61 = OpFOrdEqual %v3bool %59 %60
%62 = OpAll %bool %61
%63 = OpCompositeExtract %v3float %52 1
%64 = OpCompositeExtract %v3float %57 1
%65 = OpFOrdEqual %v3bool %63 %64
%66 = OpAll %bool %65
%67 = OpLogicalAnd %bool %62 %66
%68 = OpCompositeExtract %v3float %52 2
%69 = OpCompositeExtract %v3float %57 2
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
%95 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%96 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%97 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%98 = OpCompositeConstruct %mat3v3float %95 %96 %97
%99 = OpCompositeExtract %v3float %92 0
%100 = OpCompositeExtract %v3float %98 0
%101 = OpFOrdEqual %v3bool %99 %100
%102 = OpAll %bool %101
%103 = OpCompositeExtract %v3float %92 1
%104 = OpCompositeExtract %v3float %98 1
%105 = OpFOrdEqual %v3bool %103 %104
%106 = OpAll %bool %105
%107 = OpLogicalAnd %bool %102 %106
%108 = OpCompositeExtract %v3float %92 2
%109 = OpCompositeExtract %v3float %98 2
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
%190 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%191 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%192 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%193 = OpCompositeConstruct %mat3v3float %190 %191 %192
%194 = OpCompositeExtract %v3float %189 0
%195 = OpCompositeExtract %v3float %193 0
%196 = OpFOrdEqual %v3bool %194 %195
%197 = OpAll %bool %196
%198 = OpCompositeExtract %v3float %189 1
%199 = OpCompositeExtract %v3float %193 1
%200 = OpFOrdEqual %v3bool %198 %199
%201 = OpAll %bool %200
%202 = OpLogicalAnd %bool %197 %201
%203 = OpCompositeExtract %v3float %189 2
%204 = OpCompositeExtract %v3float %193 2
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
%228 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%229 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%230 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%231 = OpCompositeConstruct %mat3v3float %228 %229 %230
%232 = OpCompositeExtract %v3float %227 0
%233 = OpCompositeExtract %v3float %231 0
%234 = OpFOrdEqual %v3bool %232 %233
%235 = OpAll %bool %234
%236 = OpCompositeExtract %v3float %227 1
%237 = OpCompositeExtract %v3float %231 1
%238 = OpFOrdEqual %v3bool %236 %237
%239 = OpAll %bool %238
%240 = OpLogicalAnd %bool %235 %239
%241 = OpCompositeExtract %v3float %227 2
%242 = OpCompositeExtract %v3float %231 2
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
%277 = OpCompositeConstruct %v2float %float_2 %float_2
%278 = OpCompositeConstruct %v2float %float_2 %float_2
%279 = OpCompositeConstruct %mat2v2float %277 %278
%281 = OpCompositeConstruct %v2float %float_4 %float_4
%282 = OpCompositeConstruct %mat2v2float %281 %281
%283 = OpCompositeExtract %v2float %282 0
%284 = OpCompositeExtract %v2float %279 0
%285 = OpFDiv %v2float %283 %284
%286 = OpCompositeExtract %v2float %282 1
%287 = OpCompositeExtract %v2float %279 1
%288 = OpFDiv %v2float %286 %287
%289 = OpCompositeConstruct %mat2v2float %285 %288
%290 = OpCompositeConstruct %v2float %float_2 %float_2
%291 = OpCompositeConstruct %v2float %float_2 %float_2
%292 = OpCompositeConstruct %mat2v2float %290 %291
%294 = OpCompositeExtract %v2float %289 0
%295 = OpCompositeExtract %v2float %292 0
%296 = OpFOrdEqual %v2bool %294 %295
%297 = OpAll %bool %296
%298 = OpCompositeExtract %v2float %289 1
%299 = OpCompositeExtract %v2float %292 1
%300 = OpFOrdEqual %v2bool %298 %299
%301 = OpAll %bool %300
%302 = OpLogicalAnd %bool %297 %301
OpBranch %276
%276 = OpLabel
%303 = OpPhi %bool %false %249 %302 %275
OpStore %ok %303
%304 = OpLoad %bool %ok
OpReturnValue %304
OpFunctionEnd
%main = OpFunction %v4float None %305
%306 = OpFunctionParameter %_ptr_Function_v2float
%307 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%575 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%309 = OpLoad %bool %_0_ok
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%313 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%314 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%315 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%312 = OpCompositeConstruct %mat3v3float %313 %314 %315
%316 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%317 = OpCompositeConstruct %mat3v3float %316 %316 %316
%318 = OpCompositeExtract %v3float %312 0
%319 = OpCompositeExtract %v3float %317 0
%320 = OpFAdd %v3float %318 %319
%321 = OpCompositeExtract %v3float %312 1
%322 = OpCompositeExtract %v3float %317 1
%323 = OpFAdd %v3float %321 %322
%324 = OpCompositeExtract %v3float %312 2
%325 = OpCompositeExtract %v3float %317 2
%326 = OpFAdd %v3float %324 %325
%327 = OpCompositeConstruct %mat3v3float %320 %323 %326
%328 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%329 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%330 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%331 = OpCompositeConstruct %mat3v3float %328 %329 %330
%332 = OpCompositeExtract %v3float %327 0
%333 = OpCompositeExtract %v3float %331 0
%334 = OpFOrdEqual %v3bool %332 %333
%335 = OpAll %bool %334
%336 = OpCompositeExtract %v3float %327 1
%337 = OpCompositeExtract %v3float %331 1
%338 = OpFOrdEqual %v3bool %336 %337
%339 = OpAll %bool %338
%340 = OpLogicalAnd %bool %335 %339
%341 = OpCompositeExtract %v3float %327 2
%342 = OpCompositeExtract %v3float %331 2
%343 = OpFOrdEqual %v3bool %341 %342
%344 = OpAll %bool %343
%345 = OpLogicalAnd %bool %340 %344
OpBranch %311
%311 = OpLabel
%346 = OpPhi %bool %false %307 %345 %310
OpStore %_0_ok %346
%347 = OpLoad %bool %_0_ok
OpSelectionMerge %349 None
OpBranchConditional %347 %348 %349
%348 = OpLabel
%351 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%352 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%353 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%350 = OpCompositeConstruct %mat3v3float %351 %352 %353
%354 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%355 = OpCompositeConstruct %mat3v3float %354 %354 %354
%356 = OpCompositeExtract %v3float %350 0
%357 = OpCompositeExtract %v3float %355 0
%358 = OpFSub %v3float %356 %357
%359 = OpCompositeExtract %v3float %350 1
%360 = OpCompositeExtract %v3float %355 1
%361 = OpFSub %v3float %359 %360
%362 = OpCompositeExtract %v3float %350 2
%363 = OpCompositeExtract %v3float %355 2
%364 = OpFSub %v3float %362 %363
%365 = OpCompositeConstruct %mat3v3float %358 %361 %364
%366 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%367 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%368 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%369 = OpCompositeConstruct %mat3v3float %366 %367 %368
%370 = OpCompositeExtract %v3float %365 0
%371 = OpCompositeExtract %v3float %369 0
%372 = OpFOrdEqual %v3bool %370 %371
%373 = OpAll %bool %372
%374 = OpCompositeExtract %v3float %365 1
%375 = OpCompositeExtract %v3float %369 1
%376 = OpFOrdEqual %v3bool %374 %375
%377 = OpAll %bool %376
%378 = OpLogicalAnd %bool %373 %377
%379 = OpCompositeExtract %v3float %365 2
%380 = OpCompositeExtract %v3float %369 2
%381 = OpFOrdEqual %v3bool %379 %380
%382 = OpAll %bool %381
%383 = OpLogicalAnd %bool %378 %382
OpBranch %349
%349 = OpLabel
%384 = OpPhi %bool %false %311 %383 %348
OpStore %_0_ok %384
%385 = OpLoad %bool %_0_ok
OpSelectionMerge %387 None
OpBranchConditional %385 %386 %387
%386 = OpLabel
%389 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%390 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%391 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%388 = OpCompositeConstruct %mat3v3float %389 %390 %391
%392 = OpMatrixTimesScalar %mat3v3float %388 %float_4
%394 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%395 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%396 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%393 = OpCompositeConstruct %mat3v3float %394 %395 %396
%397 = OpCompositeExtract %v3float %392 0
%398 = OpCompositeExtract %v3float %393 0
%399 = OpFOrdEqual %v3bool %397 %398
%400 = OpAll %bool %399
%401 = OpCompositeExtract %v3float %392 1
%402 = OpCompositeExtract %v3float %393 1
%403 = OpFOrdEqual %v3bool %401 %402
%404 = OpAll %bool %403
%405 = OpLogicalAnd %bool %400 %404
%406 = OpCompositeExtract %v3float %392 2
%407 = OpCompositeExtract %v3float %393 2
%408 = OpFOrdEqual %v3bool %406 %407
%409 = OpAll %bool %408
%410 = OpLogicalAnd %bool %405 %409
OpBranch %387
%387 = OpLabel
%411 = OpPhi %bool %false %349 %410 %386
OpStore %_0_ok %411
%412 = OpLoad %bool %_0_ok
OpSelectionMerge %414 None
OpBranchConditional %412 %413 %414
%413 = OpLabel
%416 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%417 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%418 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%415 = OpCompositeConstruct %mat3v3float %416 %417 %418
%419 = OpMatrixTimesScalar %mat3v3float %415 %float_0_25
%421 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%422 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%423 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%420 = OpCompositeConstruct %mat3v3float %421 %422 %423
%424 = OpCompositeExtract %v3float %419 0
%425 = OpCompositeExtract %v3float %420 0
%426 = OpFOrdEqual %v3bool %424 %425
%427 = OpAll %bool %426
%428 = OpCompositeExtract %v3float %419 1
%429 = OpCompositeExtract %v3float %420 1
%430 = OpFOrdEqual %v3bool %428 %429
%431 = OpAll %bool %430
%432 = OpLogicalAnd %bool %427 %431
%433 = OpCompositeExtract %v3float %419 2
%434 = OpCompositeExtract %v3float %420 2
%435 = OpFOrdEqual %v3bool %433 %434
%436 = OpAll %bool %435
%437 = OpLogicalAnd %bool %432 %436
OpBranch %414
%414 = OpLabel
%438 = OpPhi %bool %false %387 %437 %413
OpStore %_0_ok %438
%439 = OpLoad %bool %_0_ok
OpSelectionMerge %441 None
OpBranchConditional %439 %440 %441
%440 = OpLabel
%443 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%444 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%445 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%442 = OpCompositeConstruct %mat3v3float %443 %444 %445
%446 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%447 = OpCompositeConstruct %mat3v3float %446 %446 %446
%448 = OpCompositeExtract %v3float %447 0
%449 = OpCompositeExtract %v3float %442 0
%450 = OpFAdd %v3float %448 %449
%451 = OpCompositeExtract %v3float %447 1
%452 = OpCompositeExtract %v3float %442 1
%453 = OpFAdd %v3float %451 %452
%454 = OpCompositeExtract %v3float %447 2
%455 = OpCompositeExtract %v3float %442 2
%456 = OpFAdd %v3float %454 %455
%457 = OpCompositeConstruct %mat3v3float %450 %453 %456
%458 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%459 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%460 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%461 = OpCompositeConstruct %mat3v3float %458 %459 %460
%462 = OpCompositeExtract %v3float %457 0
%463 = OpCompositeExtract %v3float %461 0
%464 = OpFOrdEqual %v3bool %462 %463
%465 = OpAll %bool %464
%466 = OpCompositeExtract %v3float %457 1
%467 = OpCompositeExtract %v3float %461 1
%468 = OpFOrdEqual %v3bool %466 %467
%469 = OpAll %bool %468
%470 = OpLogicalAnd %bool %465 %469
%471 = OpCompositeExtract %v3float %457 2
%472 = OpCompositeExtract %v3float %461 2
%473 = OpFOrdEqual %v3bool %471 %472
%474 = OpAll %bool %473
%475 = OpLogicalAnd %bool %470 %474
OpBranch %441
%441 = OpLabel
%476 = OpPhi %bool %false %414 %475 %440
OpStore %_0_ok %476
%477 = OpLoad %bool %_0_ok
OpSelectionMerge %479 None
OpBranchConditional %477 %478 %479
%478 = OpLabel
%481 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%482 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%483 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%480 = OpCompositeConstruct %mat3v3float %481 %482 %483
%484 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%485 = OpCompositeConstruct %mat3v3float %484 %484 %484
%486 = OpCompositeExtract %v3float %485 0
%487 = OpCompositeExtract %v3float %480 0
%488 = OpFSub %v3float %486 %487
%489 = OpCompositeExtract %v3float %485 1
%490 = OpCompositeExtract %v3float %480 1
%491 = OpFSub %v3float %489 %490
%492 = OpCompositeExtract %v3float %485 2
%493 = OpCompositeExtract %v3float %480 2
%494 = OpFSub %v3float %492 %493
%495 = OpCompositeConstruct %mat3v3float %488 %491 %494
%496 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%497 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%498 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%499 = OpCompositeConstruct %mat3v3float %496 %497 %498
%500 = OpCompositeExtract %v3float %495 0
%501 = OpCompositeExtract %v3float %499 0
%502 = OpFOrdEqual %v3bool %500 %501
%503 = OpAll %bool %502
%504 = OpCompositeExtract %v3float %495 1
%505 = OpCompositeExtract %v3float %499 1
%506 = OpFOrdEqual %v3bool %504 %505
%507 = OpAll %bool %506
%508 = OpLogicalAnd %bool %503 %507
%509 = OpCompositeExtract %v3float %495 2
%510 = OpCompositeExtract %v3float %499 2
%511 = OpFOrdEqual %v3bool %509 %510
%512 = OpAll %bool %511
%513 = OpLogicalAnd %bool %508 %512
OpBranch %479
%479 = OpLabel
%514 = OpPhi %bool %false %441 %513 %478
OpStore %_0_ok %514
%515 = OpLoad %bool %_0_ok
OpSelectionMerge %517 None
OpBranchConditional %515 %516 %517
%516 = OpLabel
%519 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%520 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%521 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%518 = OpCompositeConstruct %mat3v3float %519 %520 %521
%522 = OpMatrixTimesScalar %mat3v3float %518 %float_4
%524 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%525 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%526 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%523 = OpCompositeConstruct %mat3v3float %524 %525 %526
%527 = OpCompositeExtract %v3float %522 0
%528 = OpCompositeExtract %v3float %523 0
%529 = OpFOrdEqual %v3bool %527 %528
%530 = OpAll %bool %529
%531 = OpCompositeExtract %v3float %522 1
%532 = OpCompositeExtract %v3float %523 1
%533 = OpFOrdEqual %v3bool %531 %532
%534 = OpAll %bool %533
%535 = OpLogicalAnd %bool %530 %534
%536 = OpCompositeExtract %v3float %522 2
%537 = OpCompositeExtract %v3float %523 2
%538 = OpFOrdEqual %v3bool %536 %537
%539 = OpAll %bool %538
%540 = OpLogicalAnd %bool %535 %539
OpBranch %517
%517 = OpLabel
%541 = OpPhi %bool %false %479 %540 %516
OpStore %_0_ok %541
%542 = OpLoad %bool %_0_ok
OpSelectionMerge %544 None
OpBranchConditional %542 %543 %544
%543 = OpLabel
%545 = OpCompositeConstruct %v2float %float_2 %float_2
%546 = OpCompositeConstruct %v2float %float_2 %float_2
%547 = OpCompositeConstruct %mat2v2float %545 %546
%548 = OpCompositeConstruct %v2float %float_4 %float_4
%549 = OpCompositeConstruct %mat2v2float %548 %548
%550 = OpCompositeExtract %v2float %549 0
%551 = OpCompositeExtract %v2float %547 0
%552 = OpFDiv %v2float %550 %551
%553 = OpCompositeExtract %v2float %549 1
%554 = OpCompositeExtract %v2float %547 1
%555 = OpFDiv %v2float %553 %554
%556 = OpCompositeConstruct %mat2v2float %552 %555
%557 = OpCompositeConstruct %v2float %float_2 %float_2
%558 = OpCompositeConstruct %v2float %float_2 %float_2
%559 = OpCompositeConstruct %mat2v2float %557 %558
%560 = OpCompositeExtract %v2float %556 0
%561 = OpCompositeExtract %v2float %559 0
%562 = OpFOrdEqual %v2bool %560 %561
%563 = OpAll %bool %562
%564 = OpCompositeExtract %v2float %556 1
%565 = OpCompositeExtract %v2float %559 1
%566 = OpFOrdEqual %v2bool %564 %565
%567 = OpAll %bool %566
%568 = OpLogicalAnd %bool %563 %567
OpBranch %544
%544 = OpLabel
%569 = OpPhi %bool %false %517 %568 %543
OpStore %_0_ok %569
%570 = OpLoad %bool %_0_ok
OpSelectionMerge %572 None
OpBranchConditional %570 %571 %572
%571 = OpLabel
%573 = OpFunctionCall %bool %test_half_b
OpBranch %572
%572 = OpLabel
%574 = OpPhi %bool %false %544 %573 %571
OpSelectionMerge %579 None
OpBranchConditional %574 %577 %578
%577 = OpLabel
%580 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%584 = OpLoad %v4float %580
OpStore %575 %584
OpBranch %579
%578 = OpLabel
%585 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%587 = OpLoad %v4float %585
OpStore %575 %587
OpBranch %579
%579 = OpLabel
%588 = OpLoad %v4float %575
OpReturnValue %588
OpFunctionEnd

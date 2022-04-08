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
OpDecorate %86 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
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
OpDecorate %265 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
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
%49 = OpConstantComposite %v2float %float_n1_25 %float_0
%50 = OpConstantComposite %v2float %float_0_75 %float_2_25
%v2bool = OpTypeVector %bool 2
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%100 = OpConstantComposite %v2float %float_0 %float_1
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
%266 = OpVariable %_ptr_Function_v4float Function
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
%75 = OpCompositeConstruct %mat2v2float %49 %50
%76 = OpCompositeExtract %v2float %74 0
%77 = OpCompositeExtract %v2float %75 0
%78 = OpFOrdEqual %v2bool %76 %77
%79 = OpAll %bool %78
%80 = OpCompositeExtract %v2float %74 1
%81 = OpCompositeExtract %v2float %75 1
%82 = OpFOrdEqual %v2bool %80 %81
%83 = OpAll %bool %82
%84 = OpLogicalAnd %bool %79 %83
OpBranch %65
%65 = OpLabel
%85 = OpPhi %bool %false %32 %84 %64
OpStore %ok %85
%86 = OpLoad %bool %ok
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%91 = OpLoad %v4float %89
%92 = OpCompositeExtract %float %91 0
%93 = OpCompositeExtract %float %91 1
%94 = OpCompositeExtract %float %91 2
%95 = OpCompositeExtract %float %91 3
%96 = OpCompositeConstruct %v2float %92 %93
%97 = OpCompositeConstruct %v2float %94 %95
%98 = OpCompositeConstruct %mat2v2float %96 %97
%101 = OpCompositeConstruct %mat2v2float %100 %100
%102 = OpCompositeExtract %v2float %98 0
%103 = OpCompositeExtract %v2float %101 0
%104 = OpFOrdEqual %v2bool %102 %103
%105 = OpAll %bool %104
%106 = OpCompositeExtract %v2float %98 1
%107 = OpCompositeExtract %v2float %101 1
%108 = OpFOrdEqual %v2bool %106 %107
%109 = OpAll %bool %108
%110 = OpLogicalAnd %bool %105 %109
OpBranch %88
%88 = OpLabel
%111 = OpPhi %bool %false %65 %110 %87
OpStore %ok %111
%112 = OpLoad %bool %ok
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%116 = OpLoad %v4float %115
%117 = OpCompositeExtract %float %116 0
%118 = OpCompositeExtract %float %116 1
%119 = OpCompositeExtract %float %116 2
%120 = OpCompositeExtract %float %116 3
%121 = OpCompositeConstruct %v2float %117 %118
%122 = OpCompositeConstruct %v2float %119 %120
%123 = OpCompositeConstruct %mat2v2float %121 %122
%124 = OpCompositeConstruct %mat2v2float %100 %100
%125 = OpCompositeExtract %v2float %123 0
%126 = OpCompositeExtract %v2float %124 0
%127 = OpFOrdEqual %v2bool %125 %126
%128 = OpAll %bool %127
%129 = OpCompositeExtract %v2float %123 1
%130 = OpCompositeExtract %v2float %124 1
%131 = OpFOrdEqual %v2bool %129 %130
%132 = OpAll %bool %131
%133 = OpLogicalAnd %bool %128 %132
OpBranch %114
%114 = OpLabel
%134 = OpPhi %bool %false %88 %133 %113
OpStore %ok %134
%135 = OpLoad %bool %ok
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%139 = OpLoad %v4float %138
%140 = OpCompositeExtract %float %139 0
%141 = OpConvertFToS %int %140
%142 = OpCompositeExtract %float %139 1
%143 = OpConvertFToS %int %142
%144 = OpCompositeExtract %float %139 2
%145 = OpConvertFToS %int %144
%146 = OpCompositeExtract %float %139 3
%147 = OpConvertFToS %int %146
%148 = OpCompositeConstruct %v4int %141 %143 %145 %147
%150 = OpCompositeExtract %int %148 0
%151 = OpConvertSToF %float %150
%152 = OpCompositeExtract %int %148 1
%153 = OpConvertSToF %float %152
%154 = OpCompositeExtract %int %148 2
%155 = OpConvertSToF %float %154
%156 = OpCompositeExtract %int %148 3
%157 = OpConvertSToF %float %156
%158 = OpCompositeConstruct %v4float %151 %153 %155 %157
%159 = OpCompositeExtract %float %158 0
%160 = OpCompositeExtract %float %158 1
%161 = OpCompositeExtract %float %158 2
%162 = OpCompositeExtract %float %158 3
%163 = OpCompositeConstruct %v2float %159 %160
%164 = OpCompositeConstruct %v2float %161 %162
%165 = OpCompositeConstruct %mat2v2float %163 %164
%166 = OpCompositeConstruct %mat2v2float %100 %100
%167 = OpCompositeExtract %v2float %165 0
%168 = OpCompositeExtract %v2float %166 0
%169 = OpFOrdEqual %v2bool %167 %168
%170 = OpAll %bool %169
%171 = OpCompositeExtract %v2float %165 1
%172 = OpCompositeExtract %v2float %166 1
%173 = OpFOrdEqual %v2bool %171 %172
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %170 %174
OpBranch %137
%137 = OpLabel
%176 = OpPhi %bool %false %114 %175 %136
OpStore %ok %176
%177 = OpLoad %bool %ok
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%181 = OpLoad %v4float %180
%182 = OpCompositeExtract %float %181 0
%183 = OpCompositeExtract %float %181 1
%184 = OpCompositeExtract %float %181 2
%185 = OpCompositeExtract %float %181 3
%186 = OpCompositeConstruct %v2float %182 %183
%187 = OpCompositeConstruct %v2float %184 %185
%188 = OpCompositeConstruct %mat2v2float %186 %187
%189 = OpCompositeConstruct %mat2v2float %100 %100
%190 = OpCompositeExtract %v2float %188 0
%191 = OpCompositeExtract %v2float %189 0
%192 = OpFOrdEqual %v2bool %190 %191
%193 = OpAll %bool %192
%194 = OpCompositeExtract %v2float %188 1
%195 = OpCompositeExtract %v2float %189 1
%196 = OpFOrdEqual %v2bool %194 %195
%197 = OpAll %bool %196
%198 = OpLogicalAnd %bool %193 %197
OpBranch %179
%179 = OpLabel
%199 = OpPhi %bool %false %137 %198 %178
OpStore %ok %199
%200 = OpLoad %bool %ok
OpSelectionMerge %202 None
OpBranchConditional %200 %201 %202
%201 = OpLabel
%203 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%204 = OpLoad %v4float %203
%205 = OpCompositeExtract %float %204 0
%206 = OpCompositeExtract %float %204 1
%207 = OpCompositeExtract %float %204 2
%208 = OpCompositeExtract %float %204 3
%209 = OpCompositeConstruct %v2float %205 %206
%210 = OpCompositeConstruct %v2float %207 %208
%211 = OpCompositeConstruct %mat2v2float %209 %210
%212 = OpCompositeConstruct %mat2v2float %100 %100
%213 = OpCompositeExtract %v2float %211 0
%214 = OpCompositeExtract %v2float %212 0
%215 = OpFOrdEqual %v2bool %213 %214
%216 = OpAll %bool %215
%217 = OpCompositeExtract %v2float %211 1
%218 = OpCompositeExtract %v2float %212 1
%219 = OpFOrdEqual %v2bool %217 %218
%220 = OpAll %bool %219
%221 = OpLogicalAnd %bool %216 %220
OpBranch %202
%202 = OpLabel
%222 = OpPhi %bool %false %179 %221 %201
OpStore %ok %222
%223 = OpLoad %bool %ok
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
%226 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%227 = OpLoad %v4float %226
%228 = OpCompositeExtract %float %227 0
%229 = OpFUnordNotEqual %bool %228 %float_0
%230 = OpCompositeExtract %float %227 1
%231 = OpFUnordNotEqual %bool %230 %float_0
%232 = OpCompositeExtract %float %227 2
%233 = OpFUnordNotEqual %bool %232 %float_0
%234 = OpCompositeExtract %float %227 3
%235 = OpFUnordNotEqual %bool %234 %float_0
%236 = OpCompositeConstruct %v4bool %229 %231 %233 %235
%238 = OpCompositeExtract %bool %236 0
%239 = OpSelect %float %238 %float_1 %float_0
%240 = OpCompositeExtract %bool %236 1
%241 = OpSelect %float %240 %float_1 %float_0
%242 = OpCompositeExtract %bool %236 2
%243 = OpSelect %float %242 %float_1 %float_0
%244 = OpCompositeExtract %bool %236 3
%245 = OpSelect %float %244 %float_1 %float_0
%246 = OpCompositeConstruct %v4float %239 %241 %243 %245
%247 = OpCompositeExtract %float %246 0
%248 = OpCompositeExtract %float %246 1
%249 = OpCompositeExtract %float %246 2
%250 = OpCompositeExtract %float %246 3
%251 = OpCompositeConstruct %v2float %247 %248
%252 = OpCompositeConstruct %v2float %249 %250
%253 = OpCompositeConstruct %mat2v2float %251 %252
%254 = OpCompositeConstruct %mat2v2float %100 %100
%255 = OpCompositeExtract %v2float %253 0
%256 = OpCompositeExtract %v2float %254 0
%257 = OpFOrdEqual %v2bool %255 %256
%258 = OpAll %bool %257
%259 = OpCompositeExtract %v2float %253 1
%260 = OpCompositeExtract %v2float %254 1
%261 = OpFOrdEqual %v2bool %259 %260
%262 = OpAll %bool %261
%263 = OpLogicalAnd %bool %258 %262
OpBranch %225
%225 = OpLabel
%264 = OpPhi %bool %false %202 %263 %224
OpStore %ok %264
%265 = OpLoad %bool %ok
OpSelectionMerge %270 None
OpBranchConditional %265 %268 %269
%268 = OpLabel
%271 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%272 = OpLoad %v4float %271
OpStore %266 %272
OpBranch %270
%269 = OpLabel
%273 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%275 = OpLoad %v4float %273
OpStore %266 %275
OpBranch %270
%270 = OpLabel
%276 = OpLoad %v4float %266
OpReturnValue %276
OpFunctionEnd

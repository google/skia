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
OpDecorate %45 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
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
%51 = OpConstantComposite %mat2v2float %49 %50
%v2bool = OpTypeVector %bool 2
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%99 = OpConstantComposite %v2float %float_0 %float_1
%100 = OpConstantComposite %mat2v2float %99 %99
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
%260 = OpVariable %_ptr_Function_v4float Function
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
%45 = OpCompositeConstruct %mat2v2float %42 %43
%53 = OpCompositeExtract %v2float %45 0
%54 = OpCompositeExtract %v2float %51 0
%55 = OpFOrdEqual %v2bool %53 %54
%56 = OpAll %bool %55
%57 = OpCompositeExtract %v2float %45 1
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
%75 = OpCompositeExtract %v2float %74 0
%76 = OpCompositeExtract %v2float %51 0
%77 = OpFOrdEqual %v2bool %75 %76
%78 = OpAll %bool %77
%79 = OpCompositeExtract %v2float %74 1
%80 = OpCompositeExtract %v2float %51 1
%81 = OpFOrdEqual %v2bool %79 %80
%82 = OpAll %bool %81
%83 = OpLogicalAnd %bool %78 %82
OpBranch %65
%65 = OpLabel
%84 = OpPhi %bool %false %32 %83 %64
OpStore %ok %84
%85 = OpLoad %bool %ok
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%90 = OpLoad %v4float %88
%91 = OpCompositeExtract %float %90 0
%92 = OpCompositeExtract %float %90 1
%93 = OpCompositeExtract %float %90 2
%94 = OpCompositeExtract %float %90 3
%95 = OpCompositeConstruct %v2float %91 %92
%96 = OpCompositeConstruct %v2float %93 %94
%97 = OpCompositeConstruct %mat2v2float %95 %96
%101 = OpCompositeExtract %v2float %97 0
%102 = OpCompositeExtract %v2float %100 0
%103 = OpFOrdEqual %v2bool %101 %102
%104 = OpAll %bool %103
%105 = OpCompositeExtract %v2float %97 1
%106 = OpCompositeExtract %v2float %100 1
%107 = OpFOrdEqual %v2bool %105 %106
%108 = OpAll %bool %107
%109 = OpLogicalAnd %bool %104 %108
OpBranch %87
%87 = OpLabel
%110 = OpPhi %bool %false %65 %109 %86
OpStore %ok %110
%111 = OpLoad %bool %ok
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%115 = OpLoad %v4float %114
%116 = OpCompositeExtract %float %115 0
%117 = OpCompositeExtract %float %115 1
%118 = OpCompositeExtract %float %115 2
%119 = OpCompositeExtract %float %115 3
%120 = OpCompositeConstruct %v2float %116 %117
%121 = OpCompositeConstruct %v2float %118 %119
%122 = OpCompositeConstruct %mat2v2float %120 %121
%123 = OpCompositeExtract %v2float %122 0
%124 = OpCompositeExtract %v2float %100 0
%125 = OpFOrdEqual %v2bool %123 %124
%126 = OpAll %bool %125
%127 = OpCompositeExtract %v2float %122 1
%128 = OpCompositeExtract %v2float %100 1
%129 = OpFOrdEqual %v2bool %127 %128
%130 = OpAll %bool %129
%131 = OpLogicalAnd %bool %126 %130
OpBranch %113
%113 = OpLabel
%132 = OpPhi %bool %false %87 %131 %112
OpStore %ok %132
%133 = OpLoad %bool %ok
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%137 = OpLoad %v4float %136
%138 = OpCompositeExtract %float %137 0
%139 = OpConvertFToS %int %138
%140 = OpCompositeExtract %float %137 1
%141 = OpConvertFToS %int %140
%142 = OpCompositeExtract %float %137 2
%143 = OpConvertFToS %int %142
%144 = OpCompositeExtract %float %137 3
%145 = OpConvertFToS %int %144
%147 = OpCompositeConstruct %v4int %139 %141 %143 %145
%148 = OpCompositeExtract %int %147 0
%149 = OpConvertSToF %float %148
%150 = OpCompositeExtract %int %147 1
%151 = OpConvertSToF %float %150
%152 = OpCompositeExtract %int %147 2
%153 = OpConvertSToF %float %152
%154 = OpCompositeExtract %int %147 3
%155 = OpConvertSToF %float %154
%156 = OpCompositeConstruct %v4float %149 %151 %153 %155
%157 = OpCompositeExtract %float %156 0
%158 = OpCompositeExtract %float %156 1
%159 = OpCompositeExtract %float %156 2
%160 = OpCompositeExtract %float %156 3
%161 = OpCompositeConstruct %v2float %157 %158
%162 = OpCompositeConstruct %v2float %159 %160
%163 = OpCompositeConstruct %mat2v2float %161 %162
%164 = OpCompositeExtract %v2float %163 0
%165 = OpCompositeExtract %v2float %100 0
%166 = OpFOrdEqual %v2bool %164 %165
%167 = OpAll %bool %166
%168 = OpCompositeExtract %v2float %163 1
%169 = OpCompositeExtract %v2float %100 1
%170 = OpFOrdEqual %v2bool %168 %169
%171 = OpAll %bool %170
%172 = OpLogicalAnd %bool %167 %171
OpBranch %135
%135 = OpLabel
%173 = OpPhi %bool %false %113 %172 %134
OpStore %ok %173
%174 = OpLoad %bool %ok
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%178 = OpLoad %v4float %177
%179 = OpCompositeExtract %float %178 0
%180 = OpCompositeExtract %float %178 1
%181 = OpCompositeExtract %float %178 2
%182 = OpCompositeExtract %float %178 3
%183 = OpCompositeConstruct %v2float %179 %180
%184 = OpCompositeConstruct %v2float %181 %182
%185 = OpCompositeConstruct %mat2v2float %183 %184
%186 = OpCompositeExtract %v2float %185 0
%187 = OpCompositeExtract %v2float %100 0
%188 = OpFOrdEqual %v2bool %186 %187
%189 = OpAll %bool %188
%190 = OpCompositeExtract %v2float %185 1
%191 = OpCompositeExtract %v2float %100 1
%192 = OpFOrdEqual %v2bool %190 %191
%193 = OpAll %bool %192
%194 = OpLogicalAnd %bool %189 %193
OpBranch %176
%176 = OpLabel
%195 = OpPhi %bool %false %135 %194 %175
OpStore %ok %195
%196 = OpLoad %bool %ok
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%199 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%200 = OpLoad %v4float %199
%201 = OpCompositeExtract %float %200 0
%202 = OpCompositeExtract %float %200 1
%203 = OpCompositeExtract %float %200 2
%204 = OpCompositeExtract %float %200 3
%205 = OpCompositeConstruct %v2float %201 %202
%206 = OpCompositeConstruct %v2float %203 %204
%207 = OpCompositeConstruct %mat2v2float %205 %206
%208 = OpCompositeExtract %v2float %207 0
%209 = OpCompositeExtract %v2float %100 0
%210 = OpFOrdEqual %v2bool %208 %209
%211 = OpAll %bool %210
%212 = OpCompositeExtract %v2float %207 1
%213 = OpCompositeExtract %v2float %100 1
%214 = OpFOrdEqual %v2bool %212 %213
%215 = OpAll %bool %214
%216 = OpLogicalAnd %bool %211 %215
OpBranch %198
%198 = OpLabel
%217 = OpPhi %bool %false %176 %216 %197
OpStore %ok %217
%218 = OpLoad %bool %ok
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%222 = OpLoad %v4float %221
%223 = OpCompositeExtract %float %222 0
%224 = OpFUnordNotEqual %bool %223 %float_0
%225 = OpCompositeExtract %float %222 1
%226 = OpFUnordNotEqual %bool %225 %float_0
%227 = OpCompositeExtract %float %222 2
%228 = OpFUnordNotEqual %bool %227 %float_0
%229 = OpCompositeExtract %float %222 3
%230 = OpFUnordNotEqual %bool %229 %float_0
%232 = OpCompositeConstruct %v4bool %224 %226 %228 %230
%233 = OpCompositeExtract %bool %232 0
%234 = OpSelect %float %233 %float_1 %float_0
%235 = OpCompositeExtract %bool %232 1
%236 = OpSelect %float %235 %float_1 %float_0
%237 = OpCompositeExtract %bool %232 2
%238 = OpSelect %float %237 %float_1 %float_0
%239 = OpCompositeExtract %bool %232 3
%240 = OpSelect %float %239 %float_1 %float_0
%241 = OpCompositeConstruct %v4float %234 %236 %238 %240
%242 = OpCompositeExtract %float %241 0
%243 = OpCompositeExtract %float %241 1
%244 = OpCompositeExtract %float %241 2
%245 = OpCompositeExtract %float %241 3
%246 = OpCompositeConstruct %v2float %242 %243
%247 = OpCompositeConstruct %v2float %244 %245
%248 = OpCompositeConstruct %mat2v2float %246 %247
%249 = OpCompositeExtract %v2float %248 0
%250 = OpCompositeExtract %v2float %100 0
%251 = OpFOrdEqual %v2bool %249 %250
%252 = OpAll %bool %251
%253 = OpCompositeExtract %v2float %248 1
%254 = OpCompositeExtract %v2float %100 1
%255 = OpFOrdEqual %v2bool %253 %254
%256 = OpAll %bool %255
%257 = OpLogicalAnd %bool %252 %256
OpBranch %220
%220 = OpLabel
%258 = OpPhi %bool %false %198 %257 %219
OpStore %ok %258
%259 = OpLoad %bool %ok
OpSelectionMerge %264 None
OpBranchConditional %259 %262 %263
%262 = OpLabel
%265 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%266 = OpLoad %v4float %265
OpStore %260 %266
OpBranch %264
%263 = OpLabel
%267 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%269 = OpLoad %v4float %267
OpStore %260 %269
OpBranch %264
%264 = OpLabel
%270 = OpLoad %v4float %260
OpReturnValue %270
OpFunctionEnd

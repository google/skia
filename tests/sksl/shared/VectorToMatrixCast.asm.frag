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
OpDecorate %61 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
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
%95 = OpConstantComposite %v2float %float_0 %float_1
%96 = OpConstantComposite %mat2v2float %95 %95
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
%244 = OpVariable %_ptr_Function_v4float Function
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
%54 = OpFOrdEqual %v2bool %53 %49
%55 = OpAll %bool %54
%56 = OpCompositeExtract %v2float %45 1
%57 = OpFOrdEqual %v2bool %56 %50
%58 = OpAll %bool %57
%59 = OpLogicalAnd %bool %55 %58
OpBranch %32
%32 = OpLabel
%60 = OpPhi %bool %false %25 %59 %31
OpStore %ok %60
%61 = OpLoad %bool %ok
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%65 = OpLoad %v4float %64
%66 = OpCompositeExtract %float %65 0
%67 = OpCompositeExtract %float %65 1
%68 = OpCompositeExtract %float %65 2
%69 = OpCompositeExtract %float %65 3
%70 = OpCompositeConstruct %v2float %66 %67
%71 = OpCompositeConstruct %v2float %68 %69
%72 = OpCompositeConstruct %mat2v2float %70 %71
%73 = OpCompositeExtract %v2float %72 0
%74 = OpFOrdEqual %v2bool %73 %49
%75 = OpAll %bool %74
%76 = OpCompositeExtract %v2float %72 1
%77 = OpFOrdEqual %v2bool %76 %50
%78 = OpAll %bool %77
%79 = OpLogicalAnd %bool %75 %78
OpBranch %63
%63 = OpLabel
%80 = OpPhi %bool %false %32 %79 %62
OpStore %ok %80
%81 = OpLoad %bool %ok
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%86 = OpLoad %v4float %84
%87 = OpCompositeExtract %float %86 0
%88 = OpCompositeExtract %float %86 1
%89 = OpCompositeExtract %float %86 2
%90 = OpCompositeExtract %float %86 3
%91 = OpCompositeConstruct %v2float %87 %88
%92 = OpCompositeConstruct %v2float %89 %90
%93 = OpCompositeConstruct %mat2v2float %91 %92
%97 = OpCompositeExtract %v2float %93 0
%98 = OpFOrdEqual %v2bool %97 %95
%99 = OpAll %bool %98
%100 = OpCompositeExtract %v2float %93 1
%101 = OpFOrdEqual %v2bool %100 %95
%102 = OpAll %bool %101
%103 = OpLogicalAnd %bool %99 %102
OpBranch %83
%83 = OpLabel
%104 = OpPhi %bool %false %63 %103 %82
OpStore %ok %104
%105 = OpLoad %bool %ok
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%109 = OpLoad %v4float %108
%110 = OpCompositeExtract %float %109 0
%111 = OpCompositeExtract %float %109 1
%112 = OpCompositeExtract %float %109 2
%113 = OpCompositeExtract %float %109 3
%114 = OpCompositeConstruct %v2float %110 %111
%115 = OpCompositeConstruct %v2float %112 %113
%116 = OpCompositeConstruct %mat2v2float %114 %115
%117 = OpCompositeExtract %v2float %116 0
%118 = OpFOrdEqual %v2bool %117 %95
%119 = OpAll %bool %118
%120 = OpCompositeExtract %v2float %116 1
%121 = OpFOrdEqual %v2bool %120 %95
%122 = OpAll %bool %121
%123 = OpLogicalAnd %bool %119 %122
OpBranch %107
%107 = OpLabel
%124 = OpPhi %bool %false %83 %123 %106
OpStore %ok %124
%125 = OpLoad %bool %ok
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%129 = OpLoad %v4float %128
%130 = OpCompositeExtract %float %129 0
%131 = OpConvertFToS %int %130
%132 = OpCompositeExtract %float %129 1
%133 = OpConvertFToS %int %132
%134 = OpCompositeExtract %float %129 2
%135 = OpConvertFToS %int %134
%136 = OpCompositeExtract %float %129 3
%137 = OpConvertFToS %int %136
%139 = OpCompositeConstruct %v4int %131 %133 %135 %137
%140 = OpCompositeExtract %int %139 0
%141 = OpConvertSToF %float %140
%142 = OpCompositeExtract %int %139 1
%143 = OpConvertSToF %float %142
%144 = OpCompositeExtract %int %139 2
%145 = OpConvertSToF %float %144
%146 = OpCompositeExtract %int %139 3
%147 = OpConvertSToF %float %146
%148 = OpCompositeConstruct %v4float %141 %143 %145 %147
%149 = OpCompositeExtract %float %148 0
%150 = OpCompositeExtract %float %148 1
%151 = OpCompositeExtract %float %148 2
%152 = OpCompositeExtract %float %148 3
%153 = OpCompositeConstruct %v2float %149 %150
%154 = OpCompositeConstruct %v2float %151 %152
%155 = OpCompositeConstruct %mat2v2float %153 %154
%156 = OpCompositeExtract %v2float %155 0
%157 = OpFOrdEqual %v2bool %156 %95
%158 = OpAll %bool %157
%159 = OpCompositeExtract %v2float %155 1
%160 = OpFOrdEqual %v2bool %159 %95
%161 = OpAll %bool %160
%162 = OpLogicalAnd %bool %158 %161
OpBranch %127
%127 = OpLabel
%163 = OpPhi %bool %false %107 %162 %126
OpStore %ok %163
%164 = OpLoad %bool %ok
OpSelectionMerge %166 None
OpBranchConditional %164 %165 %166
%165 = OpLabel
%167 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%168 = OpLoad %v4float %167
%169 = OpCompositeExtract %float %168 0
%170 = OpCompositeExtract %float %168 1
%171 = OpCompositeExtract %float %168 2
%172 = OpCompositeExtract %float %168 3
%173 = OpCompositeConstruct %v2float %169 %170
%174 = OpCompositeConstruct %v2float %171 %172
%175 = OpCompositeConstruct %mat2v2float %173 %174
%176 = OpCompositeExtract %v2float %175 0
%177 = OpFOrdEqual %v2bool %176 %95
%178 = OpAll %bool %177
%179 = OpCompositeExtract %v2float %175 1
%180 = OpFOrdEqual %v2bool %179 %95
%181 = OpAll %bool %180
%182 = OpLogicalAnd %bool %178 %181
OpBranch %166
%166 = OpLabel
%183 = OpPhi %bool %false %127 %182 %165
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
%196 = OpCompositeExtract %v2float %195 0
%197 = OpFOrdEqual %v2bool %196 %95
%198 = OpAll %bool %197
%199 = OpCompositeExtract %v2float %195 1
%200 = OpFOrdEqual %v2bool %199 %95
%201 = OpAll %bool %200
%202 = OpLogicalAnd %bool %198 %201
OpBranch %186
%186 = OpLabel
%203 = OpPhi %bool %false %166 %202 %185
OpStore %ok %203
%204 = OpLoad %bool %ok
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%208 = OpLoad %v4float %207
%209 = OpCompositeExtract %float %208 0
%210 = OpFUnordNotEqual %bool %209 %float_0
%211 = OpCompositeExtract %float %208 1
%212 = OpFUnordNotEqual %bool %211 %float_0
%213 = OpCompositeExtract %float %208 2
%214 = OpFUnordNotEqual %bool %213 %float_0
%215 = OpCompositeExtract %float %208 3
%216 = OpFUnordNotEqual %bool %215 %float_0
%218 = OpCompositeConstruct %v4bool %210 %212 %214 %216
%219 = OpCompositeExtract %bool %218 0
%220 = OpSelect %float %219 %float_1 %float_0
%221 = OpCompositeExtract %bool %218 1
%222 = OpSelect %float %221 %float_1 %float_0
%223 = OpCompositeExtract %bool %218 2
%224 = OpSelect %float %223 %float_1 %float_0
%225 = OpCompositeExtract %bool %218 3
%226 = OpSelect %float %225 %float_1 %float_0
%227 = OpCompositeConstruct %v4float %220 %222 %224 %226
%228 = OpCompositeExtract %float %227 0
%229 = OpCompositeExtract %float %227 1
%230 = OpCompositeExtract %float %227 2
%231 = OpCompositeExtract %float %227 3
%232 = OpCompositeConstruct %v2float %228 %229
%233 = OpCompositeConstruct %v2float %230 %231
%234 = OpCompositeConstruct %mat2v2float %232 %233
%235 = OpCompositeExtract %v2float %234 0
%236 = OpFOrdEqual %v2bool %235 %95
%237 = OpAll %bool %236
%238 = OpCompositeExtract %v2float %234 1
%239 = OpFOrdEqual %v2bool %238 %95
%240 = OpAll %bool %239
%241 = OpLogicalAnd %bool %237 %240
OpBranch %206
%206 = OpLabel
%242 = OpPhi %bool %false %186 %241 %205
OpStore %ok %242
%243 = OpLoad %bool %ok
OpSelectionMerge %248 None
OpBranchConditional %243 %246 %247
%246 = OpLabel
%249 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%250 = OpLoad %v4float %249
OpStore %244 %250
OpBranch %248
%247 = OpLabel
%251 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%253 = OpLoad %v4float %251
OpStore %244 %253
OpBranch %248
%248 = OpLabel
%254 = OpLoad %v4float %244
OpReturnValue %254
OpFunctionEnd

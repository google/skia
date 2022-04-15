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
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
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
OpDecorate %223 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
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
%91 = OpConstantComposite %v2float %float_0 %float_1
%92 = OpConstantComposite %mat2v2float %91 %91
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
%228 = OpVariable %_ptr_Function_v4float Function
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
%53 = OpFOrdEqual %v2bool %42 %49
%54 = OpAll %bool %53
%55 = OpFOrdEqual %v2bool %43 %50
%56 = OpAll %bool %55
%57 = OpLogicalAnd %bool %54 %56
OpBranch %32
%32 = OpLabel
%58 = OpPhi %bool %false %25 %57 %31
OpStore %ok %58
%59 = OpLoad %bool %ok
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%63 = OpLoad %v4float %62
%64 = OpCompositeExtract %float %63 0
%65 = OpCompositeExtract %float %63 1
%66 = OpCompositeExtract %float %63 2
%67 = OpCompositeExtract %float %63 3
%68 = OpCompositeConstruct %v2float %64 %65
%69 = OpCompositeConstruct %v2float %66 %67
%70 = OpCompositeConstruct %mat2v2float %68 %69
%71 = OpFOrdEqual %v2bool %68 %49
%72 = OpAll %bool %71
%73 = OpFOrdEqual %v2bool %69 %50
%74 = OpAll %bool %73
%75 = OpLogicalAnd %bool %72 %74
OpBranch %61
%61 = OpLabel
%76 = OpPhi %bool %false %32 %75 %60
OpStore %ok %76
%77 = OpLoad %bool %ok
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%82 = OpLoad %v4float %80
%83 = OpCompositeExtract %float %82 0
%84 = OpCompositeExtract %float %82 1
%85 = OpCompositeExtract %float %82 2
%86 = OpCompositeExtract %float %82 3
%87 = OpCompositeConstruct %v2float %83 %84
%88 = OpCompositeConstruct %v2float %85 %86
%89 = OpCompositeConstruct %mat2v2float %87 %88
%93 = OpFOrdEqual %v2bool %87 %91
%94 = OpAll %bool %93
%95 = OpFOrdEqual %v2bool %88 %91
%96 = OpAll %bool %95
%97 = OpLogicalAnd %bool %94 %96
OpBranch %79
%79 = OpLabel
%98 = OpPhi %bool %false %61 %97 %78
OpStore %ok %98
%99 = OpLoad %bool %ok
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%103 = OpLoad %v4float %102
%104 = OpCompositeExtract %float %103 0
%105 = OpCompositeExtract %float %103 1
%106 = OpCompositeExtract %float %103 2
%107 = OpCompositeExtract %float %103 3
%108 = OpCompositeConstruct %v2float %104 %105
%109 = OpCompositeConstruct %v2float %106 %107
%110 = OpCompositeConstruct %mat2v2float %108 %109
%111 = OpFOrdEqual %v2bool %108 %91
%112 = OpAll %bool %111
%113 = OpFOrdEqual %v2bool %109 %91
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %112 %114
OpBranch %101
%101 = OpLabel
%116 = OpPhi %bool %false %79 %115 %100
OpStore %ok %116
%117 = OpLoad %bool %ok
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%121 = OpLoad %v4float %120
%122 = OpCompositeExtract %float %121 0
%123 = OpConvertFToS %int %122
%124 = OpCompositeExtract %float %121 1
%125 = OpConvertFToS %int %124
%126 = OpCompositeExtract %float %121 2
%127 = OpConvertFToS %int %126
%128 = OpCompositeExtract %float %121 3
%129 = OpConvertFToS %int %128
%131 = OpCompositeConstruct %v4int %123 %125 %127 %129
%132 = OpCompositeExtract %int %131 0
%133 = OpConvertSToF %float %132
%134 = OpCompositeExtract %int %131 1
%135 = OpConvertSToF %float %134
%136 = OpCompositeExtract %int %131 2
%137 = OpConvertSToF %float %136
%138 = OpCompositeExtract %int %131 3
%139 = OpConvertSToF %float %138
%140 = OpCompositeConstruct %v4float %133 %135 %137 %139
%141 = OpCompositeExtract %float %140 0
%142 = OpCompositeExtract %float %140 1
%143 = OpCompositeExtract %float %140 2
%144 = OpCompositeExtract %float %140 3
%145 = OpCompositeConstruct %v2float %141 %142
%146 = OpCompositeConstruct %v2float %143 %144
%147 = OpCompositeConstruct %mat2v2float %145 %146
%148 = OpFOrdEqual %v2bool %145 %91
%149 = OpAll %bool %148
%150 = OpFOrdEqual %v2bool %146 %91
%151 = OpAll %bool %150
%152 = OpLogicalAnd %bool %149 %151
OpBranch %119
%119 = OpLabel
%153 = OpPhi %bool %false %101 %152 %118
OpStore %ok %153
%154 = OpLoad %bool %ok
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%158 = OpLoad %v4float %157
%159 = OpCompositeExtract %float %158 0
%160 = OpCompositeExtract %float %158 1
%161 = OpCompositeExtract %float %158 2
%162 = OpCompositeExtract %float %158 3
%163 = OpCompositeConstruct %v2float %159 %160
%164 = OpCompositeConstruct %v2float %161 %162
%165 = OpCompositeConstruct %mat2v2float %163 %164
%166 = OpFOrdEqual %v2bool %163 %91
%167 = OpAll %bool %166
%168 = OpFOrdEqual %v2bool %164 %91
%169 = OpAll %bool %168
%170 = OpLogicalAnd %bool %167 %169
OpBranch %156
%156 = OpLabel
%171 = OpPhi %bool %false %119 %170 %155
OpStore %ok %171
%172 = OpLoad %bool %ok
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%175 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%176 = OpLoad %v4float %175
%177 = OpCompositeExtract %float %176 0
%178 = OpCompositeExtract %float %176 1
%179 = OpCompositeExtract %float %176 2
%180 = OpCompositeExtract %float %176 3
%181 = OpCompositeConstruct %v2float %177 %178
%182 = OpCompositeConstruct %v2float %179 %180
%183 = OpCompositeConstruct %mat2v2float %181 %182
%184 = OpFOrdEqual %v2bool %181 %91
%185 = OpAll %bool %184
%186 = OpFOrdEqual %v2bool %182 %91
%187 = OpAll %bool %186
%188 = OpLogicalAnd %bool %185 %187
OpBranch %174
%174 = OpLabel
%189 = OpPhi %bool %false %156 %188 %173
OpStore %ok %189
%190 = OpLoad %bool %ok
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%194 = OpLoad %v4float %193
%195 = OpCompositeExtract %float %194 0
%196 = OpFUnordNotEqual %bool %195 %float_0
%197 = OpCompositeExtract %float %194 1
%198 = OpFUnordNotEqual %bool %197 %float_0
%199 = OpCompositeExtract %float %194 2
%200 = OpFUnordNotEqual %bool %199 %float_0
%201 = OpCompositeExtract %float %194 3
%202 = OpFUnordNotEqual %bool %201 %float_0
%204 = OpCompositeConstruct %v4bool %196 %198 %200 %202
%205 = OpCompositeExtract %bool %204 0
%206 = OpSelect %float %205 %float_1 %float_0
%207 = OpCompositeExtract %bool %204 1
%208 = OpSelect %float %207 %float_1 %float_0
%209 = OpCompositeExtract %bool %204 2
%210 = OpSelect %float %209 %float_1 %float_0
%211 = OpCompositeExtract %bool %204 3
%212 = OpSelect %float %211 %float_1 %float_0
%213 = OpCompositeConstruct %v4float %206 %208 %210 %212
%214 = OpCompositeExtract %float %213 0
%215 = OpCompositeExtract %float %213 1
%216 = OpCompositeExtract %float %213 2
%217 = OpCompositeExtract %float %213 3
%218 = OpCompositeConstruct %v2float %214 %215
%219 = OpCompositeConstruct %v2float %216 %217
%220 = OpCompositeConstruct %mat2v2float %218 %219
%221 = OpFOrdEqual %v2bool %218 %91
%222 = OpAll %bool %221
%223 = OpFOrdEqual %v2bool %219 %91
%224 = OpAll %bool %223
%225 = OpLogicalAnd %bool %222 %224
OpBranch %192
%192 = OpLabel
%226 = OpPhi %bool %false %174 %225 %191
OpStore %ok %226
%227 = OpLoad %bool %ok
OpSelectionMerge %232 None
OpBranchConditional %227 %230 %231
%230 = OpLabel
%233 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%234 = OpLoad %v4float %233
OpStore %228 %234
OpBranch %232
%231 = OpLabel
%235 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%237 = OpLoad %v4float %235
OpStore %228 %237
OpBranch %232
%232 = OpLabel
%238 = OpLoad %v4float %228
OpReturnValue %238
OpFunctionEnd

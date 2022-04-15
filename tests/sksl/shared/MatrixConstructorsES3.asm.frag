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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f4 "f4"
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
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %69 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%59 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%60 = OpConstantComposite %v3float %float_4 %float_1 %float_2
%61 = OpConstantComposite %mat2v3float %59 %60
%v3bool = OpTypeVector %bool 3
%false = OpConstantFalse %bool
%mat2v4float = OpTypeMatrix %v4float 2
%89 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%90 = OpConstantComposite %mat2v4float %89 %89
%v4bool = OpTypeVector %bool 4
%mat3v3float = OpTypeMatrix %v3float 3
%121 = OpConstantComposite %v3float %float_3 %float_4 %float_1
%122 = OpConstantComposite %mat3v3float %59 %60 %121
%mat4v2float = OpTypeMatrix %v2float 4
%154 = OpConstantComposite %v2float %float_1 %float_2
%155 = OpConstantComposite %v2float %float_3 %float_4
%156 = OpConstantComposite %mat4v2float %154 %155 %154 %155
%v2bool = OpTypeVector %bool 2
%mat4v3float = OpTypeMatrix %v3float 4
%194 = OpConstantComposite %v3float %float_2 %float_3 %float_4
%195 = OpConstantComposite %mat4v3float %59 %60 %121 %194
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%f4 = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_bool Function
%209 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %f4 %38
%41 = OpLoad %v4float %f4
%42 = OpLoad %v4float %f4
%43 = OpVectorShuffle %v2float %42 %42 0 1
%44 = OpCompositeExtract %float %41 0
%45 = OpCompositeExtract %float %41 1
%46 = OpCompositeExtract %float %41 2
%48 = OpCompositeConstruct %v3float %44 %45 %46
%49 = OpCompositeExtract %float %41 3
%50 = OpCompositeExtract %float %43 0
%51 = OpCompositeExtract %float %43 1
%52 = OpCompositeConstruct %v3float %49 %50 %51
%54 = OpCompositeConstruct %mat2v3float %48 %52
%63 = OpFOrdEqual %v3bool %48 %59
%64 = OpAll %bool %63
%65 = OpFOrdEqual %v3bool %52 %60
%66 = OpAll %bool %65
%67 = OpLogicalAnd %bool %64 %66
OpStore %ok %67
%69 = OpLoad %bool %ok
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpLoad %v4float %f4
%73 = OpVectorShuffle %v3float %72 %72 0 1 2
%74 = OpLoad %v4float %f4
%75 = OpVectorShuffle %v4float %74 %74 3 0 1 2
%76 = OpLoad %v4float %f4
%77 = OpCompositeExtract %float %76 3
%78 = OpCompositeExtract %float %73 0
%79 = OpCompositeExtract %float %73 1
%80 = OpCompositeExtract %float %73 2
%81 = OpCompositeExtract %float %75 0
%82 = OpCompositeConstruct %v4float %78 %79 %80 %81
%83 = OpCompositeExtract %float %75 1
%84 = OpCompositeExtract %float %75 2
%85 = OpCompositeExtract %float %75 3
%86 = OpCompositeConstruct %v4float %83 %84 %85 %77
%88 = OpCompositeConstruct %mat2v4float %82 %86
%92 = OpFOrdEqual %v4bool %82 %89
%93 = OpAll %bool %92
%94 = OpFOrdEqual %v4bool %86 %89
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %93 %95
OpBranch %71
%71 = OpLabel
%97 = OpPhi %bool %false %26 %96 %70
OpStore %ok %97
%98 = OpLoad %bool %ok
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpLoad %v4float %f4
%102 = OpVectorShuffle %v2float %101 %101 0 1
%103 = OpLoad %v4float %f4
%104 = OpVectorShuffle %v2float %103 %103 2 3
%105 = OpLoad %v4float %f4
%106 = OpLoad %v4float %f4
%107 = OpCompositeExtract %float %106 0
%108 = OpCompositeExtract %float %102 0
%109 = OpCompositeExtract %float %102 1
%110 = OpCompositeExtract %float %104 0
%111 = OpCompositeConstruct %v3float %108 %109 %110
%112 = OpCompositeExtract %float %104 1
%113 = OpCompositeExtract %float %105 0
%114 = OpCompositeExtract %float %105 1
%115 = OpCompositeConstruct %v3float %112 %113 %114
%116 = OpCompositeExtract %float %105 2
%117 = OpCompositeExtract %float %105 3
%118 = OpCompositeConstruct %v3float %116 %117 %107
%120 = OpCompositeConstruct %mat3v3float %111 %115 %118
%123 = OpFOrdEqual %v3bool %111 %59
%124 = OpAll %bool %123
%125 = OpFOrdEqual %v3bool %115 %60
%126 = OpAll %bool %125
%127 = OpLogicalAnd %bool %124 %126
%128 = OpFOrdEqual %v3bool %118 %121
%129 = OpAll %bool %128
%130 = OpLogicalAnd %bool %127 %129
OpBranch %100
%100 = OpLabel
%131 = OpPhi %bool %false %71 %130 %99
OpStore %ok %131
%132 = OpLoad %bool %ok
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%135 = OpLoad %v4float %f4
%136 = OpVectorShuffle %v3float %135 %135 0 1 2
%137 = OpLoad %v4float %f4
%138 = OpVectorShuffle %v4float %137 %137 3 0 1 2
%139 = OpLoad %v4float %f4
%140 = OpCompositeExtract %float %139 3
%141 = OpCompositeExtract %float %136 0
%142 = OpCompositeExtract %float %136 1
%143 = OpCompositeConstruct %v2float %141 %142
%144 = OpCompositeExtract %float %136 2
%145 = OpCompositeExtract %float %138 0
%146 = OpCompositeConstruct %v2float %144 %145
%147 = OpCompositeExtract %float %138 1
%148 = OpCompositeExtract %float %138 2
%149 = OpCompositeConstruct %v2float %147 %148
%150 = OpCompositeExtract %float %138 3
%151 = OpCompositeConstruct %v2float %150 %140
%153 = OpCompositeConstruct %mat4v2float %143 %146 %149 %151
%158 = OpFOrdEqual %v2bool %143 %154
%159 = OpAll %bool %158
%160 = OpFOrdEqual %v2bool %146 %155
%161 = OpAll %bool %160
%162 = OpLogicalAnd %bool %159 %161
%163 = OpFOrdEqual %v2bool %149 %154
%164 = OpAll %bool %163
%165 = OpLogicalAnd %bool %162 %164
%166 = OpFOrdEqual %v2bool %151 %155
%167 = OpAll %bool %166
%168 = OpLogicalAnd %bool %165 %167
OpBranch %134
%134 = OpLabel
%169 = OpPhi %bool %false %100 %168 %133
OpStore %ok %169
%170 = OpLoad %bool %ok
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpLoad %v4float %f4
%174 = OpCompositeExtract %float %173 0
%175 = OpLoad %v4float %f4
%176 = OpVectorShuffle %v4float %175 %175 1 2 3 0
%177 = OpLoad %v4float %f4
%178 = OpVectorShuffle %v4float %177 %177 1 2 3 0
%179 = OpLoad %v4float %f4
%180 = OpVectorShuffle %v3float %179 %179 1 2 3
%181 = OpCompositeExtract %float %176 0
%182 = OpCompositeExtract %float %176 1
%183 = OpCompositeConstruct %v3float %174 %181 %182
%184 = OpCompositeExtract %float %176 2
%185 = OpCompositeExtract %float %176 3
%186 = OpCompositeExtract %float %178 0
%187 = OpCompositeConstruct %v3float %184 %185 %186
%188 = OpCompositeExtract %float %178 1
%189 = OpCompositeExtract %float %178 2
%190 = OpCompositeExtract %float %178 3
%191 = OpCompositeConstruct %v3float %188 %189 %190
%193 = OpCompositeConstruct %mat4v3float %183 %187 %191 %180
%196 = OpFOrdEqual %v3bool %183 %59
%197 = OpAll %bool %196
%198 = OpFOrdEqual %v3bool %187 %60
%199 = OpAll %bool %198
%200 = OpLogicalAnd %bool %197 %199
%201 = OpFOrdEqual %v3bool %191 %121
%202 = OpAll %bool %201
%203 = OpLogicalAnd %bool %200 %202
%204 = OpFOrdEqual %v3bool %180 %194
%205 = OpAll %bool %204
%206 = OpLogicalAnd %bool %203 %205
OpBranch %172
%172 = OpLabel
%207 = OpPhi %bool %false %134 %206 %171
OpStore %ok %207
%208 = OpLoad %bool %ok
OpSelectionMerge %212 None
OpBranchConditional %208 %210 %211
%210 = OpLabel
%213 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%216 = OpLoad %v4float %213
OpStore %209 %216
OpBranch %212
%211 = OpLabel
%217 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%219 = OpLoad %v4float %217
OpStore %209 %219
OpBranch %212
%212 = OpLabel
%220 = OpLoad %v4float %209
OpReturnValue %220
OpFunctionEnd

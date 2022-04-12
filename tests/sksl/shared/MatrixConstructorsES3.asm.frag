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
OpDecorate %71 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
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
%91 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%92 = OpConstantComposite %mat2v4float %91 %91
%v4bool = OpTypeVector %bool 4
%mat3v3float = OpTypeMatrix %v3float 3
%125 = OpConstantComposite %v3float %float_3 %float_4 %float_1
%126 = OpConstantComposite %mat3v3float %59 %60 %125
%mat4v2float = OpTypeMatrix %v2float 4
%161 = OpConstantComposite %v2float %float_1 %float_2
%162 = OpConstantComposite %v2float %float_3 %float_4
%163 = OpConstantComposite %mat4v2float %161 %162 %161 %162
%v2bool = OpTypeVector %bool 2
%mat4v3float = OpTypeMatrix %v3float 4
%205 = OpConstantComposite %v3float %float_2 %float_3 %float_4
%206 = OpConstantComposite %mat4v3float %59 %60 %125 %205
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
%224 = OpVariable %_ptr_Function_v4float Function
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
%63 = OpCompositeExtract %v3float %54 0
%64 = OpFOrdEqual %v3bool %63 %59
%65 = OpAll %bool %64
%66 = OpCompositeExtract %v3float %54 1
%67 = OpFOrdEqual %v3bool %66 %60
%68 = OpAll %bool %67
%69 = OpLogicalAnd %bool %65 %68
OpStore %ok %69
%71 = OpLoad %bool %ok
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpLoad %v4float %f4
%75 = OpVectorShuffle %v3float %74 %74 0 1 2
%76 = OpLoad %v4float %f4
%77 = OpVectorShuffle %v4float %76 %76 3 0 1 2
%78 = OpLoad %v4float %f4
%79 = OpCompositeExtract %float %78 3
%80 = OpCompositeExtract %float %75 0
%81 = OpCompositeExtract %float %75 1
%82 = OpCompositeExtract %float %75 2
%83 = OpCompositeExtract %float %77 0
%84 = OpCompositeConstruct %v4float %80 %81 %82 %83
%85 = OpCompositeExtract %float %77 1
%86 = OpCompositeExtract %float %77 2
%87 = OpCompositeExtract %float %77 3
%88 = OpCompositeConstruct %v4float %85 %86 %87 %79
%90 = OpCompositeConstruct %mat2v4float %84 %88
%94 = OpCompositeExtract %v4float %90 0
%95 = OpFOrdEqual %v4bool %94 %91
%96 = OpAll %bool %95
%97 = OpCompositeExtract %v4float %90 1
%98 = OpFOrdEqual %v4bool %97 %91
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %96 %99
OpBranch %73
%73 = OpLabel
%101 = OpPhi %bool %false %26 %100 %72
OpStore %ok %101
%102 = OpLoad %bool %ok
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %v4float %f4
%106 = OpVectorShuffle %v2float %105 %105 0 1
%107 = OpLoad %v4float %f4
%108 = OpVectorShuffle %v2float %107 %107 2 3
%109 = OpLoad %v4float %f4
%110 = OpLoad %v4float %f4
%111 = OpCompositeExtract %float %110 0
%112 = OpCompositeExtract %float %106 0
%113 = OpCompositeExtract %float %106 1
%114 = OpCompositeExtract %float %108 0
%115 = OpCompositeConstruct %v3float %112 %113 %114
%116 = OpCompositeExtract %float %108 1
%117 = OpCompositeExtract %float %109 0
%118 = OpCompositeExtract %float %109 1
%119 = OpCompositeConstruct %v3float %116 %117 %118
%120 = OpCompositeExtract %float %109 2
%121 = OpCompositeExtract %float %109 3
%122 = OpCompositeConstruct %v3float %120 %121 %111
%124 = OpCompositeConstruct %mat3v3float %115 %119 %122
%127 = OpCompositeExtract %v3float %124 0
%128 = OpFOrdEqual %v3bool %127 %59
%129 = OpAll %bool %128
%130 = OpCompositeExtract %v3float %124 1
%131 = OpFOrdEqual %v3bool %130 %60
%132 = OpAll %bool %131
%133 = OpLogicalAnd %bool %129 %132
%134 = OpCompositeExtract %v3float %124 2
%135 = OpFOrdEqual %v3bool %134 %125
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %133 %136
OpBranch %104
%104 = OpLabel
%138 = OpPhi %bool %false %73 %137 %103
OpStore %ok %138
%139 = OpLoad %bool %ok
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%142 = OpLoad %v4float %f4
%143 = OpVectorShuffle %v3float %142 %142 0 1 2
%144 = OpLoad %v4float %f4
%145 = OpVectorShuffle %v4float %144 %144 3 0 1 2
%146 = OpLoad %v4float %f4
%147 = OpCompositeExtract %float %146 3
%148 = OpCompositeExtract %float %143 0
%149 = OpCompositeExtract %float %143 1
%150 = OpCompositeConstruct %v2float %148 %149
%151 = OpCompositeExtract %float %143 2
%152 = OpCompositeExtract %float %145 0
%153 = OpCompositeConstruct %v2float %151 %152
%154 = OpCompositeExtract %float %145 1
%155 = OpCompositeExtract %float %145 2
%156 = OpCompositeConstruct %v2float %154 %155
%157 = OpCompositeExtract %float %145 3
%158 = OpCompositeConstruct %v2float %157 %147
%160 = OpCompositeConstruct %mat4v2float %150 %153 %156 %158
%165 = OpCompositeExtract %v2float %160 0
%166 = OpFOrdEqual %v2bool %165 %161
%167 = OpAll %bool %166
%168 = OpCompositeExtract %v2float %160 1
%169 = OpFOrdEqual %v2bool %168 %162
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %167 %170
%172 = OpCompositeExtract %v2float %160 2
%173 = OpFOrdEqual %v2bool %172 %161
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %171 %174
%176 = OpCompositeExtract %v2float %160 3
%177 = OpFOrdEqual %v2bool %176 %162
%178 = OpAll %bool %177
%179 = OpLogicalAnd %bool %175 %178
OpBranch %141
%141 = OpLabel
%180 = OpPhi %bool %false %104 %179 %140
OpStore %ok %180
%181 = OpLoad %bool %ok
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
%184 = OpLoad %v4float %f4
%185 = OpCompositeExtract %float %184 0
%186 = OpLoad %v4float %f4
%187 = OpVectorShuffle %v4float %186 %186 1 2 3 0
%188 = OpLoad %v4float %f4
%189 = OpVectorShuffle %v4float %188 %188 1 2 3 0
%190 = OpLoad %v4float %f4
%191 = OpVectorShuffle %v3float %190 %190 1 2 3
%192 = OpCompositeExtract %float %187 0
%193 = OpCompositeExtract %float %187 1
%194 = OpCompositeConstruct %v3float %185 %192 %193
%195 = OpCompositeExtract %float %187 2
%196 = OpCompositeExtract %float %187 3
%197 = OpCompositeExtract %float %189 0
%198 = OpCompositeConstruct %v3float %195 %196 %197
%199 = OpCompositeExtract %float %189 1
%200 = OpCompositeExtract %float %189 2
%201 = OpCompositeExtract %float %189 3
%202 = OpCompositeConstruct %v3float %199 %200 %201
%204 = OpCompositeConstruct %mat4v3float %194 %198 %202 %191
%207 = OpCompositeExtract %v3float %204 0
%208 = OpFOrdEqual %v3bool %207 %59
%209 = OpAll %bool %208
%210 = OpCompositeExtract %v3float %204 1
%211 = OpFOrdEqual %v3bool %210 %60
%212 = OpAll %bool %211
%213 = OpLogicalAnd %bool %209 %212
%214 = OpCompositeExtract %v3float %204 2
%215 = OpFOrdEqual %v3bool %214 %125
%216 = OpAll %bool %215
%217 = OpLogicalAnd %bool %213 %216
%218 = OpCompositeExtract %v3float %204 3
%219 = OpFOrdEqual %v3bool %218 %205
%220 = OpAll %bool %219
%221 = OpLogicalAnd %bool %217 %220
OpBranch %183
%183 = OpLabel
%222 = OpPhi %bool %false %141 %221 %182
OpStore %ok %222
%223 = OpLoad %bool %ok
OpSelectionMerge %227 None
OpBranchConditional %223 %225 %226
%225 = OpLabel
%228 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%231 = OpLoad %v4float %228
OpStore %224 %231
OpBranch %227
%226 = OpLabel
%232 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%234 = OpLoad %v4float %232
OpStore %224 %234
OpBranch %227
%227 = OpLabel
%235 = OpLoad %v4float %224
OpReturnValue %235
OpFunctionEnd

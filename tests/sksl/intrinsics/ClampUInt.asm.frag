OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %uintValues "uintValues"
OpName %expectedA "expectedA"
OpName %clampLow "clampLow"
OpName %expectedB "expectedB"
OpName %clampHigh "clampHigh"
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
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
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
%uint = OpTypeInt 32 0
%v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%float_200 = OpConstant %float 200
%uint_100 = OpConstant %uint 100
%uint_200 = OpConstant %uint 200
%uint_275 = OpConstant %uint 275
%uint_300 = OpConstant %uint 300
%54 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_275 %uint_300
%uint_0 = OpConstant %uint 0
%57 = OpConstantComposite %v4uint %uint_100 %uint_0 %uint_0 %uint_300
%uint_250 = OpConstant %uint 250
%uint_425 = OpConstant %uint 425
%61 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_250 %uint_425
%uint_400 = OpConstant %uint 400
%uint_500 = OpConstant %uint 500
%65 = OpConstantComposite %v4uint %uint_300 %uint_400 %uint_250 %uint_500
%false = OpConstantFalse %bool
%v2uint = OpTypeVector %uint 2
%v2bool = OpTypeVector %bool 2
%v3uint = OpTypeVector %uint 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%120 = OpConstantComposite %v2uint %uint_100 %uint_200
%128 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_275
%154 = OpConstantComposite %v2uint %uint_100 %uint_0
%155 = OpConstantComposite %v2uint %uint_300 %uint_400
%166 = OpConstantComposite %v3uint %uint_100 %uint_0 %uint_0
%167 = OpConstantComposite %v3uint %uint_300 %uint_400 %uint_250
%198 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_250
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
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
%uintValues = OpVariable %_ptr_Function_v4uint Function
%expectedA = OpVariable %_ptr_Function_v4uint Function
%clampLow = OpVariable %_ptr_Function_v4uint Function
%expectedB = OpVariable %_ptr_Function_v4uint Function
%clampHigh = OpVariable %_ptr_Function_v4uint Function
%210 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %30
%36 = OpVectorTimesScalar %v4float %34 %float_100
%38 = OpCompositeConstruct %v4float %float_200 %float_200 %float_200 %float_200
%39 = OpFAdd %v4float %36 %38
%40 = OpCompositeExtract %float %39 0
%41 = OpConvertFToU %uint %40
%42 = OpCompositeExtract %float %39 1
%43 = OpConvertFToU %uint %42
%44 = OpCompositeExtract %float %39 2
%45 = OpConvertFToU %uint %44
%46 = OpCompositeExtract %float %39 3
%47 = OpConvertFToU %uint %46
%48 = OpCompositeConstruct %v4uint %41 %43 %45 %47
OpStore %uintValues %48
OpStore %expectedA %54
OpStore %clampLow %57
OpStore %expectedB %61
OpStore %clampHigh %65
%68 = OpLoad %v4uint %uintValues
%69 = OpCompositeExtract %uint %68 0
%67 = OpExtInst %uint %1 UClamp %69 %uint_100 %uint_300
%70 = OpLoad %v4uint %expectedA
%71 = OpCompositeExtract %uint %70 0
%72 = OpIEqual %bool %67 %71
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpLoad %v4uint %uintValues
%77 = OpVectorShuffle %v2uint %76 %76 0 1
%79 = OpCompositeConstruct %v2uint %uint_100 %uint_100
%80 = OpCompositeConstruct %v2uint %uint_300 %uint_300
%75 = OpExtInst %v2uint %1 UClamp %77 %79 %80
%81 = OpLoad %v4uint %expectedA
%82 = OpVectorShuffle %v2uint %81 %81 0 1
%83 = OpIEqual %v2bool %75 %82
%85 = OpAll %bool %83
OpBranch %74
%74 = OpLabel
%86 = OpPhi %bool %false %25 %85 %73
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpLoad %v4uint %uintValues
%91 = OpVectorShuffle %v3uint %90 %90 0 1 2
%93 = OpCompositeConstruct %v3uint %uint_100 %uint_100 %uint_100
%94 = OpCompositeConstruct %v3uint %uint_300 %uint_300 %uint_300
%89 = OpExtInst %v3uint %1 UClamp %91 %93 %94
%95 = OpLoad %v4uint %expectedA
%96 = OpVectorShuffle %v3uint %95 %95 0 1 2
%97 = OpIEqual %v3bool %89 %96
%99 = OpAll %bool %97
OpBranch %88
%88 = OpLabel
%100 = OpPhi %bool %false %74 %99 %87
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpLoad %v4uint %uintValues
%105 = OpCompositeConstruct %v4uint %uint_100 %uint_100 %uint_100 %uint_100
%106 = OpCompositeConstruct %v4uint %uint_300 %uint_300 %uint_300 %uint_300
%103 = OpExtInst %v4uint %1 UClamp %104 %105 %106
%107 = OpLoad %v4uint %expectedA
%108 = OpIEqual %v4bool %103 %107
%110 = OpAll %bool %108
OpBranch %102
%102 = OpLabel
%111 = OpPhi %bool %false %88 %110 %101
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpLoad %v4uint %expectedA
%115 = OpCompositeExtract %uint %114 0
%116 = OpIEqual %bool %uint_100 %115
OpBranch %113
%113 = OpLabel
%117 = OpPhi %bool %false %102 %116 %112
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpLoad %v4uint %expectedA
%122 = OpVectorShuffle %v2uint %121 %121 0 1
%123 = OpIEqual %v2bool %120 %122
%124 = OpAll %bool %123
OpBranch %119
%119 = OpLabel
%125 = OpPhi %bool %false %113 %124 %118
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpLoad %v4uint %expectedA
%130 = OpVectorShuffle %v3uint %129 %129 0 1 2
%131 = OpIEqual %v3bool %128 %130
%132 = OpAll %bool %131
OpBranch %127
%127 = OpLabel
%133 = OpPhi %bool %false %119 %132 %126
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpLoad %v4uint %expectedA
%137 = OpIEqual %v4bool %54 %136
%138 = OpAll %bool %137
OpBranch %135
%135 = OpLabel
%139 = OpPhi %bool %false %127 %138 %134
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%143 = OpLoad %v4uint %uintValues
%144 = OpCompositeExtract %uint %143 0
%142 = OpExtInst %uint %1 UClamp %144 %uint_100 %uint_300
%145 = OpLoad %v4uint %expectedB
%146 = OpCompositeExtract %uint %145 0
%147 = OpIEqual %bool %142 %146
OpBranch %141
%141 = OpLabel
%148 = OpPhi %bool %false %135 %147 %140
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%152 = OpLoad %v4uint %uintValues
%153 = OpVectorShuffle %v2uint %152 %152 0 1
%151 = OpExtInst %v2uint %1 UClamp %153 %154 %155
%156 = OpLoad %v4uint %expectedB
%157 = OpVectorShuffle %v2uint %156 %156 0 1
%158 = OpIEqual %v2bool %151 %157
%159 = OpAll %bool %158
OpBranch %150
%150 = OpLabel
%160 = OpPhi %bool %false %141 %159 %149
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpLoad %v4uint %uintValues
%165 = OpVectorShuffle %v3uint %164 %164 0 1 2
%163 = OpExtInst %v3uint %1 UClamp %165 %166 %167
%168 = OpLoad %v4uint %expectedB
%169 = OpVectorShuffle %v3uint %168 %168 0 1 2
%170 = OpIEqual %v3bool %163 %169
%171 = OpAll %bool %170
OpBranch %162
%162 = OpLabel
%172 = OpPhi %bool %false %150 %171 %161
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%176 = OpLoad %v4uint %uintValues
%177 = OpLoad %v4uint %clampLow
%178 = OpLoad %v4uint %clampHigh
%175 = OpExtInst %v4uint %1 UClamp %176 %177 %178
%179 = OpLoad %v4uint %expectedB
%180 = OpIEqual %v4bool %175 %179
%181 = OpAll %bool %180
OpBranch %174
%174 = OpLabel
%182 = OpPhi %bool %false %162 %181 %173
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpLoad %v4uint %expectedB
%186 = OpCompositeExtract %uint %185 0
%187 = OpIEqual %bool %uint_100 %186
OpBranch %184
%184 = OpLabel
%188 = OpPhi %bool %false %174 %187 %183
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpLoad %v4uint %expectedB
%192 = OpVectorShuffle %v2uint %191 %191 0 1
%193 = OpIEqual %v2bool %120 %192
%194 = OpAll %bool %193
OpBranch %190
%190 = OpLabel
%195 = OpPhi %bool %false %184 %194 %189
OpSelectionMerge %197 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
%199 = OpLoad %v4uint %expectedB
%200 = OpVectorShuffle %v3uint %199 %199 0 1 2
%201 = OpIEqual %v3bool %198 %200
%202 = OpAll %bool %201
OpBranch %197
%197 = OpLabel
%203 = OpPhi %bool %false %190 %202 %196
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%206 = OpLoad %v4uint %expectedB
%207 = OpIEqual %v4bool %61 %206
%208 = OpAll %bool %207
OpBranch %205
%205 = OpLabel
%209 = OpPhi %bool %false %197 %208 %204
OpSelectionMerge %214 None
OpBranchConditional %209 %212 %213
%212 = OpLabel
%215 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%217 = OpLoad %v4float %215
OpStore %210 %217
OpBranch %214
%213 = OpLabel
%218 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%220 = OpLoad %v4float %218
OpStore %210 %220
OpBranch %214
%214 = OpLabel
%221 = OpLoad %v4float %210
OpReturnValue %221
OpFunctionEnd

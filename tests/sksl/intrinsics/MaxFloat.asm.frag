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
OpName %expectedA "expectedA"
OpName %expectedB "expectedB"
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
OpDecorate %expectedA RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%float_1 = OpConstant %float 1
%34 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%92 = OpConstantComposite %v2float %float_0_5 %float_0_5
%100 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_75
%int_1 = OpConstant %int 1
%173 = OpConstantComposite %v2float %float_0 %float_1
%181 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
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
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%193 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %31
OpStore %expectedB %34
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%36 = OpExtInst %float %1 FMax %42 %float_0_5
%43 = OpLoad %v4float %expectedA
%44 = OpCompositeExtract %float %43 0
%45 = OpFOrdEqual %bool %36 %44
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%48 = OpExtInst %v2float %1 FMax %51 %52
%53 = OpLoad %v4float %expectedA
%54 = OpVectorShuffle %v2float %53 %53 0 1
%55 = OpFOrdEqual %v2bool %48 %54
%57 = OpAll %bool %55
OpBranch %47
%47 = OpLabel
%58 = OpPhi %bool %false %25 %57 %46
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%66 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%61 = OpExtInst %v3float %1 FMax %64 %66
%67 = OpLoad %v4float %expectedA
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%69 = OpFOrdEqual %v3bool %61 %68
%71 = OpAll %bool %69
OpBranch %60
%60 = OpLabel
%72 = OpPhi %bool %false %47 %71 %59
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%78 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%75 = OpExtInst %v4float %1 FMax %77 %78
%79 = OpLoad %v4float %expectedA
%80 = OpFOrdEqual %v4bool %75 %79
%82 = OpAll %bool %80
OpBranch %74
%74 = OpLabel
%83 = OpPhi %bool %false %60 %82 %73
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %v4float %expectedA
%87 = OpCompositeExtract %float %86 0
%88 = OpFOrdEqual %bool %float_0_5 %87
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %74 %88 %84
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpLoad %v4float %expectedA
%94 = OpVectorShuffle %v2float %93 %93 0 1
%95 = OpFOrdEqual %v2bool %92 %94
%96 = OpAll %bool %95
OpBranch %91
%91 = OpLabel
%97 = OpPhi %bool %false %85 %96 %90
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpLoad %v4float %expectedA
%102 = OpVectorShuffle %v3float %101 %101 0 1 2
%103 = OpFOrdEqual %v3bool %100 %102
%104 = OpAll %bool %103
OpBranch %99
%99 = OpLabel
%105 = OpPhi %bool %false %91 %104 %98
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %v4float %expectedA
%109 = OpFOrdEqual %v4bool %31 %108
%110 = OpAll %bool %109
OpBranch %107
%107 = OpLabel
%111 = OpPhi %bool %false %99 %110 %106
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%116 = OpLoad %v4float %115
%117 = OpCompositeExtract %float %116 0
%118 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%120 = OpLoad %v4float %118
%121 = OpCompositeExtract %float %120 0
%114 = OpExtInst %float %1 FMax %117 %121
%122 = OpLoad %v4float %expectedB
%123 = OpCompositeExtract %float %122 0
%124 = OpFOrdEqual %bool %114 %123
OpBranch %113
%113 = OpLabel
%125 = OpPhi %bool %false %107 %124 %112
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%130 = OpLoad %v4float %129
%131 = OpVectorShuffle %v2float %130 %130 0 1
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%133 = OpLoad %v4float %132
%134 = OpVectorShuffle %v2float %133 %133 0 1
%128 = OpExtInst %v2float %1 FMax %131 %134
%135 = OpLoad %v4float %expectedB
%136 = OpVectorShuffle %v2float %135 %135 0 1
%137 = OpFOrdEqual %v2bool %128 %136
%138 = OpAll %bool %137
OpBranch %127
%127 = OpLabel
%139 = OpPhi %bool %false %113 %138 %126
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%144 = OpLoad %v4float %143
%145 = OpVectorShuffle %v3float %144 %144 0 1 2
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%147 = OpLoad %v4float %146
%148 = OpVectorShuffle %v3float %147 %147 0 1 2
%142 = OpExtInst %v3float %1 FMax %145 %148
%149 = OpLoad %v4float %expectedB
%150 = OpVectorShuffle %v3float %149 %149 0 1 2
%151 = OpFOrdEqual %v3bool %142 %150
%152 = OpAll %bool %151
OpBranch %141
%141 = OpLabel
%153 = OpPhi %bool %false %127 %152 %140
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%158 = OpLoad %v4float %157
%159 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%160 = OpLoad %v4float %159
%156 = OpExtInst %v4float %1 FMax %158 %160
%161 = OpLoad %v4float %expectedB
%162 = OpFOrdEqual %v4bool %156 %161
%163 = OpAll %bool %162
OpBranch %155
%155 = OpLabel
%164 = OpPhi %bool %false %141 %163 %154
OpSelectionMerge %166 None
OpBranchConditional %164 %165 %166
%165 = OpLabel
%167 = OpLoad %v4float %expectedB
%168 = OpCompositeExtract %float %167 0
%169 = OpFOrdEqual %bool %float_0 %168
OpBranch %166
%166 = OpLabel
%170 = OpPhi %bool %false %155 %169 %165
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%174 = OpLoad %v4float %expectedB
%175 = OpVectorShuffle %v2float %174 %174 0 1
%176 = OpFOrdEqual %v2bool %173 %175
%177 = OpAll %bool %176
OpBranch %172
%172 = OpLabel
%178 = OpPhi %bool %false %166 %177 %171
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%182 = OpLoad %v4float %expectedB
%183 = OpVectorShuffle %v3float %182 %182 0 1 2
%184 = OpFOrdEqual %v3bool %181 %183
%185 = OpAll %bool %184
OpBranch %180
%180 = OpLabel
%186 = OpPhi %bool %false %172 %185 %179
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpLoad %v4float %expectedB
%190 = OpFOrdEqual %v4bool %34 %189
%191 = OpAll %bool %190
OpBranch %188
%188 = OpLabel
%192 = OpPhi %bool %false %180 %191 %187
OpSelectionMerge %196 None
OpBranchConditional %192 %194 %195
%194 = OpLabel
%197 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%198 = OpLoad %v4float %197
OpStore %193 %198
OpBranch %196
%195 = OpLabel
%199 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%201 = OpLoad %v4float %199
OpStore %193 %201
OpBranch %196
%196 = OpLabel
%202 = OpLoad %v4float %193
OpReturnValue %202
OpFunctionEnd

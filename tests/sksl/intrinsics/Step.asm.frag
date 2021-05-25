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
OpName %constGreen "constGreen"
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
OpDecorate %constGreen RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%31 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
%33 = OpConstantComposite %v4float %float_1 %float_1 %float_0 %float_0
%false = OpConstantFalse %bool
%float_0_5 = OpConstant %float 0.5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%99 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%127 = OpConstantComposite %v2float %float_0 %float_1
%139 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%163 = OpConstantComposite %v2float %float_1 %float_1
%171 = OpConstantComposite %v3float %float_1 %float_1 %float_0
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
%constGreen = OpVariable %_ptr_Function_v4float Function
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%183 = OpVariable %_ptr_Function_v4float Function
OpStore %constGreen %29
OpStore %expectedA %31
OpStore %expectedB %33
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%35 = OpExtInst %float %1 Step %float_0_5 %42
%43 = OpLoad %v4float %expectedA
%44 = OpCompositeExtract %float %43 0
%45 = OpFOrdEqual %bool %35 %44
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v2float %51 %51 0 1
%48 = OpExtInst %v2float %1 Step %49 %52
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
%62 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%61 = OpExtInst %v3float %1 Step %62 %66
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
%76 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%78 = OpLoad %v4float %77
%75 = OpExtInst %v4float %1 Step %76 %78
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
%88 = OpFOrdEqual %bool %float_0 %87
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %74 %88 %84
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %v4float %expectedA
%93 = OpVectorShuffle %v2float %92 %92 0 1
%94 = OpFOrdEqual %v2bool %19 %93
%95 = OpAll %bool %94
OpBranch %91
%91 = OpLabel
%96 = OpPhi %bool %false %85 %95 %90
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpLoad %v4float %expectedA
%101 = OpVectorShuffle %v3float %100 %100 0 1 2
%102 = OpFOrdEqual %v3bool %99 %101
%103 = OpAll %bool %102
OpBranch %98
%98 = OpLabel
%104 = OpPhi %bool %false %91 %103 %97
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpLoad %v4float %expectedA
%108 = OpFOrdEqual %v4bool %31 %107
%109 = OpAll %bool %108
OpBranch %106
%106 = OpLabel
%110 = OpPhi %bool %false %98 %109 %105
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%115 = OpLoad %v4float %114
%116 = OpCompositeExtract %float %115 0
%113 = OpExtInst %float %1 Step %116 %float_0
%117 = OpLoad %v4float %expectedB
%118 = OpCompositeExtract %float %117 0
%119 = OpFOrdEqual %bool %113 %118
OpBranch %112
%112 = OpLabel
%120 = OpPhi %bool %false %106 %119 %111
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%125 = OpLoad %v4float %124
%126 = OpVectorShuffle %v2float %125 %125 0 1
%123 = OpExtInst %v2float %1 Step %126 %127
%128 = OpLoad %v4float %expectedB
%129 = OpVectorShuffle %v2float %128 %128 0 1
%130 = OpFOrdEqual %v2bool %123 %129
%131 = OpAll %bool %130
OpBranch %122
%122 = OpLabel
%132 = OpPhi %bool %false %112 %131 %121
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%137 = OpLoad %v4float %136
%138 = OpVectorShuffle %v3float %137 %137 0 1 2
%135 = OpExtInst %v3float %1 Step %138 %139
%140 = OpLoad %v4float %expectedB
%141 = OpVectorShuffle %v3float %140 %140 0 1 2
%142 = OpFOrdEqual %v3bool %135 %141
%143 = OpAll %bool %142
OpBranch %134
%134 = OpLabel
%144 = OpPhi %bool %false %122 %143 %133
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%149 = OpLoad %v4float %148
%150 = OpLoad %v4float %constGreen
%147 = OpExtInst %v4float %1 Step %149 %150
%151 = OpLoad %v4float %expectedB
%152 = OpFOrdEqual %v4bool %147 %151
%153 = OpAll %bool %152
OpBranch %146
%146 = OpLabel
%154 = OpPhi %bool %false %134 %153 %145
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpLoad %v4float %expectedB
%158 = OpCompositeExtract %float %157 0
%159 = OpFOrdEqual %bool %float_1 %158
OpBranch %156
%156 = OpLabel
%160 = OpPhi %bool %false %146 %159 %155
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpLoad %v4float %expectedB
%165 = OpVectorShuffle %v2float %164 %164 0 1
%166 = OpFOrdEqual %v2bool %163 %165
%167 = OpAll %bool %166
OpBranch %162
%162 = OpLabel
%168 = OpPhi %bool %false %156 %167 %161
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%172 = OpLoad %v4float %expectedB
%173 = OpVectorShuffle %v3float %172 %172 0 1 2
%174 = OpFOrdEqual %v3bool %171 %173
%175 = OpAll %bool %174
OpBranch %170
%170 = OpLabel
%176 = OpPhi %bool %false %162 %175 %169
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%179 = OpLoad %v4float %expectedB
%180 = OpFOrdEqual %v4bool %33 %179
%181 = OpAll %bool %180
OpBranch %178
%178 = OpLabel
%182 = OpPhi %bool %false %170 %181 %177
OpSelectionMerge %186 None
OpBranchConditional %182 %184 %185
%184 = OpLabel
%187 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%189 = OpLoad %v4float %187
OpStore %183 %189
OpBranch %186
%185 = OpLabel
%190 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%192 = OpLoad %v4float %190
OpStore %183 %192
OpBranch %186
%186 = OpLabel
%193 = OpLoad %v4float %183
OpReturnValue %193
OpFunctionEnd

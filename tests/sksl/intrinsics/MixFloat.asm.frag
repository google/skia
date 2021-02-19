OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%33 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%v4bool = OpTypeVector %bool 4
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%47 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
%59 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
%71 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%float_0_5 = OpConstant %float 0.5
%87 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
%v2float = OpTypeVector %float 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%int_4 = OpConstant %int 4
%float_2_25 = OpConstant %float 2.25
%145 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
%158 = OpConstantComposite %v2float %float_0 %float_0_5
%172 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%184 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%188 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%29 = OpLoad %v4float %27
%31 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%21 = OpExtInst %v4float %1 FMix %26 %29 %31
%34 = OpFOrdEqual %v4bool %21 %33
%36 = OpAll %bool %34
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %40
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%43 = OpLoad %v4float %42
%45 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%39 = OpExtInst %v4float %1 FMix %41 %43 %45
%48 = OpFOrdEqual %v4bool %39 %47
%49 = OpAll %bool %48
OpBranch %38
%38 = OpLabel
%50 = OpPhi %bool %false %19 %49 %37
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%55 = OpLoad %v4float %54
%56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%57 = OpLoad %v4float %56
%58 = OpCompositeConstruct %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%53 = OpExtInst %v4float %1 FMix %55 %57 %58
%60 = OpFOrdEqual %v4bool %53 %59
%61 = OpAll %bool %60
OpBranch %52
%52 = OpLabel
%62 = OpPhi %bool %false %38 %61 %51
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%67 = OpLoad %v4float %66
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%69 = OpLoad %v4float %68
%70 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%65 = OpExtInst %v4float %1 FMix %67 %69 %70
%72 = OpFOrdEqual %v4bool %65 %71
%73 = OpAll %bool %72
OpBranch %64
%64 = OpLabel
%74 = OpPhi %bool %false %52 %73 %63
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%80 = OpLoad %v4float %78
%81 = OpCompositeExtract %float %80 0
%82 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%84 = OpLoad %v4float %82
%85 = OpCompositeExtract %float %84 0
%77 = OpExtInst %float %1 FMix %81 %85 %float_0_5
%88 = OpCompositeExtract %float %87 0
%89 = OpFOrdEqual %bool %77 %88
OpBranch %76
%76 = OpLabel
%90 = OpPhi %bool %false %64 %89 %75
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%95 = OpLoad %v4float %94
%96 = OpVectorShuffle %v2float %95 %95 0 1
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%99 = OpLoad %v4float %98
%100 = OpVectorShuffle %v2float %99 %99 0 1
%101 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%93 = OpExtInst %v2float %1 FMix %96 %100 %101
%102 = OpVectorShuffle %v2float %87 %87 0 1
%103 = OpFOrdEqual %v2bool %93 %102
%105 = OpAll %bool %103
OpBranch %92
%92 = OpLabel
%106 = OpPhi %bool %false %76 %105 %91
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%111 = OpLoad %v4float %110
%112 = OpVectorShuffle %v3float %111 %111 0 1 2
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%115 = OpLoad %v4float %114
%116 = OpVectorShuffle %v3float %115 %115 0 1 2
%117 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%109 = OpExtInst %v3float %1 FMix %112 %116 %117
%118 = OpVectorShuffle %v3float %87 %87 0 1 2
%119 = OpFOrdEqual %v3bool %109 %118
%121 = OpAll %bool %119
OpBranch %108
%108 = OpLabel
%122 = OpPhi %bool %false %92 %121 %107
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%127 = OpLoad %v4float %126
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%129 = OpLoad %v4float %128
%130 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%125 = OpExtInst %v4float %1 FMix %127 %129 %130
%131 = OpFOrdEqual %v4bool %125 %87
%132 = OpAll %bool %131
OpBranch %124
%124 = OpLabel
%133 = OpPhi %bool %false %108 %132 %123
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%138 = OpLoad %v4float %137
%139 = OpCompositeExtract %float %138 0
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%142 = OpLoad %v4float %140
%143 = OpCompositeExtract %float %142 0
%136 = OpExtInst %float %1 FMix %139 %143 %float_0
%146 = OpCompositeExtract %float %145 0
%147 = OpFOrdEqual %bool %136 %146
OpBranch %135
%135 = OpLabel
%148 = OpPhi %bool %false %124 %147 %134
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%153 = OpLoad %v4float %152
%154 = OpVectorShuffle %v2float %153 %153 0 1
%155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%156 = OpLoad %v4float %155
%157 = OpVectorShuffle %v2float %156 %156 0 1
%151 = OpExtInst %v2float %1 FMix %154 %157 %158
%159 = OpVectorShuffle %v2float %145 %145 0 1
%160 = OpFOrdEqual %v2bool %151 %159
%161 = OpAll %bool %160
OpBranch %150
%150 = OpLabel
%162 = OpPhi %bool %false %135 %161 %149
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%167 = OpLoad %v4float %166
%168 = OpVectorShuffle %v3float %167 %167 0 1 2
%169 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%170 = OpLoad %v4float %169
%171 = OpVectorShuffle %v3float %170 %170 0 1 2
%165 = OpExtInst %v3float %1 FMix %168 %171 %172
%173 = OpVectorShuffle %v3float %145 %145 0 1 2
%174 = OpFOrdEqual %v3bool %165 %173
%175 = OpAll %bool %174
OpBranch %164
%164 = OpLabel
%176 = OpPhi %bool %false %150 %175 %163
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%181 = OpLoad %v4float %180
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%183 = OpLoad %v4float %182
%179 = OpExtInst %v4float %1 FMix %181 %183 %184
%185 = OpFOrdEqual %v4bool %179 %145
%186 = OpAll %bool %185
OpBranch %178
%178 = OpLabel
%187 = OpPhi %bool %false %164 %186 %177
OpSelectionMerge %192 None
OpBranchConditional %187 %190 %191
%190 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%194 = OpLoad %v4float %193
OpStore %188 %194
OpBranch %192
%191 = OpLabel
%195 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%196 = OpLoad %v4float %195
OpStore %188 %196
OpBranch %192
%192 = OpLabel
%197 = OpLoad %v4float %188
OpReturnValue %197
OpFunctionEnd

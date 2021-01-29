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
OpDecorate %93 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%100 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%116 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%129 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
%int_4 = OpConstant %int 4
%154 = OpConstantComposite %v2float %float_0 %float_0_5
%155 = OpConstantComposite %v2float %float_1 %float_0_5
%168 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%169 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
%180 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%float_2_25 = OpConstant %float 2.25
%182 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%186 = OpVariable %_ptr_Function_v4float Function
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
%87 = OpFOrdEqual %bool %77 %float_0_5
OpBranch %76
%76 = OpLabel
%88 = OpPhi %bool %false %64 %87 %75
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%93 = OpLoad %v4float %92
%94 = OpVectorShuffle %v2float %93 %93 0 1
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%97 = OpLoad %v4float %96
%98 = OpVectorShuffle %v2float %97 %97 0 1
%99 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%91 = OpExtInst %v2float %1 FMix %94 %98 %99
%101 = OpFOrdEqual %v2bool %91 %100
%103 = OpAll %bool %101
OpBranch %90
%90 = OpLabel
%104 = OpPhi %bool %false %76 %103 %89
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%109 = OpLoad %v4float %108
%110 = OpVectorShuffle %v3float %109 %109 0 1 2
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%113 = OpLoad %v4float %112
%114 = OpVectorShuffle %v3float %113 %113 0 1 2
%115 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%107 = OpExtInst %v3float %1 FMix %110 %114 %115
%117 = OpFOrdEqual %v3bool %107 %116
%119 = OpAll %bool %117
OpBranch %106
%106 = OpLabel
%120 = OpPhi %bool %false %90 %119 %105
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%125 = OpLoad %v4float %124
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%127 = OpLoad %v4float %126
%128 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%123 = OpExtInst %v4float %1 FMix %125 %127 %128
%130 = OpFOrdEqual %v4bool %123 %129
%131 = OpAll %bool %130
OpBranch %122
%122 = OpLabel
%132 = OpPhi %bool %false %106 %131 %121
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%137 = OpLoad %v4float %136
%138 = OpCompositeExtract %float %137 0
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%141 = OpLoad %v4float %139
%142 = OpCompositeExtract %float %141 0
%135 = OpExtInst %float %1 FMix %138 %142 %float_0
%143 = OpFOrdEqual %bool %135 %float_1
OpBranch %134
%134 = OpLabel
%144 = OpPhi %bool %false %122 %143 %133
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%149 = OpLoad %v4float %148
%150 = OpVectorShuffle %v2float %149 %149 0 1
%151 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%152 = OpLoad %v4float %151
%153 = OpVectorShuffle %v2float %152 %152 0 1
%147 = OpExtInst %v2float %1 FMix %150 %153 %154
%156 = OpFOrdEqual %v2bool %147 %155
%157 = OpAll %bool %156
OpBranch %146
%146 = OpLabel
%158 = OpPhi %bool %false %134 %157 %145
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%163 = OpLoad %v4float %162
%164 = OpVectorShuffle %v3float %163 %163 0 1 2
%165 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%166 = OpLoad %v4float %165
%167 = OpVectorShuffle %v3float %166 %166 0 1 2
%161 = OpExtInst %v3float %1 FMix %164 %167 %168
%170 = OpFOrdEqual %v3bool %161 %169
%171 = OpAll %bool %170
OpBranch %160
%160 = OpLabel
%172 = OpPhi %bool %false %146 %171 %159
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%176 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%177 = OpLoad %v4float %176
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%179 = OpLoad %v4float %178
%175 = OpExtInst %v4float %1 FMix %177 %179 %180
%183 = OpFOrdEqual %v4bool %175 %182
%184 = OpAll %bool %183
OpBranch %174
%174 = OpLabel
%185 = OpPhi %bool %false %160 %184 %173
OpSelectionMerge %190 None
OpBranchConditional %185 %188 %189
%188 = OpLabel
%191 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%192 = OpLoad %v4float %191
OpStore %186 %192
OpBranch %190
%189 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%194 = OpLoad %v4float %193
OpStore %186 %194
OpBranch %190
%190 = OpLabel
%195 = OpLoad %v4float %186
OpReturnValue %195
OpFunctionEnd

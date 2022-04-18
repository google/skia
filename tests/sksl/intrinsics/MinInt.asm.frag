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
OpName %intValues "intValues"
OpName %intGreen "intGreen"
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
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
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
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%int_1 = OpConstant %int 1
%int_n125 = OpConstant %int -125
%int_50 = OpConstant %int 50
%62 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
%int_100 = OpConstant %int 100
%65 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%75 = OpConstantComposite %v2int %int_50 %int_50
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%86 = OpConstantComposite %v3int %int_50 %int_50 %int_50
%v3bool = OpTypeVector %bool 3
%95 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
%v4bool = OpTypeVector %bool 4
%106 = OpConstantComposite %v2int %int_n125 %int_0
%113 = OpConstantComposite %v3int %int_n125 %int_0 %int_50
%165 = OpConstantComposite %v3int %int_n125 %int_0 %int_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%intValues = OpVariable %_ptr_Function_v4int Function
%intGreen = OpVariable %_ptr_Function_v4int Function
%expectedA = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%175 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %30
%35 = OpVectorTimesScalar %v4float %33 %float_100
%36 = OpCompositeExtract %float %35 0
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %35 1
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %35 2
%41 = OpConvertFToS %int %40
%42 = OpCompositeExtract %float %35 3
%43 = OpConvertFToS %int %42
%44 = OpCompositeConstruct %v4int %37 %39 %41 %43
OpStore %intValues %44
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%48 = OpLoad %v4float %46
%49 = OpVectorTimesScalar %v4float %48 %float_100
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %49 2
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %49 3
%57 = OpConvertFToS %int %56
%58 = OpCompositeConstruct %v4int %51 %53 %55 %57
OpStore %intGreen %58
OpStore %expectedA %62
OpStore %expectedB %65
%68 = OpCompositeExtract %int %44 0
%67 = OpExtInst %int %1 SMin %68 %int_50
%69 = OpIEqual %bool %67 %int_n125
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpVectorShuffle %v2int %44 %44 0 1
%72 = OpExtInst %v2int %1 SMin %73 %75
%76 = OpVectorShuffle %v2int %62 %62 0 1
%77 = OpIEqual %v2bool %72 %76
%79 = OpAll %bool %77
OpBranch %71
%71 = OpLabel
%80 = OpPhi %bool %false %25 %79 %70
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpVectorShuffle %v3int %44 %44 0 1 2
%83 = OpExtInst %v3int %1 SMin %84 %86
%87 = OpVectorShuffle %v3int %62 %62 0 1 2
%88 = OpIEqual %v3bool %83 %87
%90 = OpAll %bool %88
OpBranch %82
%82 = OpLabel
%91 = OpPhi %bool %false %71 %90 %81
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpExtInst %v4int %1 SMin %44 %95
%96 = OpIEqual %v4bool %94 %62
%98 = OpAll %bool %96
OpBranch %93
%93 = OpLabel
%99 = OpPhi %bool %false %82 %98 %92
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpIEqual %bool %int_n125 %int_n125
OpBranch %101
%101 = OpLabel
%103 = OpPhi %bool %false %93 %102 %100
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%107 = OpVectorShuffle %v2int %62 %62 0 1
%108 = OpIEqual %v2bool %106 %107
%109 = OpAll %bool %108
OpBranch %105
%105 = OpLabel
%110 = OpPhi %bool %false %101 %109 %104
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%114 = OpVectorShuffle %v3int %62 %62 0 1 2
%115 = OpIEqual %v3bool %113 %114
%116 = OpAll %bool %115
OpBranch %112
%112 = OpLabel
%117 = OpPhi %bool %false %105 %116 %111
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpIEqual %v4bool %62 %62
%121 = OpAll %bool %120
OpBranch %119
%119 = OpLabel
%122 = OpPhi %bool %false %112 %121 %118
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpCompositeExtract %int %58 0
%125 = OpExtInst %int %1 SMin %68 %126
%127 = OpIEqual %bool %125 %int_n125
OpBranch %124
%124 = OpLabel
%128 = OpPhi %bool %false %119 %127 %123
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpVectorShuffle %v2int %44 %44 0 1
%133 = OpVectorShuffle %v2int %58 %58 0 1
%131 = OpExtInst %v2int %1 SMin %132 %133
%134 = OpVectorShuffle %v2int %65 %65 0 1
%135 = OpIEqual %v2bool %131 %134
%136 = OpAll %bool %135
OpBranch %130
%130 = OpLabel
%137 = OpPhi %bool %false %124 %136 %129
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%141 = OpVectorShuffle %v3int %44 %44 0 1 2
%142 = OpVectorShuffle %v3int %58 %58 0 1 2
%140 = OpExtInst %v3int %1 SMin %141 %142
%143 = OpVectorShuffle %v3int %65 %65 0 1 2
%144 = OpIEqual %v3bool %140 %143
%145 = OpAll %bool %144
OpBranch %139
%139 = OpLabel
%146 = OpPhi %bool %false %130 %145 %138
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%149 = OpExtInst %v4int %1 SMin %44 %58
%150 = OpIEqual %v4bool %149 %65
%151 = OpAll %bool %150
OpBranch %148
%148 = OpLabel
%152 = OpPhi %bool %false %139 %151 %147
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpIEqual %bool %int_n125 %int_n125
OpBranch %154
%154 = OpLabel
%156 = OpPhi %bool %false %148 %155 %153
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%159 = OpVectorShuffle %v2int %65 %65 0 1
%160 = OpIEqual %v2bool %106 %159
%161 = OpAll %bool %160
OpBranch %158
%158 = OpLabel
%162 = OpPhi %bool %false %154 %161 %157
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpVectorShuffle %v3int %65 %65 0 1 2
%167 = OpIEqual %v3bool %165 %166
%168 = OpAll %bool %167
OpBranch %164
%164 = OpLabel
%169 = OpPhi %bool %false %158 %168 %163
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpIEqual %v4bool %65 %65
%173 = OpAll %bool %172
OpBranch %171
%171 = OpLabel
%174 = OpPhi %bool %false %164 %173 %170
OpSelectionMerge %179 None
OpBranchConditional %174 %177 %178
%177 = OpLabel
%180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%181 = OpLoad %v4float %180
OpStore %175 %181
OpBranch %179
%178 = OpLabel
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%184 = OpLoad %v4float %182
OpStore %175 %184
OpBranch %179
%179 = OpLabel
%185 = OpLoad %v4float %175
OpReturnValue %185
OpFunctionEnd

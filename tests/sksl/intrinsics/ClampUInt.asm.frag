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
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
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
%uint = OpTypeInt 32 0
%v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%float_200 = OpConstant %float 200
%38 = OpConstantComposite %v4float %float_200 %float_200 %float_200 %float_200
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
%75 = OpConstantComposite %v2uint %uint_100 %uint_100
%76 = OpConstantComposite %v2uint %uint_300 %uint_300
%v2bool = OpTypeVector %bool 2
%v3uint = OpTypeVector %uint 3
%87 = OpConstantComposite %v3uint %uint_100 %uint_100 %uint_100
%88 = OpConstantComposite %v3uint %uint_300 %uint_300 %uint_300
%v3bool = OpTypeVector %bool 3
%97 = OpConstantComposite %v4uint %uint_100 %uint_100 %uint_100 %uint_100
%98 = OpConstantComposite %v4uint %uint_300 %uint_300 %uint_300 %uint_300
%v4bool = OpTypeVector %bool 4
%109 = OpConstantComposite %v2uint %uint_100 %uint_200
%116 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_275
%135 = OpConstantComposite %v2uint %uint_100 %uint_0
%136 = OpConstantComposite %v2uint %uint_300 %uint_400
%145 = OpConstantComposite %v3uint %uint_100 %uint_0 %uint_0
%146 = OpConstantComposite %v3uint %uint_300 %uint_400 %uint_250
%169 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_250
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
%179 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %30
%36 = OpVectorTimesScalar %v4float %34 %float_100
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
%68 = OpCompositeExtract %uint %48 0
%67 = OpExtInst %uint %1 UClamp %68 %uint_100 %uint_300
%69 = OpIEqual %bool %67 %uint_100
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpVectorShuffle %v2uint %48 %48 0 1
%72 = OpExtInst %v2uint %1 UClamp %73 %75 %76
%77 = OpVectorShuffle %v2uint %54 %54 0 1
%78 = OpIEqual %v2bool %72 %77
%80 = OpAll %bool %78
OpBranch %71
%71 = OpLabel
%81 = OpPhi %bool %false %25 %80 %70
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpVectorShuffle %v3uint %48 %48 0 1 2
%84 = OpExtInst %v3uint %1 UClamp %85 %87 %88
%89 = OpVectorShuffle %v3uint %54 %54 0 1 2
%90 = OpIEqual %v3bool %84 %89
%92 = OpAll %bool %90
OpBranch %83
%83 = OpLabel
%93 = OpPhi %bool %false %71 %92 %82
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpExtInst %v4uint %1 UClamp %48 %97 %98
%99 = OpIEqual %v4bool %96 %54
%101 = OpAll %bool %99
OpBranch %95
%95 = OpLabel
%102 = OpPhi %bool %false %83 %101 %94
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpIEqual %bool %uint_100 %uint_100
OpBranch %104
%104 = OpLabel
%106 = OpPhi %bool %false %95 %105 %103
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpVectorShuffle %v2uint %54 %54 0 1
%111 = OpIEqual %v2bool %109 %110
%112 = OpAll %bool %111
OpBranch %108
%108 = OpLabel
%113 = OpPhi %bool %false %104 %112 %107
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpVectorShuffle %v3uint %54 %54 0 1 2
%118 = OpIEqual %v3bool %116 %117
%119 = OpAll %bool %118
OpBranch %115
%115 = OpLabel
%120 = OpPhi %bool %false %108 %119 %114
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%123 = OpIEqual %v4bool %54 %54
%124 = OpAll %bool %123
OpBranch %122
%122 = OpLabel
%125 = OpPhi %bool %false %115 %124 %121
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpExtInst %uint %1 UClamp %68 %uint_100 %uint_300
%129 = OpIEqual %bool %128 %uint_100
OpBranch %127
%127 = OpLabel
%130 = OpPhi %bool %false %122 %129 %126
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%134 = OpVectorShuffle %v2uint %48 %48 0 1
%133 = OpExtInst %v2uint %1 UClamp %134 %135 %136
%137 = OpVectorShuffle %v2uint %61 %61 0 1
%138 = OpIEqual %v2bool %133 %137
%139 = OpAll %bool %138
OpBranch %132
%132 = OpLabel
%140 = OpPhi %bool %false %127 %139 %131
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%144 = OpVectorShuffle %v3uint %48 %48 0 1 2
%143 = OpExtInst %v3uint %1 UClamp %144 %145 %146
%147 = OpVectorShuffle %v3uint %61 %61 0 1 2
%148 = OpIEqual %v3bool %143 %147
%149 = OpAll %bool %148
OpBranch %142
%142 = OpLabel
%150 = OpPhi %bool %false %132 %149 %141
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpExtInst %v4uint %1 UClamp %48 %57 %65
%154 = OpIEqual %v4bool %153 %61
%155 = OpAll %bool %154
OpBranch %152
%152 = OpLabel
%156 = OpPhi %bool %false %142 %155 %151
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%159 = OpIEqual %bool %uint_100 %uint_100
OpBranch %158
%158 = OpLabel
%160 = OpPhi %bool %false %152 %159 %157
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpVectorShuffle %v2uint %61 %61 0 1
%164 = OpIEqual %v2bool %109 %163
%165 = OpAll %bool %164
OpBranch %162
%162 = OpLabel
%166 = OpPhi %bool %false %158 %165 %161
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%170 = OpVectorShuffle %v3uint %61 %61 0 1 2
%171 = OpIEqual %v3bool %169 %170
%172 = OpAll %bool %171
OpBranch %168
%168 = OpLabel
%173 = OpPhi %bool %false %162 %172 %167
OpSelectionMerge %175 None
OpBranchConditional %173 %174 %175
%174 = OpLabel
%176 = OpIEqual %v4bool %61 %61
%177 = OpAll %bool %176
OpBranch %175
%175 = OpLabel
%178 = OpPhi %bool %false %168 %177 %174
OpSelectionMerge %183 None
OpBranchConditional %178 %181 %182
%181 = OpLabel
%184 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%186 = OpLoad %v4float %184
OpStore %179 %186
OpBranch %183
%182 = OpLabel
%187 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%189 = OpLoad %v4float %187
OpStore %179 %189
OpBranch %183
%183 = OpLabel
%190 = OpLoad %v4float %179
OpReturnValue %190
OpFunctionEnd

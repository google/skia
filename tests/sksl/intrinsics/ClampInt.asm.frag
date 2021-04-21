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
OpName %intValues "intValues"
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
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
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
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_n100 = OpConstant %int -100
%int_0 = OpConstant %int 0
%int_75 = OpConstant %int 75
%int_100 = OpConstant %int 100
%34 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_100 = OpConstant %float 100
%int_n200 = OpConstant %int -200
%52 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
%int_50 = OpConstant %int 50
%int_225 = OpConstant %int 225
%56 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
%int_200 = OpConstant %int 200
%int_300 = OpConstant %int 300
%60 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
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
%expectedA = OpVariable %_ptr_Function_v4int Function
%intValues = OpVariable %_ptr_Function_v4int Function
%clampLow = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%clampHigh = OpVariable %_ptr_Function_v4int Function
%158 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %34
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %36
%40 = OpVectorTimesScalar %v4float %38 %float_100
%41 = OpCompositeExtract %float %40 0
%42 = OpConvertFToS %int %41
%43 = OpCompositeExtract %float %40 1
%44 = OpConvertFToS %int %43
%45 = OpCompositeExtract %float %40 2
%46 = OpConvertFToS %int %45
%47 = OpCompositeExtract %float %40 3
%48 = OpConvertFToS %int %47
%49 = OpCompositeConstruct %v4int %42 %44 %46 %48
OpStore %intValues %49
OpStore %clampLow %52
OpStore %expectedB %56
OpStore %clampHigh %60
%63 = OpLoad %v4int %intValues
%64 = OpCompositeExtract %int %63 0
%62 = OpExtInst %int %1 SClamp %64 %int_n100 %int_100
%65 = OpLoad %v4int %expectedA
%66 = OpCompositeExtract %int %65 0
%67 = OpIEqual %bool %62 %66
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpLoad %v4int %intValues
%72 = OpVectorShuffle %v2int %71 %71 0 1
%74 = OpCompositeConstruct %v2int %int_n100 %int_n100
%75 = OpCompositeConstruct %v2int %int_100 %int_100
%70 = OpExtInst %v2int %1 SClamp %72 %74 %75
%76 = OpLoad %v4int %expectedA
%77 = OpVectorShuffle %v2int %76 %76 0 1
%78 = OpIEqual %v2bool %70 %77
%80 = OpAll %bool %78
OpBranch %69
%69 = OpLabel
%81 = OpPhi %bool %false %25 %80 %68
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpLoad %v4int %intValues
%86 = OpVectorShuffle %v3int %85 %85 0 1 2
%88 = OpCompositeConstruct %v3int %int_n100 %int_n100 %int_n100
%89 = OpCompositeConstruct %v3int %int_100 %int_100 %int_100
%84 = OpExtInst %v3int %1 SClamp %86 %88 %89
%90 = OpLoad %v4int %expectedA
%91 = OpVectorShuffle %v3int %90 %90 0 1 2
%92 = OpIEqual %v3bool %84 %91
%94 = OpAll %bool %92
OpBranch %83
%83 = OpLabel
%95 = OpPhi %bool %false %69 %94 %82
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpLoad %v4int %intValues
%100 = OpCompositeConstruct %v4int %int_n100 %int_n100 %int_n100 %int_n100
%101 = OpCompositeConstruct %v4int %int_100 %int_100 %int_100 %int_100
%98 = OpExtInst %v4int %1 SClamp %99 %100 %101
%102 = OpLoad %v4int %expectedA
%103 = OpIEqual %v4bool %98 %102
%105 = OpAll %bool %103
OpBranch %97
%97 = OpLabel
%106 = OpPhi %bool %false %83 %105 %96
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpLoad %v4int %intValues
%111 = OpCompositeExtract %int %110 0
%112 = OpLoad %v4int %clampLow
%113 = OpCompositeExtract %int %112 0
%114 = OpLoad %v4int %clampHigh
%115 = OpCompositeExtract %int %114 0
%109 = OpExtInst %int %1 SClamp %111 %113 %115
%116 = OpLoad %v4int %expectedB
%117 = OpCompositeExtract %int %116 0
%118 = OpIEqual %bool %109 %117
OpBranch %108
%108 = OpLabel
%119 = OpPhi %bool %false %97 %118 %107
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpLoad %v4int %intValues
%124 = OpVectorShuffle %v2int %123 %123 0 1
%125 = OpLoad %v4int %clampLow
%126 = OpVectorShuffle %v2int %125 %125 0 1
%127 = OpLoad %v4int %clampHigh
%128 = OpVectorShuffle %v2int %127 %127 0 1
%122 = OpExtInst %v2int %1 SClamp %124 %126 %128
%129 = OpLoad %v4int %expectedB
%130 = OpVectorShuffle %v2int %129 %129 0 1
%131 = OpIEqual %v2bool %122 %130
%132 = OpAll %bool %131
OpBranch %121
%121 = OpLabel
%133 = OpPhi %bool %false %108 %132 %120
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%137 = OpLoad %v4int %intValues
%138 = OpVectorShuffle %v3int %137 %137 0 1 2
%139 = OpLoad %v4int %clampLow
%140 = OpVectorShuffle %v3int %139 %139 0 1 2
%141 = OpLoad %v4int %clampHigh
%142 = OpVectorShuffle %v3int %141 %141 0 1 2
%136 = OpExtInst %v3int %1 SClamp %138 %140 %142
%143 = OpLoad %v4int %expectedB
%144 = OpVectorShuffle %v3int %143 %143 0 1 2
%145 = OpIEqual %v3bool %136 %144
%146 = OpAll %bool %145
OpBranch %135
%135 = OpLabel
%147 = OpPhi %bool %false %121 %146 %134
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%151 = OpLoad %v4int %intValues
%152 = OpLoad %v4int %clampLow
%153 = OpLoad %v4int %clampHigh
%150 = OpExtInst %v4int %1 SClamp %151 %152 %153
%154 = OpLoad %v4int %expectedB
%155 = OpIEqual %v4bool %150 %154
%156 = OpAll %bool %155
OpBranch %149
%149 = OpLabel
%157 = OpPhi %bool %false %135 %156 %148
OpSelectionMerge %162 None
OpBranchConditional %157 %160 %161
%160 = OpLabel
%163 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%165 = OpLoad %v4float %163
OpStore %158 %165
OpBranch %162
%161 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%168 = OpLoad %v4float %166
OpStore %158 %168
OpBranch %162
%162 = OpLabel
%169 = OpLoad %v4float %158
OpReturnValue %169
OpFunctionEnd

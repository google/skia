OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %expectedA "expectedA"
OpName %intValues "intValues"
OpName %clampLow "clampLow"
OpName %expectedB "expectedB"
OpName %clampHigh "clampHigh"
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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_n100 = OpConstant %int -100
%int_0 = OpConstant %int 0
%int_75 = OpConstant %int 75
%int_100 = OpConstant %int 100
%28 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_100 = OpConstant %float 100
%int_n200 = OpConstant %int -200
%46 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
%int_50 = OpConstant %int 50
%int_225 = OpConstant %int 225
%50 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
%int_200 = OpConstant %int 200
%int_300 = OpConstant %int 300
%54 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%expectedA = OpVariable %_ptr_Function_v4int Function
%intValues = OpVariable %_ptr_Function_v4int Function
%clampLow = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%clampHigh = OpVariable %_ptr_Function_v4int Function
%158 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %28
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %30
%34 = OpVectorTimesScalar %v4float %32 %float_100
%35 = OpCompositeExtract %float %34 0
%36 = OpConvertFToS %int %35
%37 = OpCompositeExtract %float %34 1
%38 = OpConvertFToS %int %37
%39 = OpCompositeExtract %float %34 2
%40 = OpConvertFToS %int %39
%41 = OpCompositeExtract %float %34 3
%42 = OpConvertFToS %int %41
%43 = OpCompositeConstruct %v4int %36 %38 %40 %42
OpStore %intValues %43
OpStore %clampLow %46
OpStore %expectedB %50
OpStore %clampHigh %54
%57 = OpLoad %v4int %intValues
%58 = OpCompositeExtract %int %57 0
%56 = OpExtInst %int %1 SClamp %58 %int_n100 %int_100
%59 = OpLoad %v4int %expectedA
%60 = OpCompositeExtract %int %59 0
%61 = OpIEqual %bool %56 %60
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpLoad %v4int %intValues
%66 = OpVectorShuffle %v2int %65 %65 0 1
%68 = OpCompositeConstruct %v2int %int_n100 %int_n100
%69 = OpCompositeConstruct %v2int %int_100 %int_100
%64 = OpExtInst %v2int %1 SClamp %66 %68 %69
%70 = OpLoad %v4int %expectedA
%71 = OpVectorShuffle %v2int %70 %70 0 1
%72 = OpIEqual %v2bool %64 %71
%74 = OpAll %bool %72
OpBranch %63
%63 = OpLabel
%75 = OpPhi %bool %false %19 %74 %62
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpLoad %v4int %intValues
%80 = OpVectorShuffle %v3int %79 %79 0 1 2
%82 = OpCompositeConstruct %v3int %int_n100 %int_n100 %int_n100
%83 = OpCompositeConstruct %v3int %int_100 %int_100 %int_100
%78 = OpExtInst %v3int %1 SClamp %80 %82 %83
%84 = OpLoad %v4int %expectedA
%85 = OpVectorShuffle %v3int %84 %84 0 1 2
%86 = OpIEqual %v3bool %78 %85
%88 = OpAll %bool %86
OpBranch %77
%77 = OpLabel
%89 = OpPhi %bool %false %63 %88 %76
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpLoad %v4int %intValues
%94 = OpVectorShuffle %v4int %93 %93 0 1 2 3
%95 = OpCompositeConstruct %v4int %int_n100 %int_n100 %int_n100 %int_n100
%96 = OpCompositeConstruct %v4int %int_100 %int_100 %int_100 %int_100
%92 = OpExtInst %v4int %1 SClamp %94 %95 %96
%97 = OpLoad %v4int %expectedA
%98 = OpVectorShuffle %v4int %97 %97 0 1 2 3
%99 = OpIEqual %v4bool %92 %98
%101 = OpAll %bool %99
OpBranch %91
%91 = OpLabel
%102 = OpPhi %bool %false %77 %101 %90
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpLoad %v4int %intValues
%107 = OpCompositeExtract %int %106 0
%108 = OpLoad %v4int %clampLow
%109 = OpCompositeExtract %int %108 0
%110 = OpLoad %v4int %clampHigh
%111 = OpCompositeExtract %int %110 0
%105 = OpExtInst %int %1 SClamp %107 %109 %111
%112 = OpLoad %v4int %expectedB
%113 = OpCompositeExtract %int %112 0
%114 = OpIEqual %bool %105 %113
OpBranch %104
%104 = OpLabel
%115 = OpPhi %bool %false %91 %114 %103
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%119 = OpLoad %v4int %intValues
%120 = OpVectorShuffle %v2int %119 %119 0 1
%121 = OpLoad %v4int %clampLow
%122 = OpVectorShuffle %v2int %121 %121 0 1
%123 = OpLoad %v4int %clampHigh
%124 = OpVectorShuffle %v2int %123 %123 0 1
%118 = OpExtInst %v2int %1 SClamp %120 %122 %124
%125 = OpLoad %v4int %expectedB
%126 = OpVectorShuffle %v2int %125 %125 0 1
%127 = OpIEqual %v2bool %118 %126
%128 = OpAll %bool %127
OpBranch %117
%117 = OpLabel
%129 = OpPhi %bool %false %104 %128 %116
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%133 = OpLoad %v4int %intValues
%134 = OpVectorShuffle %v3int %133 %133 0 1 2
%135 = OpLoad %v4int %clampLow
%136 = OpVectorShuffle %v3int %135 %135 0 1 2
%137 = OpLoad %v4int %clampHigh
%138 = OpVectorShuffle %v3int %137 %137 0 1 2
%132 = OpExtInst %v3int %1 SClamp %134 %136 %138
%139 = OpLoad %v4int %expectedB
%140 = OpVectorShuffle %v3int %139 %139 0 1 2
%141 = OpIEqual %v3bool %132 %140
%142 = OpAll %bool %141
OpBranch %131
%131 = OpLabel
%143 = OpPhi %bool %false %117 %142 %130
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%147 = OpLoad %v4int %intValues
%148 = OpVectorShuffle %v4int %147 %147 0 1 2 3
%149 = OpLoad %v4int %clampLow
%150 = OpVectorShuffle %v4int %149 %149 0 1 2 3
%151 = OpLoad %v4int %clampHigh
%152 = OpVectorShuffle %v4int %151 %151 0 1 2 3
%146 = OpExtInst %v4int %1 SClamp %148 %150 %152
%153 = OpLoad %v4int %expectedB
%154 = OpVectorShuffle %v4int %153 %153 0 1 2 3
%155 = OpIEqual %v4bool %146 %154
%156 = OpAll %bool %155
OpBranch %145
%145 = OpLabel
%157 = OpPhi %bool %false %131 %156 %144
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

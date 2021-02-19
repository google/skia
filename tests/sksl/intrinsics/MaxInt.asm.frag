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
OpName %intValues "intValues"
OpName %intGreen "intGreen"
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
OpDecorate %27 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%int_50 = OpConstant %int 50
%int_75 = OpConstant %int 75
%int_225 = OpConstant %int 225
%60 = OpConstantComposite %v4int %int_50 %int_50 %int_75 %int_225
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_100 = OpConstant %int 100
%104 = OpConstantComposite %v4int %int_0 %int_100 %int_75 %int_225
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%intValues = OpVariable %_ptr_Function_v4int Function
%intGreen = OpVariable %_ptr_Function_v4int Function
%138 = OpVariable %_ptr_Function_v4float Function
%24 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %24
%29 = OpVectorTimesScalar %v4float %27 %float_100
%30 = OpCompositeExtract %float %29 0
%31 = OpConvertFToS %int %30
%32 = OpCompositeExtract %float %29 1
%33 = OpConvertFToS %int %32
%34 = OpCompositeExtract %float %29 2
%35 = OpConvertFToS %int %34
%36 = OpCompositeExtract %float %29 3
%37 = OpConvertFToS %int %36
%38 = OpCompositeConstruct %v4int %31 %33 %35 %37
OpStore %intValues %38
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%42 = OpLoad %v4float %40
%43 = OpVectorTimesScalar %v4float %42 %float_100
%44 = OpCompositeExtract %float %43 0
%45 = OpConvertFToS %int %44
%46 = OpCompositeExtract %float %43 1
%47 = OpConvertFToS %int %46
%48 = OpCompositeExtract %float %43 2
%49 = OpConvertFToS %int %48
%50 = OpCompositeExtract %float %43 3
%51 = OpConvertFToS %int %50
%52 = OpCompositeConstruct %v4int %45 %47 %49 %51
OpStore %intGreen %52
%55 = OpLoad %v4int %intValues
%56 = OpCompositeExtract %int %55 0
%54 = OpExtInst %int %1 SMax %56 %int_50
%61 = OpCompositeExtract %int %60 0
%62 = OpIEqual %bool %54 %61
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpLoad %v4int %intValues
%67 = OpVectorShuffle %v2int %66 %66 0 1
%69 = OpCompositeConstruct %v2int %int_50 %int_50
%65 = OpExtInst %v2int %1 SMax %67 %69
%70 = OpVectorShuffle %v2int %60 %60 0 1
%71 = OpIEqual %v2bool %65 %70
%73 = OpAll %bool %71
OpBranch %64
%64 = OpLabel
%74 = OpPhi %bool %false %19 %73 %63
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpLoad %v4int %intValues
%79 = OpVectorShuffle %v3int %78 %78 0 1 2
%81 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%77 = OpExtInst %v3int %1 SMax %79 %81
%82 = OpVectorShuffle %v3int %60 %60 0 1 2
%83 = OpIEqual %v3bool %77 %82
%85 = OpAll %bool %83
OpBranch %76
%76 = OpLabel
%86 = OpPhi %bool %false %64 %85 %75
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpLoad %v4int %intValues
%91 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%89 = OpExtInst %v4int %1 SMax %90 %91
%92 = OpIEqual %v4bool %89 %60
%94 = OpAll %bool %92
OpBranch %88
%88 = OpLabel
%95 = OpPhi %bool %false %76 %94 %87
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpLoad %v4int %intValues
%100 = OpCompositeExtract %int %99 0
%101 = OpLoad %v4int %intGreen
%102 = OpCompositeExtract %int %101 0
%98 = OpExtInst %int %1 SMax %100 %102
%105 = OpCompositeExtract %int %104 0
%106 = OpIEqual %bool %98 %105
OpBranch %97
%97 = OpLabel
%107 = OpPhi %bool %false %88 %106 %96
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%111 = OpLoad %v4int %intValues
%112 = OpVectorShuffle %v2int %111 %111 0 1
%113 = OpLoad %v4int %intGreen
%114 = OpVectorShuffle %v2int %113 %113 0 1
%110 = OpExtInst %v2int %1 SMax %112 %114
%115 = OpVectorShuffle %v2int %104 %104 0 1
%116 = OpIEqual %v2bool %110 %115
%117 = OpAll %bool %116
OpBranch %109
%109 = OpLabel
%118 = OpPhi %bool %false %97 %117 %108
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%122 = OpLoad %v4int %intValues
%123 = OpVectorShuffle %v3int %122 %122 0 1 2
%124 = OpLoad %v4int %intGreen
%125 = OpVectorShuffle %v3int %124 %124 0 1 2
%121 = OpExtInst %v3int %1 SMax %123 %125
%126 = OpVectorShuffle %v3int %104 %104 0 1 2
%127 = OpIEqual %v3bool %121 %126
%128 = OpAll %bool %127
OpBranch %120
%120 = OpLabel
%129 = OpPhi %bool %false %109 %128 %119
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%133 = OpLoad %v4int %intValues
%134 = OpLoad %v4int %intGreen
%132 = OpExtInst %v4int %1 SMax %133 %134
%135 = OpIEqual %v4bool %132 %104
%136 = OpAll %bool %135
OpBranch %131
%131 = OpLabel
%137 = OpPhi %bool %false %120 %136 %130
OpSelectionMerge %142 None
OpBranchConditional %137 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%144 = OpLoad %v4float %143
OpStore %138 %144
OpBranch %142
%141 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%147 = OpLoad %v4float %145
OpStore %138 %147
OpBranch %142
%142 = OpLabel
%148 = OpLoad %v4float %138
OpReturnValue %148
OpFunctionEnd

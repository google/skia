OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "N"
OpMemberName %_UniformBuffer 1 "I"
OpMemberName %_UniformBuffer 2 "NRef"
OpMemberName %_UniformBuffer 3 "colorGreen"
OpMemberName %_UniformBuffer 4 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedPos "expectedPos"
OpName %expectedNeg "expectedNeg"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expectedPos RelaxedPrecision
OpDecorate %expectedNeg RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%32 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%38 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%true = OpConstantTrue %bool
%110 = OpConstantComposite %v2float %float_n1 %float_n2
%117 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
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
%expectedPos = OpVariable %_ptr_Function_v4float Function
%expectedNeg = OpVariable %_ptr_Function_v4float Function
%125 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedPos %32
OpStore %expectedNeg %38
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%45 = OpLoad %v4float %41
%46 = OpCompositeExtract %float %45 0
%47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%49 = OpLoad %v4float %47
%50 = OpCompositeExtract %float %49 0
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%53 = OpLoad %v4float %51
%54 = OpCompositeExtract %float %53 0
%40 = OpExtInst %float %1 FaceForward %46 %50 %54
%55 = OpFOrdEqual %bool %40 %float_n1
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %59
%61 = OpVectorShuffle %v2float %60 %60 0 1
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v2float %63 %63 0 1
%65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v2float %66 %66 0 1
%58 = OpExtInst %v2float %1 FaceForward %61 %64 %67
%68 = OpVectorShuffle %v2float %38 %38 0 1
%69 = OpFOrdEqual %v2bool %58 %68
%71 = OpAll %bool %69
OpBranch %57
%57 = OpLabel
%72 = OpPhi %bool %false %25 %71 %56
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%78 = OpVectorShuffle %v3float %77 %77 0 1 2
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%81 = OpLoad %v4float %80
%82 = OpVectorShuffle %v3float %81 %81 0 1 2
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%84 = OpLoad %v4float %83
%85 = OpVectorShuffle %v3float %84 %84 0 1 2
%75 = OpExtInst %v3float %1 FaceForward %78 %82 %85
%86 = OpVectorShuffle %v3float %32 %32 0 1 2
%87 = OpFOrdEqual %v3bool %75 %86
%89 = OpAll %bool %87
OpBranch %74
%74 = OpLabel
%90 = OpPhi %bool %false %57 %89 %73
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%95 = OpLoad %v4float %94
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%97 = OpLoad %v4float %96
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%99 = OpLoad %v4float %98
%93 = OpExtInst %v4float %1 FaceForward %95 %97 %99
%100 = OpFOrdEqual %v4bool %93 %32
%102 = OpAll %bool %100
OpBranch %92
%92 = OpLabel
%103 = OpPhi %bool %false %74 %102 %91
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
OpBranch %105
%105 = OpLabel
%107 = OpPhi %bool %false %92 %true %104
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%111 = OpVectorShuffle %v2float %38 %38 0 1
%112 = OpFOrdEqual %v2bool %110 %111
%113 = OpAll %bool %112
OpBranch %109
%109 = OpLabel
%114 = OpPhi %bool %false %105 %113 %108
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpVectorShuffle %v3float %32 %32 0 1 2
%119 = OpFOrdEqual %v3bool %117 %118
%120 = OpAll %bool %119
OpBranch %116
%116 = OpLabel
%121 = OpPhi %bool %false %109 %120 %115
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
OpBranch %123
%123 = OpLabel
%124 = OpPhi %bool %false %116 %true %122
OpSelectionMerge %128 None
OpBranchConditional %124 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%131 = OpLoad %v4float %129
OpStore %125 %131
OpBranch %128
%127 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%134 = OpLoad %v4float %132
OpStore %125 %134
OpBranch %128
%128 = OpLabel
%135 = OpLoad %v4float %125
OpReturnValue %135
OpFunctionEnd

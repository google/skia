OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "I"
OpMemberName %_UniformBuffer 1 "N"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedX "expectedX"
OpName %expectedXY "expectedXY"
OpName %expectedXYZ "expectedXYZ"
OpName %expectedXYZW "expectedXYZW"
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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expectedX RelaxedPrecision
OpDecorate %expectedXY RelaxedPrecision
OpDecorate %expectedXYZ RelaxedPrecision
OpDecorate %expectedXYZW RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_n49 = OpConstant %float -49
%float_n169 = OpConstant %float -169
%float_202 = OpConstant %float 202
%32 = OpConstantComposite %v2float %float_n169 %float_202
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_n379 = OpConstant %float -379
%float_454 = OpConstant %float 454
%float_n529 = OpConstant %float -529
%39 = OpConstantComposite %v3float %float_n379 %float_454 %float_n529
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n699 = OpConstant %float -699
%float_838 = OpConstant %float 838
%float_n977 = OpConstant %float -977
%float_1116 = OpConstant %float 1116
%46 = OpConstantComposite %v4float %float_n699 %float_838 %float_n977 %float_1116
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%expectedX = OpVariable %_ptr_Function_float Function
%expectedXY = OpVariable %_ptr_Function_v2float Function
%expectedXYZ = OpVariable %_ptr_Function_v3float Function
%expectedXYZW = OpVariable %_ptr_Function_v4float Function
%116 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedX %float_n49
OpStore %expectedXY %32
OpStore %expectedXYZ %39
OpStore %expectedXYZW %46
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %49
%54 = OpCompositeExtract %float %53 0
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%57 = OpLoad %v4float %55
%58 = OpCompositeExtract %float %57 0
%48 = OpExtInst %float %1 Reflect %54 %58
%59 = OpFOrdEqual %bool %48 %float_n49
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%64 = OpLoad %v4float %63
%65 = OpVectorShuffle %v2float %64 %64 0 1
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v2float %67 %67 0 1
%62 = OpExtInst %v2float %1 Reflect %65 %68
%69 = OpFOrdEqual %v2bool %62 %32
%71 = OpAll %bool %69
OpBranch %61
%61 = OpLabel
%72 = OpPhi %bool %false %25 %71 %60
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%78 = OpVectorShuffle %v3float %77 %77 0 1 2
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%80 = OpLoad %v4float %79
%81 = OpVectorShuffle %v3float %80 %80 0 1 2
%75 = OpExtInst %v3float %1 Reflect %78 %81
%82 = OpFOrdEqual %v3bool %75 %39
%84 = OpAll %bool %82
OpBranch %74
%74 = OpLabel
%85 = OpPhi %bool %false %61 %84 %73
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%90 = OpLoad %v4float %89
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%92 = OpLoad %v4float %91
%88 = OpExtInst %v4float %1 Reflect %90 %92
%93 = OpFOrdEqual %v4bool %88 %46
%95 = OpAll %bool %93
OpBranch %87
%87 = OpLabel
%96 = OpPhi %bool %false %74 %95 %86
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpFOrdEqual %bool %float_n49 %float_n49
OpBranch %98
%98 = OpLabel
%100 = OpPhi %bool %false %87 %99 %97
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpFOrdEqual %v2bool %32 %32
%104 = OpAll %bool %103
OpBranch %102
%102 = OpLabel
%105 = OpPhi %bool %false %98 %104 %101
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpFOrdEqual %v3bool %39 %39
%109 = OpAll %bool %108
OpBranch %107
%107 = OpLabel
%110 = OpPhi %bool %false %102 %109 %106
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpFOrdEqual %v4bool %46 %46
%114 = OpAll %bool %113
OpBranch %112
%112 = OpLabel
%115 = OpPhi %bool %false %107 %114 %111
OpSelectionMerge %119 None
OpBranchConditional %115 %117 %118
%117 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%122 = OpLoad %v4float %120
OpStore %116 %122
OpBranch %119
%118 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%125 = OpLoad %v4float %123
OpStore %116 %125
OpBranch %119
%119 = OpLabel
%126 = OpLoad %v4float %116
OpReturnValue %126
OpFunctionEnd

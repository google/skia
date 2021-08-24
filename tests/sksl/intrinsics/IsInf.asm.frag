OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testMatrix2x2"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %infiniteValue "infiniteValue"
OpName %finiteValue "finiteValue"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 32
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 48
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %infiniteValue RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %finiteValue RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%infiniteValue = OpVariable %_ptr_Function_v4float Function
%finiteValue = OpVariable %_ptr_Function_v4float Function
%118 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
%39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%42 = OpLoad %v4float %39
%43 = OpCompositeExtract %float %42 0
%45 = OpFDiv %float %float_1 %43
%46 = OpVectorTimesScalar %v4float %38 %45
OpStore %infiniteValue %46
%48 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%49 = OpLoad %mat2v2float %48
%50 = OpCompositeExtract %float %49 0 0
%51 = OpCompositeExtract %float %49 0 1
%52 = OpCompositeExtract %float %49 1 0
%53 = OpCompositeExtract %float %49 1 1
%54 = OpCompositeConstruct %v4float %50 %51 %52 %53
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%56 = OpLoad %v4float %55
%57 = OpCompositeExtract %float %56 1
%58 = OpFDiv %float %float_1 %57
%59 = OpVectorTimesScalar %v4float %54 %58
OpStore %finiteValue %59
%62 = OpLoad %v4float %infiniteValue
%63 = OpCompositeExtract %float %62 0
%61 = OpIsInf %bool %63
OpSelectionMerge %65 None
OpBranchConditional %61 %64 %65
%64 = OpLabel
%68 = OpLoad %v4float %infiniteValue
%69 = OpVectorShuffle %v2float %68 %68 0 1
%67 = OpIsInf %v2bool %69
%66 = OpAll %bool %67
OpBranch %65
%65 = OpLabel
%71 = OpPhi %bool %false %26 %66 %64
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%76 = OpLoad %v4float %infiniteValue
%77 = OpVectorShuffle %v3float %76 %76 0 1 2
%75 = OpIsInf %v3bool %77
%74 = OpAll %bool %75
OpBranch %73
%73 = OpLabel
%80 = OpPhi %bool %false %65 %74 %72
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%85 = OpLoad %v4float %infiniteValue
%84 = OpIsInf %v4bool %85
%83 = OpAll %bool %84
OpBranch %82
%82 = OpLabel
%87 = OpPhi %bool %false %73 %83 %81
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%92 = OpLoad %v4float %finiteValue
%93 = OpCompositeExtract %float %92 0
%91 = OpIsInf %bool %93
%90 = OpLogicalNot %bool %91
OpBranch %89
%89 = OpLabel
%94 = OpPhi %bool %false %82 %90 %88
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%100 = OpLoad %v4float %finiteValue
%101 = OpVectorShuffle %v2float %100 %100 0 1
%99 = OpIsInf %v2bool %101
%98 = OpAny %bool %99
%97 = OpLogicalNot %bool %98
OpBranch %96
%96 = OpLabel
%102 = OpPhi %bool %false %89 %97 %95
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%108 = OpLoad %v4float %finiteValue
%109 = OpVectorShuffle %v3float %108 %108 0 1 2
%107 = OpIsInf %v3bool %109
%106 = OpAny %bool %107
%105 = OpLogicalNot %bool %106
OpBranch %104
%104 = OpLabel
%110 = OpPhi %bool %false %96 %105 %103
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%116 = OpLoad %v4float %finiteValue
%115 = OpIsInf %v4bool %116
%114 = OpAny %bool %115
%113 = OpLogicalNot %bool %114
OpBranch %112
%112 = OpLabel
%117 = OpPhi %bool %false %104 %113 %111
OpSelectionMerge %121 None
OpBranchConditional %117 %119 %120
%119 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%123 = OpLoad %v4float %122
OpStore %118 %123
OpBranch %121
%120 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%126 = OpLoad %v4float %124
OpStore %118 %126
OpBranch %121
%121 = OpLabel
%127 = OpLoad %v4float %118
OpReturnValue %127
OpFunctionEnd

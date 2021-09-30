OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputH4"
OpMemberName %_UniformBuffer 1 "expectedH4"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inputVal "inputVal"
OpName %expected "expected"
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
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%true = OpConstantTrue %bool
%87 = OpConstantComposite %v2bool %false %true
%95 = OpConstantComposite %v3bool %false %true %false
%103 = OpConstantComposite %v4bool %false %true %false %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%inputVal = OpVariable %_ptr_Function_v4bool Function
%expected = OpVariable %_ptr_Function_v4bool Function
%108 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %29
%34 = OpCompositeExtract %float %33 0
%35 = OpFUnordNotEqual %bool %34 %float_0
%36 = OpCompositeExtract %float %33 1
%37 = OpFUnordNotEqual %bool %36 %float_0
%38 = OpCompositeExtract %float %33 2
%39 = OpFUnordNotEqual %bool %38 %float_0
%40 = OpCompositeExtract %float %33 3
%41 = OpFUnordNotEqual %bool %40 %float_0
%42 = OpCompositeConstruct %v4bool %35 %37 %39 %41
OpStore %inputVal %42
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%46 = OpLoad %v4float %44
%47 = OpCompositeExtract %float %46 0
%48 = OpFUnordNotEqual %bool %47 %float_0
%49 = OpCompositeExtract %float %46 1
%50 = OpFUnordNotEqual %bool %49 %float_0
%51 = OpCompositeExtract %float %46 2
%52 = OpFUnordNotEqual %bool %51 %float_0
%53 = OpCompositeExtract %float %46 3
%54 = OpFUnordNotEqual %bool %53 %float_0
%55 = OpCompositeConstruct %v4bool %48 %50 %52 %54
OpStore %expected %55
%58 = OpLoad %v4bool %inputVal
%59 = OpVectorShuffle %v2bool %58 %58 0 1
%57 = OpLogicalNot %v2bool %59
%61 = OpLoad %v4bool %expected
%62 = OpVectorShuffle %v2bool %61 %61 0 1
%63 = OpLogicalEqual %v2bool %57 %62
%64 = OpAll %bool %63
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpLoad %v4bool %inputVal
%69 = OpVectorShuffle %v3bool %68 %68 0 1 2
%67 = OpLogicalNot %v3bool %69
%71 = OpLoad %v4bool %expected
%72 = OpVectorShuffle %v3bool %71 %71 0 1 2
%73 = OpLogicalEqual %v3bool %67 %72
%74 = OpAll %bool %73
OpBranch %66
%66 = OpLabel
%75 = OpPhi %bool %false %25 %74 %65
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpLoad %v4bool %inputVal
%78 = OpLogicalNot %v4bool %79
%80 = OpLoad %v4bool %expected
%81 = OpLogicalEqual %v4bool %78 %80
%82 = OpAll %bool %81
OpBranch %77
%77 = OpLabel
%83 = OpPhi %bool %false %66 %82 %76
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%88 = OpLoad %v4bool %expected
%89 = OpVectorShuffle %v2bool %88 %88 0 1
%90 = OpLogicalEqual %v2bool %87 %89
%91 = OpAll %bool %90
OpBranch %85
%85 = OpLabel
%92 = OpPhi %bool %false %77 %91 %84
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLoad %v4bool %expected
%97 = OpVectorShuffle %v3bool %96 %96 0 1 2
%98 = OpLogicalEqual %v3bool %95 %97
%99 = OpAll %bool %98
OpBranch %94
%94 = OpLabel
%100 = OpPhi %bool %false %85 %99 %93
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpLoad %v4bool %expected
%105 = OpLogicalEqual %v4bool %103 %104
%106 = OpAll %bool %105
OpBranch %102
%102 = OpLabel
%107 = OpPhi %bool %false %94 %106 %101
OpSelectionMerge %112 None
OpBranchConditional %107 %110 %111
%110 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%115 = OpLoad %v4float %113
OpStore %108 %115
OpBranch %112
%111 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%118 = OpLoad %v4float %116
OpStore %108 %118
OpBranch %112
%112 = OpLabel
%119 = OpLoad %v4float %108
OpReturnValue %119
OpFunctionEnd

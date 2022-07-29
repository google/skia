OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputH4"
OpMemberName %_UniformBuffer 1 "expectedH4"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inputVal "inputVal"
OpName %expected "expected"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
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
%81 = OpConstantComposite %v2bool %false %true
%88 = OpConstantComposite %v3bool %false %true %false
%95 = OpConstantComposite %v4bool %false %true %false %true
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
%99 = OpVariable %_ptr_Function_v4float Function
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
%58 = OpVectorShuffle %v2bool %42 %42 0 1
%57 = OpLogicalNot %v2bool %58
%60 = OpVectorShuffle %v2bool %55 %55 0 1
%61 = OpLogicalEqual %v2bool %57 %60
%62 = OpAll %bool %61
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpVectorShuffle %v3bool %42 %42 0 1 2
%65 = OpLogicalNot %v3bool %66
%68 = OpVectorShuffle %v3bool %55 %55 0 1 2
%69 = OpLogicalEqual %v3bool %65 %68
%70 = OpAll %bool %69
OpBranch %64
%64 = OpLabel
%71 = OpPhi %bool %false %25 %70 %63
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpLogicalNot %v4bool %42
%75 = OpLogicalEqual %v4bool %74 %55
%76 = OpAll %bool %75
OpBranch %73
%73 = OpLabel
%77 = OpPhi %bool %false %64 %76 %72
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%82 = OpVectorShuffle %v2bool %55 %55 0 1
%83 = OpLogicalEqual %v2bool %81 %82
%84 = OpAll %bool %83
OpBranch %79
%79 = OpLabel
%85 = OpPhi %bool %false %73 %84 %78
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpVectorShuffle %v3bool %55 %55 0 1 2
%90 = OpLogicalEqual %v3bool %88 %89
%91 = OpAll %bool %90
OpBranch %87
%87 = OpLabel
%92 = OpPhi %bool %false %79 %91 %86
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLogicalEqual %v4bool %95 %55
%97 = OpAll %bool %96
OpBranch %94
%94 = OpLabel
%98 = OpPhi %bool %false %87 %97 %93
OpSelectionMerge %103 None
OpBranchConditional %98 %101 %102
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%106 = OpLoad %v4float %104
OpStore %99 %106
OpBranch %103
%102 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%109 = OpLoad %v4float %107
OpStore %99 %109
OpBranch %103
%103 = OpLabel
%110 = OpLoad %v4float %99
OpReturnValue %110
OpFunctionEnd

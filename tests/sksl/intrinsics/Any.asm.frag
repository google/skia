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
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
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
%88 = OpVariable %_ptr_Function_v4float Function
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
%57 = OpAny %bool %58
%60 = OpCompositeExtract %bool %55 0
%61 = OpLogicalEqual %bool %57 %60
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpVectorShuffle %v3bool %42 %42 0 1 2
%64 = OpAny %bool %65
%67 = OpCompositeExtract %bool %55 1
%68 = OpLogicalEqual %bool %64 %67
OpBranch %63
%63 = OpLabel
%69 = OpPhi %bool %false %25 %68 %62
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpAny %bool %42
%73 = OpCompositeExtract %bool %55 2
%74 = OpLogicalEqual %bool %72 %73
OpBranch %71
%71 = OpLabel
%75 = OpPhi %bool %false %63 %74 %70
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpLogicalEqual %bool %false %60
OpBranch %77
%77 = OpLabel
%79 = OpPhi %bool %false %71 %78 %76
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpCompositeExtract %bool %55 1
OpBranch %81
%81 = OpLabel
%83 = OpPhi %bool %false %77 %82 %80
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpCompositeExtract %bool %55 2
OpBranch %85
%85 = OpLabel
%87 = OpPhi %bool %false %81 %86 %84
OpSelectionMerge %92 None
OpBranchConditional %87 %90 %91
%90 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%95 = OpLoad %v4float %93
OpStore %88 %95
OpBranch %92
%91 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%98 = OpLoad %v4float %96
OpStore %88 %98
OpBranch %92
%92 = OpLabel
%99 = OpLoad %v4float %88
OpReturnValue %99
OpFunctionEnd

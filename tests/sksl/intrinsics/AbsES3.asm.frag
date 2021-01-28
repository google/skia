OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "minus1234"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
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
OpDecorate %26 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
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
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2float = OpTypeVector %float 2
%v2int = OpTypeVector %int 2
%int_2 = OpConstant %int 2
%45 = OpConstantComposite %v2int %int_1 %int_2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%int_3 = OpConstant %int 3
%66 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%v3bool = OpTypeVector %bool 3
%v4int = OpTypeVector %int 4
%int_4 = OpConstant %int 4
%87 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%92 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%28 = OpConvertFToS %int %27
%21 = OpExtInst %int %1 SAbs %28
%30 = OpIEqual %bool %21 %int_1
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%35 = OpLoad %v4float %34
%36 = OpVectorShuffle %v2float %35 %35 0 1
%38 = OpCompositeExtract %float %36 0
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %36 1
%41 = OpConvertFToS %int %40
%42 = OpCompositeConstruct %v2int %39 %41
%33 = OpExtInst %v2int %1 SAbs %42
%46 = OpIEqual %v2bool %33 %45
%48 = OpAll %bool %46
OpBranch %32
%32 = OpLabel
%49 = OpPhi %bool %false %19 %48 %31
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpVectorShuffle %v3float %54 %54 0 1 2
%57 = OpCompositeExtract %float %55 0
%58 = OpConvertFToS %int %57
%59 = OpCompositeExtract %float %55 1
%60 = OpConvertFToS %int %59
%61 = OpCompositeExtract %float %55 2
%62 = OpConvertFToS %int %61
%63 = OpCompositeConstruct %v3int %58 %60 %62
%52 = OpExtInst %v3int %1 SAbs %63
%67 = OpIEqual %v3bool %52 %66
%69 = OpAll %bool %67
OpBranch %51
%51 = OpLabel
%70 = OpPhi %bool %false %32 %69 %50
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%75 = OpLoad %v4float %74
%76 = OpCompositeExtract %float %75 0
%77 = OpConvertFToS %int %76
%78 = OpCompositeExtract %float %75 1
%79 = OpConvertFToS %int %78
%80 = OpCompositeExtract %float %75 2
%81 = OpConvertFToS %int %80
%82 = OpCompositeExtract %float %75 3
%83 = OpConvertFToS %int %82
%84 = OpCompositeConstruct %v4int %77 %79 %81 %83
%73 = OpExtInst %v4int %1 SAbs %84
%88 = OpIEqual %v4bool %73 %87
%90 = OpAll %bool %88
OpBranch %72
%72 = OpLabel
%91 = OpPhi %bool %false %51 %90 %71
OpSelectionMerge %96 None
OpBranchConditional %91 %94 %95
%94 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%98 = OpLoad %v4float %97
OpStore %92 %98
OpBranch %96
%95 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%100 = OpLoad %v4float %99
OpStore %92 %100
OpBranch %96
%96 = OpLabel
%101 = OpLoad %v4float %92
OpReturnValue %101
OpFunctionEnd

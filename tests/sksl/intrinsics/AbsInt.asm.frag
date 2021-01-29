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
OpDecorate %53 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
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
%44 = OpConstantComposite %v2int %int_1 %int_0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%64 = OpConstantComposite %v3int %int_1 %int_0 %int_0
%v3bool = OpTypeVector %bool 3
%v4int = OpTypeVector %int 4
%int_2 = OpConstant %int 2
%85 = OpConstantComposite %v4int %int_1 %int_0 %int_0 %int_2
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
%90 = OpVariable %_ptr_Function_v4float Function
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
%45 = OpIEqual %v2bool %33 %44
%47 = OpAll %bool %45
OpBranch %32
%32 = OpLabel
%48 = OpPhi %bool %false %19 %47 %31
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v3float %53 %53 0 1 2
%56 = OpCompositeExtract %float %54 0
%57 = OpConvertFToS %int %56
%58 = OpCompositeExtract %float %54 1
%59 = OpConvertFToS %int %58
%60 = OpCompositeExtract %float %54 2
%61 = OpConvertFToS %int %60
%62 = OpCompositeConstruct %v3int %57 %59 %61
%51 = OpExtInst %v3int %1 SAbs %62
%65 = OpIEqual %v3bool %51 %64
%67 = OpAll %bool %65
OpBranch %50
%50 = OpLabel
%68 = OpPhi %bool %false %32 %67 %49
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%73 = OpLoad %v4float %72
%74 = OpCompositeExtract %float %73 0
%75 = OpConvertFToS %int %74
%76 = OpCompositeExtract %float %73 1
%77 = OpConvertFToS %int %76
%78 = OpCompositeExtract %float %73 2
%79 = OpConvertFToS %int %78
%80 = OpCompositeExtract %float %73 3
%81 = OpConvertFToS %int %80
%82 = OpCompositeConstruct %v4int %75 %77 %79 %81
%71 = OpExtInst %v4int %1 SAbs %82
%86 = OpIEqual %v4bool %71 %85
%88 = OpAll %bool %86
OpBranch %70
%70 = OpLabel
%89 = OpPhi %bool %false %50 %88 %69
OpSelectionMerge %94 None
OpBranchConditional %89 %92 %93
%92 = OpLabel
%95 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%96 = OpLoad %v4float %95
OpStore %90 %96
OpBranch %94
%93 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%98 = OpLoad %v4float %97
OpStore %90 %98
OpBranch %94
%94 = OpLabel
%99 = OpLoad %v4float %90
OpReturnValue %99
OpFunctionEnd

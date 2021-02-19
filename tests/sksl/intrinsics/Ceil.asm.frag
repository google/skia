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
OpDecorate %39 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_3 = OpConstant %float 3
%32 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_3
%v2float = OpTypeVector %float 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
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
%68 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%21 = OpExtInst %float %1 Ceil %27
%33 = OpCompositeExtract %float %32 0
%34 = OpFOrdEqual %bool %21 %33
OpSelectionMerge %36 None
OpBranchConditional %34 %35 %36
%35 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %38
%40 = OpVectorShuffle %v2float %39 %39 0 1
%37 = OpExtInst %v2float %1 Ceil %40
%42 = OpVectorShuffle %v2float %32 %32 0 1
%43 = OpFOrdEqual %v2bool %37 %42
%45 = OpAll %bool %43
OpBranch %36
%36 = OpLabel
%46 = OpPhi %bool %false %19 %45 %35
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v3float %51 %51 0 1 2
%49 = OpExtInst %v3float %1 Ceil %52
%54 = OpVectorShuffle %v3float %32 %32 0 1 2
%55 = OpFOrdEqual %v3bool %49 %54
%57 = OpAll %bool %55
OpBranch %48
%48 = OpLabel
%58 = OpPhi %bool %false %36 %57 %47
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%63 = OpLoad %v4float %62
%61 = OpExtInst %v4float %1 Ceil %63
%64 = OpFOrdEqual %v4bool %61 %32
%66 = OpAll %bool %64
OpBranch %60
%60 = OpLabel
%67 = OpPhi %bool %false %48 %66 %59
OpSelectionMerge %72 None
OpBranchConditional %67 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%75 = OpLoad %v4float %73
OpStore %68 %75
OpBranch %72
%71 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%78 = OpLoad %v4float %76
OpStore %68 %78
OpBranch %72
%72 = OpLabel
%79 = OpLoad %v4float %68
OpReturnValue %79
OpFunctionEnd

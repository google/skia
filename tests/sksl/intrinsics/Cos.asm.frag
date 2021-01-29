OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "pi"
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
OpDecorate %27 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
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
%float_0_00100000005 = OpConstant %float 0.00100000005
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%float_n1 = OpConstant %float -1
%43 = OpConstantComposite %v2float %float_0 %float_n1
%45 = OpConstantComposite %v2float %float_0_00100000005 %float_0_00100000005
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%58 = OpConstantComposite %v3float %float_0 %float_n1 %float_0
%60 = OpConstantComposite %v3float %float_0_00100000005 %float_0_00100000005 %float_0_00100000005
%v3bool = OpTypeVector %bool 3
%float_1 = OpConstant %float 1
%72 = OpConstantComposite %v4float %float_0 %float_n1 %float_0 %float_1
%74 = OpConstantComposite %v4float %float_0_00100000005 %float_0_00100000005 %float_0_00100000005 %float_0_00100000005
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
%77 = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %23
%28 = OpCompositeExtract %float %27 0
%22 = OpExtInst %float %1 Cos %28
%21 = OpExtInst %float %1 FAbs %22
%30 = OpFOrdLessThan %bool %21 %float_0_00100000005
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %37
%39 = OpVectorShuffle %v2float %38 %38 0 1
%36 = OpExtInst %v2float %1 Cos %39
%44 = OpFSub %v2float %36 %43
%35 = OpExtInst %v2float %1 FAbs %44
%34 = OpFOrdLessThan %v2bool %35 %45
%33 = OpAll %bool %34
OpBranch %32
%32 = OpLabel
%47 = OpPhi %bool %false %19 %33 %31
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%55 = OpLoad %v4float %54
%56 = OpVectorShuffle %v3float %55 %55 0 1 2
%53 = OpExtInst %v3float %1 Cos %56
%59 = OpFSub %v3float %53 %58
%52 = OpExtInst %v3float %1 FAbs %59
%51 = OpFOrdLessThan %v3bool %52 %60
%50 = OpAll %bool %51
OpBranch %49
%49 = OpLabel
%62 = OpPhi %bool %false %32 %50 %48
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%70 = OpLoad %v4float %69
%68 = OpExtInst %v4float %1 Cos %70
%73 = OpFSub %v4float %68 %72
%67 = OpExtInst %v4float %1 FAbs %73
%66 = OpFOrdLessThan %v4bool %67 %74
%65 = OpAll %bool %66
OpBranch %64
%64 = OpLabel
%76 = OpPhi %bool %false %49 %65 %63
OpSelectionMerge %81 None
OpBranchConditional %76 %79 %80
%79 = OpLabel
%82 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%84 = OpLoad %v4float %82
OpStore %77 %84
OpBranch %81
%80 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%87 = OpLoad %v4float %85
OpStore %77 %87
OpBranch %81
%81 = OpLabel
%88 = OpLoad %v4float %77
OpReturnValue %88
OpFunctionEnd

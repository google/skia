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
OpDecorate %30 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_0_00100000005 = OpConstant %float 0.00100000005
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%44 = OpConstantComposite %v2float %float_1 %float_0
%46 = OpConstantComposite %v2float %float_0_00100000005 %float_0_00100000005
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%float_n1 = OpConstant %float -1
%60 = OpConstantComposite %v3float %float_1 %float_0 %float_n1
%62 = OpConstantComposite %v3float %float_0_00100000005 %float_0_00100000005 %float_0_00100000005
%v3bool = OpTypeVector %bool 3
%73 = OpConstantComposite %v4float %float_1 %float_0 %float_n1 %float_0
%75 = OpConstantComposite %v4float %float_0_00100000005 %float_0_00100000005 %float_0_00100000005 %float_0_00100000005
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
%78 = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %23
%28 = OpCompositeExtract %float %27 0
%22 = OpExtInst %float %1 Sin %28
%30 = OpFSub %float %22 %float_1
%21 = OpExtInst %float %1 FAbs %30
%32 = OpFOrdLessThan %bool %21 %float_0_00100000005
OpSelectionMerge %34 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
%39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %39
%41 = OpVectorShuffle %v2float %40 %40 0 1
%38 = OpExtInst %v2float %1 Sin %41
%45 = OpFSub %v2float %38 %44
%37 = OpExtInst %v2float %1 FAbs %45
%36 = OpFOrdLessThan %v2bool %37 %46
%35 = OpAll %bool %36
OpBranch %34
%34 = OpLabel
%48 = OpPhi %bool %false %19 %35 %33
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%56 = OpLoad %v4float %55
%57 = OpVectorShuffle %v3float %56 %56 0 1 2
%54 = OpExtInst %v3float %1 Sin %57
%61 = OpFSub %v3float %54 %60
%53 = OpExtInst %v3float %1 FAbs %61
%52 = OpFOrdLessThan %v3bool %53 %62
%51 = OpAll %bool %52
OpBranch %50
%50 = OpLabel
%64 = OpPhi %bool %false %34 %51 %49
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%70 = OpExtInst %v4float %1 Sin %72
%74 = OpFSub %v4float %70 %73
%69 = OpExtInst %v4float %1 FAbs %74
%68 = OpFOrdLessThan %v4bool %69 %75
%67 = OpAll %bool %68
OpBranch %66
%66 = OpLabel
%77 = OpPhi %bool %false %50 %67 %65
OpSelectionMerge %82 None
OpBranchConditional %77 %80 %81
%80 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%85 = OpLoad %v4float %83
OpStore %78 %85
OpBranch %82
%81 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%88 = OpLoad %v4float %86
OpStore %78 %88
OpBranch %82
%82 = OpLabel
%89 = OpLoad %v4float %78
OpReturnValue %89
OpFunctionEnd

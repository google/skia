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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%25 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2float = OpTypeVector %float 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
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
%expected = OpVariable %_ptr_Function_v4float Function
%73 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %25
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpCompositeExtract %float %32 0
%27 = OpExtInst %float %1 FSign %33
%34 = OpLoad %v4float %expected
%35 = OpCompositeExtract %float %34 0
%36 = OpFOrdEqual %bool %27 %35
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %40
%42 = OpVectorShuffle %v2float %41 %41 0 1
%39 = OpExtInst %v2float %1 FSign %42
%44 = OpLoad %v4float %expected
%45 = OpVectorShuffle %v2float %44 %44 0 1
%46 = OpFOrdEqual %v2bool %39 %45
%48 = OpAll %bool %46
OpBranch %38
%38 = OpLabel
%49 = OpPhi %bool %false %19 %48 %37
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpVectorShuffle %v3float %54 %54 0 1 2
%52 = OpExtInst %v3float %1 FSign %55
%57 = OpLoad %v4float %expected
%58 = OpVectorShuffle %v3float %57 %57 0 1 2
%59 = OpFOrdEqual %v3bool %52 %58
%61 = OpAll %bool %59
OpBranch %51
%51 = OpLabel
%62 = OpPhi %bool %false %38 %61 %50
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%67 = OpLoad %v4float %66
%65 = OpExtInst %v4float %1 FSign %67
%68 = OpLoad %v4float %expected
%69 = OpFOrdEqual %v4bool %65 %68
%71 = OpAll %bool %69
OpBranch %64
%64 = OpLabel
%72 = OpPhi %bool %false %51 %71 %63
OpSelectionMerge %76 None
OpBranchConditional %72 %74 %75
%74 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%79 = OpLoad %v4float %77
OpStore %73 %79
OpBranch %76
%75 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%82 = OpLoad %v4float %80
OpStore %73 %82
OpBranch %76
%76 = OpLabel
%83 = OpLoad %v4float %73
OpReturnValue %83
OpFunctionEnd

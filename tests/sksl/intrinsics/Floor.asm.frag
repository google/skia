OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
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
OpDecorate %expected RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n2 = OpConstant %float -2
%float_2 = OpConstant %float 2
%30 = OpConstantComposite %v4float %float_n2 %float_0 %float_0 %float_2
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
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
%expected = OpVariable %_ptr_Function_v4float Function
%77 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %30
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 0
%32 = OpExtInst %float %1 Floor %38
%39 = OpLoad %v4float %expected
%40 = OpCompositeExtract %float %39 0
%41 = OpFOrdEqual %bool %32 %40
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%46 = OpLoad %v4float %45
%47 = OpVectorShuffle %v2float %46 %46 0 1
%44 = OpExtInst %v2float %1 Floor %47
%48 = OpLoad %v4float %expected
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpFOrdEqual %v2bool %44 %49
%52 = OpAll %bool %50
OpBranch %43
%43 = OpLabel
%53 = OpPhi %bool %false %25 %52 %42
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%58 = OpLoad %v4float %57
%59 = OpVectorShuffle %v3float %58 %58 0 1 2
%56 = OpExtInst %v3float %1 Floor %59
%61 = OpLoad %v4float %expected
%62 = OpVectorShuffle %v3float %61 %61 0 1 2
%63 = OpFOrdEqual %v3bool %56 %62
%65 = OpAll %bool %63
OpBranch %55
%55 = OpLabel
%66 = OpPhi %bool %false %43 %65 %54
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%70 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%71 = OpLoad %v4float %70
%69 = OpExtInst %v4float %1 Floor %71
%72 = OpLoad %v4float %expected
%73 = OpFOrdEqual %v4bool %69 %72
%75 = OpAll %bool %73
OpBranch %68
%68 = OpLabel
%76 = OpPhi %bool %false %55 %75 %67
OpSelectionMerge %80 None
OpBranchConditional %76 %78 %79
%78 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%83 = OpLoad %v4float %81
OpStore %77 %83
OpBranch %80
%79 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%86 = OpLoad %v4float %84
OpStore %77 %86
OpBranch %80
%80 = OpLabel
%87 = OpLoad %v4float %77
OpReturnValue %87
OpFunctionEnd

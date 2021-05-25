OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "input"
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
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
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
%float_3 = OpConstant %float 3
%float_5 = OpConstant %float 5
%float_13 = OpConstant %float 13
%31 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v3float = OpTypeVector %float 3
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
%97 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %31
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %34
%39 = OpCompositeExtract %float %38 0
%33 = OpExtInst %float %1 Length %39
%40 = OpLoad %v4float %expected
%41 = OpCompositeExtract %float %40 0
%42 = OpFOrdEqual %bool %33 %41
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%47 = OpLoad %v4float %46
%48 = OpVectorShuffle %v2float %47 %47 0 1
%45 = OpExtInst %float %1 Length %48
%49 = OpLoad %v4float %expected
%50 = OpCompositeExtract %float %49 1
%51 = OpFOrdEqual %bool %45 %50
OpBranch %44
%44 = OpLabel
%52 = OpPhi %bool %false %25 %51 %43
OpSelectionMerge %54 None
OpBranchConditional %52 %53 %54
%53 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%57 = OpLoad %v4float %56
%58 = OpVectorShuffle %v3float %57 %57 0 1 2
%55 = OpExtInst %float %1 Length %58
%60 = OpLoad %v4float %expected
%61 = OpCompositeExtract %float %60 2
%62 = OpFOrdEqual %bool %55 %61
OpBranch %54
%54 = OpLabel
%63 = OpPhi %bool %false %44 %62 %53
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%68 = OpLoad %v4float %67
%66 = OpExtInst %float %1 Length %68
%69 = OpLoad %v4float %expected
%70 = OpCompositeExtract %float %69 3
%71 = OpFOrdEqual %bool %66 %70
OpBranch %65
%65 = OpLabel
%72 = OpPhi %bool %false %54 %71 %64
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%75 = OpLoad %v4float %expected
%76 = OpCompositeExtract %float %75 0
%77 = OpFOrdEqual %bool %float_3 %76
OpBranch %74
%74 = OpLabel
%78 = OpPhi %bool %false %65 %77 %73
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpLoad %v4float %expected
%82 = OpCompositeExtract %float %81 1
%83 = OpFOrdEqual %bool %float_3 %82
OpBranch %80
%80 = OpLabel
%84 = OpPhi %bool %false %74 %83 %79
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %v4float %expected
%88 = OpCompositeExtract %float %87 2
%89 = OpFOrdEqual %bool %float_5 %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %80 %89 %85
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %v4float %expected
%94 = OpCompositeExtract %float %93 3
%95 = OpFOrdEqual %bool %float_13 %94
OpBranch %92
%92 = OpLabel
%96 = OpPhi %bool %false %86 %95 %91
OpSelectionMerge %100 None
OpBranchConditional %96 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%103 = OpLoad %v4float %101
OpStore %97 %103
OpBranch %100
%99 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%106 = OpLoad %v4float %104
OpStore %97 %106
OpBranch %100
%100 = OpLabel
%107 = OpLoad %v4float %97
OpReturnValue %107
OpFunctionEnd

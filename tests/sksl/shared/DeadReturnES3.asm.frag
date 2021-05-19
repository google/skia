OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_return_b "test_return_b"
OpName %test_break_b "test_break_b"
OpName %test_continue_b "test_continue_b"
OpName %test_if_return_b "test_if_return_b"
OpName %test_if_break_b "test_if_break_b"
OpName %test_else_b "test_else_b"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %16 Binding 0
OpDecorate %16 DescriptorSet 0
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%21 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%29 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%92 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %21
%22 = OpLabel
%26 = OpVariable %_ptr_Function_v2float Function
OpStore %26 %25
%28 = OpFunctionCall %v4float %main %26
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd
%test_return_b = OpFunction %bool None %29
%30 = OpLabel
OpBranch %31
%31 = OpLabel
OpLoopMerge %35 %34 None
OpBranch %32
%32 = OpLabel
OpReturnValue %true
%33 = OpLabel
OpBranch %34
%34 = OpLabel
OpBranchConditional %false %31 %35
%35 = OpLabel
OpUnreachable
OpFunctionEnd
%test_break_b = OpFunction %bool None %29
%38 = OpLabel
OpBranch %39
%39 = OpLabel
OpLoopMerge %43 %42 None
OpBranch %40
%40 = OpLabel
OpBranch %43
%41 = OpLabel
OpBranch %42
%42 = OpLabel
OpBranchConditional %false %39 %43
%43 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_continue_b = OpFunction %bool None %29
%44 = OpLabel
OpBranch %45
%45 = OpLabel
OpLoopMerge %49 %48 None
OpBranch %46
%46 = OpLabel
OpBranch %48
%47 = OpLabel
OpBranch %48
%48 = OpLabel
OpBranchConditional %false %45 %49
%49 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_if_return_b = OpFunction %bool None %29
%50 = OpLabel
OpBranch %51
%51 = OpLabel
OpLoopMerge %55 %54 None
OpBranch %52
%52 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%60 = OpLoad %v4float %56
%61 = OpCompositeExtract %float %60 1
%62 = OpFOrdGreaterThan %bool %61 %float_0
OpSelectionMerge %65 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
OpReturnValue %true
%64 = OpLabel
OpBranch %55
%65 = OpLabel
OpBranch %54
%53 = OpLabel
OpBranch %54
%54 = OpLabel
OpBranchConditional %false %51 %55
%55 = OpLabel
OpReturnValue %false
OpFunctionEnd
%test_if_break_b = OpFunction %bool None %29
%66 = OpLabel
OpBranch %67
%67 = OpLabel
OpLoopMerge %71 %70 None
OpBranch %68
%68 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%73 = OpLoad %v4float %72
%74 = OpCompositeExtract %float %73 1
%75 = OpFOrdGreaterThan %bool %74 %float_0
OpSelectionMerge %78 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
OpBranch %71
%77 = OpLabel
OpBranch %70
%78 = OpLabel
OpBranch %69
%69 = OpLabel
OpBranch %70
%70 = OpLabel
OpBranchConditional %false %67 %71
%71 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_else_b = OpFunction %bool None %29
%79 = OpLabel
OpBranch %80
%80 = OpLabel
OpLoopMerge %84 %83 None
OpBranch %81
%81 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%86 = OpLoad %v4float %85
%87 = OpCompositeExtract %float %86 1
%88 = OpFOrdEqual %bool %87 %float_0
OpSelectionMerge %91 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
OpReturnValue %false
%90 = OpLabel
OpReturnValue %true
%91 = OpLabel
OpBranch %82
%82 = OpLabel
OpBranch %83
%83 = OpLabel
OpBranchConditional %false %80 %84
%84 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %92
%93 = OpFunctionParameter %_ptr_Function_v2float
%94 = OpLabel
%116 = OpVariable %_ptr_Function_v4float Function
%95 = OpFunctionCall %bool %test_return_b
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpFunctionCall %bool %test_break_b
OpBranch %97
%97 = OpLabel
%99 = OpPhi %bool %false %94 %98 %96
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpFunctionCall %bool %test_continue_b
OpBranch %101
%101 = OpLabel
%103 = OpPhi %bool %false %97 %102 %100
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpFunctionCall %bool %test_if_return_b
OpBranch %105
%105 = OpLabel
%107 = OpPhi %bool %false %101 %106 %104
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpFunctionCall %bool %test_if_break_b
OpBranch %109
%109 = OpLabel
%111 = OpPhi %bool %false %105 %110 %108
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpFunctionCall %bool %test_else_b
OpBranch %113
%113 = OpLabel
%115 = OpPhi %bool %false %109 %114 %112
OpSelectionMerge %120 None
OpBranchConditional %115 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%122 = OpLoad %v4float %121
OpStore %116 %122
OpBranch %120
%119 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%125 = OpLoad %v4float %123
OpStore %116 %125
OpBranch %120
%120 = OpLabel
%126 = OpLoad %v4float %116
OpReturnValue %126
OpFunctionEnd

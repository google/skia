OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint "_entrypoint"
OpName %return_on_both_sides "return_on_both_sides"
OpName %for_inside_body "for_inside_body"
OpName %x "x"
OpName %after_for_body "after_for_body"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return "for_with_double_sided_conditional_return"
OpName %x_1 "x"
OpName %if_else_chain "if_else_chain"
OpName %main "main"
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
OpDecorate %15 Binding 0
OpDecorate %15 DescriptorSet 0
OpDecorate %29 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%20 = OpTypeFunction %void
%23 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%108 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %20
%21 = OpLabel
%22 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%return_on_both_sides = OpFunction %bool None %23
%24 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%29 = OpLoad %float %25
%31 = OpFOrdEqual %bool %29 %float_1
OpSelectionMerge %34 None
OpBranchConditional %31 %32 %33
%32 = OpLabel
OpReturnValue %true
%33 = OpLabel
OpReturnValue %true
%34 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body = OpFunction %bool None %23
%36 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %40
%40 = OpLabel
OpLoopMerge %44 %43 None
OpBranch %41
%41 = OpLabel
%45 = OpLoad %int %x
%47 = OpSLessThanEqual %bool %45 %int_10
OpBranchConditional %47 %42 %44
%42 = OpLabel
OpReturnValue %true
%43 = OpLabel
%49 = OpLoad %int %x
%50 = OpIAdd %int %49 %int_1
OpStore %x %50
OpBranch %40
%44 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body = OpFunction %bool None %23
%51 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %53
%53 = OpLabel
OpLoopMerge %57 %56 None
OpBranch %54
%54 = OpLabel
%58 = OpLoad %int %x_0
%59 = OpSLessThanEqual %bool %58 %int_10
OpBranchConditional %59 %55 %57
%55 = OpLabel
OpBranch %56
%56 = OpLabel
%60 = OpLoad %int %x_0
%61 = OpIAdd %int %60 %int_1
OpStore %x_0 %61
OpBranch %53
%57 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return = OpFunction %bool None %23
%62 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %64
%64 = OpLabel
OpLoopMerge %68 %67 None
OpBranch %65
%65 = OpLabel
%69 = OpLoad %int %x_1
%70 = OpSLessThanEqual %bool %69 %int_10
OpBranchConditional %70 %66 %68
%66 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%72 = OpLoad %float %71
%73 = OpFOrdEqual %bool %72 %float_1
OpSelectionMerge %76 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
OpReturnValue %true
%75 = OpLabel
OpReturnValue %true
%76 = OpLabel
OpBranch %67
%67 = OpLabel
%77 = OpLoad %int %x_1
%78 = OpIAdd %int %77 %int_1
OpStore %x_1 %78
OpBranch %64
%68 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain = OpFunction %bool None %23
%79 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%81 = OpLoad %float %80
%82 = OpFOrdEqual %bool %81 %float_1
OpSelectionMerge %85 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
OpReturnValue %true
%84 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%87 = OpLoad %float %86
%89 = OpFOrdEqual %bool %87 %float_2
OpSelectionMerge %92 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
OpReturnValue %false
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%95 = OpLoad %float %94
%97 = OpFOrdEqual %bool %95 %float_3
OpSelectionMerge %100 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
OpReturnValue %true
%99 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%102 = OpLoad %float %101
%104 = OpFOrdEqual %bool %102 %float_4
OpSelectionMerge %107 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
OpReturnValue %false
%106 = OpLabel
OpReturnValue %true
%107 = OpLabel
OpBranch %100
%100 = OpLabel
OpBranch %92
%92 = OpLabel
OpBranch %85
%85 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %108
%109 = OpLabel
%130 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %111 None
OpBranchConditional %true %110 %111
%110 = OpLabel
%112 = OpFunctionCall %bool %return_on_both_sides
OpBranch %111
%111 = OpLabel
%113 = OpPhi %bool %false %109 %112 %110
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpFunctionCall %bool %for_inside_body
OpBranch %115
%115 = OpLabel
%117 = OpPhi %bool %false %111 %116 %114
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpFunctionCall %bool %after_for_body
OpBranch %119
%119 = OpLabel
%121 = OpPhi %bool %false %115 %120 %118
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpFunctionCall %bool %for_with_double_sided_conditional_return
OpBranch %123
%123 = OpLabel
%125 = OpPhi %bool %false %119 %124 %122
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpFunctionCall %bool %if_else_chain
OpBranch %127
%127 = OpLabel
%129 = OpPhi %bool %false %123 %128 %126
OpSelectionMerge %134 None
OpBranchConditional %129 %132 %133
%132 = OpLabel
%135 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
%137 = OpLoad %v4float %135
OpStore %130 %137
OpBranch %134
%133 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
%139 = OpLoad %v4float %138
OpStore %130 %139
OpBranch %134
%134 = OpLabel
%140 = OpLoad %v4float %130
OpReturnValue %140
OpFunctionEnd

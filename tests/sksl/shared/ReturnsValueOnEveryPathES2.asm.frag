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
OpName %for_inside_body "for_inside_body"
OpName %x "x"
OpName %after_for_body "after_for_body"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return "for_with_double_sided_conditional_return"
OpName %x_1 "x"
OpName %if_else_chain "if_else_chain"
OpName %main "main"
OpName %_0_return_on_both_sides "_0_return_on_both_sides"
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
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %63 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%22 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%true = OpConstantTrue %bool
%int_1 = OpConstant %int 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%100 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %19
%20 = OpLabel
%21 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%for_inside_body = OpFunction %bool None %22
%23 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %28
%28 = OpLabel
OpLoopMerge %32 %31 None
OpBranch %29
%29 = OpLabel
%33 = OpLoad %int %x
%35 = OpSLessThanEqual %bool %33 %int_10
OpBranchConditional %35 %30 %32
%30 = OpLabel
OpReturnValue %true
%31 = OpLabel
%38 = OpLoad %int %x
%39 = OpIAdd %int %38 %int_1
OpStore %x %39
OpBranch %28
%32 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body = OpFunction %bool None %22
%40 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %42
%42 = OpLabel
OpLoopMerge %46 %45 None
OpBranch %43
%43 = OpLabel
%47 = OpLoad %int %x_0
%48 = OpSLessThanEqual %bool %47 %int_10
OpBranchConditional %48 %44 %46
%44 = OpLabel
OpBranch %45
%45 = OpLabel
%49 = OpLoad %int %x_0
%50 = OpIAdd %int %49 %int_1
OpStore %x_0 %50
OpBranch %42
%46 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return = OpFunction %bool None %22
%51 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %53
%53 = OpLabel
OpLoopMerge %57 %56 None
OpBranch %54
%54 = OpLabel
%58 = OpLoad %int %x_1
%59 = OpSLessThanEqual %bool %58 %int_10
OpBranchConditional %59 %55 %57
%55 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_float %14 %int_2
%63 = OpLoad %float %60
%65 = OpFOrdEqual %bool %63 %float_1
OpSelectionMerge %68 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
OpReturnValue %true
%67 = OpLabel
OpReturnValue %true
%68 = OpLabel
OpBranch %56
%56 = OpLabel
%69 = OpLoad %int %x_1
%70 = OpIAdd %int %69 %int_1
OpStore %x_1 %70
OpBranch %53
%57 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain = OpFunction %bool None %22
%71 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_float %14 %int_2
%73 = OpLoad %float %72
%74 = OpFOrdEqual %bool %73 %float_1
OpSelectionMerge %77 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
OpReturnValue %true
%76 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_float %14 %int_2
%79 = OpLoad %float %78
%81 = OpFOrdEqual %bool %79 %float_2
OpSelectionMerge %84 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
OpReturnValue %false
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_float %14 %int_2
%87 = OpLoad %float %86
%89 = OpFOrdEqual %bool %87 %float_3
OpSelectionMerge %92 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
OpReturnValue %true
%91 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_float %14 %int_2
%94 = OpLoad %float %93
%96 = OpFOrdEqual %bool %94 %float_4
OpSelectionMerge %99 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
OpReturnValue %false
%98 = OpLabel
OpReturnValue %true
%99 = OpLabel
OpBranch %92
%92 = OpLabel
OpBranch %84
%84 = OpLabel
OpBranch %77
%77 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %100
%101 = OpLabel
%_0_return_on_both_sides = OpVariable %_ptr_Function_bool Function
%127 = OpVariable %_ptr_Function_v4float Function
%104 = OpAccessChain %_ptr_Uniform_float %14 %int_2
%105 = OpLoad %float %104
%106 = OpFOrdEqual %bool %105 %float_1
OpSelectionMerge %109 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
OpStore %_0_return_on_both_sides %true
OpBranch %109
%108 = OpLabel
OpStore %_0_return_on_both_sides %true
OpBranch %109
%109 = OpLabel
%110 = OpLoad %bool %_0_return_on_both_sides
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpFunctionCall %bool %for_inside_body
OpBranch %112
%112 = OpLabel
%114 = OpPhi %bool %false %109 %113 %111
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%117 = OpFunctionCall %bool %after_for_body
OpBranch %116
%116 = OpLabel
%118 = OpPhi %bool %false %112 %117 %115
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%121 = OpFunctionCall %bool %for_with_double_sided_conditional_return
OpBranch %120
%120 = OpLabel
%122 = OpPhi %bool %false %116 %121 %119
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpFunctionCall %bool %if_else_chain
OpBranch %124
%124 = OpLabel
%126 = OpPhi %bool %false %120 %125 %123
OpSelectionMerge %131 None
OpBranchConditional %126 %129 %130
%129 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%134 = OpLoad %v4float %132
OpStore %127 %134
OpBranch %131
%130 = OpLabel
%135 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%136 = OpLoad %v4float %135
OpStore %127 %136
OpBranch %131
%131 = OpLabel
%137 = OpLoad %v4float %127
OpReturnValue %137
OpFunctionEnd

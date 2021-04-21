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
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %return_on_both_sides_b "return_on_both_sides_b"
OpName %for_inside_body_b "for_inside_body_b"
OpName %x "x"
OpName %after_for_body_b "after_for_body_b"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return_b "for_with_double_sided_conditional_return_b"
OpName %x_1 "x"
OpName %if_else_chain_b "if_else_chain_b"
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
OpDecorate %34 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%28 = OpTypeFunction %bool
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
%113 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %20
%21 = OpLabel
%25 = OpVariable %_ptr_Function_v2float Function
OpStore %25 %24
%27 = OpFunctionCall %v4float %main %25
OpStore %sk_FragColor %27
OpReturn
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %28
%29 = OpLabel
%30 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%34 = OpLoad %float %30
%36 = OpFOrdEqual %bool %34 %float_1
OpSelectionMerge %39 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
OpReturnValue %true
%38 = OpLabel
OpReturnValue %true
%39 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %28
%41 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %45
%45 = OpLabel
OpLoopMerge %49 %48 None
OpBranch %46
%46 = OpLabel
%50 = OpLoad %int %x
%52 = OpSLessThanEqual %bool %50 %int_10
OpBranchConditional %52 %47 %49
%47 = OpLabel
OpReturnValue %true
%48 = OpLabel
%54 = OpLoad %int %x
%55 = OpIAdd %int %54 %int_1
OpStore %x %55
OpBranch %45
%49 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body_b = OpFunction %bool None %28
%56 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %58
%58 = OpLabel
OpLoopMerge %62 %61 None
OpBranch %59
%59 = OpLabel
%63 = OpLoad %int %x_0
%64 = OpSLessThanEqual %bool %63 %int_10
OpBranchConditional %64 %60 %62
%60 = OpLabel
OpBranch %61
%61 = OpLabel
%65 = OpLoad %int %x_0
%66 = OpIAdd %int %65 %int_1
OpStore %x_0 %66
OpBranch %58
%62 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %28
%67 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %69
%69 = OpLabel
OpLoopMerge %73 %72 None
OpBranch %70
%70 = OpLabel
%74 = OpLoad %int %x_1
%75 = OpSLessThanEqual %bool %74 %int_10
OpBranchConditional %75 %71 %73
%71 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%77 = OpLoad %float %76
%78 = OpFOrdEqual %bool %77 %float_1
OpSelectionMerge %81 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
OpReturnValue %true
%80 = OpLabel
OpReturnValue %true
%81 = OpLabel
OpBranch %72
%72 = OpLabel
%82 = OpLoad %int %x_1
%83 = OpIAdd %int %82 %int_1
OpStore %x_1 %83
OpBranch %69
%73 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %28
%84 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%86 = OpLoad %float %85
%87 = OpFOrdEqual %bool %86 %float_1
OpSelectionMerge %90 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
OpReturnValue %true
%89 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%92 = OpLoad %float %91
%94 = OpFOrdEqual %bool %92 %float_2
OpSelectionMerge %97 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
OpReturnValue %false
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%100 = OpLoad %float %99
%102 = OpFOrdEqual %bool %100 %float_3
OpSelectionMerge %105 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
OpReturnValue %true
%104 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%107 = OpLoad %float %106
%109 = OpFOrdEqual %bool %107 %float_4
OpSelectionMerge %112 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
OpReturnValue %false
%111 = OpLabel
OpReturnValue %true
%112 = OpLabel
OpBranch %105
%105 = OpLabel
OpBranch %97
%97 = OpLabel
OpBranch %90
%90 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %113
%114 = OpFunctionParameter %_ptr_Function_v2float
%115 = OpLabel
%136 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %117 None
OpBranchConditional %true %116 %117
%116 = OpLabel
%118 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %117
%117 = OpLabel
%119 = OpPhi %bool %false %115 %118 %116
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpFunctionCall %bool %for_inside_body_b
OpBranch %121
%121 = OpLabel
%123 = OpPhi %bool %false %117 %122 %120
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpFunctionCall %bool %after_for_body_b
OpBranch %125
%125 = OpLabel
%127 = OpPhi %bool %false %121 %126 %124
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%130 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %129
%129 = OpLabel
%131 = OpPhi %bool %false %125 %130 %128
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpFunctionCall %bool %if_else_chain_b
OpBranch %133
%133 = OpLabel
%135 = OpPhi %bool %false %129 %134 %132
OpSelectionMerge %140 None
OpBranchConditional %135 %138 %139
%138 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
%143 = OpLoad %v4float %141
OpStore %136 %143
OpBranch %140
%139 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
%145 = OpLoad %v4float %144
OpStore %136 %145
OpBranch %140
%140 = OpLabel
%146 = OpLoad %v4float %136
OpReturnValue %146
OpFunctionEnd

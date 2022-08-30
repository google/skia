OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %simple_b "simple_b"
OpName %return_on_both_sides_b "return_on_both_sides_b"
OpName %for_inside_body_b "for_inside_body_b"
OpName %x "x"
OpName %after_for_body_b "after_for_body_b"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return_b "for_with_double_sided_conditional_return_b"
OpName %x_1 "x"
OpName %if_else_chain_b "if_else_chain_b"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %16 Binding 0
OpDecorate %16 DescriptorSet 0
OpDecorate %37 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%21 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%29 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%116 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %21
%22 = OpLabel
%26 = OpVariable %_ptr_Function_v2float Function
OpStore %26 %25
%28 = OpFunctionCall %v4float %main %26
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd
%simple_b = OpFunction %bool None %29
%30 = OpLabel
OpReturnValue %true
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %29
%32 = OpLabel
%33 = OpAccessChain %_ptr_Uniform_float %16 %int_2
%37 = OpLoad %float %33
%39 = OpFOrdEqual %bool %37 %float_1
OpSelectionMerge %42 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
OpReturnValue %true
%41 = OpLabel
OpReturnValue %true
%42 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %29
%43 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %47
%47 = OpLabel
OpLoopMerge %51 %50 None
OpBranch %48
%48 = OpLabel
%52 = OpLoad %int %x
%54 = OpSLessThanEqual %bool %52 %int_10
OpBranchConditional %54 %49 %51
%49 = OpLabel
OpReturnValue %true
%50 = OpLabel
%56 = OpLoad %int %x
%57 = OpIAdd %int %56 %int_1
OpStore %x %57
OpBranch %47
%51 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body_b = OpFunction %bool None %29
%58 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %60
%60 = OpLabel
OpLoopMerge %64 %63 None
OpBranch %61
%61 = OpLabel
%65 = OpLoad %int %x_0
%66 = OpSLessThanEqual %bool %65 %int_10
OpBranchConditional %66 %62 %64
%62 = OpLabel
%67 = OpFunctionCall %bool %simple_b
OpBranch %63
%63 = OpLabel
%68 = OpLoad %int %x_0
%69 = OpIAdd %int %68 %int_1
OpStore %x_0 %69
OpBranch %60
%64 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %29
%70 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %72
%72 = OpLabel
OpLoopMerge %76 %75 None
OpBranch %73
%73 = OpLabel
%77 = OpLoad %int %x_1
%78 = OpSLessThanEqual %bool %77 %int_10
OpBranchConditional %78 %74 %76
%74 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_float %16 %int_2
%80 = OpLoad %float %79
%81 = OpFOrdEqual %bool %80 %float_1
OpSelectionMerge %84 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
OpReturnValue %true
%83 = OpLabel
OpReturnValue %true
%84 = OpLabel
OpBranch %75
%75 = OpLabel
%85 = OpLoad %int %x_1
%86 = OpIAdd %int %85 %int_1
OpStore %x_1 %86
OpBranch %72
%76 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %29
%87 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_float %16 %int_2
%89 = OpLoad %float %88
%90 = OpFOrdEqual %bool %89 %float_1
OpSelectionMerge %93 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
OpReturnValue %true
%92 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_float %16 %int_2
%95 = OpLoad %float %94
%97 = OpFOrdEqual %bool %95 %float_2
OpSelectionMerge %100 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
OpReturnValue %false
%99 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_float %16 %int_2
%103 = OpLoad %float %102
%105 = OpFOrdEqual %bool %103 %float_3
OpSelectionMerge %108 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
OpReturnValue %true
%107 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_float %16 %int_2
%110 = OpLoad %float %109
%112 = OpFOrdEqual %bool %110 %float_4
OpSelectionMerge %115 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
OpReturnValue %false
%114 = OpLabel
OpReturnValue %true
%115 = OpLabel
OpBranch %108
%108 = OpLabel
OpBranch %100
%100 = OpLabel
OpBranch %93
%93 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %116
%117 = OpFunctionParameter %_ptr_Function_v2float
%118 = OpLabel
%140 = OpVariable %_ptr_Function_v4float Function
%119 = OpFunctionCall %bool %simple_b
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %121
%121 = OpLabel
%123 = OpPhi %bool %false %118 %122 %120
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpFunctionCall %bool %for_inside_body_b
OpBranch %125
%125 = OpLabel
%127 = OpPhi %bool %false %121 %126 %124
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%130 = OpFunctionCall %bool %after_for_body_b
OpBranch %129
%129 = OpLabel
%131 = OpPhi %bool %false %125 %130 %128
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %133
%133 = OpLabel
%135 = OpPhi %bool %false %129 %134 %132
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpFunctionCall %bool %if_else_chain_b
OpBranch %137
%137 = OpLabel
%139 = OpPhi %bool %false %133 %138 %136
OpSelectionMerge %144 None
OpBranchConditional %139 %142 %143
%142 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%147 = OpLoad %v4float %145
OpStore %140 %147
OpBranch %144
%143 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%149 = OpLoad %v4float %148
OpStore %140 %149
OpBranch %144
%144 = OpLabel
%150 = OpLoad %v4float %140
OpReturnValue %150
OpFunctionEnd

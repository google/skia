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
OpName %test_loop_return_b "test_loop_return_b"
OpName %test_loop_break_b "test_loop_break_b"
OpName %x "x"
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
OpDecorate %18 Binding 0
OpDecorate %18 DescriptorSet 0
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%23 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%27 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%31 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%108 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %23
%24 = OpLabel
%28 = OpVariable %_ptr_Function_v2float Function
OpStore %28 %27
%30 = OpFunctionCall %v4float %main %28
OpStore %sk_FragColor %30
OpReturn
OpFunctionEnd
%test_return_b = OpFunction %bool None %31
%32 = OpLabel
OpBranch %33
%33 = OpLabel
OpLoopMerge %37 %36 None
OpBranch %34
%34 = OpLabel
OpReturnValue %true
%35 = OpLabel
OpBranch %36
%36 = OpLabel
OpBranchConditional %false %33 %37
%37 = OpLabel
OpUnreachable
OpFunctionEnd
%test_break_b = OpFunction %bool None %31
%40 = OpLabel
OpBranch %41
%41 = OpLabel
OpLoopMerge %45 %44 None
OpBranch %42
%42 = OpLabel
OpBranch %45
%43 = OpLabel
OpBranch %44
%44 = OpLabel
OpBranchConditional %false %41 %45
%45 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_continue_b = OpFunction %bool None %31
%46 = OpLabel
OpBranch %47
%47 = OpLabel
OpLoopMerge %51 %50 None
OpBranch %48
%48 = OpLabel
OpBranch %50
%49 = OpLabel
OpBranch %50
%50 = OpLabel
OpBranchConditional %false %47 %51
%51 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_if_return_b = OpFunction %bool None %31
%52 = OpLabel
OpBranch %53
%53 = OpLabel
OpLoopMerge %57 %56 None
OpBranch %54
%54 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%62 = OpLoad %v4float %58
%63 = OpCompositeExtract %float %62 1
%64 = OpFOrdGreaterThan %bool %63 %float_0
OpSelectionMerge %67 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
OpReturnValue %true
%66 = OpLabel
OpBranch %57
%67 = OpLabel
OpBranch %56
%55 = OpLabel
OpBranch %56
%56 = OpLabel
OpBranchConditional %false %53 %57
%57 = OpLabel
OpReturnValue %false
OpFunctionEnd
%test_if_break_b = OpFunction %bool None %31
%68 = OpLabel
OpBranch %69
%69 = OpLabel
OpLoopMerge %73 %72 None
OpBranch %70
%70 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%75 = OpLoad %v4float %74
%76 = OpCompositeExtract %float %75 1
%77 = OpFOrdGreaterThan %bool %76 %float_0
OpSelectionMerge %80 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
OpBranch %73
%79 = OpLabel
OpBranch %72
%80 = OpLabel
OpBranch %71
%71 = OpLabel
OpBranch %72
%72 = OpLabel
OpBranchConditional %false %69 %73
%73 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_else_b = OpFunction %bool None %31
%81 = OpLabel
OpBranch %82
%82 = OpLabel
OpLoopMerge %86 %85 None
OpBranch %83
%83 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%88 = OpLoad %v4float %87
%89 = OpCompositeExtract %float %88 1
%90 = OpFOrdEqual %bool %89 %float_0
OpSelectionMerge %93 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
OpReturnValue %false
%92 = OpLabel
OpReturnValue %true
%93 = OpLabel
OpBranch %84
%84 = OpLabel
OpBranch %85
%85 = OpLabel
OpBranchConditional %false %82 %86
%86 = OpLabel
OpUnreachable
OpFunctionEnd
%test_loop_return_b = OpFunction %bool None %31
%94 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_loop_break_b = OpFunction %bool None %31
%95 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %98
%98 = OpLabel
OpLoopMerge %102 %101 None
OpBranch %99
%99 = OpLabel
%103 = OpLoad %int %x
%105 = OpSLessThanEqual %bool %103 %int_1
OpBranchConditional %105 %100 %102
%100 = OpLabel
OpBranch %102
%101 = OpLabel
%106 = OpLoad %int %x
%107 = OpIAdd %int %106 %int_1
OpStore %x %107
OpBranch %98
%102 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %108
%109 = OpFunctionParameter %_ptr_Function_v2float
%110 = OpLabel
%140 = OpVariable %_ptr_Function_v4float Function
%111 = OpFunctionCall %bool %test_return_b
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpFunctionCall %bool %test_break_b
OpBranch %113
%113 = OpLabel
%115 = OpPhi %bool %false %110 %114 %112
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpFunctionCall %bool %test_continue_b
OpBranch %117
%117 = OpLabel
%119 = OpPhi %bool %false %113 %118 %116
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpFunctionCall %bool %test_if_return_b
OpBranch %121
%121 = OpLabel
%123 = OpPhi %bool %false %117 %122 %120
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpFunctionCall %bool %test_if_break_b
OpBranch %125
%125 = OpLabel
%127 = OpPhi %bool %false %121 %126 %124
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%130 = OpFunctionCall %bool %test_else_b
OpBranch %129
%129 = OpLabel
%131 = OpPhi %bool %false %125 %130 %128
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpFunctionCall %bool %test_loop_return_b
OpBranch %133
%133 = OpLabel
%135 = OpPhi %bool %false %129 %134 %132
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpFunctionCall %bool %test_loop_break_b
OpBranch %137
%137 = OpLabel
%139 = OpPhi %bool %false %133 %138 %136
OpSelectionMerge %144 None
OpBranchConditional %139 %142 %143
%142 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%146 = OpLoad %v4float %145
OpStore %140 %146
OpBranch %144
%143 = OpLabel
%147 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%148 = OpLoad %v4float %147
OpStore %140 %148
OpBranch %144
%144 = OpLabel
%149 = OpLoad %v4float %140
OpReturnValue %149
OpFunctionEnd

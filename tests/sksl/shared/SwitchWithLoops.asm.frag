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
OpName %switch_with_continue_in_loop_bi "switch_with_continue_in_loop_bi"
OpName %val "val"
OpName %i "i"
OpName %loop_with_break_in_switch_bi "loop_with_break_in_switch_bi"
OpName %val_0 "val"
OpName %i_0 "i"
OpName %main "main"
OpName %x "x"
OpName %_0_val "_0_val"
OpName %_1_i "_1_i"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%26 = OpTypeFunction %bool %_ptr_Function_int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%int_11 = OpConstant %int 11
%false = OpConstantFalse %bool
%int_20 = OpConstant %int 20
%83 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%switch_with_continue_in_loop_bi = OpFunction %bool None %26
%28 = OpFunctionParameter %_ptr_Function_int
%29 = OpLabel
%val = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
OpStore %val %int_0
%32 = OpLoad %int %28
OpSelectionMerge %33 None
OpSwitch %32 %35 1 %34
%34 = OpLabel
OpStore %i %int_0
OpBranch %37
%37 = OpLabel
OpLoopMerge %41 %40 None
OpBranch %38
%38 = OpLabel
%42 = OpLoad %int %i
%44 = OpSLessThan %bool %42 %int_10
OpBranchConditional %44 %39 %41
%39 = OpLabel
%46 = OpLoad %int %val
%47 = OpIAdd %int %46 %int_1
OpStore %val %47
OpBranch %40
%49 = OpLabel
%48 = OpLoad %int %val
%50 = OpIAdd %int %48 %int_1
OpStore %val %50
OpBranch %40
%40 = OpLabel
%51 = OpLoad %int %i
%52 = OpIAdd %int %51 %int_1
OpStore %i %52
OpBranch %37
%41 = OpLabel
OpBranch %35
%35 = OpLabel
%53 = OpLoad %int %val
%54 = OpIAdd %int %53 %int_1
OpStore %val %54
OpBranch %33
%33 = OpLabel
%55 = OpLoad %int %val
%57 = OpIEqual %bool %55 %int_11
OpReturnValue %57
OpFunctionEnd
%loop_with_break_in_switch_bi = OpFunction %bool None %26
%58 = OpFunctionParameter %_ptr_Function_int
%59 = OpLabel
%val_0 = OpVariable %_ptr_Function_int Function
%i_0 = OpVariable %_ptr_Function_int Function
OpStore %val_0 %int_0
OpStore %i_0 %int_0
OpBranch %62
%62 = OpLabel
OpLoopMerge %66 %65 None
OpBranch %63
%63 = OpLabel
%67 = OpLoad %int %i_0
%68 = OpSLessThan %bool %67 %int_10
OpBranchConditional %68 %64 %66
%64 = OpLabel
%69 = OpLoad %int %58
OpSelectionMerge %70 None
OpSwitch %69 %72 1 %71
%71 = OpLabel
%73 = OpLoad %int %val_0
%74 = OpIAdd %int %73 %int_1
OpStore %val_0 %74
OpBranch %70
%72 = OpLabel
OpReturnValue %false
%70 = OpLabel
%76 = OpLoad %int %val_0
%77 = OpIAdd %int %76 %int_1
OpStore %val_0 %77
OpBranch %65
%65 = OpLabel
%78 = OpLoad %int %i_0
%79 = OpIAdd %int %78 %int_1
OpStore %i_0 %79
OpBranch %62
%66 = OpLabel
%80 = OpLoad %int %val_0
%82 = OpIEqual %bool %80 %int_20
OpReturnValue %82
OpFunctionEnd
%main = OpFunction %v4float None %83
%84 = OpFunctionParameter %_ptr_Function_v2float
%85 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%_0_val = OpVariable %_ptr_Function_int Function
%_1_i = OpVariable %_ptr_Function_int Function
%120 = OpVariable %_ptr_Function_int Function
%126 = OpVariable %_ptr_Function_int Function
%129 = OpVariable %_ptr_Function_v4float Function
%87 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%89 = OpLoad %v4float %87
%90 = OpCompositeExtract %float %89 1
%91 = OpConvertFToS %int %90
OpStore %x %91
OpStore %_0_val %int_0
%93 = OpLoad %int %x
OpSelectionMerge %94 None
OpSwitch %93 %96 1 %95
%95 = OpLabel
OpStore %_1_i %int_0
OpBranch %98
%98 = OpLabel
OpLoopMerge %102 %101 None
OpBranch %99
%99 = OpLabel
%103 = OpLoad %int %_1_i
%104 = OpSLessThan %bool %103 %int_10
OpBranchConditional %104 %100 %102
%100 = OpLabel
%105 = OpLoad %int %_0_val
%106 = OpIAdd %int %105 %int_1
OpStore %_0_val %106
OpBranch %102
%108 = OpLabel
%107 = OpLoad %int %_0_val
%109 = OpIAdd %int %107 %int_1
OpStore %_0_val %109
OpBranch %101
%101 = OpLabel
%110 = OpLoad %int %_1_i
%111 = OpIAdd %int %110 %int_1
OpStore %_1_i %111
OpBranch %98
%102 = OpLabel
OpBranch %96
%96 = OpLabel
%112 = OpLoad %int %_0_val
%113 = OpIAdd %int %112 %int_1
OpStore %_0_val %113
OpBranch %94
%94 = OpLabel
%114 = OpLoad %int %_0_val
%116 = OpIEqual %bool %114 %int_2
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpLoad %int %x
OpStore %120 %119
%121 = OpFunctionCall %bool %switch_with_continue_in_loop_bi %120
OpBranch %118
%118 = OpLabel
%122 = OpPhi %bool %false %94 %121 %117
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpLoad %int %x
OpStore %126 %125
%127 = OpFunctionCall %bool %loop_with_break_in_switch_bi %126
OpBranch %124
%124 = OpLabel
%128 = OpPhi %bool %false %118 %127 %123
OpSelectionMerge %133 None
OpBranchConditional %128 %131 %132
%131 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%135 = OpLoad %v4float %134
OpStore %129 %135
OpBranch %133
%132 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%137 = OpLoad %v4float %136
OpStore %129 %137
OpBranch %133
%133 = OpLabel
%138 = OpLoad %v4float %129
OpReturnValue %138
OpFunctionEnd

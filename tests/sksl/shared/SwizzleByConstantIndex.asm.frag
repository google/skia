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
OpName %constant_swizzle "constant_swizzle"
OpName %v "v"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %w "w"
OpName %foldable_index "foldable_index"
OpName %ZERO "ZERO"
OpName %ONE "ONE"
OpName %TWO "TWO"
OpName %THREE "THREE"
OpName %x_0 "x"
OpName %y_0 "y"
OpName %z_0 "z"
OpName %w_0 "w"
OpName %foldable "foldable"
OpName %v_0 "v"
OpName %x_1 "x"
OpName %y_1 "y"
OpName %z_1 "z"
OpName %w_1 "w"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpName %c "c"
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
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%21 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%84 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%114 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%v4bool = OpTypeVector %bool 4
%_entrypoint = OpFunction %void None %18
%19 = OpLabel
%20 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %20
OpReturn
OpFunctionEnd
%constant_swizzle = OpFunction %v4float None %21
%22 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_float Function
%w = OpVariable %_ptr_Function_float Function
%25 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%29 = OpLoad %v4float %25
OpStore %v %29
%32 = OpLoad %v4float %v
%33 = OpCompositeExtract %float %32 0
OpStore %x %33
%35 = OpLoad %v4float %v
%36 = OpCompositeExtract %float %35 1
OpStore %y %36
%38 = OpLoad %v4float %v
%39 = OpCompositeExtract %float %38 2
OpStore %z %39
%41 = OpLoad %v4float %v
%42 = OpCompositeExtract %float %41 3
OpStore %w %42
%43 = OpLoad %float %x
%44 = OpLoad %float %y
%45 = OpLoad %float %z
%46 = OpLoad %float %w
%47 = OpCompositeConstruct %v4float %43 %44 %45 %46
OpReturnValue %47
OpFunctionEnd
%foldable_index = OpFunction %v4float None %21
%48 = OpLabel
%ZERO = OpVariable %_ptr_Function_int Function
%ONE = OpVariable %_ptr_Function_int Function
%TWO = OpVariable %_ptr_Function_int Function
%THREE = OpVariable %_ptr_Function_int Function
%x_0 = OpVariable %_ptr_Function_float Function
%y_0 = OpVariable %_ptr_Function_float Function
%z_0 = OpVariable %_ptr_Function_float Function
%w_0 = OpVariable %_ptr_Function_float Function
OpStore %ZERO %int_0
OpStore %ONE %int_1
OpStore %TWO %int_2
OpStore %THREE %int_3
%58 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%59 = OpLoad %v4float %58
%60 = OpCompositeExtract %float %59 0
OpStore %x_0 %60
%62 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%63 = OpLoad %v4float %62
%64 = OpCompositeExtract %float %63 1
OpStore %y_0 %64
%66 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%67 = OpLoad %v4float %66
%68 = OpCompositeExtract %float %67 2
OpStore %z_0 %68
%70 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%71 = OpLoad %v4float %70
%72 = OpCompositeExtract %float %71 3
OpStore %w_0 %72
%73 = OpLoad %float %x_0
%74 = OpLoad %float %y_0
%75 = OpLoad %float %z_0
%76 = OpLoad %float %w_0
%77 = OpCompositeConstruct %v4float %73 %74 %75 %76
OpReturnValue %77
OpFunctionEnd
%foldable = OpFunction %v4float None %21
%78 = OpLabel
%v_0 = OpVariable %_ptr_Function_v4float Function
%x_1 = OpVariable %_ptr_Function_float Function
%y_1 = OpVariable %_ptr_Function_float Function
%z_1 = OpVariable %_ptr_Function_float Function
%w_1 = OpVariable %_ptr_Function_float Function
OpStore %v_0 %84
%86 = OpLoad %v4float %v_0
%87 = OpCompositeExtract %float %86 0
OpStore %x_1 %87
%89 = OpLoad %v4float %v_0
%90 = OpCompositeExtract %float %89 1
OpStore %y_1 %90
%92 = OpLoad %v4float %v_0
%93 = OpCompositeExtract %float %92 2
OpStore %z_1 %93
%95 = OpLoad %v4float %v_0
%96 = OpCompositeExtract %float %95 3
OpStore %w_1 %96
%97 = OpLoad %float %x_1
%98 = OpLoad %float %y_1
%99 = OpLoad %float %z_1
%100 = OpLoad %float %w_1
%101 = OpCompositeConstruct %v4float %97 %98 %99 %100
OpReturnValue %101
OpFunctionEnd
%main = OpFunction %v4float None %21
%102 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%c = OpVariable %_ptr_Function_v4float Function
%130 = OpVariable %_ptr_Function_v4float Function
%104 = OpFunctionCall %v4float %constant_swizzle
OpStore %a %104
%106 = OpFunctionCall %v4float %foldable_index
OpStore %b %106
%108 = OpFunctionCall %v4float %foldable
OpStore %c %108
%110 = OpLoad %v4float %a
%115 = OpFOrdEqual %v4bool %110 %114
%117 = OpAll %bool %115
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpLoad %v4float %b
%121 = OpFOrdEqual %v4bool %120 %114
%122 = OpAll %bool %121
OpBranch %119
%119 = OpLabel
%123 = OpPhi %bool %false %102 %122 %118
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpLoad %v4float %c
%127 = OpFOrdEqual %v4bool %126 %84
%128 = OpAll %bool %127
OpBranch %125
%125 = OpLabel
%129 = OpPhi %bool %false %119 %128 %124
OpSelectionMerge %133 None
OpBranchConditional %129 %131 %132
%131 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%135 = OpLoad %v4float %134
OpStore %130 %135
OpBranch %133
%132 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %13 %int_2
%137 = OpLoad %v4float %136
OpStore %130 %137
OpBranch %133
%133 = OpLabel
%138 = OpLoad %v4float %130
OpReturnValue %138
OpFunctionEnd

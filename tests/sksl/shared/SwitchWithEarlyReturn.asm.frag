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
OpName %return_in_one_case_bi "return_in_one_case_bi"
OpName %val "val"
OpName %return_in_default_bi "return_in_default_bi"
OpName %return_in_every_case_bi "return_in_every_case_bi"
OpName %return_in_every_case_no_default_bi "return_in_every_case_no_default_bi"
OpName %val_0 "val"
OpName %return_in_some_cases_bi "return_in_some_cases_bi"
OpName %val_1 "val"
OpName %return_with_fallthrough_bi "return_with_fallthrough_bi"
OpName %val_2 "val"
OpName %main "main"
OpName %x "x"
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
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%31 = OpTypeFunction %bool %_ptr_Function_int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%88 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %21
%22 = OpLabel
%26 = OpVariable %_ptr_Function_v2float Function
OpStore %26 %25
%28 = OpFunctionCall %v4float %main %26
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd
%return_in_one_case_bi = OpFunction %bool None %31
%32 = OpFunctionParameter %_ptr_Function_int
%33 = OpLabel
%val = OpVariable %_ptr_Function_int Function
OpStore %val %int_0
%36 = OpLoad %int %32
OpSelectionMerge %37 None
OpSwitch %36 %39 1 %38
%38 = OpLabel
%41 = OpIAdd %int %int_0 %int_1
OpStore %val %41
OpReturnValue %false
%39 = OpLabel
%43 = OpLoad %int %val
%44 = OpIAdd %int %43 %int_1
OpStore %val %44
OpBranch %37
%37 = OpLabel
%45 = OpLoad %int %val
%46 = OpIEqual %bool %45 %int_1
OpReturnValue %46
OpFunctionEnd
%return_in_default_bi = OpFunction %bool None %31
%47 = OpFunctionParameter %_ptr_Function_int
%48 = OpLabel
%49 = OpLoad %int %47
OpSelectionMerge %50 None
OpSwitch %49 %51
%51 = OpLabel
OpReturnValue %true
%50 = OpLabel
OpUnreachable
OpFunctionEnd
%return_in_every_case_bi = OpFunction %bool None %31
%53 = OpFunctionParameter %_ptr_Function_int
%54 = OpLabel
%55 = OpLoad %int %53
OpSelectionMerge %56 None
OpSwitch %55 %58 1 %57
%57 = OpLabel
OpReturnValue %false
%58 = OpLabel
OpReturnValue %true
%56 = OpLabel
OpUnreachable
OpFunctionEnd
%return_in_every_case_no_default_bi = OpFunction %bool None %31
%59 = OpFunctionParameter %_ptr_Function_int
%60 = OpLabel
%val_0 = OpVariable %_ptr_Function_int Function
OpStore %val_0 %int_0
%62 = OpLoad %int %59
OpSelectionMerge %63 None
OpSwitch %62 %63 1 %64 2 %65
%64 = OpLabel
OpReturnValue %false
%65 = OpLabel
OpReturnValue %true
%63 = OpLabel
%66 = OpIAdd %int %int_0 %int_1
OpStore %val_0 %66
%67 = OpIEqual %bool %66 %int_1
OpReturnValue %67
OpFunctionEnd
%return_in_some_cases_bi = OpFunction %bool None %31
%68 = OpFunctionParameter %_ptr_Function_int
%69 = OpLabel
%val_1 = OpVariable %_ptr_Function_int Function
OpStore %val_1 %int_0
%71 = OpLoad %int %68
OpSelectionMerge %72 None
OpSwitch %71 %75 1 %73 2 %74
%73 = OpLabel
OpReturnValue %false
%74 = OpLabel
OpReturnValue %true
%75 = OpLabel
OpBranch %72
%72 = OpLabel
%76 = OpIAdd %int %int_0 %int_1
OpStore %val_1 %76
%77 = OpIEqual %bool %76 %int_1
OpReturnValue %77
OpFunctionEnd
%return_with_fallthrough_bi = OpFunction %bool None %31
%78 = OpFunctionParameter %_ptr_Function_int
%79 = OpLabel
%val_2 = OpVariable %_ptr_Function_int Function
OpStore %val_2 %int_0
%81 = OpLoad %int %78
OpSelectionMerge %82 None
OpSwitch %81 %85 1 %83 2 %84
%83 = OpLabel
OpBranch %84
%84 = OpLabel
OpReturnValue %true
%85 = OpLabel
OpBranch %82
%82 = OpLabel
%86 = OpIAdd %int %int_0 %int_1
OpStore %val_2 %86
%87 = OpIEqual %bool %86 %int_1
OpReturnValue %87
OpFunctionEnd
%main = OpFunction %v4float None %88
%89 = OpFunctionParameter %_ptr_Function_v2float
%90 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%97 = OpVariable %_ptr_Function_int Function
%101 = OpVariable %_ptr_Function_int Function
%106 = OpVariable %_ptr_Function_int Function
%111 = OpVariable %_ptr_Function_int Function
%116 = OpVariable %_ptr_Function_int Function
%121 = OpVariable %_ptr_Function_int Function
%124 = OpVariable %_ptr_Function_v4float Function
%92 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%94 = OpLoad %v4float %92
%95 = OpCompositeExtract %float %94 1
%96 = OpConvertFToS %int %95
OpStore %x %96
OpStore %97 %96
%98 = OpFunctionCall %bool %return_in_one_case_bi %97
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
OpStore %101 %96
%102 = OpFunctionCall %bool %return_in_default_bi %101
OpBranch %100
%100 = OpLabel
%103 = OpPhi %bool %false %90 %102 %99
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
OpStore %106 %96
%107 = OpFunctionCall %bool %return_in_every_case_bi %106
OpBranch %105
%105 = OpLabel
%108 = OpPhi %bool %false %100 %107 %104
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
OpStore %111 %96
%112 = OpFunctionCall %bool %return_in_every_case_no_default_bi %111
OpBranch %110
%110 = OpLabel
%113 = OpPhi %bool %false %105 %112 %109
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
OpStore %116 %96
%117 = OpFunctionCall %bool %return_in_some_cases_bi %116
OpBranch %115
%115 = OpLabel
%118 = OpPhi %bool %false %110 %117 %114
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
OpStore %121 %96
%122 = OpFunctionCall %bool %return_with_fallthrough_bi %121
OpBranch %120
%120 = OpLabel
%123 = OpPhi %bool %false %115 %122 %119
OpSelectionMerge %128 None
OpBranchConditional %123 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%130 = OpLoad %v4float %129
OpStore %124 %130
OpBranch %128
%127 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%132 = OpLoad %v4float %131
OpStore %124 %132
OpBranch %128
%128 = OpLabel
%133 = OpLoad %v4float %124
OpReturnValue %133
OpFunctionEnd

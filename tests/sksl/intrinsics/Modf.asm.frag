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
OpName %main "main"
OpName %value "value"
OpName %whole "whole"
OpName %fraction "fraction"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_2_5 = OpConstant %float 2.5
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%float_0_5 = OpConstant %float 0.5
%_ptr_Function_bool = OpTypePointer Function %bool
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%value = OpVariable %_ptr_Function_v4float Function
%whole = OpVariable %_ptr_Function_v4float Function
%fraction = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%46 = OpVariable %_ptr_Function_float Function
%66 = OpVariable %_ptr_Function_v2float Function
%87 = OpVariable %_ptr_Function_v3float Function
%120 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpVectorShuffle %v4float %32 %32 1 1 1 1
%35 = OpVectorTimesScalar %v4float %33 %float_2_5
OpStore %value %35
%42 = OpLoad %v4float %value
%43 = OpCompositeExtract %float %42 0
%44 = OpAccessChain %_ptr_Function_float %whole %int_0
%41 = OpExtInst %float %1 Modf %43 %46
%47 = OpLoad %float %46
OpStore %44 %47
%48 = OpAccessChain %_ptr_Function_float %fraction %int_0
OpStore %48 %41
%50 = OpLoad %v4float %whole
%51 = OpCompositeExtract %float %50 0
%53 = OpFOrdEqual %bool %51 %float_2
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%56 = OpLoad %v4float %fraction
%57 = OpCompositeExtract %float %56 0
%59 = OpFOrdEqual %bool %57 %float_0_5
OpBranch %55
%55 = OpLabel
%60 = OpPhi %bool %false %25 %59 %54
%61 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %61 %60
%64 = OpLoad %v4float %value
%65 = OpVectorShuffle %v2float %64 %64 0 1
%63 = OpExtInst %v2float %1 Modf %65 %66
%67 = OpLoad %v2float %66
%68 = OpLoad %v4float %whole
%69 = OpVectorShuffle %v4float %68 %67 4 5 2 3
OpStore %whole %69
%70 = OpLoad %v4float %fraction
%71 = OpVectorShuffle %v4float %70 %63 4 5 2 3
OpStore %fraction %71
%72 = OpLoad %v4float %whole
%73 = OpCompositeExtract %float %72 1
%74 = OpFOrdEqual %bool %73 %float_2
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %v4float %fraction
%78 = OpCompositeExtract %float %77 1
%79 = OpFOrdEqual %bool %78 %float_0_5
OpBranch %76
%76 = OpLabel
%80 = OpPhi %bool %false %55 %79 %75
%81 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %81 %80
%84 = OpLoad %v4float %value
%85 = OpVectorShuffle %v3float %84 %84 0 1 2
%83 = OpExtInst %v3float %1 Modf %85 %87
%89 = OpLoad %v3float %87
%90 = OpLoad %v4float %whole
%91 = OpVectorShuffle %v4float %90 %89 4 5 6 3
OpStore %whole %91
%92 = OpLoad %v4float %fraction
%93 = OpVectorShuffle %v4float %92 %83 4 5 6 3
OpStore %fraction %93
%94 = OpLoad %v4float %whole
%95 = OpCompositeExtract %float %94 2
%96 = OpFOrdEqual %bool %95 %float_2
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %v4float %fraction
%100 = OpCompositeExtract %float %99 2
%101 = OpFOrdEqual %bool %100 %float_0_5
OpBranch %98
%98 = OpLabel
%102 = OpPhi %bool %false %76 %101 %97
%103 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %103 %102
%106 = OpLoad %v4float %value
%105 = OpExtInst %v4float %1 Modf %106 %whole
OpStore %fraction %105
%107 = OpLoad %v4float %whole
%108 = OpCompositeExtract %float %107 3
%109 = OpFOrdEqual %bool %108 %float_2
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpLoad %v4float %fraction
%113 = OpCompositeExtract %float %112 3
%114 = OpFOrdEqual %bool %113 %float_0_5
OpBranch %111
%111 = OpLabel
%115 = OpPhi %bool %false %98 %114 %110
%116 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %116 %115
%119 = OpLoad %v4bool %ok
%118 = OpAll %bool %119
OpSelectionMerge %123 None
OpBranchConditional %118 %121 %122
%121 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%125 = OpLoad %v4float %124
OpStore %120 %125
OpBranch %123
%122 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%127 = OpLoad %v4float %126
OpStore %120 %127
OpBranch %123
%123 = OpLabel
%128 = OpLoad %v4float %120
OpReturnValue %128
OpFunctionEnd

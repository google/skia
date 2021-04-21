### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %61 = OpExtInst %v2float %1 Modf %63 %4294967295

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
OpDecorate %108 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
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
%109 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpVectorShuffle %v4float %32 %32 1 1 1 1
%35 = OpVectorTimesScalar %v4float %33 %float_2_5
OpStore %value %35
%42 = OpLoad %v4float %value
%43 = OpCompositeExtract %float %42 0
%44 = OpAccessChain %_ptr_Function_float %whole %int_0
%41 = OpExtInst %float %1 Modf %43 %44
%46 = OpAccessChain %_ptr_Function_float %fraction %int_0
OpStore %46 %41
%48 = OpLoad %v4float %whole
%49 = OpCompositeExtract %float %48 0
%51 = OpFOrdEqual %bool %49 %float_2
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%54 = OpLoad %v4float %fraction
%55 = OpCompositeExtract %float %54 0
%57 = OpFOrdEqual %bool %55 %float_0_5
OpBranch %53
%53 = OpLabel
%58 = OpPhi %bool %false %25 %57 %52
%59 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %59 %58
%62 = OpLoad %v4float %value
%63 = OpVectorShuffle %v2float %62 %62 0 1
%61 = OpExtInst %v2float %1 Modf %63 %4294967295
%64 = OpLoad %v4float %fraction
%65 = OpVectorShuffle %v4float %64 %61 4 5 2 3
OpStore %fraction %65
%66 = OpLoad %v4float %whole
%67 = OpCompositeExtract %float %66 1
%68 = OpFOrdEqual %bool %67 %float_2
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %v4float %fraction
%72 = OpCompositeExtract %float %71 1
%73 = OpFOrdEqual %bool %72 %float_0_5
OpBranch %70
%70 = OpLabel
%74 = OpPhi %bool %false %53 %73 %69
%75 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %75 %74
%78 = OpLoad %v4float %value
%79 = OpVectorShuffle %v3float %78 %78 0 1 2
%77 = OpExtInst %v3float %1 Modf %79 %4294967295
%81 = OpLoad %v4float %fraction
%82 = OpVectorShuffle %v4float %81 %77 4 5 6 3
OpStore %fraction %82
%83 = OpLoad %v4float %whole
%84 = OpCompositeExtract %float %83 2
%85 = OpFOrdEqual %bool %84 %float_2
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %v4float %fraction
%89 = OpCompositeExtract %float %88 2
%90 = OpFOrdEqual %bool %89 %float_0_5
OpBranch %87
%87 = OpLabel
%91 = OpPhi %bool %false %70 %90 %86
%92 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %92 %91
%95 = OpLoad %v4float %value
%94 = OpExtInst %v4float %1 Modf %95 %whole
OpStore %fraction %94
%96 = OpLoad %v4float %whole
%97 = OpCompositeExtract %float %96 3
%98 = OpFOrdEqual %bool %97 %float_2
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpLoad %v4float %fraction
%102 = OpCompositeExtract %float %101 3
%103 = OpFOrdEqual %bool %102 %float_0_5
OpBranch %100
%100 = OpLabel
%104 = OpPhi %bool %false %87 %103 %99
%105 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %105 %104
%108 = OpLoad %v4bool %ok
%107 = OpAll %bool %108
OpSelectionMerge %112 None
OpBranchConditional %107 %110 %111
%110 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%114 = OpLoad %v4float %113
OpStore %109 %114
OpBranch %112
%111 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%116 = OpLoad %v4float %115
OpStore %109 %116
OpBranch %112
%112 = OpLabel
%117 = OpLoad %v4float %109
OpReturnValue %117
OpFunctionEnd

1 error

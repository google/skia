### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %55 = OpExtInst %v2float %1 Modf %57 %4294967295

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
OpName %_entrypoint "_entrypoint"
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
OpDecorate %26 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
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
%v2float = OpTypeVector %float 2
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%value = OpVariable %_ptr_Function_v4float Function
%whole = OpVariable %_ptr_Function_v4float Function
%fraction = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%104 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpVectorShuffle %v4float %26 %26 1 1 1 1
%29 = OpVectorTimesScalar %v4float %27 %float_2_5
OpStore %value %29
%36 = OpLoad %v4float %value
%37 = OpCompositeExtract %float %36 0
%38 = OpAccessChain %_ptr_Function_float %whole %int_0
%35 = OpExtInst %float %1 Modf %37 %38
%40 = OpAccessChain %_ptr_Function_float %fraction %int_0
OpStore %40 %35
%42 = OpLoad %v4float %whole
%43 = OpCompositeExtract %float %42 0
%45 = OpFOrdEqual %bool %43 %float_2
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%48 = OpLoad %v4float %fraction
%49 = OpCompositeExtract %float %48 0
%51 = OpFOrdEqual %bool %49 %float_0_5
OpBranch %47
%47 = OpLabel
%52 = OpPhi %bool %false %19 %51 %46
%53 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %53 %52
%56 = OpLoad %v4float %value
%57 = OpVectorShuffle %v2float %56 %56 0 1
%55 = OpExtInst %v2float %1 Modf %57 %4294967295
%59 = OpLoad %v4float %fraction
%60 = OpVectorShuffle %v4float %59 %55 4 5 2 3
OpStore %fraction %60
%61 = OpLoad %v4float %whole
%62 = OpCompositeExtract %float %61 1
%63 = OpFOrdEqual %bool %62 %float_2
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %v4float %fraction
%67 = OpCompositeExtract %float %66 1
%68 = OpFOrdEqual %bool %67 %float_0_5
OpBranch %65
%65 = OpLabel
%69 = OpPhi %bool %false %47 %68 %64
%70 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %70 %69
%73 = OpLoad %v4float %value
%74 = OpVectorShuffle %v3float %73 %73 0 1 2
%72 = OpExtInst %v3float %1 Modf %74 %4294967295
%76 = OpLoad %v4float %fraction
%77 = OpVectorShuffle %v4float %76 %72 4 5 6 3
OpStore %fraction %77
%78 = OpLoad %v4float %whole
%79 = OpCompositeExtract %float %78 2
%80 = OpFOrdEqual %bool %79 %float_2
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %v4float %fraction
%84 = OpCompositeExtract %float %83 2
%85 = OpFOrdEqual %bool %84 %float_0_5
OpBranch %82
%82 = OpLabel
%86 = OpPhi %bool %false %65 %85 %81
%87 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %87 %86
%90 = OpLoad %v4float %value
%89 = OpExtInst %v4float %1 Modf %90 %whole
OpStore %fraction %89
%91 = OpLoad %v4float %whole
%92 = OpCompositeExtract %float %91 3
%93 = OpFOrdEqual %bool %92 %float_2
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpLoad %v4float %fraction
%97 = OpCompositeExtract %float %96 3
%98 = OpFOrdEqual %bool %97 %float_0_5
OpBranch %95
%95 = OpLabel
%99 = OpPhi %bool %false %82 %98 %94
%100 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %100 %99
%103 = OpLoad %v4bool %ok
%102 = OpAll %bool %103
OpSelectionMerge %107 None
OpBranchConditional %102 %105 %106
%105 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%109 = OpLoad %v4float %108
OpStore %104 %109
OpBranch %107
%106 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%111 = OpLoad %v4float %110
OpStore %104 %111
OpBranch %107
%107 = OpLabel
%112 = OpLoad %v4float %104
OpReturnValue %112
OpFunctionEnd

1 error

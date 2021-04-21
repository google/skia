### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %64 = OpExtInst %v2float %1 Frexp %66 %4294967295

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
OpName %exp "exp"
OpName %result "result"
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
OpDecorate %110 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
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
%float_6 = OpConstant %float 6
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%float_0_75 = OpConstant %float 0.75
%int_3 = OpConstant %int 3
%_ptr_Function_bool = OpTypePointer Function %bool
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%int_2 = OpConstant %int 2
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
%exp = OpVariable %_ptr_Function_v4int Function
%result = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%111 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpVectorShuffle %v4float %32 %32 1 1 1 1
%35 = OpVectorTimesScalar %v4float %33 %float_6
OpStore %value %35
%44 = OpLoad %v4float %value
%45 = OpCompositeExtract %float %44 0
%46 = OpAccessChain %_ptr_Function_int %exp %int_0
%43 = OpExtInst %float %1 Frexp %45 %46
%48 = OpAccessChain %_ptr_Function_float %result %int_0
OpStore %48 %43
%51 = OpLoad %v4float %result
%52 = OpCompositeExtract %float %51 0
%54 = OpFOrdEqual %bool %52 %float_0_75
OpSelectionMerge %56 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
%57 = OpLoad %v4int %exp
%58 = OpCompositeExtract %int %57 0
%60 = OpIEqual %bool %58 %int_3
OpBranch %56
%56 = OpLabel
%61 = OpPhi %bool %false %25 %60 %55
%62 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %62 %61
%65 = OpLoad %v4float %value
%66 = OpVectorShuffle %v2float %65 %65 0 1
%64 = OpExtInst %v2float %1 Frexp %66 %4294967295
%67 = OpLoad %v4float %result
%68 = OpVectorShuffle %v4float %67 %64 4 5 2 3
OpStore %result %68
%69 = OpLoad %v4float %result
%70 = OpCompositeExtract %float %69 1
%71 = OpFOrdEqual %bool %70 %float_0_75
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpLoad %v4int %exp
%75 = OpCompositeExtract %int %74 1
%76 = OpIEqual %bool %75 %int_3
OpBranch %73
%73 = OpLabel
%77 = OpPhi %bool %false %56 %76 %72
%78 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %78 %77
%81 = OpLoad %v4float %value
%82 = OpVectorShuffle %v3float %81 %81 0 1 2
%80 = OpExtInst %v3float %1 Frexp %82 %4294967295
%84 = OpLoad %v4float %result
%85 = OpVectorShuffle %v4float %84 %80 4 5 6 3
OpStore %result %85
%86 = OpLoad %v4float %result
%87 = OpCompositeExtract %float %86 2
%88 = OpFOrdEqual %bool %87 %float_0_75
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpLoad %v4int %exp
%92 = OpCompositeExtract %int %91 2
%93 = OpIEqual %bool %92 %int_3
OpBranch %90
%90 = OpLabel
%94 = OpPhi %bool %false %73 %93 %89
%95 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %95 %94
%98 = OpLoad %v4float %value
%97 = OpExtInst %v4float %1 Frexp %98 %exp
OpStore %result %97
%99 = OpLoad %v4float %result
%100 = OpCompositeExtract %float %99 3
%101 = OpFOrdEqual %bool %100 %float_0_75
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%104 = OpLoad %v4int %exp
%105 = OpCompositeExtract %int %104 3
%106 = OpIEqual %bool %105 %int_3
OpBranch %103
%103 = OpLabel
%107 = OpPhi %bool %false %90 %106 %102
%108 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %108 %107
%110 = OpLoad %v4bool %ok
%109 = OpAll %bool %110
OpSelectionMerge %114 None
OpBranchConditional %109 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%116 = OpLoad %v4float %115
OpStore %111 %116
OpBranch %114
%113 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%118 = OpLoad %v4float %117
OpStore %111 %118
OpBranch %114
%114 = OpLabel
%119 = OpLoad %v4float %111
OpReturnValue %119
OpFunctionEnd

1 error

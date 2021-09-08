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
OpDecorate %124 RelaxedPrecision
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
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
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
%48 = OpVariable %_ptr_Function_int Function
%69 = OpVariable %_ptr_Function_v2int Function
%92 = OpVariable %_ptr_Function_v3int Function
%125 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpVectorShuffle %v4float %32 %32 1 1 1 1
%35 = OpVectorTimesScalar %v4float %33 %float_6
OpStore %value %35
%44 = OpLoad %v4float %value
%45 = OpCompositeExtract %float %44 0
%46 = OpAccessChain %_ptr_Function_int %exp %int_0
%43 = OpExtInst %float %1 Frexp %45 %48
%49 = OpLoad %int %48
OpStore %46 %49
%50 = OpAccessChain %_ptr_Function_float %result %int_0
OpStore %50 %43
%53 = OpLoad %v4float %result
%54 = OpCompositeExtract %float %53 0
%56 = OpFOrdEqual %bool %54 %float_0_75
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%59 = OpLoad %v4int %exp
%60 = OpCompositeExtract %int %59 0
%62 = OpIEqual %bool %60 %int_3
OpBranch %58
%58 = OpLabel
%63 = OpPhi %bool %false %25 %62 %57
%64 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %64 %63
%67 = OpLoad %v4float %value
%68 = OpVectorShuffle %v2float %67 %67 0 1
%66 = OpExtInst %v2float %1 Frexp %68 %69
%72 = OpLoad %v2int %69
%73 = OpLoad %v4int %exp
%74 = OpVectorShuffle %v4int %73 %72 4 5 2 3
OpStore %exp %74
%75 = OpLoad %v4float %result
%76 = OpVectorShuffle %v4float %75 %66 4 5 2 3
OpStore %result %76
%77 = OpLoad %v4float %result
%78 = OpCompositeExtract %float %77 1
%79 = OpFOrdEqual %bool %78 %float_0_75
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %v4int %exp
%83 = OpCompositeExtract %int %82 1
%84 = OpIEqual %bool %83 %int_3
OpBranch %81
%81 = OpLabel
%85 = OpPhi %bool %false %58 %84 %80
%86 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %86 %85
%89 = OpLoad %v4float %value
%90 = OpVectorShuffle %v3float %89 %89 0 1 2
%88 = OpExtInst %v3float %1 Frexp %90 %92
%95 = OpLoad %v3int %92
%96 = OpLoad %v4int %exp
%97 = OpVectorShuffle %v4int %96 %95 4 5 6 3
OpStore %exp %97
%98 = OpLoad %v4float %result
%99 = OpVectorShuffle %v4float %98 %88 4 5 6 3
OpStore %result %99
%100 = OpLoad %v4float %result
%101 = OpCompositeExtract %float %100 2
%102 = OpFOrdEqual %bool %101 %float_0_75
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %v4int %exp
%106 = OpCompositeExtract %int %105 2
%107 = OpIEqual %bool %106 %int_3
OpBranch %104
%104 = OpLabel
%108 = OpPhi %bool %false %81 %107 %103
%109 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %109 %108
%112 = OpLoad %v4float %value
%111 = OpExtInst %v4float %1 Frexp %112 %exp
OpStore %result %111
%113 = OpLoad %v4float %result
%114 = OpCompositeExtract %float %113 3
%115 = OpFOrdEqual %bool %114 %float_0_75
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpLoad %v4int %exp
%119 = OpCompositeExtract %int %118 3
%120 = OpIEqual %bool %119 %int_3
OpBranch %117
%117 = OpLabel
%121 = OpPhi %bool %false %104 %120 %116
%122 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %122 %121
%124 = OpLoad %v4bool %ok
%123 = OpAll %bool %124
OpSelectionMerge %128 None
OpBranchConditional %123 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%130 = OpLoad %v4float %129
OpStore %125 %130
OpBranch %128
%127 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %131
OpStore %125 %132
OpBranch %128
%128 = OpLabel
%133 = OpLoad %v4float %125
OpReturnValue %133
OpFunctionEnd

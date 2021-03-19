### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %58 = OpExtInst %v2float %1 Frexp %60 %4294967295

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
OpDecorate %26 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%value = OpVariable %_ptr_Function_v4float Function
%exp = OpVariable %_ptr_Function_v4int Function
%result = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%106 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpVectorShuffle %v4float %26 %26 1 1 1 1
%29 = OpVectorTimesScalar %v4float %27 %float_6
OpStore %value %29
%38 = OpLoad %v4float %value
%39 = OpCompositeExtract %float %38 0
%40 = OpAccessChain %_ptr_Function_int %exp %int_0
%37 = OpExtInst %float %1 Frexp %39 %40
%42 = OpAccessChain %_ptr_Function_float %result %int_0
OpStore %42 %37
%45 = OpLoad %v4float %result
%46 = OpCompositeExtract %float %45 0
%48 = OpFOrdEqual %bool %46 %float_0_75
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%51 = OpLoad %v4int %exp
%52 = OpCompositeExtract %int %51 0
%54 = OpIEqual %bool %52 %int_3
OpBranch %50
%50 = OpLabel
%55 = OpPhi %bool %false %19 %54 %49
%56 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %56 %55
%59 = OpLoad %v4float %value
%60 = OpVectorShuffle %v2float %59 %59 0 1
%58 = OpExtInst %v2float %1 Frexp %60 %4294967295
%62 = OpLoad %v4float %result
%63 = OpVectorShuffle %v4float %62 %58 4 5 2 3
OpStore %result %63
%64 = OpLoad %v4float %result
%65 = OpCompositeExtract %float %64 1
%66 = OpFOrdEqual %bool %65 %float_0_75
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%69 = OpLoad %v4int %exp
%70 = OpCompositeExtract %int %69 1
%71 = OpIEqual %bool %70 %int_3
OpBranch %68
%68 = OpLabel
%72 = OpPhi %bool %false %50 %71 %67
%73 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %73 %72
%76 = OpLoad %v4float %value
%77 = OpVectorShuffle %v3float %76 %76 0 1 2
%75 = OpExtInst %v3float %1 Frexp %77 %4294967295
%79 = OpLoad %v4float %result
%80 = OpVectorShuffle %v4float %79 %75 4 5 6 3
OpStore %result %80
%81 = OpLoad %v4float %result
%82 = OpCompositeExtract %float %81 2
%83 = OpFOrdEqual %bool %82 %float_0_75
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %v4int %exp
%87 = OpCompositeExtract %int %86 2
%88 = OpIEqual %bool %87 %int_3
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %68 %88 %84
%90 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %90 %89
%93 = OpLoad %v4float %value
%92 = OpExtInst %v4float %1 Frexp %93 %exp
OpStore %result %92
%94 = OpLoad %v4float %result
%95 = OpCompositeExtract %float %94 3
%96 = OpFOrdEqual %bool %95 %float_0_75
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %v4int %exp
%100 = OpCompositeExtract %int %99 3
%101 = OpIEqual %bool %100 %int_3
OpBranch %98
%98 = OpLabel
%102 = OpPhi %bool %false %85 %101 %97
%103 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %103 %102
%105 = OpLoad %v4bool %ok
%104 = OpAll %bool %105
OpSelectionMerge %109 None
OpBranchConditional %104 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%111 = OpLoad %v4float %110
OpStore %106 %111
OpBranch %109
%108 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%113 = OpLoad %v4float %112
OpStore %106 %113
OpBranch %109
%109 = OpLabel
%114 = OpLoad %v4float %106
OpReturnValue %114
OpFunctionEnd

1 error

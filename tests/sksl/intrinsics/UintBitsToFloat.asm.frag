OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInput"
OpMemberName %_UniformBuffer 1 "testMatrix2x2"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inputVal "inputVal"
OpName %expectedB "expectedB"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 ColMajor
OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 Offset 48
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %float %mat2v2float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_n1 = OpConstant %float -1
%41 = OpConstantComposite %v4float %float_1 %float_1 %float_n1 %float_n1
%uint = OpTypeInt 32 0
%v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%uint_1065353216 = OpConstant %uint 1065353216
%uint_1073741824 = OpConstant %uint 1073741824
%uint_3225419776 = OpConstant %uint 3225419776
%uint_3229614080 = OpConstant %uint 3229614080
%51 = OpConstantComposite %v4uint %uint_1065353216 %uint_1073741824 %uint_3225419776 %uint_3229614080
%false = OpConstantFalse %bool
%v2uint = OpTypeVector %uint 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3uint = OpTypeVector %uint 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%inputVal = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4uint Function
%93 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_1
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
%42 = OpFMul %v4float %38 %41
OpStore %inputVal %42
OpStore %expectedB %51
%53 = OpLoad %v4float %inputVal
%54 = OpCompositeExtract %float %53 0
%56 = OpLoad %v4uint %expectedB
%57 = OpCompositeExtract %uint %56 0
%55 = OpBitcast %float %57
%58 = OpFOrdEqual %bool %54 %55
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %v4float %inputVal
%62 = OpVectorShuffle %v2float %61 %61 0 1
%64 = OpLoad %v4uint %expectedB
%65 = OpVectorShuffle %v2uint %64 %64 0 1
%63 = OpBitcast %v2float %65
%67 = OpFOrdEqual %v2bool %62 %63
%69 = OpAll %bool %67
OpBranch %60
%60 = OpLabel
%70 = OpPhi %bool %false %26 %69 %59
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpLoad %v4float %inputVal
%74 = OpVectorShuffle %v3float %73 %73 0 1 2
%77 = OpLoad %v4uint %expectedB
%78 = OpVectorShuffle %v3uint %77 %77 0 1 2
%76 = OpBitcast %v3float %78
%80 = OpFOrdEqual %v3bool %74 %76
%82 = OpAll %bool %80
OpBranch %72
%72 = OpLabel
%83 = OpPhi %bool %false %60 %82 %71
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %v4float %inputVal
%88 = OpLoad %v4uint %expectedB
%87 = OpBitcast %v4float %88
%89 = OpFOrdEqual %v4bool %86 %87
%91 = OpAll %bool %89
OpBranch %85
%85 = OpLabel
%92 = OpPhi %bool %false %72 %91 %84
OpSelectionMerge %96 None
OpBranchConditional %92 %94 %95
%94 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%100 = OpLoad %v4float %97
OpStore %93 %100
OpBranch %96
%95 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%103 = OpLoad %v4float %101
OpStore %93 %103
OpBranch %96
%96 = OpLabel
%104 = OpLoad %v4float %93
OpReturnValue %104
OpFunctionEnd

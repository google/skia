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
OpDecorate %96 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
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
%63 = OpConstantComposite %v2uint %uint_1065353216 %uint_1073741824
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3uint = OpTypeVector %uint 3
%75 = OpConstantComposite %v3uint %uint_1065353216 %uint_1073741824 %uint_3225419776
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
%89 = OpVariable %_ptr_Function_v4float Function
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
%54 = OpLoad %v4float %inputVal
%55 = OpCompositeExtract %float %54 0
%53 = OpBitcast %uint %55
%56 = OpIEqual %bool %53 %uint_1065353216
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%60 = OpLoad %v4float %inputVal
%61 = OpVectorShuffle %v2float %60 %60 0 1
%59 = OpBitcast %v2uint %61
%64 = OpIEqual %v2bool %59 %63
%66 = OpAll %bool %64
OpBranch %58
%58 = OpLabel
%67 = OpPhi %bool %false %26 %66 %57
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpLoad %v4float %inputVal
%72 = OpVectorShuffle %v3float %71 %71 0 1 2
%70 = OpBitcast %v3uint %72
%76 = OpIEqual %v3bool %70 %75
%78 = OpAll %bool %76
OpBranch %69
%69 = OpLabel
%79 = OpPhi %bool %false %58 %78 %68
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%83 = OpLoad %v4float %inputVal
%82 = OpBitcast %v4uint %83
%84 = OpLoad %v4uint %expectedB
%85 = OpIEqual %v4bool %82 %84
%87 = OpAll %bool %85
OpBranch %81
%81 = OpLabel
%88 = OpPhi %bool %false %69 %87 %80
OpSelectionMerge %92 None
OpBranchConditional %88 %90 %91
%90 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%96 = OpLoad %v4float %93
OpStore %89 %96
OpBranch %92
%91 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%99 = OpLoad %v4float %97
OpStore %89 %99
OpBranch %92
%92 = OpLabel
%100 = OpLoad %v4float %89
OpReturnValue %100
OpFunctionEnd

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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%48 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%v4bool = OpTypeVector %bool 4
%v4int = OpTypeVector %int 4
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%85 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%104 = OpConstantComposite %v4bool %true %true %true %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%ok = OpVariable %_ptr_Function_bool Function
%109 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%31 = OpLoad %bool %ok
OpSelectionMerge %33 None
OpBranchConditional %31 %32 %33
%32 = OpLabel
%34 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%38 = OpLoad %mat2v2float %34
%39 = OpCompositeExtract %float %38 0 0
%40 = OpCompositeExtract %float %38 0 1
%41 = OpCompositeExtract %float %38 1 0
%42 = OpCompositeExtract %float %38 1 1
%43 = OpCompositeConstruct %v4float %39 %40 %41 %42
%49 = OpFOrdEqual %v4bool %43 %48
%51 = OpAll %bool %49
OpBranch %33
%33 = OpLabel
%52 = OpPhi %bool %false %26 %51 %32
OpStore %ok %52
%53 = OpLoad %bool %ok
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%57 = OpLoad %mat2v2float %56
%58 = OpCompositeExtract %float %57 0 0
%59 = OpCompositeExtract %float %57 0 1
%60 = OpCompositeExtract %float %57 1 0
%61 = OpCompositeExtract %float %57 1 1
%62 = OpCompositeConstruct %v4float %58 %59 %60 %61
%63 = OpFOrdEqual %v4bool %62 %48
%64 = OpAll %bool %63
OpBranch %55
%55 = OpLabel
%65 = OpPhi %bool %false %33 %64 %54
OpStore %ok %65
%66 = OpLoad %bool %ok
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%70 = OpLoad %mat2v2float %69
%71 = OpCompositeExtract %float %70 0 0
%72 = OpCompositeExtract %float %70 0 1
%73 = OpCompositeExtract %float %70 1 0
%74 = OpCompositeExtract %float %70 1 1
%75 = OpCompositeConstruct %v4float %71 %72 %73 %74
%76 = OpConvertFToS %int %71
%77 = OpConvertFToS %int %72
%78 = OpConvertFToS %int %73
%79 = OpConvertFToS %int %74
%81 = OpCompositeConstruct %v4int %76 %77 %78 %79
%86 = OpIEqual %v4bool %81 %85
%87 = OpAll %bool %86
OpBranch %68
%68 = OpLabel
%88 = OpPhi %bool %false %55 %87 %67
OpStore %ok %88
%89 = OpLoad %bool %ok
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%93 = OpLoad %mat2v2float %92
%94 = OpCompositeExtract %float %93 0 0
%95 = OpCompositeExtract %float %93 0 1
%96 = OpCompositeExtract %float %93 1 0
%97 = OpCompositeExtract %float %93 1 1
%98 = OpCompositeConstruct %v4float %94 %95 %96 %97
%99 = OpFUnordNotEqual %bool %94 %float_0
%100 = OpFUnordNotEqual %bool %95 %float_0
%101 = OpFUnordNotEqual %bool %96 %float_0
%102 = OpFUnordNotEqual %bool %97 %float_0
%103 = OpCompositeConstruct %v4bool %99 %100 %101 %102
%105 = OpLogicalEqual %v4bool %103 %104
%106 = OpAll %bool %105
OpBranch %91
%91 = OpLabel
%107 = OpPhi %bool %false %68 %106 %90
OpStore %ok %107
%108 = OpLoad %bool %ok
OpSelectionMerge %113 None
OpBranchConditional %108 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%117 = OpLoad %v4float %114
OpStore %109 %117
OpBranch %113
%112 = OpLabel
%118 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%119 = OpLoad %v4float %118
OpStore %109 %119
OpBranch %113
%113 = OpLabel
%120 = OpLoad %v4float %109
OpReturnValue %120
OpFunctionEnd

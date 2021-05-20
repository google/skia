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
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_ok "_0_ok"
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
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%int_1 = OpConstant %int 1
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%int_2 = OpConstant %int 2
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%123 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%33 = OpLoad %bool %_0_ok
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%36 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%41 = OpAccessChain %_ptr_Uniform_v3float %36 %int_0
%43 = OpLoad %v3float %41
%44 = OpCompositeExtract %float %43 0
%46 = OpFOrdEqual %bool %44 %float_1
OpBranch %35
%35 = OpLabel
%47 = OpPhi %bool %false %28 %46 %34
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%51 = OpAccessChain %_ptr_Uniform_v3float %50 %int_0
%52 = OpLoad %v3float %51
%53 = OpCompositeExtract %float %52 1
%55 = OpFOrdEqual %bool %53 %float_2
OpBranch %49
%49 = OpLabel
%56 = OpPhi %bool %false %35 %55 %48
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%60 = OpAccessChain %_ptr_Uniform_v3float %59 %int_0
%61 = OpLoad %v3float %60
%62 = OpCompositeExtract %float %61 2
%64 = OpFOrdEqual %bool %62 %float_3
OpBranch %58
%58 = OpLabel
%65 = OpPhi %bool %false %49 %64 %57
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%70 = OpAccessChain %_ptr_Uniform_v3float %68 %int_1
%71 = OpLoad %v3float %70
%72 = OpCompositeExtract %float %71 0
%74 = OpFOrdEqual %bool %72 %float_4
OpBranch %67
%67 = OpLabel
%75 = OpPhi %bool %false %58 %74 %66
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%79 = OpAccessChain %_ptr_Uniform_v3float %78 %int_1
%80 = OpLoad %v3float %79
%81 = OpCompositeExtract %float %80 1
%83 = OpFOrdEqual %bool %81 %float_5
OpBranch %77
%77 = OpLabel
%84 = OpPhi %bool %false %67 %83 %76
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%88 = OpAccessChain %_ptr_Uniform_v3float %87 %int_1
%89 = OpLoad %v3float %88
%90 = OpCompositeExtract %float %89 2
%92 = OpFOrdEqual %bool %90 %float_6
OpBranch %86
%86 = OpLabel
%93 = OpPhi %bool %false %77 %92 %85
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%98 = OpAccessChain %_ptr_Uniform_v3float %96 %int_2
%99 = OpLoad %v3float %98
%100 = OpCompositeExtract %float %99 0
%102 = OpFOrdEqual %bool %100 %float_7
OpBranch %95
%95 = OpLabel
%103 = OpPhi %bool %false %86 %102 %94
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%107 = OpAccessChain %_ptr_Uniform_v3float %106 %int_2
%108 = OpLoad %v3float %107
%109 = OpCompositeExtract %float %108 1
%111 = OpFOrdEqual %bool %109 %float_8
OpBranch %105
%105 = OpLabel
%112 = OpPhi %bool %false %95 %111 %104
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%116 = OpAccessChain %_ptr_Uniform_v3float %115 %int_2
%117 = OpLoad %v3float %116
%118 = OpCompositeExtract %float %117 2
%120 = OpFOrdEqual %bool %118 %float_9
OpBranch %114
%114 = OpLabel
%121 = OpPhi %bool %false %105 %120 %113
OpStore %_0_ok %121
%122 = OpLoad %bool %_0_ok
OpSelectionMerge %127 None
OpBranchConditional %122 %125 %126
%125 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%130 = OpLoad %v4float %128
OpStore %123 %130
OpBranch %127
%126 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %131
OpStore %123 %132
OpBranch %127
%127 = OpLabel
%133 = OpLoad %v4float %123
OpReturnValue %133
OpFunctionEnd

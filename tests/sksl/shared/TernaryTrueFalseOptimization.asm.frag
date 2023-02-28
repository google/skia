OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%ok = OpVariable %_ptr_Function_bool Function
%116 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
OpSelectionMerge %31 None
OpBranchConditional %true %30 %31
%30 = OpLabel
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 1
%39 = OpFOrdEqual %bool %37 %float_1
OpBranch %31
%31 = OpLabel
%40 = OpPhi %bool %false %25 %39 %30
OpStore %ok %40
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%45 = OpLoad %v4float %44
%46 = OpCompositeExtract %float %45 0
%47 = OpFOrdEqual %bool %46 %float_1
%43 = OpLogicalNot %bool %47
OpBranch %42
%42 = OpLabel
%48 = OpPhi %bool %false %31 %43 %41
OpStore %ok %48
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%52 = OpLoad %v4float %51
%53 = OpVectorShuffle %v2float %52 %52 1 0
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%56 = OpLoad %v4float %54
%57 = OpVectorShuffle %v2float %56 %56 0 1
%58 = OpFOrdEqual %v2bool %53 %57
%60 = OpAll %bool %58
OpBranch %50
%50 = OpLabel
%61 = OpPhi %bool %false %42 %60 %49
OpStore %ok %61
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v2float %66 %66 1 0
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%69 = OpLoad %v4float %68
%70 = OpVectorShuffle %v2float %69 %69 0 1
%71 = OpFUnordNotEqual %v2bool %67 %70
%72 = OpAny %bool %71
%64 = OpLogicalNot %bool %72
OpBranch %63
%63 = OpLabel
%73 = OpPhi %bool %false %50 %64 %62
OpStore %ok %73
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%78 = OpVectorShuffle %v2float %77 %77 1 0
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%80 = OpLoad %v4float %79
%81 = OpVectorShuffle %v2float %80 %80 0 1
%82 = OpFOrdEqual %v2bool %78 %81
%83 = OpAll %bool %82
OpSelectionMerge %85 None
OpBranchConditional %83 %85 %84
%84 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%87 = OpLoad %v4float %86
%88 = OpCompositeExtract %float %87 3
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%90 = OpLoad %v4float %89
%91 = OpCompositeExtract %float %90 3
%92 = OpFUnordNotEqual %bool %88 %91
OpBranch %85
%85 = OpLabel
%93 = OpPhi %bool %true %74 %92 %84
OpBranch %75
%75 = OpLabel
%94 = OpPhi %bool %false %63 %93 %85
OpStore %ok %94
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%98 = OpLoad %v4float %97
%99 = OpVectorShuffle %v2float %98 %98 1 0
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%101 = OpLoad %v4float %100
%102 = OpVectorShuffle %v2float %101 %101 0 1
%103 = OpFUnordNotEqual %v2bool %99 %102
%104 = OpAny %bool %103
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%108 = OpLoad %v4float %107
%109 = OpCompositeExtract %float %108 3
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%111 = OpLoad %v4float %110
%112 = OpCompositeExtract %float %111 3
%113 = OpFOrdEqual %bool %109 %112
OpBranch %106
%106 = OpLabel
%114 = OpPhi %bool %false %95 %113 %105
OpBranch %96
%96 = OpLabel
%115 = OpPhi %bool %false %75 %114 %106
OpStore %ok %115
OpSelectionMerge %120 None
OpBranchConditional %115 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%122 = OpLoad %v4float %121
OpStore %116 %122
OpBranch %120
%119 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%124 = OpLoad %v4float %123
OpStore %116 %124
OpBranch %120
%120 = OpLabel
%125 = OpLoad %v4float %116
OpReturnValue %125
OpFunctionEnd

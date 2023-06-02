OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpName %_entrypoint_v "_entrypoint_v"
OpName %IsEqual_bh4h4 "IsEqual_bh4h4"
OpName %main "main"
OpName %colorBlue "colorBlue"
OpName %colorGreen "colorGreen"
OpName %colorRed "colorRed"
OpName %result "result"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %colorBlue RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %colorGreen RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %colorRed RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%25 = OpTypeFunction %bool %_ptr_Function_v4float %_ptr_Function_v4float
%v4bool = OpTypeVector %bool 4
%34 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%IsEqual_bh4h4 = OpFunction %bool None %25
%26 = OpFunctionParameter %_ptr_Function_v4float
%27 = OpFunctionParameter %_ptr_Function_v4float
%28 = OpLabel
%29 = OpLoad %v4float %26
%30 = OpLoad %v4float %27
%31 = OpFOrdEqual %v4bool %29 %30
%33 = OpAll %bool %31
OpReturnValue %33
OpFunctionEnd
%main = OpFunction %v4float None %34
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpLabel
%colorBlue = OpVariable %_ptr_Function_v4float Function
%colorGreen = OpVariable %_ptr_Function_v4float Function
%colorRed = OpVariable %_ptr_Function_v4float Function
%result = OpVariable %_ptr_Function_v4float Function
%67 = OpVariable %_ptr_Function_v4float Function
%68 = OpVariable %_ptr_Function_v4float Function
%70 = OpVariable %_ptr_Function_v4float Function
%74 = OpVariable %_ptr_Function_v4float Function
%75 = OpVariable %_ptr_Function_v4float Function
%80 = OpVariable %_ptr_Function_v4float Function
%81 = OpVariable %_ptr_Function_v4float Function
%88 = OpVariable %_ptr_Function_v4float Function
%89 = OpVariable %_ptr_Function_v4float Function
%91 = OpVariable %_ptr_Function_v4float Function
%98 = OpVariable %_ptr_Function_v4float Function
%99 = OpVariable %_ptr_Function_v4float Function
%101 = OpVariable %_ptr_Function_v4float Function
%105 = OpVariable %_ptr_Function_v4float Function
%108 = OpVariable %_ptr_Function_v4float Function
%38 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%42 = OpLoad %v4float %38
%43 = OpVectorShuffle %v2float %42 %42 2 3
%44 = OpCompositeExtract %float %43 0
%45 = OpCompositeExtract %float %43 1
%46 = OpCompositeConstruct %v4float %float_0 %float_0 %44 %45
OpStore %colorBlue %46
%48 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%49 = OpLoad %v4float %48
%50 = OpCompositeExtract %float %49 1
%51 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%52 = OpLoad %v4float %51
%53 = OpCompositeExtract %float %52 3
%54 = OpCompositeConstruct %v4float %float_0 %50 %float_0 %53
OpStore %colorGreen %54
%56 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%57 = OpLoad %v4float %56
%58 = OpCompositeExtract %float %57 0
%59 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%60 = OpLoad %v4float %59
%61 = OpCompositeExtract %float %60 3
%62 = OpCompositeConstruct %v4float %58 %float_0 %float_0 %61
OpStore %colorRed %62
%65 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%66 = OpLoad %v4float %65
OpStore %67 %66
OpStore %68 %46
%69 = OpFunctionCall %bool %IsEqual_bh4h4 %67 %68
%64 = OpLogicalNot %bool %69
OpSelectionMerge %73 None
OpBranchConditional %64 %71 %72
%71 = OpLabel
OpStore %74 %54
OpStore %75 %62
%76 = OpFunctionCall %bool %IsEqual_bh4h4 %74 %75
%77 = OpCompositeConstruct %v4bool %76 %76 %76 %76
%78 = OpSelect %v4float %77 %62 %54
OpStore %70 %78
OpBranch %73
%72 = OpLabel
OpStore %80 %62
OpStore %81 %54
%82 = OpFunctionCall %bool %IsEqual_bh4h4 %80 %81
%79 = OpLogicalNot %bool %82
%83 = OpCompositeConstruct %v4bool %79 %79 %79 %79
%85 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%86 = OpLoad %v4float %85
%84 = OpSelect %v4float %83 %46 %86
OpStore %70 %84
OpBranch %73
%73 = OpLabel
%87 = OpLoad %v4float %70
OpStore %result %87
OpStore %88 %62
OpStore %89 %46
%90 = OpFunctionCall %bool %IsEqual_bh4h4 %88 %89
OpSelectionMerge %94 None
OpBranchConditional %90 %92 %93
%92 = OpLabel
%95 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%96 = OpLoad %v4float %95
OpStore %91 %96
OpBranch %94
%93 = OpLabel
OpStore %98 %62
OpStore %99 %54
%100 = OpFunctionCall %bool %IsEqual_bh4h4 %98 %99
%97 = OpLogicalNot %bool %100
OpSelectionMerge %104 None
OpBranchConditional %97 %102 %103
%102 = OpLabel
OpStore %101 %87
OpBranch %104
%103 = OpLabel
OpStore %105 %62
%106 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%107 = OpLoad %v4float %106
OpStore %108 %107
%109 = OpFunctionCall %bool %IsEqual_bh4h4 %105 %108
%110 = OpCompositeConstruct %v4bool %109 %109 %109 %109
%111 = OpSelect %v4float %110 %46 %62
OpStore %101 %111
OpBranch %104
%104 = OpLabel
%112 = OpLoad %v4float %101
OpStore %91 %112
OpBranch %94
%94 = OpLabel
%113 = OpLoad %v4float %91
OpReturnValue %113
OpFunctionEnd

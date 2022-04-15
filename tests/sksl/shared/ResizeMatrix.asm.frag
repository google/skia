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
OpName %result "result"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
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
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%33 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%34 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%35 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%mat3v3float = OpTypeMatrix %v3float 3
%37 = OpConstantComposite %mat3v3float %33 %34 %35
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%49 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%50 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
%51 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
%52 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%mat4v4float = OpTypeMatrix %v4float 4
%54 = OpConstantComposite %mat4v4float %49 %50 %51 %52
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%76 = OpConstantComposite %v2float %float_1 %float_0
%77 = OpConstantComposite %v2float %float_0 %float_1
%78 = OpConstantComposite %mat2v2float %76 %77
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
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
%result = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_mat2v2float Function
%b = OpVariable %_ptr_Function_mat2v2float Function
%c = OpVariable %_ptr_Function_mat3v3float Function
%d = OpVariable %_ptr_Function_mat3v3float Function
%e = OpVariable %_ptr_Function_mat4v4float Function
%f = OpVariable %_ptr_Function_mat2v2float Function
%108 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%38 = OpVectorShuffle %v2float %33 %33 0 1
%39 = OpVectorShuffle %v2float %34 %34 0 1
%40 = OpCompositeConstruct %mat2v2float %38 %39
OpStore %a %40
%41 = OpLoad %float %result
%44 = OpAccessChain %_ptr_Function_v2float %a %int_0
%45 = OpLoad %v2float %44
%46 = OpCompositeExtract %float %45 0
%47 = OpFAdd %float %41 %46
OpStore %result %47
%55 = OpVectorShuffle %v2float %49 %49 0 1
%56 = OpVectorShuffle %v2float %50 %50 0 1
%57 = OpCompositeConstruct %mat2v2float %55 %56
OpStore %b %57
%58 = OpLoad %float %result
%59 = OpAccessChain %_ptr_Function_v2float %b %int_0
%60 = OpLoad %v2float %59
%61 = OpCompositeExtract %float %60 0
%62 = OpFAdd %float %58 %61
OpStore %result %62
%65 = OpVectorShuffle %v3float %49 %49 0 1 2
%66 = OpVectorShuffle %v3float %50 %50 0 1 2
%67 = OpVectorShuffle %v3float %51 %51 0 1 2
%68 = OpCompositeConstruct %mat3v3float %65 %66 %67
OpStore %c %68
%69 = OpLoad %float %result
%70 = OpAccessChain %_ptr_Function_v3float %c %int_0
%72 = OpLoad %v3float %70
%73 = OpCompositeExtract %float %72 0
%74 = OpFAdd %float %69 %73
OpStore %result %74
OpStore %d %37
%79 = OpLoad %float %result
%80 = OpAccessChain %_ptr_Function_v3float %d %int_0
%81 = OpLoad %v3float %80
%82 = OpCompositeExtract %float %81 0
%83 = OpFAdd %float %79 %82
OpStore %result %83
OpStore %e %54
%86 = OpLoad %float %result
%87 = OpAccessChain %_ptr_Function_v4float %e %int_0
%89 = OpLoad %v4float %87
%90 = OpCompositeExtract %float %89 0
%91 = OpFAdd %float %86 %90
OpStore %result %91
%93 = OpVectorShuffle %v3float %49 %49 0 1 2
%94 = OpVectorShuffle %v3float %50 %50 0 1 2
%95 = OpVectorShuffle %v3float %51 %51 0 1 2
%96 = OpCompositeConstruct %mat3v3float %93 %94 %95
%97 = OpVectorShuffle %v2float %93 %93 0 1
%98 = OpVectorShuffle %v2float %94 %94 0 1
%99 = OpCompositeConstruct %mat2v2float %97 %98
OpStore %f %99
%100 = OpLoad %float %result
%101 = OpAccessChain %_ptr_Function_v2float %f %int_0
%102 = OpLoad %v2float %101
%103 = OpCompositeExtract %float %102 0
%104 = OpFAdd %float %100 %103
OpStore %result %104
%105 = OpLoad %float %result
%107 = OpFOrdEqual %bool %105 %float_6
OpSelectionMerge %111 None
OpBranchConditional %107 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%114 = OpLoad %v4float %112
OpStore %108 %114
OpBranch %111
%110 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%117 = OpLoad %v4float %115
OpStore %108 %117
OpBranch %111
%111 = OpLabel
%118 = OpLoad %v4float %108
OpReturnValue %118
OpFunctionEnd

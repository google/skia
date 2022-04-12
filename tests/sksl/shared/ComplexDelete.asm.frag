OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %s "s"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorXform"
OpName %main "main"
OpName %tmpColor "tmpColor"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %s RelaxedPrecision
OpDecorate %s Binding 0
OpDecorate %s DescriptorSet 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %24 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%12 = OpTypeSampledImage %13
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%s = OpVariable %_ptr_UniformConstant_12 UniformConstant
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%27 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%34 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%35 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
%36 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
%37 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%38 = OpConstantComposite %mat4v4float %34 %35 %36 %37
%v4bool = OpTypeVector %bool 4
%v3float = OpTypeVector %float 3
%71 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%main = OpFunction %void None %19
%20 = OpLabel
%tmpColor = OpVariable %_ptr_Function_v4float Function
%55 = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %12 %s
%23 = OpImageSampleImplicitLod %v4float %24 %27
OpStore %tmpColor %23
%28 = OpAccessChain %_ptr_Uniform_mat4v4float %14 %int_0
%32 = OpLoad %mat4v4float %28
%40 = OpCompositeExtract %v4float %32 0
%41 = OpFUnordNotEqual %v4bool %40 %34
%42 = OpAny %bool %41
%43 = OpCompositeExtract %v4float %32 1
%44 = OpFUnordNotEqual %v4bool %43 %35
%45 = OpAny %bool %44
%46 = OpLogicalOr %bool %42 %45
%47 = OpCompositeExtract %v4float %32 2
%48 = OpFUnordNotEqual %v4bool %47 %36
%49 = OpAny %bool %48
%50 = OpLogicalOr %bool %46 %49
%51 = OpCompositeExtract %v4float %32 3
%52 = OpFUnordNotEqual %v4bool %51 %37
%53 = OpAny %bool %52
%54 = OpLogicalOr %bool %50 %53
OpSelectionMerge %58 None
OpBranchConditional %54 %56 %57
%56 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_mat4v4float %14 %int_0
%61 = OpLoad %mat4v4float %60
%62 = OpLoad %v4float %tmpColor
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%65 = OpCompositeExtract %float %63 0
%66 = OpCompositeExtract %float %63 1
%67 = OpCompositeExtract %float %63 2
%68 = OpCompositeConstruct %v4float %65 %66 %67 %float_1
%69 = OpMatrixTimesVector %v4float %61 %68
%70 = OpVectorShuffle %v3float %69 %69 0 1 2
%72 = OpLoad %v4float %tmpColor
%73 = OpCompositeExtract %float %72 3
%74 = OpCompositeConstruct %v3float %73 %73 %73
%59 = OpExtInst %v3float %1 FClamp %70 %71 %74
%75 = OpCompositeExtract %float %59 0
%76 = OpCompositeExtract %float %59 1
%77 = OpCompositeExtract %float %59 2
%78 = OpLoad %v4float %tmpColor
%79 = OpCompositeExtract %float %78 3
%80 = OpCompositeConstruct %v4float %75 %76 %77 %79
OpStore %55 %80
OpBranch %58
%57 = OpLabel
%81 = OpLoad %v4float %tmpColor
OpStore %55 %81
OpBranch %58
%58 = OpLabel
%82 = OpLoad %v4float %55
OpStore %sk_FragColor %82
OpReturn
OpFunctionEnd

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
OpMemberDecorate %_UniformBuffer 0 DescriptorSet 0
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
%v2float = OpTypeVector %float 2
%float_1 = OpConstant %float 1
%27 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%v4bool = OpTypeVector %bool 4
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %19
%20 = OpLabel
%tmpColor = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %12 %s
%23 = OpImageSampleImplicitLod %v4float %24 %27
OpStore %tmpColor %23
%28 = OpAccessChain %_ptr_Uniform_mat4v4float %14 %int_0
%32 = OpLoad %mat4v4float %28
%35 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%36 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%37 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%38 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%33 = OpCompositeConstruct %mat4v4float %35 %36 %37 %38
%40 = OpCompositeExtract %v4float %32 0
%41 = OpCompositeExtract %v4float %33 0
%42 = OpFOrdNotEqual %v4bool %40 %41
%43 = OpAny %bool %42
%44 = OpCompositeExtract %v4float %32 1
%45 = OpCompositeExtract %v4float %33 1
%46 = OpFOrdNotEqual %v4bool %44 %45
%47 = OpAny %bool %46
%48 = OpLogicalOr %bool %43 %47
%49 = OpCompositeExtract %v4float %32 2
%50 = OpCompositeExtract %v4float %33 2
%51 = OpFOrdNotEqual %v4bool %49 %50
%52 = OpAny %bool %51
%53 = OpLogicalOr %bool %48 %52
%54 = OpCompositeExtract %v4float %32 3
%55 = OpCompositeExtract %v4float %33 3
%56 = OpFOrdNotEqual %v4bool %54 %55
%57 = OpAny %bool %56
%58 = OpLogicalOr %bool %53 %57
OpSelectionMerge %62 None
OpBranchConditional %58 %60 %61
%60 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_mat4v4float %14 %int_0
%65 = OpLoad %mat4v4float %64
%66 = OpLoad %v4float %tmpColor
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%69 = OpCompositeExtract %float %67 0
%70 = OpCompositeExtract %float %67 1
%71 = OpCompositeExtract %float %67 2
%72 = OpCompositeConstruct %v4float %69 %70 %71 %float_1
%73 = OpMatrixTimesVector %v4float %65 %72
%74 = OpVectorShuffle %v3float %73 %73 0 1 2
%75 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%76 = OpLoad %v4float %tmpColor
%77 = OpCompositeExtract %float %76 3
%78 = OpCompositeConstruct %v3float %77 %77 %77
%63 = OpExtInst %v3float %1 FClamp %74 %75 %78
%79 = OpCompositeExtract %float %63 0
%80 = OpCompositeExtract %float %63 1
%81 = OpCompositeExtract %float %63 2
%82 = OpLoad %v4float %tmpColor
%83 = OpCompositeExtract %float %82 3
%84 = OpCompositeConstruct %v4float %79 %80 %81 %83
OpStore %59 %84
OpBranch %62
%61 = OpLabel
%85 = OpLoad %v4float %tmpColor
OpStore %59 %85
OpBranch %62
%62 = OpLabel
%86 = OpLoad %v4float %59
OpStore %sk_FragColor %86
OpReturn
OpFunctionEnd

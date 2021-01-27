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
OpName %colorXform "colorXform"
OpName %main "main"
OpName %tmpColor "tmpColor"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
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
OpDecorate %colorXform DescriptorSet 0
OpDecorate %26 RelaxedPrecision
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
%_ptr_Private_mat4v4float = OpTypePointer Private %mat4v4float
%colorXform = OpVariable %_ptr_Private_mat4v4float Private
%void = OpTypeVoid
%21 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v2float = OpTypeVector %float 2
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v2float %float_1 %float_1
%float_0 = OpConstant %float 0
%v4bool = OpTypeVector %bool 4
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %21
%22 = OpLabel
%tmpColor = OpVariable %_ptr_Function_v4float Function
%57 = OpVariable %_ptr_Function_v4float Function
%26 = OpLoad %12 %s
%25 = OpImageSampleImplicitLod %v4float %26 %29
OpStore %tmpColor %25
%30 = OpLoad %mat4v4float %colorXform
%33 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%34 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%35 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%36 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%31 = OpCompositeConstruct %mat4v4float %33 %34 %35 %36
%38 = OpCompositeExtract %v4float %30 0
%39 = OpCompositeExtract %v4float %31 0
%40 = OpFOrdNotEqual %v4bool %38 %39
%41 = OpAny %bool %40
%42 = OpCompositeExtract %v4float %30 1
%43 = OpCompositeExtract %v4float %31 1
%44 = OpFOrdNotEqual %v4bool %42 %43
%45 = OpAny %bool %44
%46 = OpLogicalOr %bool %41 %45
%47 = OpCompositeExtract %v4float %30 2
%48 = OpCompositeExtract %v4float %31 2
%49 = OpFOrdNotEqual %v4bool %47 %48
%50 = OpAny %bool %49
%51 = OpLogicalOr %bool %46 %50
%52 = OpCompositeExtract %v4float %30 3
%53 = OpCompositeExtract %v4float %31 3
%54 = OpFOrdNotEqual %v4bool %52 %53
%55 = OpAny %bool %54
%56 = OpLogicalOr %bool %51 %55
OpSelectionMerge %60 None
OpBranchConditional %56 %58 %59
%58 = OpLabel
%62 = OpLoad %mat4v4float %colorXform
%63 = OpLoad %v4float %tmpColor
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%66 = OpCompositeExtract %float %64 0
%67 = OpCompositeExtract %float %64 1
%68 = OpCompositeExtract %float %64 2
%69 = OpCompositeConstruct %v4float %66 %67 %68 %float_1
%70 = OpMatrixTimesVector %v4float %62 %69
%71 = OpVectorShuffle %v3float %70 %70 0 1 2
%72 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%73 = OpLoad %v4float %tmpColor
%74 = OpCompositeExtract %float %73 3
%75 = OpCompositeConstruct %v3float %74 %74 %74
%61 = OpExtInst %v3float %1 FClamp %71 %72 %75
%76 = OpCompositeExtract %float %61 0
%77 = OpCompositeExtract %float %61 1
%78 = OpCompositeExtract %float %61 2
%79 = OpLoad %v4float %tmpColor
%80 = OpCompositeExtract %float %79 3
%81 = OpCompositeConstruct %v4float %76 %77 %78 %80
OpStore %57 %81
OpBranch %60
%59 = OpLabel
%82 = OpLoad %v4float %tmpColor
OpStore %57 %82
OpBranch %60
%60 = OpLabel
%83 = OpLoad %v4float %57
OpStore %sk_FragColor %83
OpReturn
OpFunctionEnd

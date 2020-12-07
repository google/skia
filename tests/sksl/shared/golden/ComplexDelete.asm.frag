### Compilation failed:

error: SPIR-V validation error: Uniform OpVariable <id> '10[%colorXform]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %colorXform = OpVariable %_ptr_Uniform_mat4v4float Uniform

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %colorXform "colorXform"
OpName %s "s"
OpName %main "main"
OpName %tmpColor "tmpColor"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %colorXform DescriptorSet 0
OpDecorate %s RelaxedPrecision
OpDecorate %s Binding 0
OpDecorate %23 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%colorXform = OpVariable %_ptr_Uniform_mat4v4float Uniform
%16 = OpTypeImage %float 2D 0 0 0 1 Unknown
%15 = OpTypeSampledImage %16
%_ptr_UniformConstant_15 = OpTypePointer UniformConstant %15
%s = OpVariable %_ptr_UniformConstant_15 UniformConstant
%void = OpTypeVoid
%18 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%24 = OpConstantComposite %v2float %float_1 %float_1
%float_0 = OpConstant %float 0
%v4bool = OpTypeVector %bool 4
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %18
%19 = OpLabel
%tmpColor = OpVariable %_ptr_Function_v4float Function
%54 = OpVariable %_ptr_Function_v4float Function
%23 = OpLoad %15 %s
%22 = OpImageSampleImplicitLod %v4float %23 %24
OpStore %tmpColor %22
%27 = OpLoad %mat4v4float %colorXform
%30 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%31 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%32 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%33 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%28 = OpCompositeConstruct %mat4v4float %30 %31 %32 %33
%35 = OpCompositeExtract %v4float %27 0
%36 = OpCompositeExtract %v4float %28 0
%37 = OpFOrdNotEqual %v4bool %35 %36
%38 = OpAny %bool %37
%39 = OpCompositeExtract %v4float %27 1
%40 = OpCompositeExtract %v4float %28 1
%41 = OpFOrdNotEqual %v4bool %39 %40
%42 = OpAny %bool %41
%43 = OpLogicalOr %bool %38 %42
%44 = OpCompositeExtract %v4float %27 2
%45 = OpCompositeExtract %v4float %28 2
%46 = OpFOrdNotEqual %v4bool %44 %45
%47 = OpAny %bool %46
%48 = OpLogicalOr %bool %43 %47
%49 = OpCompositeExtract %v4float %27 3
%50 = OpCompositeExtract %v4float %28 3
%51 = OpFOrdNotEqual %v4bool %49 %50
%52 = OpAny %bool %51
%53 = OpLogicalOr %bool %48 %52
OpSelectionMerge %57 None
OpBranchConditional %53 %55 %56
%55 = OpLabel
%59 = OpLoad %mat4v4float %colorXform
%60 = OpLoad %v4float %tmpColor
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%63 = OpCompositeExtract %float %61 0
%64 = OpCompositeExtract %float %61 1
%65 = OpCompositeExtract %float %61 2
%66 = OpCompositeConstruct %v4float %63 %64 %65 %float_1
%67 = OpMatrixTimesVector %v4float %59 %66
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%69 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%70 = OpLoad %v4float %tmpColor
%71 = OpCompositeExtract %float %70 3
%72 = OpCompositeConstruct %v3float %71 %71 %71
%58 = OpExtInst %v3float %1 FClamp %68 %69 %72
%73 = OpCompositeExtract %float %58 0
%74 = OpCompositeExtract %float %58 1
%75 = OpCompositeExtract %float %58 2
%76 = OpLoad %v4float %tmpColor
%77 = OpCompositeExtract %float %76 3
%78 = OpCompositeConstruct %v4float %73 %74 %75 %77
OpStore %54 %78
OpBranch %57
%56 = OpLabel
%79 = OpLoad %v4float %tmpColor
OpStore %54 %79
OpBranch %57
%57 = OpLabel
%80 = OpLoad %v4float %54
OpStore %sk_FragColor %80
OpReturn
OpFunctionEnd

1 error

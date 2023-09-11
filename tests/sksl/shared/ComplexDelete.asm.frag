               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %s "s"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorXform"
               OpName %main "main"
               OpName %tmpColor "tmpColor"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %s RelaxedPrecision
               OpDecorate %s Binding 0
               OpDecorate %s DescriptorSet 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %21 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
          %s = OpVariable %_ptr_UniformConstant_9 UniformConstant
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %31 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
         %32 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
         %33 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
         %34 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
         %35 = OpConstantComposite %mat4v4float %31 %32 %33 %34
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
    %v3float = OpTypeVector %float 3
         %68 = OpConstantComposite %v3float %float_0 %float_0 %float_0
       %main = OpFunction %void None %16
         %17 = OpLabel
   %tmpColor = OpVariable %_ptr_Function_v4float Function
         %53 = OpVariable %_ptr_Function_v4float Function
         %21 = OpLoad %9 %s
         %20 = OpImageSampleImplicitLod %v4float %21 %24
               OpStore %tmpColor %20
         %25 = OpAccessChain %_ptr_Uniform_mat4v4float %11 %int_0
         %29 = OpLoad %mat4v4float %25
         %38 = OpCompositeExtract %v4float %29 0
         %39 = OpFUnordNotEqual %v4bool %38 %31
         %40 = OpAny %bool %39
         %41 = OpCompositeExtract %v4float %29 1
         %42 = OpFUnordNotEqual %v4bool %41 %32
         %43 = OpAny %bool %42
         %44 = OpLogicalOr %bool %40 %43
         %45 = OpCompositeExtract %v4float %29 2
         %46 = OpFUnordNotEqual %v4bool %45 %33
         %47 = OpAny %bool %46
         %48 = OpLogicalOr %bool %44 %47
         %49 = OpCompositeExtract %v4float %29 3
         %50 = OpFUnordNotEqual %v4bool %49 %34
         %51 = OpAny %bool %50
         %52 = OpLogicalOr %bool %48 %51
               OpSelectionMerge %56 None
               OpBranchConditional %52 %54 %55
         %54 = OpLabel
         %58 = OpAccessChain %_ptr_Uniform_mat4v4float %11 %int_0
         %59 = OpLoad %mat4v4float %58
         %60 = OpVectorShuffle %v3float %20 %20 0 1 2
         %62 = OpCompositeExtract %float %60 0
         %63 = OpCompositeExtract %float %60 1
         %64 = OpCompositeExtract %float %60 2
         %65 = OpCompositeConstruct %v4float %62 %63 %64 %float_1
         %66 = OpMatrixTimesVector %v4float %59 %65
         %67 = OpVectorShuffle %v3float %66 %66 0 1 2
         %69 = OpCompositeExtract %float %20 3
         %70 = OpCompositeConstruct %v3float %69 %69 %69
         %57 = OpExtInst %v3float %1 FClamp %67 %68 %70
         %71 = OpCompositeExtract %float %57 0
         %72 = OpCompositeExtract %float %57 1
         %73 = OpCompositeExtract %float %57 2
         %74 = OpCompositeConstruct %v4float %71 %72 %73 %69
               OpStore %53 %74
               OpBranch %56
         %55 = OpLabel
               OpStore %53 %20
               OpBranch %56
         %56 = OpLabel
         %75 = OpLoad %v4float %53
               OpStore %sk_FragColor %75
               OpReturn
               OpFunctionEnd

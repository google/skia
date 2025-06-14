               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %s "s"                        ; id %11
               OpName %_UniformBuffer "_UniformBuffer"  ; id %17
               OpMemberName %_UniformBuffer 0 "colorXform"
               OpName %main "main"                  ; id %6
               OpName %tmpColor "tmpColor"          ; id %22

               ; Annotations
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
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %12 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %13 = OpTypeSampledImage %12
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
          %s = OpVariable %_ptr_UniformConstant_13 UniformConstant  ; RelaxedPrecision, Binding 0, DescriptorSet 0
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float         ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %28 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %34 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
         %35 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
         %36 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
         %37 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
         %38 = OpConstantComposite %mat4v4float %34 %35 %36 %37
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
    %v3float = OpTypeVector %float 3
         %71 = OpConstantComposite %v3float %float_0 %float_0 %float_0


               ; Function main
       %main = OpFunction %void None %20

         %21 = OpLabel
   %tmpColor =   OpVariable %_ptr_Function_v4float Function
         %56 =   OpVariable %_ptr_Function_v4float Function
         %25 =   OpLoad %13 %s                      ; RelaxedPrecision
         %24 =   OpImageSampleImplicitLod %v4float %25 %28  ; RelaxedPrecision
                 OpStore %tmpColor %24
         %29 =   OpAccessChain %_ptr_Uniform_mat4v4float %15 %int_0
         %32 =   OpLoad %mat4v4float %29
         %41 =   OpCompositeExtract %v4float %32 0
         %42 =   OpFUnordNotEqual %v4bool %41 %34
         %43 =   OpAny %bool %42
         %44 =   OpCompositeExtract %v4float %32 1
         %45 =   OpFUnordNotEqual %v4bool %44 %35
         %46 =   OpAny %bool %45
         %47 =   OpLogicalOr %bool %43 %46
         %48 =   OpCompositeExtract %v4float %32 2
         %49 =   OpFUnordNotEqual %v4bool %48 %36
         %50 =   OpAny %bool %49
         %51 =   OpLogicalOr %bool %47 %50
         %52 =   OpCompositeExtract %v4float %32 3
         %53 =   OpFUnordNotEqual %v4bool %52 %37
         %54 =   OpAny %bool %53
         %55 =   OpLogicalOr %bool %51 %54
                 OpSelectionMerge %59 None
                 OpBranchConditional %55 %57 %58

         %57 =     OpLabel
         %61 =       OpAccessChain %_ptr_Uniform_mat4v4float %15 %int_0
         %62 =       OpLoad %mat4v4float %61
         %63 =       OpVectorShuffle %v3float %24 %24 0 1 2
         %65 =       OpCompositeExtract %float %63 0
         %66 =       OpCompositeExtract %float %63 1
         %67 =       OpCompositeExtract %float %63 2
         %68 =       OpCompositeConstruct %v4float %65 %66 %67 %float_1
         %69 =       OpMatrixTimesVector %v4float %62 %68
         %70 =       OpVectorShuffle %v3float %69 %69 0 1 2
         %72 =       OpCompositeExtract %float %24 3
         %73 =       OpCompositeConstruct %v3float %72 %72 %72
         %60 =       OpExtInst %v3float %5 FClamp %70 %71 %73
         %74 =       OpCompositeExtract %float %60 0
         %75 =       OpCompositeExtract %float %60 1
         %76 =       OpCompositeExtract %float %60 2
         %77 =       OpCompositeConstruct %v4float %74 %75 %76 %72
                     OpStore %56 %77
                     OpBranch %59

         %58 =     OpLabel
                     OpStore %56 %24
                     OpBranch %59

         %59 = OpLabel
         %78 =   OpLoad %v4float %56
                 OpStore %sk_FragColor %78
                 OpReturn
               OpFunctionEnd

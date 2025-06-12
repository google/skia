               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %main "main"                  ; id %2

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3


               ; Function main
       %main = OpFunction %void None %11

         %12 = OpLabel
         %14 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %18 =   OpLoad %v4float %14                ; RelaxedPrecision
         %19 =   OpCompositeExtract %float %18 3    ; RelaxedPrecision
         %20 =   OpFSub %float %float_1 %19         ; RelaxedPrecision
         %21 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %23 =   OpLoad %v4float %21                ; RelaxedPrecision
         %24 =   OpVectorShuffle %v3float %23 %23 0 1 2     ; RelaxedPrecision
         %26 =   OpVectorTimesScalar %v3float %24 %20       ; RelaxedPrecision
         %27 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %28 =   OpLoad %v4float %27                ; RelaxedPrecision
         %29 =   OpCompositeExtract %float %28 3    ; RelaxedPrecision
         %30 =   OpFSub %float %float_1 %29         ; RelaxedPrecision
         %31 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %32 =   OpLoad %v4float %31                ; RelaxedPrecision
         %33 =   OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
         %34 =   OpVectorTimesScalar %v3float %33 %30       ; RelaxedPrecision
         %35 =   OpFAdd %v3float %26 %34                    ; RelaxedPrecision
         %36 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %37 =   OpLoad %v4float %36                ; RelaxedPrecision
         %38 =   OpVectorShuffle %v3float %37 %37 0 1 2     ; RelaxedPrecision
         %39 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %40 =   OpLoad %v4float %39                ; RelaxedPrecision
         %41 =   OpVectorShuffle %v3float %40 %40 0 1 2     ; RelaxedPrecision
         %42 =   OpFMul %v3float %38 %41                    ; RelaxedPrecision
         %43 =   OpFAdd %v3float %35 %42                    ; RelaxedPrecision
         %44 =   OpCompositeExtract %float %43 0            ; RelaxedPrecision
         %45 =   OpCompositeExtract %float %43 1            ; RelaxedPrecision
         %46 =   OpCompositeExtract %float %43 2            ; RelaxedPrecision
         %47 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %48 =   OpLoad %v4float %47                ; RelaxedPrecision
         %49 =   OpCompositeExtract %float %48 3    ; RelaxedPrecision
         %50 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %51 =   OpLoad %v4float %50                ; RelaxedPrecision
         %52 =   OpCompositeExtract %float %51 3    ; RelaxedPrecision
         %53 =   OpFSub %float %float_1 %52         ; RelaxedPrecision
         %54 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %55 =   OpLoad %v4float %54                ; RelaxedPrecision
         %56 =   OpCompositeExtract %float %55 3    ; RelaxedPrecision
         %57 =   OpFMul %float %53 %56              ; RelaxedPrecision
         %58 =   OpFAdd %float %49 %57              ; RelaxedPrecision
         %59 =   OpCompositeConstruct %v4float %44 %45 %46 %58  ; RelaxedPrecision
                 OpStore %sk_FragColor %59
                 OpReturn
               OpFunctionEnd

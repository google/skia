               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %main "main"                  ; id %6

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %v3float = OpTypeVector %float 3
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %17 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %20 =   OpLoad %v4float %17                ; RelaxedPrecision
         %21 =   OpVectorShuffle %v3float %20 %20 0 1 2     ; RelaxedPrecision
         %23 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %25 =   OpLoad %v4float %23                ; RelaxedPrecision
         %26 =   OpVectorShuffle %v3float %25 %25 0 1 2     ; RelaxedPrecision
         %27 =   OpFAdd %v3float %21 %26                    ; RelaxedPrecision
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %31 =   OpLoad %v4float %30                ; RelaxedPrecision
         %32 =   OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
         %33 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %34 =   OpLoad %v4float %33                ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 3    ; RelaxedPrecision
         %36 =   OpVectorTimesScalar %v3float %32 %35   ; RelaxedPrecision
         %37 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %38 =   OpLoad %v4float %37                ; RelaxedPrecision
         %39 =   OpVectorShuffle %v3float %38 %38 0 1 2     ; RelaxedPrecision
         %40 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %41 =   OpLoad %v4float %40                ; RelaxedPrecision
         %42 =   OpCompositeExtract %float %41 3    ; RelaxedPrecision
         %43 =   OpVectorTimesScalar %v3float %39 %42   ; RelaxedPrecision
         %29 =   OpExtInst %v3float %5 FMin %36 %43     ; RelaxedPrecision
         %44 =   OpVectorTimesScalar %v3float %29 %float_2  ; RelaxedPrecision
         %45 =   OpFSub %v3float %27 %44                    ; RelaxedPrecision
         %46 =   OpCompositeExtract %float %45 0            ; RelaxedPrecision
         %47 =   OpCompositeExtract %float %45 1            ; RelaxedPrecision
         %48 =   OpCompositeExtract %float %45 2            ; RelaxedPrecision
         %49 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %50 3    ; RelaxedPrecision
         %53 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %54 =   OpLoad %v4float %53                ; RelaxedPrecision
         %55 =   OpCompositeExtract %float %54 3    ; RelaxedPrecision
         %56 =   OpFSub %float %float_1 %55         ; RelaxedPrecision
         %57 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %58 =   OpLoad %v4float %57                ; RelaxedPrecision
         %59 =   OpCompositeExtract %float %58 3    ; RelaxedPrecision
         %60 =   OpFMul %float %56 %59              ; RelaxedPrecision
         %61 =   OpFAdd %float %51 %60              ; RelaxedPrecision
         %62 =   OpCompositeConstruct %v4float %46 %47 %48 %61  ; RelaxedPrecision
                 OpStore %sk_FragColor %62
                 OpReturn
               OpFunctionEnd

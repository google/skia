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
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision

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
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %17 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %20 =   OpLoad %v4float %17                ; RelaxedPrecision
         %21 =   OpVectorShuffle %v3float %20 %20 0 1 2     ; RelaxedPrecision
         %23 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %25 =   OpLoad %v4float %23                ; RelaxedPrecision
         %26 =   OpVectorShuffle %v3float %25 %25 0 1 2     ; RelaxedPrecision
         %27 =   OpFAdd %v3float %21 %26                    ; RelaxedPrecision
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %30 =   OpLoad %v4float %29                ; RelaxedPrecision
         %31 =   OpVectorShuffle %v3float %30 %30 0 1 2     ; RelaxedPrecision
         %32 =   OpVectorTimesScalar %v3float %31 %float_2  ; RelaxedPrecision
         %33 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %34 =   OpLoad %v4float %33                ; RelaxedPrecision
         %35 =   OpVectorShuffle %v3float %34 %34 0 1 2     ; RelaxedPrecision
         %36 =   OpFMul %v3float %32 %35                    ; RelaxedPrecision
         %37 =   OpFSub %v3float %27 %36                    ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 0            ; RelaxedPrecision
         %39 =   OpCompositeExtract %float %37 1            ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %37 2            ; RelaxedPrecision
         %41 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %42 =   OpLoad %v4float %41                ; RelaxedPrecision
         %43 =   OpCompositeExtract %float %42 3    ; RelaxedPrecision
         %45 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %46 =   OpLoad %v4float %45                ; RelaxedPrecision
         %47 =   OpCompositeExtract %float %46 3    ; RelaxedPrecision
         %48 =   OpFSub %float %float_1 %47         ; RelaxedPrecision
         %49 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %50 3    ; RelaxedPrecision
         %52 =   OpFMul %float %48 %51              ; RelaxedPrecision
         %53 =   OpFAdd %float %43 %52              ; RelaxedPrecision
         %54 =   OpCompositeConstruct %v4float %38 %39 %40 %53  ; RelaxedPrecision
                 OpStore %sk_FragColor %54
                 OpReturn
               OpFunctionEnd

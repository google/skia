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
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
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
    %float_1 = OpConstant %float 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %18 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %21 =   OpLoad %v4float %18                ; RelaxedPrecision
         %22 =   OpCompositeExtract %float %21 3    ; RelaxedPrecision
         %23 =   OpFSub %float %float_1 %22         ; RelaxedPrecision
         %24 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %26 =   OpLoad %v4float %24                ; RelaxedPrecision
         %27 =   OpVectorShuffle %v3float %26 %26 0 1 2     ; RelaxedPrecision
         %29 =   OpVectorTimesScalar %v3float %27 %23       ; RelaxedPrecision
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %31 =   OpLoad %v4float %30                ; RelaxedPrecision
         %32 =   OpCompositeExtract %float %31 3    ; RelaxedPrecision
         %33 =   OpFSub %float %float_1 %32         ; RelaxedPrecision
         %34 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %35 =   OpLoad %v4float %34                ; RelaxedPrecision
         %36 =   OpVectorShuffle %v3float %35 %35 0 1 2     ; RelaxedPrecision
         %37 =   OpVectorTimesScalar %v3float %36 %33       ; RelaxedPrecision
         %38 =   OpFAdd %v3float %29 %37                    ; RelaxedPrecision
         %39 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %40 =   OpLoad %v4float %39                ; RelaxedPrecision
         %41 =   OpVectorShuffle %v3float %40 %40 0 1 2     ; RelaxedPrecision
         %42 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %43 =   OpLoad %v4float %42                ; RelaxedPrecision
         %44 =   OpVectorShuffle %v3float %43 %43 0 1 2     ; RelaxedPrecision
         %45 =   OpFMul %v3float %41 %44                    ; RelaxedPrecision
         %46 =   OpFAdd %v3float %38 %45                    ; RelaxedPrecision
         %47 =   OpCompositeExtract %float %46 0            ; RelaxedPrecision
         %48 =   OpCompositeExtract %float %46 1            ; RelaxedPrecision
         %49 =   OpCompositeExtract %float %46 2            ; RelaxedPrecision
         %50 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %51 =   OpLoad %v4float %50                ; RelaxedPrecision
         %52 =   OpCompositeExtract %float %51 3    ; RelaxedPrecision
         %53 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %54 =   OpLoad %v4float %53                ; RelaxedPrecision
         %55 =   OpCompositeExtract %float %54 3    ; RelaxedPrecision
         %56 =   OpFSub %float %float_1 %55         ; RelaxedPrecision
         %57 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %58 =   OpLoad %v4float %57                ; RelaxedPrecision
         %59 =   OpCompositeExtract %float %58 3    ; RelaxedPrecision
         %60 =   OpFMul %float %56 %59              ; RelaxedPrecision
         %61 =   OpFAdd %float %52 %60              ; RelaxedPrecision
         %62 =   OpCompositeConstruct %v4float %47 %48 %49 %61  ; RelaxedPrecision
                 OpStore %sk_FragColor %62
                 OpReturn
               OpFunctionEnd

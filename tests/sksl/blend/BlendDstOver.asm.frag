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
               OpDecorate %30 RelaxedPrecision

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
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %18 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %21 =   OpLoad %v4float %18                ; RelaxedPrecision
         %22 =   OpCompositeExtract %float %21 3    ; RelaxedPrecision
         %23 =   OpFSub %float %float_1 %22         ; RelaxedPrecision
         %24 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %26 =   OpLoad %v4float %24                ; RelaxedPrecision
         %27 =   OpVectorTimesScalar %v4float %26 %23   ; RelaxedPrecision
         %28 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %29 =   OpLoad %v4float %28                ; RelaxedPrecision
         %30 =   OpFAdd %v4float %27 %29            ; RelaxedPrecision
                 OpStore %sk_FragColor %30
                 OpReturn
               OpFunctionEnd

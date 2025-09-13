               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "a"
               OpName %main "main"                  ; id %6

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %uint = OpTypeInt 32 0
%_UniformBuffer = OpTypeStruct %uint                ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %int_0 = OpConstant %int 0
    %v2float = OpTypeVector %float 2


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
         %19 =   OpAccessChain %_ptr_Uniform_uint %11 %int_0
         %22 =   OpLoad %uint %19
         %18 =   OpExtInst %v2float %5 UnpackHalf2x16 %22
         %24 =   OpLoad %v4float %sk_FragColor      ; RelaxedPrecision
         %25 =   OpVectorShuffle %v4float %24 %18 4 5 2 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %25
         %27 =   OpAccessChain %_ptr_Uniform_uint %11 %int_0
         %28 =   OpLoad %uint %27
         %26 =   OpExtInst %v2float %5 UnpackUnorm2x16 %28
         %29 =   OpLoad %v4float %sk_FragColor      ; RelaxedPrecision
         %30 =   OpVectorShuffle %v4float %29 %26 4 5 2 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %30
         %32 =   OpAccessChain %_ptr_Uniform_uint %11 %int_0
         %33 =   OpLoad %uint %32
         %31 =   OpExtInst %v2float %5 UnpackSnorm2x16 %33
         %34 =   OpLoad %v4float %sk_FragColor      ; RelaxedPrecision
         %35 =   OpVectorShuffle %v4float %34 %31 4 5 2 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %35
         %37 =   OpAccessChain %_ptr_Uniform_uint %11 %int_0
         %38 =   OpLoad %uint %37
         %36 =   OpExtInst %v4float %5 UnpackUnorm4x8 %38
                 OpStore %sk_FragColor %36
         %40 =   OpAccessChain %_ptr_Uniform_uint %11 %int_0
         %41 =   OpLoad %uint %40
         %39 =   OpExtInst %v4float %5 UnpackSnorm4x8 %41
                 OpStore %sk_FragColor %39
                 OpReturn
               OpFunctionEnd

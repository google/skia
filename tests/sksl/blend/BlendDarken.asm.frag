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
               OpName %_0_a "_0_a"                  ; id %17
               OpName %_1_b "_1_b"                  ; id %33

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
               OpDecorate %_0_a RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %_1_b RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision

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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
       %_0_a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
       %_1_b =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
         %19 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %22 =   OpLoad %v4float %19                ; RelaxedPrecision
         %24 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %25 =   OpLoad %v4float %24                ; RelaxedPrecision
         %26 =   OpCompositeExtract %float %25 3    ; RelaxedPrecision
         %27 =   OpFSub %float %float_1 %26         ; RelaxedPrecision
         %28 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %30 =   OpLoad %v4float %28                ; RelaxedPrecision
         %31 =   OpVectorTimesScalar %v4float %30 %27   ; RelaxedPrecision
         %32 =   OpFAdd %v4float %22 %31                ; RelaxedPrecision
                 OpStore %_0_a %32
         %36 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %37 =   OpLoad %v4float %36                ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 3    ; RelaxedPrecision
         %39 =   OpFSub %float %float_1 %38         ; RelaxedPrecision
         %40 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %41 =   OpLoad %v4float %40                ; RelaxedPrecision
         %42 =   OpVectorShuffle %v3float %41 %41 0 1 2     ; RelaxedPrecision
         %43 =   OpVectorTimesScalar %v3float %42 %39       ; RelaxedPrecision
         %44 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %45 =   OpLoad %v4float %44                ; RelaxedPrecision
         %46 =   OpVectorShuffle %v3float %45 %45 0 1 2     ; RelaxedPrecision
         %47 =   OpFAdd %v3float %43 %46                    ; RelaxedPrecision
                 OpStore %_1_b %47
         %49 =   OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
         %48 =   OpExtInst %v3float %5 FMin %49 %47         ; RelaxedPrecision
         %50 =   OpLoad %v4float %_0_a                      ; RelaxedPrecision
         %51 =   OpVectorShuffle %v4float %50 %48 4 5 6 3   ; RelaxedPrecision
                 OpStore %_0_a %51
                 OpStore %sk_FragColor %51
                 OpReturn
               OpFunctionEnd

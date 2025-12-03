               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "a"
               OpMemberName %_UniformBuffer 1 "b"
               OpMemberName %_UniformBuffer 2 "c"
               OpMemberName %_UniformBuffer 3 "d"
               OpMemberName %_UniformBuffer 4 "e"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %result "result"                  ; id %27

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 4
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 8
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 16
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 4 Offset 32
               OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %result RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %float %float %float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_65504 = OpConstant %float 65504
    %float_2 = OpConstant %float 2
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
  %float_0_5 = OpConstant %float 0.5
%float_n0_866025388 = OpConstant %float -0.866025388
         %59 = OpConstantComposite %v2float %float_0_5 %float_n0_866025388
    %v3float = OpTypeVector %float 3
         %63 = OpConstantComposite %v3float %float_0_5 %float_0 %float_n0_866025388
         %66 = OpConstantComposite %v4float %float_0_5 %float_0 %float_0 %float_n0_866025388


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
     %result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %32 =   OpFMul %float %float_65504 %float_2            ; RelaxedPrecision
         %29 =   OpExtInst %float %5 Refract %32 %float_2 %float_2  ; RelaxedPrecision
         %33 =   OpCompositeConstruct %v4float %29 %29 %29 %29      ; RelaxedPrecision
                 OpStore %result %33
         %35 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %38 =   OpLoad %float %35                  ; RelaxedPrecision
         %39 =   OpAccessChain %_ptr_Uniform_float %11 %int_1
         %41 =   OpLoad %float %39                  ; RelaxedPrecision
         %42 =   OpAccessChain %_ptr_Uniform_float %11 %int_2
         %44 =   OpLoad %float %42                  ; RelaxedPrecision
         %34 =   OpExtInst %float %5 Refract %38 %41 %44    ; RelaxedPrecision
         %45 =   OpAccessChain %_ptr_Function_float %result %int_0
                 OpStore %45 %34
         %48 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %51 =   OpLoad %v4float %48                ; RelaxedPrecision
         %52 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_4
         %54 =   OpLoad %v4float %52                ; RelaxedPrecision
         %55 =   OpAccessChain %_ptr_Uniform_float %11 %int_2
         %56 =   OpLoad %float %55                  ; RelaxedPrecision
         %47 =   OpExtInst %v4float %5 Refract %51 %54 %56  ; RelaxedPrecision
                 OpStore %result %47
         %60 =   OpLoad %v4float %result            ; RelaxedPrecision
         %61 =   OpVectorShuffle %v4float %60 %59 4 5 2 3   ; RelaxedPrecision
                 OpStore %result %61
         %64 =   OpLoad %v4float %result            ; RelaxedPrecision
         %65 =   OpVectorShuffle %v4float %64 %63 4 5 6 3   ; RelaxedPrecision
                 OpStore %result %65
                 OpStore %result %66
                 OpReturnValue %66
               OpFunctionEnd

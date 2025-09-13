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
               OpDecorate %37 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision

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
%float_6_00000015e_26 = OpConstant %float 6.00000015e+26
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
         %58 = OpConstantComposite %v2float %float_0_5 %float_n0_866025388
    %v3float = OpTypeVector %float 3
         %62 = OpConstantComposite %v3float %float_0_5 %float_0 %float_n0_866025388
         %65 = OpConstantComposite %v4float %float_0_5 %float_0 %float_0 %float_n0_866025388


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
         %29 =   OpExtInst %float %5 Refract %float_6_00000015e_26 %float_2 %float_2    ; RelaxedPrecision
         %32 =   OpCompositeConstruct %v4float %29 %29 %29 %29                          ; RelaxedPrecision
                 OpStore %result %32
         %34 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %37 =   OpLoad %float %34                  ; RelaxedPrecision
         %38 =   OpAccessChain %_ptr_Uniform_float %11 %int_1
         %40 =   OpLoad %float %38                  ; RelaxedPrecision
         %41 =   OpAccessChain %_ptr_Uniform_float %11 %int_2
         %43 =   OpLoad %float %41                  ; RelaxedPrecision
         %33 =   OpExtInst %float %5 Refract %37 %40 %43    ; RelaxedPrecision
         %44 =   OpAccessChain %_ptr_Function_float %result %int_0
                 OpStore %44 %33
         %47 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %50 =   OpLoad %v4float %47                ; RelaxedPrecision
         %51 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_4
         %53 =   OpLoad %v4float %51                ; RelaxedPrecision
         %54 =   OpAccessChain %_ptr_Uniform_float %11 %int_2
         %55 =   OpLoad %float %54                  ; RelaxedPrecision
         %46 =   OpExtInst %v4float %5 Refract %50 %53 %55  ; RelaxedPrecision
                 OpStore %result %46
         %59 =   OpLoad %v4float %result            ; RelaxedPrecision
         %60 =   OpVectorShuffle %v4float %59 %58 4 5 2 3   ; RelaxedPrecision
                 OpStore %result %60
         %63 =   OpLoad %v4float %result            ; RelaxedPrecision
         %64 =   OpVectorShuffle %v4float %63 %62 4 5 6 3   ; RelaxedPrecision
                 OpStore %result %64
                 OpStore %result %65
                 OpReturnValue %65
               OpFunctionEnd

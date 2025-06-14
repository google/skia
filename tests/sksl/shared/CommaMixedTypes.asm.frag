               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "unknownInput"
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
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %result RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %float          ; Block
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
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
    %float_2 = OpConstant %float 2
         %41 = OpConstantComposite %v2float %float_2 %float_2
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %48 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %int_2 = OpConstant %int 2
    %float_4 = OpConstant %float 4
         %55 = OpConstantComposite %v2float %float_4 %float_0
         %56 = OpConstantComposite %v2float %float_0 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
         %58 = OpConstantComposite %mat2v2float %55 %56
      %int_3 = OpConstant %int 3


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
         %29 =   OpAccessChain %_ptr_Uniform_float %11 %int_1
         %32 =   OpLoad %float %29
         %33 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %36 =   OpLoad %v4float %33                ; RelaxedPrecision
         %37 =   OpCompositeExtract %float %36 0    ; RelaxedPrecision
         %38 =   OpAccessChain %_ptr_Function_float %result %int_0
                 OpStore %38 %37
         %42 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %43 =   OpLoad %v4float %42                ; RelaxedPrecision
         %44 =   OpCompositeExtract %float %43 1    ; RelaxedPrecision
         %45 =   OpAccessChain %_ptr_Function_float %result %int_1
                 OpStore %45 %44
         %49 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %50 2    ; RelaxedPrecision
         %52 =   OpAccessChain %_ptr_Function_float %result %int_2
                 OpStore %52 %51
         %59 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %60 =   OpLoad %v4float %59                ; RelaxedPrecision
         %61 =   OpCompositeExtract %float %60 3    ; RelaxedPrecision
         %62 =   OpAccessChain %_ptr_Function_float %result %int_3
                 OpStore %62 %61
         %64 =   OpLoad %v4float %result            ; RelaxedPrecision
                 OpReturnValue %64
               OpFunctionEnd

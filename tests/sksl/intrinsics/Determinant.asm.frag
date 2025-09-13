               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 32
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 48
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %44 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_0 = OpConstant %int 0
   %float_n2 = OpConstant %float -2
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %25         ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_Function_v2float

         %27 = OpLabel
         %36 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_0
         %32 =   OpLoad %mat2v2float %29
         %28 =   OpExtInst %float %5 Determinant %32
         %34 =   OpFOrdEqual %bool %28 %float_n2
                 OpSelectionMerge %40 None
                 OpBranchConditional %34 %38 %39

         %38 =     OpLabel
         %41 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %44 =       OpLoad %v4float %41            ; RelaxedPrecision
                     OpStore %36 %44
                     OpBranch %40

         %39 =     OpLabel
         %45 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %47 =       OpLoad %v4float %45            ; RelaxedPrecision
                     OpStore %36 %47
                     OpBranch %40

         %40 = OpLabel
         %48 =   OpLoad %v4float %36                ; RelaxedPrecision
                 OpReturnValue %48
               OpFunctionEnd

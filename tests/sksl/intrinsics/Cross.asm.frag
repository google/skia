               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "testMatrix3x3"
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
               OpMemberDecorate %_UniformBuffer 1 Offset 48
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 64
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %70 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat3v3float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
      %int_1 = OpConstant %int 1
   %float_n3 = OpConstant %float -3
    %float_6 = OpConstant %float 6
         %44 = OpConstantComposite %v3float %float_n3 %float_6 %float_n3
     %v3bool = OpTypeVector %bool 3
      %int_2 = OpConstant %int 2
  %float_n12 = OpConstant %float -12
         %59 = OpConstantComposite %v3float %float_6 %float_n12 %float_6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %26         ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_Function_v2float

         %28 = OpLabel
         %63 =   OpVariable %_ptr_Function_v4float Function
         %32 =   OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_0
         %35 =   OpAccessChain %_ptr_Uniform_v3float %32 %int_0
         %37 =   OpLoad %v3float %35
         %38 =   OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_0
         %40 =   OpAccessChain %_ptr_Uniform_v3float %38 %int_1
         %41 =   OpLoad %v3float %40
         %31 =   OpExtInst %v3float %5 Cross %37 %41
         %45 =   OpFOrdEqual %v3bool %31 %44
         %47 =   OpAll %bool %45
                 OpSelectionMerge %49 None
                 OpBranchConditional %47 %48 %49

         %48 =     OpLabel
         %51 =       OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_0
         %53 =       OpAccessChain %_ptr_Uniform_v3float %51 %int_2
         %54 =       OpLoad %v3float %53
         %55 =       OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_0
         %56 =       OpAccessChain %_ptr_Uniform_v3float %55 %int_0
         %57 =       OpLoad %v3float %56
         %50 =       OpExtInst %v3float %5 Cross %54 %57
         %60 =       OpFOrdEqual %v3bool %50 %59
         %61 =       OpAll %bool %60
                     OpBranch %49

         %49 = OpLabel
         %62 =   OpPhi %bool %false %28 %61 %48
                 OpSelectionMerge %67 None
                 OpBranchConditional %62 %65 %66

         %65 =     OpLabel
         %68 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %70 =       OpLoad %v4float %68            ; RelaxedPrecision
                     OpStore %63 %70
                     OpBranch %67

         %66 =     OpLabel
         %71 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %72 =       OpLoad %v4float %71            ; RelaxedPrecision
                     OpStore %63 %72
                     OpBranch %67

         %67 = OpLabel
         %73 =   OpLoad %v4float %63                ; RelaxedPrecision
                 OpReturnValue %73
               OpFunctionEnd

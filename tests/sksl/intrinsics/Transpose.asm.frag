               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %16
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "testMatrix3x3"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %18
               OpName %main "main"                      ; id %6
               OpName %testMatrix2x3 "testMatrix2x3"    ; id %30

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 32
               OpMemberDecorate %_UniformBuffer 1 ColMajor
               OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 2 Offset 80
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 96
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %115 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat2v2float %mat3v3float %v4float %v4float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %27 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
         %39 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %40 = OpConstantComposite %v3float %float_4 %float_5 %float_6
         %41 = OpConstantComposite %mat2v3float %39 %40
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_0 = OpConstant %int 0
         %49 = OpConstantComposite %v2float %float_1 %float_3
         %50 = OpConstantComposite %v2float %float_2 %float_4
         %51 = OpConstantComposite %mat2v2float %49 %50
     %v2bool = OpTypeVector %bool 2
%mat3v2float = OpTypeMatrix %v2float 3
         %64 = OpConstantComposite %v2float %float_1 %float_4
         %65 = OpConstantComposite %v2float %float_2 %float_5
         %66 = OpConstantComposite %v2float %float_3 %float_6
         %67 = OpConstantComposite %mat3v2float %64 %65 %66
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_1 = OpConstant %int 1
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
         %90 = OpConstantComposite %v3float %float_1 %float_4 %float_7
         %91 = OpConstantComposite %v3float %float_2 %float_5 %float_8
         %92 = OpConstantComposite %v3float %float_3 %float_6 %float_9
         %93 = OpConstantComposite %mat3v3float %90 %91 %92
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %20

         %21 = OpLabel
         %24 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %24 %23
         %26 =   OpFunctionCall %v4float %main %24
                 OpStore %sk_FragColor %26
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %27         ; RelaxedPrecision
         %28 = OpFunctionParameter %_ptr_Function_v2float

         %29 = OpLabel
%testMatrix2x3 =   OpVariable %_ptr_Function_mat2v3float Function
        %107 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %testMatrix2x3 %41
         %45 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_0
         %48 =   OpLoad %mat2v2float %45
         %44 =   OpTranspose %mat2v2float %48
         %53 =   OpCompositeExtract %v2float %44 0
         %54 =   OpFOrdEqual %v2bool %53 %49
         %55 =   OpAll %bool %54
         %56 =   OpCompositeExtract %v2float %44 1
         %57 =   OpFOrdEqual %v2bool %56 %50
         %58 =   OpAll %bool %57
         %59 =   OpLogicalAnd %bool %55 %58
                 OpSelectionMerge %61 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
         %62 =       OpTranspose %mat3v2float %41
         %68 =       OpCompositeExtract %v2float %62 0
         %69 =       OpFOrdEqual %v2bool %68 %64
         %70 =       OpAll %bool %69
         %71 =       OpCompositeExtract %v2float %62 1
         %72 =       OpFOrdEqual %v2bool %71 %65
         %73 =       OpAll %bool %72
         %74 =       OpLogicalAnd %bool %70 %73
         %75 =       OpCompositeExtract %v2float %62 2
         %76 =       OpFOrdEqual %v2bool %75 %66
         %77 =       OpAll %bool %76
         %78 =       OpLogicalAnd %bool %74 %77
                     OpBranch %61

         %61 = OpLabel
         %79 =   OpPhi %bool %false %29 %78 %60
                 OpSelectionMerge %81 None
                 OpBranchConditional %79 %80 %81

         %80 =     OpLabel
         %83 =       OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_1
         %86 =       OpLoad %mat3v3float %83
         %82 =       OpTranspose %mat3v3float %86
         %95 =       OpCompositeExtract %v3float %82 0
         %96 =       OpFOrdEqual %v3bool %95 %90
         %97 =       OpAll %bool %96
         %98 =       OpCompositeExtract %v3float %82 1
         %99 =       OpFOrdEqual %v3bool %98 %91
        %100 =       OpAll %bool %99
        %101 =       OpLogicalAnd %bool %97 %100
        %102 =       OpCompositeExtract %v3float %82 2
        %103 =       OpFOrdEqual %v3bool %102 %92
        %104 =       OpAll %bool %103
        %105 =       OpLogicalAnd %bool %101 %104
                     OpBranch %81

         %81 = OpLabel
        %106 =   OpPhi %bool %false %61 %105 %80
                 OpSelectionMerge %111 None
                 OpBranchConditional %106 %109 %110

        %109 =     OpLabel
        %112 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %115 =       OpLoad %v4float %112           ; RelaxedPrecision
                     OpStore %107 %115
                     OpBranch %111

        %110 =     OpLabel
        %116 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %118 =       OpLoad %v4float %116           ; RelaxedPrecision
                     OpStore %107 %118
                     OpBranch %111

        %111 = OpLabel
        %119 =   OpLoad %v4float %107               ; RelaxedPrecision
                 OpReturnValue %119
               OpFunctionEnd

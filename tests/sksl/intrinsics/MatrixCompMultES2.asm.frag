               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %16
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpMemberName %_UniformBuffer 3 "testMatrix3x3"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %18
               OpName %main "main"                      ; id %6
               OpName %smallM22 "smallM22"              ; id %30
               OpName %h22 "h22"                        ; id %35
               OpName %hugeM22 "hugeM22"                ; id %39
               OpName %f22 "f22"                        ; id %52
               OpName %h33 "h33"                        ; id %66

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 3 Offset 64
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %smallM22 RelaxedPrecision
               OpDecorate %h22 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %h33 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision

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
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %27 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
 %float_1000 = OpConstant %float 1000
         %33 = OpConstantComposite %v2float %float_1000 %float_1000
         %34 = OpConstantComposite %mat2v2float %33 %33
%float_1_00000002e_30 = OpConstant %float 1.00000002e+30
         %41 = OpConstantComposite %v2float %float_1_00000002e_30 %float_1_00000002e_30
         %42 = OpConstantComposite %mat2v2float %41 %41
    %float_5 = OpConstant %float 5
   %float_10 = OpConstant %float 10
   %float_15 = OpConstant %float 15
         %49 = OpConstantComposite %v2float %float_0 %float_5
         %50 = OpConstantComposite %v2float %float_10 %float_15
         %51 = OpConstantComposite %mat2v2float %49 %50
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
         %58 = OpConstantComposite %v2float %float_1 %float_0
         %59 = OpConstantComposite %v2float %float_0 %float_1
         %60 = OpConstantComposite %mat2v2float %58 %59
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_3 = OpConstant %int 3
    %float_2 = OpConstant %float 2
         %73 = OpConstantComposite %v3float %float_2 %float_2 %float_2
         %74 = OpConstantComposite %mat3v3float %73 %73 %73
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %float_4 = OpConstant %float 4
         %93 = OpConstantComposite %v2float %float_0 %float_4
         %94 = OpConstantComposite %mat2v2float %58 %93
    %float_6 = OpConstant %float 6
    %float_8 = OpConstant %float 8
   %float_12 = OpConstant %float 12
   %float_14 = OpConstant %float 14
   %float_16 = OpConstant %float 16
   %float_18 = OpConstant %float 18
        %109 = OpConstantComposite %v3float %float_2 %float_4 %float_6
        %110 = OpConstantComposite %v3float %float_8 %float_10 %float_12
        %111 = OpConstantComposite %v3float %float_14 %float_16 %float_18
        %112 = OpConstantComposite %mat3v3float %109 %110 %111
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


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
   %smallM22 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
        %h22 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
    %hugeM22 =   OpVariable %_ptr_Function_mat2v2float Function
        %f22 =   OpVariable %_ptr_Function_mat2v2float Function
        %h33 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
        %123 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %smallM22 %34
         %36 =   OpFMul %v2float %33 %33            ; RelaxedPrecision
         %37 =   OpFMul %v2float %33 %33            ; RelaxedPrecision
         %38 =   OpCompositeConstruct %mat2v2float %36 %37  ; RelaxedPrecision
                 OpStore %h22 %38
                 OpStore %hugeM22 %42
         %43 =   OpFMul %v2float %41 %41
         %44 =   OpFMul %v2float %41 %41
         %45 =   OpCompositeConstruct %mat2v2float %43 %44
                 OpStore %h22 %45
                 OpStore %h22 %51
         %53 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %56 =   OpLoad %mat2v2float %53
         %61 =   OpCompositeExtract %v2float %56 0
         %62 =   OpFMul %v2float %61 %58
         %63 =   OpCompositeExtract %v2float %56 1
         %64 =   OpFMul %v2float %63 %59
         %65 =   OpCompositeConstruct %mat2v2float %62 %64
                 OpStore %f22 %65
         %68 =   OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
         %71 =   OpLoad %mat3v3float %68            ; RelaxedPrecision
         %75 =   OpCompositeExtract %v3float %71 0  ; RelaxedPrecision
         %76 =   OpFMul %v3float %75 %73            ; RelaxedPrecision
         %77 =   OpCompositeExtract %v3float %71 1  ; RelaxedPrecision
         %78 =   OpFMul %v3float %77 %73            ; RelaxedPrecision
         %79 =   OpCompositeExtract %v3float %71 2  ; RelaxedPrecision
         %80 =   OpFMul %v3float %79 %73            ; RelaxedPrecision
         %81 =   OpCompositeConstruct %mat3v3float %76 %78 %80  ; RelaxedPrecision
                 OpStore %h33 %81
         %85 =   OpFOrdEqual %v2bool %49 %49        ; RelaxedPrecision
         %86 =   OpAll %bool %85
         %87 =   OpFOrdEqual %v2bool %50 %50        ; RelaxedPrecision
         %88 =   OpAll %bool %87
         %89 =   OpLogicalAnd %bool %86 %88
                 OpSelectionMerge %91 None
                 OpBranchConditional %89 %90 %91

         %90 =     OpLabel
         %95 =       OpFOrdEqual %v2bool %62 %58
         %96 =       OpAll %bool %95
         %97 =       OpFOrdEqual %v2bool %64 %93
         %98 =       OpAll %bool %97
         %99 =       OpLogicalAnd %bool %96 %98
                     OpBranch %91

         %91 = OpLabel
        %100 =   OpPhi %bool %false %29 %99 %90
                 OpSelectionMerge %102 None
                 OpBranchConditional %100 %101 %102

        %101 =     OpLabel
        %114 =       OpFOrdEqual %v3bool %76 %109   ; RelaxedPrecision
        %115 =       OpAll %bool %114
        %116 =       OpFOrdEqual %v3bool %78 %110   ; RelaxedPrecision
        %117 =       OpAll %bool %116
        %118 =       OpLogicalAnd %bool %115 %117
        %119 =       OpFOrdEqual %v3bool %80 %111   ; RelaxedPrecision
        %120 =       OpAll %bool %119
        %121 =       OpLogicalAnd %bool %118 %120
                     OpBranch %102

        %102 = OpLabel
        %122 =   OpPhi %bool %false %91 %121 %101
                 OpSelectionMerge %127 None
                 OpBranchConditional %122 %125 %126

        %125 =     OpLabel
        %128 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %131 =       OpLoad %v4float %128           ; RelaxedPrecision
                     OpStore %123 %131
                     OpBranch %127

        %126 =     OpLabel
        %132 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %134 =       OpLoad %v4float %132           ; RelaxedPrecision
                     OpStore %123 %134
                     OpBranch %127

        %127 = OpLabel
        %135 =   OpLoad %v4float %123               ; RelaxedPrecision
                 OpReturnValue %135
               OpFunctionEnd

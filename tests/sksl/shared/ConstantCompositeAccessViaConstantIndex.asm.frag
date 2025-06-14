               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %globalArray "globalArray"    ; id %11
               OpName %globalMatrix "globalMatrix"  ; id %17
               OpName %_UniformBuffer "_UniformBuffer"  ; id %24
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "testMatrix2x2"
               OpMemberName %_UniformBuffer 2 "testArray"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %26
               OpName %main "main"                      ; id %6
               OpName %localArray "localArray"          ; id %38
               OpName %localMatrix "localMatrix"        ; id %44

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %globalArray RelaxedPrecision
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpDecorate %16 RelaxedPrecision
               OpDecorate %globalMatrix RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 ColMajor
               OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 48
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %23 Binding 0
               OpDecorate %23 DescriptorSet 0
               OpDecorate %localArray RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %localMatrix RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5       ; ArrayStride 16
%_ptr_Private__arr_float_int_5 = OpTypePointer Private %_arr_float_int_5
%globalArray = OpVariable %_ptr_Private__arr_float_int_5 Private    ; RelaxedPrecision
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Private_mat2v2float = OpTypePointer Private %mat2v2float
%globalMatrix = OpVariable %_ptr_Private_mat2v2float Private    ; RelaxedPrecision
         %21 = OpConstantComposite %v2float %float_1 %float_1
         %22 = OpConstantComposite %mat2v2float %21 %21
%_UniformBuffer = OpTypeStruct %v4float %mat2v2float %_arr_float_int_5  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %23 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %28 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %35 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_5 = OpTypePointer Function %_arr_float_int_5
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %46 = OpConstantComposite %v2float %float_0 %float_1
         %47 = OpConstantComposite %v2float %float_2 %float_3
         %48 = OpConstantComposite %mat2v2float %46 %47
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
      %int_2 = OpConstant %int 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_1 = OpConstant %int 1
        %137 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %28

         %29 = OpLabel
         %32 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %32 %31
         %34 =   OpFunctionCall %v4float %main %32
                 OpStore %sk_FragColor %34
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %35         ; RelaxedPrecision
         %36 = OpFunctionParameter %_ptr_Function_v2float

         %37 = OpLabel
 %localArray =   OpVariable %_ptr_Function__arr_float_int_5 Function    ; RelaxedPrecision
%localMatrix =   OpVariable %_ptr_Function_mat2v2float Function         ; RelaxedPrecision
         %16 =   OpCompositeConstruct %_arr_float_int_5 %float_1 %float_1 %float_1 %float_1 %float_1    ; RelaxedPrecision
                 OpStore %globalArray %16
                 OpStore %globalMatrix %22
         %43 =   OpCompositeConstruct %_arr_float_int_5 %float_0 %float_1 %float_2 %float_3 %float_4    ; RelaxedPrecision
                 OpStore %localArray %43
                 OpStore %localMatrix %48
         %51 =   OpAccessChain %_ptr_Uniform__arr_float_int_5 %23 %int_2
         %54 =   OpLoad %_arr_float_int_5 %51       ; RelaxedPrecision
         %55 =   OpCompositeExtract %float %54 0    ; RelaxedPrecision
         %56 =   OpFOrdEqual %bool %float_1 %55
         %57 =   OpCompositeExtract %float %54 1    ; RelaxedPrecision
         %58 =   OpFOrdEqual %bool %float_1 %57
         %59 =   OpLogicalAnd %bool %58 %56
         %60 =   OpCompositeExtract %float %54 2    ; RelaxedPrecision
         %61 =   OpFOrdEqual %bool %float_1 %60
         %62 =   OpLogicalAnd %bool %61 %59
         %63 =   OpCompositeExtract %float %54 3    ; RelaxedPrecision
         %64 =   OpFOrdEqual %bool %float_1 %63
         %65 =   OpLogicalAnd %bool %64 %62
         %66 =   OpCompositeExtract %float %54 4    ; RelaxedPrecision
         %67 =   OpFOrdEqual %bool %float_1 %66
         %68 =   OpLogicalAnd %bool %67 %65
                 OpSelectionMerge %70 None
                 OpBranchConditional %68 %70 %69

         %69 =     OpLabel
         %71 =       OpAccessChain %_ptr_Uniform_v4float %23 %int_0
         %74 =       OpLoad %v4float %71            ; RelaxedPrecision
         %75 =       OpVectorShuffle %v2float %74 %74 0 1   ; RelaxedPrecision
         %76 =       OpFOrdEqual %v2bool %21 %75
         %78 =       OpAll %bool %76
                     OpBranch %70

         %70 = OpLabel
         %79 =   OpPhi %bool %true %37 %78 %69
                 OpSelectionMerge %81 None
                 OpBranchConditional %79 %81 %80

         %80 =     OpLabel
         %82 =       OpAccessChain %_ptr_Uniform_mat2v2float %23 %int_1
         %85 =       OpLoad %mat2v2float %82        ; RelaxedPrecision
         %86 =       OpCompositeExtract %v2float %85 0  ; RelaxedPrecision
         %87 =       OpFOrdEqual %v2bool %21 %86        ; RelaxedPrecision
         %88 =       OpAll %bool %87
         %89 =       OpCompositeExtract %v2float %85 1  ; RelaxedPrecision
         %90 =       OpFOrdEqual %v2bool %21 %89        ; RelaxedPrecision
         %91 =       OpAll %bool %90
         %92 =       OpLogicalAnd %bool %88 %91
                     OpBranch %81

         %81 = OpLabel
         %93 =   OpPhi %bool %true %70 %92 %80
                 OpSelectionMerge %95 None
                 OpBranchConditional %93 %95 %94

         %94 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform__arr_float_int_5 %23 %int_2
         %97 =       OpLoad %_arr_float_int_5 %96   ; RelaxedPrecision
         %98 =       OpCompositeExtract %float %97 0    ; RelaxedPrecision
         %99 =       OpFOrdEqual %bool %float_0 %98
        %100 =       OpCompositeExtract %float %97 1    ; RelaxedPrecision
        %101 =       OpFOrdEqual %bool %float_1 %100
        %102 =       OpLogicalAnd %bool %101 %99
        %103 =       OpCompositeExtract %float %97 2    ; RelaxedPrecision
        %104 =       OpFOrdEqual %bool %float_2 %103
        %105 =       OpLogicalAnd %bool %104 %102
        %106 =       OpCompositeExtract %float %97 3    ; RelaxedPrecision
        %107 =       OpFOrdEqual %bool %float_3 %106
        %108 =       OpLogicalAnd %bool %107 %105
        %109 =       OpCompositeExtract %float %97 4    ; RelaxedPrecision
        %110 =       OpFOrdEqual %bool %float_4 %109
        %111 =       OpLogicalAnd %bool %110 %108
                     OpBranch %95

         %95 = OpLabel
        %112 =   OpPhi %bool %true %81 %111 %94
                 OpSelectionMerge %114 None
                 OpBranchConditional %112 %114 %113

        %113 =     OpLabel
        %115 =       OpAccessChain %_ptr_Uniform_v4float %23 %int_0
        %116 =       OpLoad %v4float %115           ; RelaxedPrecision
        %117 =       OpVectorShuffle %v2float %116 %116 0 1     ; RelaxedPrecision
        %118 =       OpFOrdEqual %v2bool %21 %117
        %119 =       OpAll %bool %118
                     OpBranch %114

        %114 = OpLabel
        %120 =   OpPhi %bool %true %95 %119 %113
                 OpSelectionMerge %122 None
                 OpBranchConditional %120 %122 %121

        %121 =     OpLabel
        %123 =       OpAccessChain %_ptr_Uniform_mat2v2float %23 %int_1
        %124 =       OpLoad %mat2v2float %123       ; RelaxedPrecision
        %125 =       OpCompositeExtract %v2float %124 0     ; RelaxedPrecision
        %126 =       OpFOrdEqual %v2bool %46 %125           ; RelaxedPrecision
        %127 =       OpAll %bool %126
        %128 =       OpCompositeExtract %v2float %124 1     ; RelaxedPrecision
        %129 =       OpFOrdEqual %v2bool %47 %128           ; RelaxedPrecision
        %130 =       OpAll %bool %129
        %131 =       OpLogicalAnd %bool %127 %130
                     OpBranch %122

        %122 = OpLabel
        %132 =   OpPhi %bool %true %114 %131 %121
                 OpSelectionMerge %134 None
                 OpBranchConditional %132 %133 %134

        %133 =     OpLabel
        %135 =       OpAccessChain %_ptr_Uniform_v4float %23 %int_0
        %136 =       OpLoad %v4float %135           ; RelaxedPrecision
                     OpReturnValue %136

        %134 = OpLabel
                 OpReturnValue %137
               OpFunctionEnd

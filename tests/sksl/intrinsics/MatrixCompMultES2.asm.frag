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
               OpName %h22 "h22"                        ; id %30
               OpName %hugeM22 "hugeM22"                ; id %35
               OpName %f22 "f22"                        ; id %48
               OpName %h33 "h33"                        ; id %62

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
               OpDecorate %h22 RelaxedPrecision
               OpDecorate %hugeM22 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %h33 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision

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
%float_1000000 = OpConstant %float 1000000
         %33 = OpConstantComposite %v2float %float_1000000 %float_1000000
         %34 = OpConstantComposite %mat2v2float %33 %33
%float_1_00000002e_30 = OpConstant %float 1.00000002e+30
         %37 = OpConstantComposite %v2float %float_1_00000002e_30 %float_1_00000002e_30
         %38 = OpConstantComposite %mat2v2float %37 %37
    %float_5 = OpConstant %float 5
   %float_10 = OpConstant %float 10
   %float_15 = OpConstant %float 15
         %45 = OpConstantComposite %v2float %float_0 %float_5
         %46 = OpConstantComposite %v2float %float_10 %float_15
         %47 = OpConstantComposite %mat2v2float %45 %46
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
         %54 = OpConstantComposite %v2float %float_1 %float_0
         %55 = OpConstantComposite %v2float %float_0 %float_1
         %56 = OpConstantComposite %mat2v2float %54 %55
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_3 = OpConstant %int 3
    %float_2 = OpConstant %float 2
         %69 = OpConstantComposite %v3float %float_2 %float_2 %float_2
         %70 = OpConstantComposite %mat3v3float %69 %69 %69
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %float_4 = OpConstant %float 4
         %89 = OpConstantComposite %v2float %float_0 %float_4
         %90 = OpConstantComposite %mat2v2float %54 %89
    %float_6 = OpConstant %float 6
    %float_8 = OpConstant %float 8
   %float_12 = OpConstant %float 12
   %float_14 = OpConstant %float 14
   %float_16 = OpConstant %float 16
   %float_18 = OpConstant %float 18
        %105 = OpConstantComposite %v3float %float_2 %float_4 %float_6
        %106 = OpConstantComposite %v3float %float_8 %float_10 %float_12
        %107 = OpConstantComposite %v3float %float_14 %float_16 %float_18
        %108 = OpConstantComposite %mat3v3float %105 %106 %107
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
        %h22 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
    %hugeM22 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
        %f22 =   OpVariable %_ptr_Function_mat2v2float Function
        %h33 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
        %119 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %h22 %34
                 OpStore %hugeM22 %38
         %39 =   OpFMul %v2float %37 %37            ; RelaxedPrecision
         %40 =   OpFMul %v2float %37 %37            ; RelaxedPrecision
         %41 =   OpCompositeConstruct %mat2v2float %39 %40  ; RelaxedPrecision
                 OpStore %h22 %41
                 OpStore %h22 %47
         %49 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %52 =   OpLoad %mat2v2float %49
         %57 =   OpCompositeExtract %v2float %52 0
         %58 =   OpFMul %v2float %57 %54
         %59 =   OpCompositeExtract %v2float %52 1
         %60 =   OpFMul %v2float %59 %55
         %61 =   OpCompositeConstruct %mat2v2float %58 %60
                 OpStore %f22 %61
         %64 =   OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
         %67 =   OpLoad %mat3v3float %64            ; RelaxedPrecision
         %71 =   OpCompositeExtract %v3float %67 0  ; RelaxedPrecision
         %72 =   OpFMul %v3float %71 %69            ; RelaxedPrecision
         %73 =   OpCompositeExtract %v3float %67 1  ; RelaxedPrecision
         %74 =   OpFMul %v3float %73 %69            ; RelaxedPrecision
         %75 =   OpCompositeExtract %v3float %67 2  ; RelaxedPrecision
         %76 =   OpFMul %v3float %75 %69            ; RelaxedPrecision
         %77 =   OpCompositeConstruct %mat3v3float %72 %74 %76  ; RelaxedPrecision
                 OpStore %h33 %77
         %81 =   OpFOrdEqual %v2bool %45 %45        ; RelaxedPrecision
         %82 =   OpAll %bool %81
         %83 =   OpFOrdEqual %v2bool %46 %46        ; RelaxedPrecision
         %84 =   OpAll %bool %83
         %85 =   OpLogicalAnd %bool %82 %84
                 OpSelectionMerge %87 None
                 OpBranchConditional %85 %86 %87

         %86 =     OpLabel
         %91 =       OpFOrdEqual %v2bool %58 %54
         %92 =       OpAll %bool %91
         %93 =       OpFOrdEqual %v2bool %60 %89
         %94 =       OpAll %bool %93
         %95 =       OpLogicalAnd %bool %92 %94
                     OpBranch %87

         %87 = OpLabel
         %96 =   OpPhi %bool %false %29 %95 %86
                 OpSelectionMerge %98 None
                 OpBranchConditional %96 %97 %98

         %97 =     OpLabel
        %110 =       OpFOrdEqual %v3bool %72 %105   ; RelaxedPrecision
        %111 =       OpAll %bool %110
        %112 =       OpFOrdEqual %v3bool %74 %106   ; RelaxedPrecision
        %113 =       OpAll %bool %112
        %114 =       OpLogicalAnd %bool %111 %113
        %115 =       OpFOrdEqual %v3bool %76 %107   ; RelaxedPrecision
        %116 =       OpAll %bool %115
        %117 =       OpLogicalAnd %bool %114 %116
                     OpBranch %98

         %98 = OpLabel
        %118 =   OpPhi %bool %false %87 %117 %97
                 OpSelectionMerge %123 None
                 OpBranchConditional %118 %121 %122

        %121 =     OpLabel
        %124 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %127 =       OpLoad %v4float %124           ; RelaxedPrecision
                     OpStore %119 %127
                     OpBranch %123

        %122 =     OpLabel
        %128 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %130 =       OpLoad %v4float %128           ; RelaxedPrecision
                     OpStore %119 %130
                     OpBranch %123

        %123 = OpLabel
        %131 =   OpLoad %v4float %119               ; RelaxedPrecision
                 OpReturnValue %131
               OpFunctionEnd

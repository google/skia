               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %17
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix3x3"
               OpMemberName %_UniformBuffer 3 "testMatrix4x4"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %19
               OpName %test3x3_b "test3x3_b"            ; id %6
               OpName %matrix "matrix"                  ; id %32
               OpName %values "values"                  ; id %34
               OpName %index "index"                    ; id %40
               OpName %test4x4_b "test4x4_b"            ; id %7
               OpName %matrix_0 "matrix"                ; id %81
               OpName %values_0 "values"                ; id %83
               OpName %index_0 "index"                  ; id %87
               OpName %main "main"                      ; id %8

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
               OpMemberDecorate %_UniformBuffer 3 Offset 80
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %143 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat3v3float %mat4v4float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %30 = OpTypeFunction %bool
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %39 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
         %55 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %86 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
      %int_4 = OpConstant %int 4
        %100 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
     %v4bool = OpTypeVector %bool 4
        %128 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %21

         %22 = OpLabel
         %26 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %26 %25
         %28 =   OpFunctionCall %v4float %main %26
                 OpStore %sk_FragColor %28
                 OpReturn
               OpFunctionEnd


               ; Function test3x3_b
  %test3x3_b = OpFunction %bool None %30

         %31 = OpLabel
     %matrix =   OpVariable %_ptr_Function_mat3v3float Function
     %values =   OpVariable %_ptr_Function_v3float Function
      %index =   OpVariable %_ptr_Function_int Function
                 OpStore %values %39
                 OpStore %index %int_0
                 OpBranch %43

         %43 = OpLabel
                 OpLoopMerge %47 %46 None
                 OpBranch %44

         %44 =     OpLabel
         %48 =       OpLoad %int %index
         %50 =       OpSLessThan %bool %48 %int_3
                     OpBranchConditional %50 %45 %47

         %45 =         OpLabel
         %51 =           OpLoad %v3float %values
         %52 =           OpLoad %int %index
         %53 =           OpAccessChain %_ptr_Function_v3float %matrix %52
                         OpStore %53 %51
         %54 =           OpLoad %v3float %values
         %56 =           OpFAdd %v3float %54 %55
                         OpStore %values %56
                         OpBranch %46

         %46 =   OpLabel
         %58 =     OpLoad %int %index
         %59 =     OpIAdd %int %58 %int_1
                   OpStore %index %59
                   OpBranch %43

         %47 = OpLabel
         %60 =   OpLoad %mat3v3float %matrix
         %61 =   OpAccessChain %_ptr_Uniform_mat3v3float %13 %int_2
         %64 =   OpLoad %mat3v3float %61
         %66 =   OpCompositeExtract %v3float %60 0
         %67 =   OpCompositeExtract %v3float %64 0
         %68 =   OpFOrdEqual %v3bool %66 %67
         %69 =   OpAll %bool %68
         %70 =   OpCompositeExtract %v3float %60 1
         %71 =   OpCompositeExtract %v3float %64 1
         %72 =   OpFOrdEqual %v3bool %70 %71
         %73 =   OpAll %bool %72
         %74 =   OpLogicalAnd %bool %69 %73
         %75 =   OpCompositeExtract %v3float %60 2
         %76 =   OpCompositeExtract %v3float %64 2
         %77 =   OpFOrdEqual %v3bool %75 %76
         %78 =   OpAll %bool %77
         %79 =   OpLogicalAnd %bool %74 %78
                 OpReturnValue %79
               OpFunctionEnd


               ; Function test4x4_b
  %test4x4_b = OpFunction %bool None %30

         %80 = OpLabel
   %matrix_0 =   OpVariable %_ptr_Function_mat4v4float Function
   %values_0 =   OpVariable %_ptr_Function_v4float Function
    %index_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %values_0 %86
                 OpStore %index_0 %int_0
                 OpBranch %88

         %88 = OpLabel
                 OpLoopMerge %92 %91 None
                 OpBranch %89

         %89 =     OpLabel
         %93 =       OpLoad %int %index_0
         %95 =       OpSLessThan %bool %93 %int_4
                     OpBranchConditional %95 %90 %92

         %90 =         OpLabel
         %96 =           OpLoad %v4float %values_0
         %97 =           OpLoad %int %index_0
         %98 =           OpAccessChain %_ptr_Function_v4float %matrix_0 %97
                         OpStore %98 %96
         %99 =           OpLoad %v4float %values_0
        %101 =           OpFAdd %v4float %99 %100
                         OpStore %values_0 %101
                         OpBranch %91

         %91 =   OpLabel
        %102 =     OpLoad %int %index_0
        %103 =     OpIAdd %int %102 %int_1
                   OpStore %index_0 %103
                   OpBranch %88

         %92 = OpLabel
        %104 =   OpLoad %mat4v4float %matrix_0
        %105 =   OpAccessChain %_ptr_Uniform_mat4v4float %13 %int_3
        %107 =   OpLoad %mat4v4float %105
        %109 =   OpCompositeExtract %v4float %104 0
        %110 =   OpCompositeExtract %v4float %107 0
        %111 =   OpFOrdEqual %v4bool %109 %110
        %112 =   OpAll %bool %111
        %113 =   OpCompositeExtract %v4float %104 1
        %114 =   OpCompositeExtract %v4float %107 1
        %115 =   OpFOrdEqual %v4bool %113 %114
        %116 =   OpAll %bool %115
        %117 =   OpLogicalAnd %bool %112 %116
        %118 =   OpCompositeExtract %v4float %104 2
        %119 =   OpCompositeExtract %v4float %107 2
        %120 =   OpFOrdEqual %v4bool %118 %119
        %121 =   OpAll %bool %120
        %122 =   OpLogicalAnd %bool %117 %121
        %123 =   OpCompositeExtract %v4float %104 3
        %124 =   OpCompositeExtract %v4float %107 3
        %125 =   OpFOrdEqual %v4bool %123 %124
        %126 =   OpAll %bool %125
        %127 =   OpLogicalAnd %bool %122 %126
                 OpReturnValue %127
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %128        ; RelaxedPrecision
        %129 = OpFunctionParameter %_ptr_Function_v2float

        %130 = OpLabel
        %137 =   OpVariable %_ptr_Function_v4float Function
        %132 =   OpFunctionCall %bool %test3x3_b
                 OpSelectionMerge %134 None
                 OpBranchConditional %132 %133 %134

        %133 =     OpLabel
        %135 =       OpFunctionCall %bool %test4x4_b
                     OpBranch %134

        %134 = OpLabel
        %136 =   OpPhi %bool %false %130 %135 %133
                 OpSelectionMerge %140 None
                 OpBranchConditional %136 %138 %139

        %138 =     OpLabel
        %141 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %143 =       OpLoad %v4float %141           ; RelaxedPrecision
                     OpStore %137 %143
                     OpBranch %140

        %139 =     OpLabel
        %144 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %145 =       OpLoad %v4float %144           ; RelaxedPrecision
                     OpStore %137 %145
                     OpBranch %140

        %140 = OpLabel
        %146 =   OpLoad %v4float %137               ; RelaxedPrecision
                 OpReturnValue %146
               OpFunctionEnd

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
               OpName %expected "expected"              ; id %32
               OpName %c "c"                            ; id %38
               OpName %vec "vec"                        ; id %49
               OpName %r "r"                            ; id %57
               OpName %test4x4_b "test4x4_b"            ; id %7
               OpName %expected_0 "expected"            ; id %86
               OpName %c_0 "c"                          ; id %90
               OpName %vec_0 "vec"                      ; id %99
               OpName %r_0 "r"                          ; id %106
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
               OpDecorate %144 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision

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
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_3 = OpConstant %float 3
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
         %37 = OpConstantComposite %v3float %float_3 %float_2 %float_1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
      %false = OpConstantFalse %bool
      %int_1 = OpConstant %int 1
         %80 = OpConstantComposite %v3float %float_3 %float_3 %float_3
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %89 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
      %int_4 = OpConstant %int 4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %127 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %131 = OpTypeFunction %v4float %_ptr_Function_v2float


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
   %expected =   OpVariable %_ptr_Function_v3float Function
          %c =   OpVariable %_ptr_Function_int Function
        %vec =   OpVariable %_ptr_Function_v3float Function
          %r =   OpVariable %_ptr_Function_int Function
                 OpStore %expected %37
                 OpStore %c %int_0
                 OpBranch %41

         %41 = OpLabel
                 OpLoopMerge %45 %44 None
                 OpBranch %42

         %42 =     OpLabel
         %46 =       OpLoad %int %c
         %48 =       OpSLessThan %bool %46 %int_3
                     OpBranchConditional %48 %43 %45

         %43 =         OpLabel
         %50 =           OpAccessChain %_ptr_Uniform_mat3v3float %13 %int_2
         %53 =           OpLoad %int %c
         %54 =           OpAccessChain %_ptr_Uniform_v3float %50 %53
         %56 =           OpLoad %v3float %54
                         OpStore %vec %56
                         OpStore %r %int_0
                         OpBranch %58

         %58 =         OpLabel
                         OpLoopMerge %62 %61 None
                         OpBranch %59

         %59 =             OpLabel
         %63 =               OpLoad %int %r
         %64 =               OpSLessThan %bool %63 %int_3
                             OpBranchConditional %64 %60 %62

         %60 =                 OpLabel
         %65 =                   OpLoad %v3float %vec
         %66 =                   OpVectorShuffle %v3float %65 %65 2 1 0
         %67 =                   OpLoad %int %r
         %68 =                   OpVectorExtractDynamic %float %66 %67
         %69 =                   OpLoad %v3float %expected
         %70 =                   OpLoad %int %r
         %71 =                   OpVectorExtractDynamic %float %69 %70
         %72 =                   OpFUnordNotEqual %bool %68 %71
                                 OpSelectionMerge %74 None
                                 OpBranchConditional %72 %73 %74

         %73 =                     OpLabel
                                     OpReturnValue %false

         %74 =                 OpLabel
                                 OpBranch %61

         %61 =           OpLabel
         %77 =             OpLoad %int %r
         %78 =             OpIAdd %int %77 %int_1
                           OpStore %r %78
                           OpBranch %58

         %62 =         OpLabel
         %79 =           OpLoad %v3float %expected
         %81 =           OpFAdd %v3float %79 %80
                         OpStore %expected %81
                         OpBranch %44

         %44 =   OpLabel
         %82 =     OpLoad %int %c
         %83 =     OpIAdd %int %82 %int_1
                   OpStore %c %83
                   OpBranch %41

         %45 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function test4x4_b
  %test4x4_b = OpFunction %bool None %30

         %85 = OpLabel
 %expected_0 =   OpVariable %_ptr_Function_v4float Function
        %c_0 =   OpVariable %_ptr_Function_int Function
      %vec_0 =   OpVariable %_ptr_Function_v4float Function
        %r_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %expected_0 %89
                 OpStore %c_0 %int_0
                 OpBranch %91

         %91 = OpLabel
                 OpLoopMerge %95 %94 None
                 OpBranch %92

         %92 =     OpLabel
         %96 =       OpLoad %int %c_0
         %98 =       OpSLessThan %bool %96 %int_4
                     OpBranchConditional %98 %93 %95

         %93 =         OpLabel
        %100 =           OpAccessChain %_ptr_Uniform_mat4v4float %13 %int_3
        %102 =           OpLoad %int %c_0
        %103 =           OpAccessChain %_ptr_Uniform_v4float %100 %102
        %105 =           OpLoad %v4float %103
                         OpStore %vec_0 %105
                         OpStore %r_0 %int_0
                         OpBranch %107

        %107 =         OpLabel
                         OpLoopMerge %111 %110 None
                         OpBranch %108

        %108 =             OpLabel
        %112 =               OpLoad %int %r_0
        %113 =               OpSLessThan %bool %112 %int_4
                             OpBranchConditional %113 %109 %111

        %109 =                 OpLabel
        %114 =                   OpLoad %v4float %vec_0
        %115 =                   OpVectorShuffle %v4float %114 %114 3 2 1 0
        %116 =                   OpLoad %int %r_0
        %117 =                   OpVectorExtractDynamic %float %115 %116
        %118 =                   OpLoad %v4float %expected_0
        %119 =                   OpLoad %int %r_0
        %120 =                   OpVectorExtractDynamic %float %118 %119
        %121 =                   OpFUnordNotEqual %bool %117 %120
                                 OpSelectionMerge %123 None
                                 OpBranchConditional %121 %122 %123

        %122 =                     OpLabel
                                     OpReturnValue %false

        %123 =                 OpLabel
                                 OpBranch %110

        %110 =           OpLabel
        %124 =             OpLoad %int %r_0
        %125 =             OpIAdd %int %124 %int_1
                           OpStore %r_0 %125
                           OpBranch %107

        %111 =         OpLabel
        %126 =           OpLoad %v4float %expected_0
        %128 =           OpFAdd %v4float %126 %127
                         OpStore %expected_0 %128
                         OpBranch %94

         %94 =   OpLabel
        %129 =     OpLoad %int %c_0
        %130 =     OpIAdd %int %129 %int_1
                   OpStore %c_0 %130
                   OpBranch %91

         %95 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %131        ; RelaxedPrecision
        %132 = OpFunctionParameter %_ptr_Function_v2float

        %133 = OpLabel
        %139 =   OpVariable %_ptr_Function_v4float Function
        %134 =   OpFunctionCall %bool %test3x3_b
                 OpSelectionMerge %136 None
                 OpBranchConditional %134 %135 %136

        %135 =     OpLabel
        %137 =       OpFunctionCall %bool %test4x4_b
                     OpBranch %136

        %136 = OpLabel
        %138 =   OpPhi %bool %false %133 %137 %135
                 OpSelectionMerge %142 None
                 OpBranchConditional %138 %140 %141

        %140 =     OpLabel
        %143 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %144 =       OpLoad %v4float %143           ; RelaxedPrecision
                     OpStore %139 %144
                     OpBranch %142

        %141 =     OpLabel
        %145 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %146 =       OpLoad %v4float %145           ; RelaxedPrecision
                     OpStore %139 %146
                     OpBranch %142

        %142 = OpLabel
        %147 =   OpLoad %v4float %139               ; RelaxedPrecision
                 OpReturnValue %147
               OpFunctionEnd

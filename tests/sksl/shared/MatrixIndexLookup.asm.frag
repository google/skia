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
               OpName %expected "expected"              ; id %38
               OpName %index "index"                    ; id %44
               OpName %test4x4_b "test4x4_b"            ; id %7
               OpName %matrix_0 "matrix"                ; id %73
               OpName %expected_0 "expected"            ; id %78
               OpName %index_0 "index"                  ; id %82
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
               OpDecorate %119 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision

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
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %43 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
     %v3bool = OpTypeVector %bool 3
      %false = OpConstantFalse %bool
         %66 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
       %true = OpConstantTrue %bool
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %81 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
      %int_4 = OpConstant %int 4
     %v4bool = OpTypeVector %bool 4
        %101 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %105 = OpTypeFunction %v4float %_ptr_Function_v2float
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
   %expected =   OpVariable %_ptr_Function_v3float Function
      %index =   OpVariable %_ptr_Function_int Function
         %34 =   OpAccessChain %_ptr_Uniform_mat3v3float %13 %int_2
         %37 =   OpLoad %mat3v3float %34
                 OpStore %matrix %37
                 OpStore %expected %43
                 OpStore %index %int_0
                 OpBranch %47

         %47 = OpLabel
                 OpLoopMerge %51 %50 None
                 OpBranch %48

         %48 =     OpLabel
         %52 =       OpLoad %int %index
         %54 =       OpSLessThan %bool %52 %int_3
                     OpBranchConditional %54 %49 %51

         %49 =         OpLabel
         %55 =           OpLoad %int %index
         %56 =           OpAccessChain %_ptr_Function_v3float %matrix %55
         %57 =           OpLoad %v3float %56
         %58 =           OpLoad %v3float %expected
         %59 =           OpFUnordNotEqual %v3bool %57 %58
         %61 =           OpAny %bool %59
                         OpSelectionMerge %63 None
                         OpBranchConditional %61 %62 %63

         %62 =             OpLabel
                             OpReturnValue %false

         %63 =         OpLabel
         %65 =           OpLoad %v3float %expected
         %67 =           OpFAdd %v3float %65 %66
                         OpStore %expected %67
                         OpBranch %50

         %50 =   OpLabel
         %69 =     OpLoad %int %index
         %70 =     OpIAdd %int %69 %int_1
                   OpStore %index %70
                   OpBranch %47

         %51 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function test4x4_b
  %test4x4_b = OpFunction %bool None %30

         %72 = OpLabel
   %matrix_0 =   OpVariable %_ptr_Function_mat4v4float Function
 %expected_0 =   OpVariable %_ptr_Function_v4float Function
    %index_0 =   OpVariable %_ptr_Function_int Function
         %75 =   OpAccessChain %_ptr_Uniform_mat4v4float %13 %int_3
         %77 =   OpLoad %mat4v4float %75
                 OpStore %matrix_0 %77
                 OpStore %expected_0 %81
                 OpStore %index_0 %int_0
                 OpBranch %83

         %83 = OpLabel
                 OpLoopMerge %87 %86 None
                 OpBranch %84

         %84 =     OpLabel
         %88 =       OpLoad %int %index_0
         %90 =       OpSLessThan %bool %88 %int_4
                     OpBranchConditional %90 %85 %87

         %85 =         OpLabel
         %91 =           OpLoad %int %index_0
         %92 =           OpAccessChain %_ptr_Function_v4float %matrix_0 %91
         %93 =           OpLoad %v4float %92
         %94 =           OpLoad %v4float %expected_0
         %95 =           OpFUnordNotEqual %v4bool %93 %94
         %97 =           OpAny %bool %95
                         OpSelectionMerge %99 None
                         OpBranchConditional %97 %98 %99

         %98 =             OpLabel
                             OpReturnValue %false

         %99 =         OpLabel
        %100 =           OpLoad %v4float %expected_0
        %102 =           OpFAdd %v4float %100 %101
                         OpStore %expected_0 %102
                         OpBranch %86

         %86 =   OpLabel
        %103 =     OpLoad %int %index_0
        %104 =     OpIAdd %int %103 %int_1
                   OpStore %index_0 %104
                   OpBranch %83

         %87 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %105        ; RelaxedPrecision
        %106 = OpFunctionParameter %_ptr_Function_v2float

        %107 = OpLabel
        %113 =   OpVariable %_ptr_Function_v4float Function
        %108 =   OpFunctionCall %bool %test3x3_b
                 OpSelectionMerge %110 None
                 OpBranchConditional %108 %109 %110

        %109 =     OpLabel
        %111 =       OpFunctionCall %bool %test4x4_b
                     OpBranch %110

        %110 = OpLabel
        %112 =   OpPhi %bool %false %107 %111 %109
                 OpSelectionMerge %116 None
                 OpBranchConditional %112 %114 %115

        %114 =     OpLabel
        %117 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %119 =       OpLoad %v4float %117           ; RelaxedPrecision
                     OpStore %113 %119
                     OpBranch %116

        %115 =     OpLabel
        %120 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %121 =       OpLoad %v4float %120           ; RelaxedPrecision
                     OpStore %113 %121
                     OpBranch %116

        %116 = OpLabel
        %122 =   OpLoad %v4float %113               ; RelaxedPrecision
                 OpReturnValue %122
               OpFunctionEnd

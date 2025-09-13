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
               OpName %vec "vec"                        ; id %38
               OpName %c "c"                            ; id %39
               OpName %r "r"                            ; id %50
               OpName %test4x4_b "test4x4_b"            ; id %7
               OpName %expected_0 "expected"            ; id %91
               OpName %vec_0 "vec"                      ; id %95
               OpName %c_0 "c"                          ; id %96
               OpName %r_0 "r"                          ; id %105
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
               OpDecorate %153 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision

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
      %int_1 = OpConstant %int 1
      %v3int = OpTypeVector %int 3
         %69 = OpConstantComposite %v3int %int_2 %int_1 %int_0
%_ptr_Function_float = OpTypePointer Function %float
     %v3bool = OpTypeVector %bool 3
      %false = OpConstantFalse %bool
         %85 = OpConstantComposite %v3float %float_3 %float_3 %float_3
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %94 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
      %int_4 = OpConstant %int 4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %v4int = OpTypeVector %int 4
        %122 = OpConstantComposite %v4int %int_3 %int_2 %int_1 %int_0
     %v4bool = OpTypeVector %bool 4
        %136 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %140 = OpTypeFunction %v4float %_ptr_Function_v2float


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
        %vec =   OpVariable %_ptr_Function_v3float Function
          %c =   OpVariable %_ptr_Function_int Function
          %r =   OpVariable %_ptr_Function_int Function
                 OpStore %expected %37
                 OpStore %c %int_0
                 OpBranch %42

         %42 = OpLabel
                 OpLoopMerge %46 %45 None
                 OpBranch %43

         %43 =     OpLabel
         %47 =       OpLoad %int %c
         %49 =       OpSLessThan %bool %47 %int_3
                     OpBranchConditional %49 %44 %46

         %44 =         OpLabel
                         OpStore %r %int_0
                         OpBranch %51

         %51 =         OpLabel
                         OpLoopMerge %55 %54 None
                         OpBranch %52

         %52 =             OpLabel
         %56 =               OpLoad %int %r
         %57 =               OpSLessThan %bool %56 %int_3
                             OpBranchConditional %57 %53 %55

         %53 =                 OpLabel
         %58 =                   OpAccessChain %_ptr_Uniform_mat3v3float %13 %int_2
         %61 =                   OpLoad %int %c
         %62 =                   OpAccessChain %_ptr_Uniform_v3float %58 %61
         %64 =                   OpLoad %v3float %62
         %65 =                   OpLoad %int %r
         %66 =                   OpVectorExtractDynamic %float %64 %65
         %70 =                   OpLoad %int %r
         %71 =                   OpVectorExtractDynamic %int %69 %70
         %72 =                   OpAccessChain %_ptr_Function_float %vec %71
                                 OpStore %72 %66
                                 OpBranch %54

         %54 =           OpLabel
         %74 =             OpLoad %int %r
         %75 =             OpIAdd %int %74 %int_1
                           OpStore %r %75
                           OpBranch %51

         %55 =         OpLabel
         %76 =           OpLoad %v3float %vec
         %77 =           OpLoad %v3float %expected
         %78 =           OpFUnordNotEqual %v3bool %76 %77
         %80 =           OpAny %bool %78
                         OpSelectionMerge %82 None
                         OpBranchConditional %80 %81 %82

         %81 =             OpLabel
                             OpReturnValue %false

         %82 =         OpLabel
         %84 =           OpLoad %v3float %expected
         %86 =           OpFAdd %v3float %84 %85
                         OpStore %expected %86
                         OpBranch %45

         %45 =   OpLabel
         %87 =     OpLoad %int %c
         %88 =     OpIAdd %int %87 %int_1
                   OpStore %c %88
                   OpBranch %42

         %46 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function test4x4_b
  %test4x4_b = OpFunction %bool None %30

         %90 = OpLabel
 %expected_0 =   OpVariable %_ptr_Function_v4float Function
      %vec_0 =   OpVariable %_ptr_Function_v4float Function
        %c_0 =   OpVariable %_ptr_Function_int Function
        %r_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %expected_0 %94
                 OpStore %c_0 %int_0
                 OpBranch %97

         %97 = OpLabel
                 OpLoopMerge %101 %100 None
                 OpBranch %98

         %98 =     OpLabel
        %102 =       OpLoad %int %c_0
        %104 =       OpSLessThan %bool %102 %int_4
                     OpBranchConditional %104 %99 %101

         %99 =         OpLabel
                         OpStore %r_0 %int_0
                         OpBranch %106

        %106 =         OpLabel
                         OpLoopMerge %110 %109 None
                         OpBranch %107

        %107 =             OpLabel
        %111 =               OpLoad %int %r_0
        %112 =               OpSLessThan %bool %111 %int_4
                             OpBranchConditional %112 %108 %110

        %108 =                 OpLabel
        %113 =                   OpAccessChain %_ptr_Uniform_mat4v4float %13 %int_3
        %115 =                   OpLoad %int %c_0
        %116 =                   OpAccessChain %_ptr_Uniform_v4float %113 %115
        %118 =                   OpLoad %v4float %116
        %119 =                   OpLoad %int %r_0
        %120 =                   OpVectorExtractDynamic %float %118 %119
        %123 =                   OpLoad %int %r_0
        %124 =                   OpVectorExtractDynamic %int %122 %123
        %125 =                   OpAccessChain %_ptr_Function_float %vec_0 %124
                                 OpStore %125 %120
                                 OpBranch %109

        %109 =           OpLabel
        %126 =             OpLoad %int %r_0
        %127 =             OpIAdd %int %126 %int_1
                           OpStore %r_0 %127
                           OpBranch %106

        %110 =         OpLabel
        %128 =           OpLoad %v4float %vec_0
        %129 =           OpLoad %v4float %expected_0
        %130 =           OpFUnordNotEqual %v4bool %128 %129
        %132 =           OpAny %bool %130
                         OpSelectionMerge %134 None
                         OpBranchConditional %132 %133 %134

        %133 =             OpLabel
                             OpReturnValue %false

        %134 =         OpLabel
        %135 =           OpLoad %v4float %expected_0
        %137 =           OpFAdd %v4float %135 %136
                         OpStore %expected_0 %137
                         OpBranch %100

        %100 =   OpLabel
        %138 =     OpLoad %int %c_0
        %139 =     OpIAdd %int %138 %int_1
                   OpStore %c_0 %139
                   OpBranch %97

        %101 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %140        ; RelaxedPrecision
        %141 = OpFunctionParameter %_ptr_Function_v2float

        %142 = OpLabel
        %148 =   OpVariable %_ptr_Function_v4float Function
        %143 =   OpFunctionCall %bool %test3x3_b
                 OpSelectionMerge %145 None
                 OpBranchConditional %143 %144 %145

        %144 =     OpLabel
        %146 =       OpFunctionCall %bool %test4x4_b
                     OpBranch %145

        %145 = OpLabel
        %147 =   OpPhi %bool %false %142 %146 %144
                 OpSelectionMerge %151 None
                 OpBranchConditional %147 %149 %150

        %149 =     OpLabel
        %152 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %153 =       OpLoad %v4float %152           ; RelaxedPrecision
                     OpStore %148 %153
                     OpBranch %151

        %150 =     OpLabel
        %154 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %155 =       OpLoad %v4float %154           ; RelaxedPrecision
                     OpStore %148 %155
                     OpBranch %151

        %151 = OpLabel
        %156 =   OpLoad %v4float %148               ; RelaxedPrecision
                 OpReturnValue %156
               OpFunctionEnd

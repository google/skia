               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %16
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix3x3"
               OpMemberName %_UniformBuffer 3 "testMatrix4x4"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %18
               OpName %test4x4_b "test4x4_b"            ; id %6
               OpName %matrix "matrix"                  ; id %31
               OpName %values "values"                  ; id %33
               OpName %index "index"                    ; id %40
               OpName %main "main"                      ; id %7
               OpName %_0_matrix "_0_matrix"            ; id %97
               OpName %_1_values "_1_values"            ; id %99
               OpName %_2_index "_2_index"              ; id %102

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
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %158 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision

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
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %29 = OpTypeFunction %bool
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
    %float_3 = OpConstant %float 3
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
         %39 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_4 = OpConstant %int 4
         %64 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
      %int_1 = OpConstant %int 1
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_3 = OpConstant %int 3
     %v4bool = OpTypeVector %bool 4
         %94 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
        %101 = OpConstantComposite %v3float %float_3 %float_2 %float_1
%_ptr_Function_float = OpTypePointer Function %float
        %123 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %20

         %21 = OpLabel
         %25 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %25 %24
         %27 =   OpFunctionCall %v4float %main %25
                 OpStore %sk_FragColor %27
                 OpReturn
               OpFunctionEnd


               ; Function test4x4_b
  %test4x4_b = OpFunction %bool None %29

         %30 = OpLabel
     %matrix =   OpVariable %_ptr_Function_mat4v4float Function
     %values =   OpVariable %_ptr_Function_v4float Function
      %index =   OpVariable %_ptr_Function_int Function
                 OpStore %values %39
                 OpStore %index %int_0
                 OpBranch %43

         %43 = OpLabel
                 OpLoopMerge %47 %46 None
                 OpBranch %44

         %44 =     OpLabel
         %48 =       OpLoad %int %index
         %50 =       OpSLessThan %bool %48 %int_4
                     OpBranchConditional %50 %45 %47

         %45 =         OpLabel
         %51 =           OpLoad %v4float %values
         %52 =           OpVectorShuffle %v2float %51 %51 0 3
         %53 =           OpLoad %int %index
         %54 =           OpAccessChain %_ptr_Function_v4float %matrix %53
         %55 =           OpLoad %v4float %54
         %56 =           OpVectorShuffle %v4float %55 %52 5 1 2 4
                         OpStore %54 %56
         %57 =           OpLoad %v4float %values
         %58 =           OpVectorShuffle %v2float %57 %57 1 2
         %59 =           OpLoad %int %index
         %60 =           OpAccessChain %_ptr_Function_v4float %matrix %59
         %61 =           OpLoad %v4float %60
         %62 =           OpVectorShuffle %v4float %61 %58 0 5 4 3
                         OpStore %60 %62
         %63 =           OpLoad %v4float %values
         %65 =           OpFAdd %v4float %63 %64
                         OpStore %values %65
                         OpBranch %46

         %46 =   OpLabel
         %67 =     OpLoad %int %index
         %68 =     OpIAdd %int %67 %int_1
                   OpStore %index %68
                   OpBranch %43

         %47 = OpLabel
         %69 =   OpLoad %mat4v4float %matrix
         %70 =   OpAccessChain %_ptr_Uniform_mat4v4float %12 %int_3
         %73 =   OpLoad %mat4v4float %70
         %75 =   OpCompositeExtract %v4float %69 0
         %76 =   OpCompositeExtract %v4float %73 0
         %77 =   OpFOrdEqual %v4bool %75 %76
         %78 =   OpAll %bool %77
         %79 =   OpCompositeExtract %v4float %69 1
         %80 =   OpCompositeExtract %v4float %73 1
         %81 =   OpFOrdEqual %v4bool %79 %80
         %82 =   OpAll %bool %81
         %83 =   OpLogicalAnd %bool %78 %82
         %84 =   OpCompositeExtract %v4float %69 2
         %85 =   OpCompositeExtract %v4float %73 2
         %86 =   OpFOrdEqual %v4bool %84 %85
         %87 =   OpAll %bool %86
         %88 =   OpLogicalAnd %bool %83 %87
         %89 =   OpCompositeExtract %v4float %69 3
         %90 =   OpCompositeExtract %v4float %73 3
         %91 =   OpFOrdEqual %v4bool %89 %90
         %92 =   OpAll %bool %91
         %93 =   OpLogicalAnd %bool %88 %92
                 OpReturnValue %93
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %94         ; RelaxedPrecision
         %95 = OpFunctionParameter %_ptr_Function_v2float

         %96 = OpLabel
  %_0_matrix =   OpVariable %_ptr_Function_mat3v3float Function
  %_1_values =   OpVariable %_ptr_Function_v3float Function
   %_2_index =   OpVariable %_ptr_Function_int Function
        %152 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %_1_values %101
                 OpStore %_2_index %int_0
                 OpBranch %103

        %103 = OpLabel
                 OpLoopMerge %107 %106 None
                 OpBranch %104

        %104 =     OpLabel
        %108 =       OpLoad %int %_2_index
        %109 =       OpSLessThan %bool %108 %int_3
                     OpBranchConditional %109 %105 %107

        %105 =         OpLabel
        %110 =           OpLoad %v3float %_1_values
        %111 =           OpVectorShuffle %v2float %110 %110 0 2
        %112 =           OpLoad %int %_2_index
        %113 =           OpAccessChain %_ptr_Function_v3float %_0_matrix %112
        %114 =           OpLoad %v3float %113
        %115 =           OpVectorShuffle %v3float %114 %111 4 1 3
                         OpStore %113 %115
        %116 =           OpLoad %v3float %_1_values
        %117 =           OpCompositeExtract %float %116 1
        %118 =           OpLoad %int %_2_index
        %119 =           OpAccessChain %_ptr_Function_v3float %_0_matrix %118
        %120 =           OpAccessChain %_ptr_Function_float %119 %int_1
                         OpStore %120 %117
        %122 =           OpLoad %v3float %_1_values
        %124 =           OpFAdd %v3float %122 %123
                         OpStore %_1_values %124
                         OpBranch %106

        %106 =   OpLabel
        %125 =     OpLoad %int %_2_index
        %126 =     OpIAdd %int %125 %int_1
                   OpStore %_2_index %126
                   OpBranch %103

        %107 = OpLabel
        %128 =   OpLoad %mat3v3float %_0_matrix
        %129 =   OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_2
        %132 =   OpLoad %mat3v3float %129
        %134 =   OpCompositeExtract %v3float %128 0
        %135 =   OpCompositeExtract %v3float %132 0
        %136 =   OpFOrdEqual %v3bool %134 %135
        %137 =   OpAll %bool %136
        %138 =   OpCompositeExtract %v3float %128 1
        %139 =   OpCompositeExtract %v3float %132 1
        %140 =   OpFOrdEqual %v3bool %138 %139
        %141 =   OpAll %bool %140
        %142 =   OpLogicalAnd %bool %137 %141
        %143 =   OpCompositeExtract %v3float %128 2
        %144 =   OpCompositeExtract %v3float %132 2
        %145 =   OpFOrdEqual %v3bool %143 %144
        %146 =   OpAll %bool %145
        %147 =   OpLogicalAnd %bool %142 %146
                 OpSelectionMerge %149 None
                 OpBranchConditional %147 %148 %149

        %148 =     OpLabel
        %150 =       OpFunctionCall %bool %test4x4_b
                     OpBranch %149

        %149 = OpLabel
        %151 =   OpPhi %bool %false %107 %150 %148
                 OpSelectionMerge %155 None
                 OpBranchConditional %151 %153 %154

        %153 =     OpLabel
        %156 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %158 =       OpLoad %v4float %156           ; RelaxedPrecision
                     OpStore %152 %158
                     OpBranch %155

        %154 =     OpLabel
        %159 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %160 =       OpLoad %v4float %159           ; RelaxedPrecision
                     OpStore %152 %160
                     OpBranch %155

        %155 = OpLabel
        %161 =   OpLoad %v4float %152               ; RelaxedPrecision
                 OpReturnValue %161
               OpFunctionEnd

               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "N"
               OpMemberName %_UniformBuffer 1 "I"
               OpMemberName %_UniformBuffer 2 "NRef"
               OpMemberName %_UniformBuffer 3 "colorGreen"
               OpMemberName %_UniformBuffer 4 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %huge "huge"                      ; id %27
               OpName %huge2 "huge2"                    ; id %32
               OpName %huge3 "huge3"                    ; id %36
               OpName %huge4 "huge4"                    ; id %42
               OpName %expectedPos "expectedPos"        ; id %47
               OpName %expectedNeg "expectedNeg"        ; id %51

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
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 4 Offset 64
               OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %expectedPos RelaxedPrecision
               OpDecorate %expectedNeg RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float     ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
%float_1_00000002e_30 = OpConstant %float 1.00000002e+30
         %34 = OpConstantComposite %v2float %float_1 %float_1
         %35 = OpConstantComposite %v2float %float_1_00000002e_30 %float_1_00000002e_30
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %40 = OpConstantComposite %v3float %float_1 %float_1 %float_1
         %41 = OpConstantComposite %v3float %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %45 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %46 = OpConstantComposite %v4float %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %58 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
   %float_n1 = OpConstant %float -1
   %float_n2 = OpConstant %float -2
   %float_n3 = OpConstant %float -3
   %float_n4 = OpConstant %float -4
         %63 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %134 = OpConstantComposite %v2float %float_n1 %float_n2
        %141 = OpConstantComposite %v3float %float_1 %float_2 %float_3
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
       %huge =   OpVariable %_ptr_Function_float Function
      %huge2 =   OpVariable %_ptr_Function_v2float Function
      %huge3 =   OpVariable %_ptr_Function_v3float Function
      %huge4 =   OpVariable %_ptr_Function_v4float Function
%expectedPos =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
%expectedNeg =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %149 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpExtInst %float %5 FaceForward %float_1 %float_1_00000002e_30 %float_1_00000002e_30   ; RelaxedPrecision
                 OpStore %huge %29
         %33 =   OpExtInst %v2float %5 FaceForward %34 %35 %35
                 OpStore %huge2 %33
         %39 =   OpExtInst %v3float %5 FaceForward %40 %41 %41
                 OpStore %huge3 %39
         %44 =   OpExtInst %v4float %5 FaceForward %45 %46 %46
                 OpStore %huge4 %44
         %48 =   OpCompositeConstruct %v4float %29 %29 %29 %29
         %49 =   OpVectorShuffle %v4float %33 %33 0 0 0 0
         %50 =   OpFAdd %v4float %48 %49
                 OpStore %expectedPos %50
         %52 =   OpVectorShuffle %v4float %39 %39 0 0 0 0
         %53 =   OpVectorShuffle %v4float %44 %44 0 0 0 0
         %54 =   OpFAdd %v4float %52 %53
                 OpStore %expectedNeg %54
                 OpStore %expectedPos %58
                 OpStore %expectedNeg %63
         %67 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %70 =   OpLoad %v4float %67                ; RelaxedPrecision
         %71 =   OpCompositeExtract %float %70 0    ; RelaxedPrecision
         %72 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %74 =   OpLoad %v4float %72                ; RelaxedPrecision
         %75 =   OpCompositeExtract %float %74 0    ; RelaxedPrecision
         %76 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %78 =   OpLoad %v4float %76                ; RelaxedPrecision
         %79 =   OpCompositeExtract %float %78 0    ; RelaxedPrecision
         %66 =   OpExtInst %float %5 FaceForward %71 %75 %79    ; RelaxedPrecision
         %80 =   OpFOrdEqual %bool %66 %float_n1
                 OpSelectionMerge %82 None
                 OpBranchConditional %80 %81 %82

         %81 =     OpLabel
         %84 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %85 =       OpLoad %v4float %84            ; RelaxedPrecision
         %86 =       OpVectorShuffle %v2float %85 %85 0 1   ; RelaxedPrecision
         %87 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %88 =       OpLoad %v4float %87            ; RelaxedPrecision
         %89 =       OpVectorShuffle %v2float %88 %88 0 1   ; RelaxedPrecision
         %90 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %91 =       OpLoad %v4float %90            ; RelaxedPrecision
         %92 =       OpVectorShuffle %v2float %91 %91 0 1   ; RelaxedPrecision
         %83 =       OpExtInst %v2float %5 FaceForward %86 %89 %92  ; RelaxedPrecision
         %93 =       OpVectorShuffle %v2float %63 %63 0 1           ; RelaxedPrecision
         %94 =       OpFOrdEqual %v2bool %83 %93
         %96 =       OpAll %bool %94
                     OpBranch %82

         %82 = OpLabel
         %97 =   OpPhi %bool %false %26 %96 %81
                 OpSelectionMerge %99 None
                 OpBranchConditional %97 %98 %99

         %98 =     OpLabel
        %101 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %102 =       OpLoad %v4float %101           ; RelaxedPrecision
        %103 =       OpVectorShuffle %v3float %102 %102 0 1 2   ; RelaxedPrecision
        %104 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %105 =       OpLoad %v4float %104           ; RelaxedPrecision
        %106 =       OpVectorShuffle %v3float %105 %105 0 1 2   ; RelaxedPrecision
        %107 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %108 =       OpLoad %v4float %107           ; RelaxedPrecision
        %109 =       OpVectorShuffle %v3float %108 %108 0 1 2   ; RelaxedPrecision
        %100 =       OpExtInst %v3float %5 FaceForward %103 %106 %109   ; RelaxedPrecision
        %110 =       OpVectorShuffle %v3float %58 %58 0 1 2             ; RelaxedPrecision
        %111 =       OpFOrdEqual %v3bool %100 %110
        %113 =       OpAll %bool %111
                     OpBranch %99

         %99 = OpLabel
        %114 =   OpPhi %bool %false %82 %113 %98
                 OpSelectionMerge %116 None
                 OpBranchConditional %114 %115 %116

        %115 =     OpLabel
        %118 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %119 =       OpLoad %v4float %118           ; RelaxedPrecision
        %120 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %121 =       OpLoad %v4float %120           ; RelaxedPrecision
        %122 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %123 =       OpLoad %v4float %122           ; RelaxedPrecision
        %117 =       OpExtInst %v4float %5 FaceForward %119 %121 %123   ; RelaxedPrecision
        %124 =       OpFOrdEqual %v4bool %117 %58
        %126 =       OpAll %bool %124
                     OpBranch %116

        %116 = OpLabel
        %127 =   OpPhi %bool %false %99 %126 %115
                 OpSelectionMerge %129 None
                 OpBranchConditional %127 %128 %129

        %128 =     OpLabel
                     OpBranch %129

        %129 = OpLabel
        %131 =   OpPhi %bool %false %116 %true %128
                 OpSelectionMerge %133 None
                 OpBranchConditional %131 %132 %133

        %132 =     OpLabel
        %135 =       OpVectorShuffle %v2float %63 %63 0 1   ; RelaxedPrecision
        %136 =       OpFOrdEqual %v2bool %134 %135
        %137 =       OpAll %bool %136
                     OpBranch %133

        %133 = OpLabel
        %138 =   OpPhi %bool %false %129 %137 %132
                 OpSelectionMerge %140 None
                 OpBranchConditional %138 %139 %140

        %139 =     OpLabel
        %142 =       OpVectorShuffle %v3float %58 %58 0 1 2     ; RelaxedPrecision
        %143 =       OpFOrdEqual %v3bool %141 %142
        %144 =       OpAll %bool %143
                     OpBranch %140

        %140 = OpLabel
        %145 =   OpPhi %bool %false %133 %144 %139
                 OpSelectionMerge %147 None
                 OpBranchConditional %145 %146 %147

        %146 =     OpLabel
                     OpBranch %147

        %147 = OpLabel
        %148 =   OpPhi %bool %false %140 %true %146
                 OpSelectionMerge %152 None
                 OpBranchConditional %148 %150 %151

        %150 =     OpLabel
        %153 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %155 =       OpLoad %v4float %153           ; RelaxedPrecision
                     OpStore %149 %155
                     OpBranch %152

        %151 =     OpLabel
        %156 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %158 =       OpLoad %v4float %156           ; RelaxedPrecision
                     OpStore %149 %158
                     OpBranch %152

        %152 = OpLabel
        %159 =   OpLoad %v4float %149               ; RelaxedPrecision
                 OpReturnValue %159
               OpFunctionEnd

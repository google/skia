               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %checkIntrinsicAsFunctionArg_bf3i3 "checkIntrinsicAsFunctionArg_bf3i3"    ; id %6
               OpName %main "main"                                                              ; id %7
               OpName %value "value"                                                            ; id %52
               OpName %exp "exp"                                                                ; id %61
               OpName %result "result"                                                          ; id %64
               OpName %ok "ok"                                                                  ; id %65
               OpName %funcOk "funcOk"                                                          ; id %137

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
         %30 = OpTypeFunction %bool %_ptr_Function_v3float %_ptr_Function_v3int
      %false = OpConstantFalse %bool
 %float_0_75 = OpConstant %float 0.75
         %37 = OpConstantComposite %v3float %float_0_75 %float_0_75 %float_0_75
     %v3bool = OpTypeVector %bool 3
      %int_3 = OpConstant %int 3
         %45 = OpConstantComposite %v3int %int_3 %int_3 %int_3
         %49 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_6 = OpConstant %float 6
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_bool = OpTypePointer Function %bool
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function checkIntrinsicAsFunctionArg_bf3i3
%checkIntrinsicAsFunctionArg_bf3i3 = OpFunction %bool None %30
         %31 = OpFunctionParameter %_ptr_Function_v3float
         %32 = OpFunctionParameter %_ptr_Function_v3int

         %33 = OpLabel
         %35 =   OpLoad %v3float %31
         %38 =   OpFOrdEqual %v3bool %35 %37
         %40 =   OpAll %bool %38
                 OpSelectionMerge %42 None
                 OpBranchConditional %40 %41 %42

         %41 =     OpLabel
         %43 =       OpLoad %v3int %32
         %46 =       OpIEqual %v3bool %43 %45
         %47 =       OpAll %bool %46
                     OpBranch %42

         %42 = OpLabel
         %48 =   OpPhi %bool %false %33 %47 %41
                 OpReturnValue %48
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %49         ; RelaxedPrecision
         %50 = OpFunctionParameter %_ptr_Function_v2float

         %51 = OpLabel
      %value =   OpVariable %_ptr_Function_v4float Function
        %exp =   OpVariable %_ptr_Function_v4int Function
     %result =   OpVariable %_ptr_Function_v4float Function
         %ok =   OpVariable %_ptr_Function_v4bool Function
         %72 =   OpVariable %_ptr_Function_int Function
         %90 =   OpVariable %_ptr_Function_v2int Function
        %110 =   OpVariable %_ptr_Function_v3int Function
        %127 =   OpVariable %_ptr_Function_v4int Function
     %funcOk =   OpVariable %_ptr_Function_bool Function
        %141 =   OpVariable %_ptr_Function_v3int Function
        %146 =   OpVariable %_ptr_Function_v3float Function
        %148 =   OpVariable %_ptr_Function_v3int Function
        %155 =   OpVariable %_ptr_Function_v4float Function
         %54 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %57 =   OpLoad %v4float %54                ; RelaxedPrecision
         %58 =   OpVectorShuffle %v4float %57 %57 1 1 1 1   ; RelaxedPrecision
         %60 =   OpVectorTimesScalar %v4float %58 %float_6  ; RelaxedPrecision
                 OpStore %value %60
         %69 =   OpCompositeExtract %float %60 0
         %70 =   OpAccessChain %_ptr_Function_int %exp %int_0
         %68 =   OpExtInst %float %5 Frexp %69 %72
         %73 =   OpLoad %int %72
                 OpStore %70 %73
         %74 =   OpAccessChain %_ptr_Function_float %result %int_0
                 OpStore %74 %68
         %76 =   OpLoad %v4float %result
         %77 =   OpCompositeExtract %float %76 0
         %78 =   OpFOrdEqual %bool %77 %float_0_75
                 OpSelectionMerge %80 None
                 OpBranchConditional %78 %79 %80

         %79 =     OpLabel
         %81 =       OpLoad %v4int %exp
         %82 =       OpCompositeExtract %int %81 0
         %83 =       OpIEqual %bool %82 %int_3
                     OpBranch %80

         %80 = OpLabel
         %84 =   OpPhi %bool %false %51 %83 %79
         %85 =   OpAccessChain %_ptr_Function_bool %ok %int_0
                 OpStore %85 %84
         %88 =   OpLoad %v4float %value
         %89 =   OpVectorShuffle %v2float %88 %88 0 1
         %87 =   OpExtInst %v2float %5 Frexp %89 %90
         %93 =   OpLoad %v2int %90
         %94 =   OpLoad %v4int %exp
         %95 =   OpVectorShuffle %v4int %94 %93 4 5 2 3
                 OpStore %exp %95
         %96 =   OpLoad %v4float %result
         %97 =   OpVectorShuffle %v4float %96 %87 4 5 2 3
                 OpStore %result %97
         %98 =   OpCompositeExtract %float %97 1
         %99 =   OpFOrdEqual %bool %98 %float_0_75
                 OpSelectionMerge %101 None
                 OpBranchConditional %99 %100 %101

        %100 =     OpLabel
        %102 =       OpCompositeExtract %int %95 1
        %103 =       OpIEqual %bool %102 %int_3
                     OpBranch %101

        %101 = OpLabel
        %104 =   OpPhi %bool %false %80 %103 %100
        %105 =   OpAccessChain %_ptr_Function_bool %ok %int_1
                 OpStore %105 %104
        %108 =   OpLoad %v4float %value
        %109 =   OpVectorShuffle %v3float %108 %108 0 1 2
        %107 =   OpExtInst %v3float %5 Frexp %109 %110
        %111 =   OpLoad %v3int %110
        %112 =   OpLoad %v4int %exp
        %113 =   OpVectorShuffle %v4int %112 %111 4 5 6 3
                 OpStore %exp %113
        %114 =   OpLoad %v4float %result
        %115 =   OpVectorShuffle %v4float %114 %107 4 5 6 3
                 OpStore %result %115
        %116 =   OpCompositeExtract %float %115 2
        %117 =   OpFOrdEqual %bool %116 %float_0_75
                 OpSelectionMerge %119 None
                 OpBranchConditional %117 %118 %119

        %118 =     OpLabel
        %120 =       OpCompositeExtract %int %113 2
        %121 =       OpIEqual %bool %120 %int_3
                     OpBranch %119

        %119 = OpLabel
        %122 =   OpPhi %bool %false %101 %121 %118
        %123 =   OpAccessChain %_ptr_Function_bool %ok %int_2
                 OpStore %123 %122
        %126 =   OpLoad %v4float %value
        %125 =   OpExtInst %v4float %5 Frexp %126 %127
        %128 =   OpLoad %v4int %127
                 OpStore %exp %128
                 OpStore %result %125
        %129 =   OpCompositeExtract %float %125 3
        %130 =   OpFOrdEqual %bool %129 %float_0_75
                 OpSelectionMerge %132 None
                 OpBranchConditional %130 %131 %132

        %131 =     OpLabel
        %133 =       OpCompositeExtract %int %128 3
        %134 =       OpIEqual %bool %133 %int_3
                     OpBranch %132

        %132 = OpLabel
        %135 =   OpPhi %bool %false %119 %134 %131
        %136 =   OpAccessChain %_ptr_Function_bool %ok %int_3
                 OpStore %136 %135
        %139 =   OpLoad %v4float %value
        %140 =   OpVectorShuffle %v3float %139 %139 3 2 1
        %138 =   OpExtInst %v3float %5 Frexp %140 %141
        %142 =   OpLoad %v3int %141
        %143 =   OpLoad %v4int %exp
        %144 =   OpVectorShuffle %v4int %143 %142 5 1 4 6
                 OpStore %exp %144
        %145 =   OpVectorShuffle %v3float %138 %138 1 0 2
                 OpStore %146 %145
        %147 =   OpVectorShuffle %v3int %144 %144 1 0 2
                 OpStore %148 %147
        %149 =   OpFunctionCall %bool %checkIntrinsicAsFunctionArg_bf3i3 %146 %148
                 OpStore %funcOk %149
        %151 =   OpLoad %v4bool %ok                 ; RelaxedPrecision
        %150 =   OpAll %bool %151
                 OpSelectionMerge %153 None
                 OpBranchConditional %150 %152 %153

        %152 =     OpLabel
                     OpBranch %153

        %153 = OpLabel
        %154 =   OpPhi %bool %false %132 %149 %152
                 OpSelectionMerge %158 None
                 OpBranchConditional %154 %156 %157

        %156 =     OpLabel
        %159 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %160 =       OpLoad %v4float %159           ; RelaxedPrecision
                     OpStore %155 %160
                     OpBranch %158

        %157 =     OpLabel
        %161 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %162 =       OpLoad %v4float %161           ; RelaxedPrecision
                     OpStore %155 %162
                     OpBranch %158

        %158 = OpLabel
        %163 =   OpLoad %v4float %155               ; RelaxedPrecision
                 OpReturnValue %163
               OpFunctionEnd

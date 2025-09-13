               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "inputVal"
               OpMemberName %_UniformBuffer 1 "expected"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6

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
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
         %95 = OpConstantComposite %v3float %float_0 %float_0 %float_0
        %104 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1
        %128 = OpConstantComposite %v2float %float_1 %float_1
        %141 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %153 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3


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
        %189 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %33 =   OpLoad %v4float %30                ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 0    ; RelaxedPrecision
         %29 =   OpExtInst %float %5 Atan %34       ; RelaxedPrecision
         %35 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %37 =   OpLoad %v4float %35                ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 0    ; RelaxedPrecision
         %39 =   OpFOrdEqual %bool %29 %38
                 OpSelectionMerge %41 None
                 OpBranchConditional %39 %40 %41

         %40 =     OpLabel
         %43 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %44 =       OpLoad %v4float %43            ; RelaxedPrecision
         %45 =       OpVectorShuffle %v2float %44 %44 0 1   ; RelaxedPrecision
         %42 =       OpExtInst %v2float %5 Atan %45         ; RelaxedPrecision
         %46 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %47 =       OpLoad %v4float %46            ; RelaxedPrecision
         %48 =       OpVectorShuffle %v2float %47 %47 0 1   ; RelaxedPrecision
         %49 =       OpFOrdEqual %v2bool %42 %48
         %51 =       OpAll %bool %49
                     OpBranch %41

         %41 = OpLabel
         %52 =   OpPhi %bool %false %26 %51 %40
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
         %56 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %57 =       OpLoad %v4float %56            ; RelaxedPrecision
         %58 =       OpVectorShuffle %v3float %57 %57 0 1 2     ; RelaxedPrecision
         %55 =       OpExtInst %v3float %5 Atan %58             ; RelaxedPrecision
         %60 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %61 =       OpLoad %v4float %60            ; RelaxedPrecision
         %62 =       OpVectorShuffle %v3float %61 %61 0 1 2     ; RelaxedPrecision
         %63 =       OpFOrdEqual %v3bool %55 %62
         %65 =       OpAll %bool %63
                     OpBranch %54

         %54 = OpLabel
         %66 =   OpPhi %bool %false %41 %65 %53
                 OpSelectionMerge %68 None
                 OpBranchConditional %66 %67 %68

         %67 =     OpLabel
         %70 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %71 =       OpLoad %v4float %70            ; RelaxedPrecision
         %69 =       OpExtInst %v4float %5 Atan %71     ; RelaxedPrecision
         %72 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %73 =       OpLoad %v4float %72            ; RelaxedPrecision
         %74 =       OpFOrdEqual %v4bool %69 %73
         %76 =       OpAll %bool %74
                     OpBranch %68

         %68 = OpLabel
         %77 =   OpPhi %bool %false %54 %76 %67
                 OpSelectionMerge %79 None
                 OpBranchConditional %77 %78 %79

         %78 =     OpLabel
         %80 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %81 =       OpLoad %v4float %80            ; RelaxedPrecision
         %82 =       OpCompositeExtract %float %81 0    ; RelaxedPrecision
         %83 =       OpFOrdEqual %bool %float_0 %82
                     OpBranch %79

         %79 = OpLabel
         %84 =   OpPhi %bool %false %68 %83 %78
                 OpSelectionMerge %86 None
                 OpBranchConditional %84 %85 %86

         %85 =     OpLabel
         %87 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %88 =       OpLoad %v4float %87            ; RelaxedPrecision
         %89 =       OpVectorShuffle %v2float %88 %88 0 1   ; RelaxedPrecision
         %90 =       OpFOrdEqual %v2bool %20 %89
         %91 =       OpAll %bool %90
                     OpBranch %86

         %86 = OpLabel
         %92 =   OpPhi %bool %false %79 %91 %85
                 OpSelectionMerge %94 None
                 OpBranchConditional %92 %93 %94

         %93 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %97 =       OpLoad %v4float %96            ; RelaxedPrecision
         %98 =       OpVectorShuffle %v3float %97 %97 0 1 2     ; RelaxedPrecision
         %99 =       OpFOrdEqual %v3bool %95 %98
        %100 =       OpAll %bool %99
                     OpBranch %94

         %94 = OpLabel
        %101 =   OpPhi %bool %false %86 %100 %93
                 OpSelectionMerge %103 None
                 OpBranchConditional %101 %102 %103

        %102 =     OpLabel
        %105 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %106 =       OpLoad %v4float %105           ; RelaxedPrecision
        %107 =       OpFOrdEqual %v4bool %104 %106
        %108 =       OpAll %bool %107
                     OpBranch %103

        %103 = OpLabel
        %109 =   OpPhi %bool %false %94 %108 %102
                 OpSelectionMerge %111 None
                 OpBranchConditional %109 %110 %111

        %110 =     OpLabel
        %113 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %114 =       OpLoad %v4float %113           ; RelaxedPrecision
        %115 =       OpCompositeExtract %float %114 0   ; RelaxedPrecision
        %112 =       OpExtInst %float %5 Atan2 %115 %float_1    ; RelaxedPrecision
        %117 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %118 =       OpLoad %v4float %117           ; RelaxedPrecision
        %119 =       OpCompositeExtract %float %118 0   ; RelaxedPrecision
        %120 =       OpFOrdEqual %bool %112 %119
                     OpBranch %111

        %111 = OpLabel
        %121 =   OpPhi %bool %false %103 %120 %110
                 OpSelectionMerge %123 None
                 OpBranchConditional %121 %122 %123

        %122 =     OpLabel
        %125 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %126 =       OpLoad %v4float %125           ; RelaxedPrecision
        %127 =       OpVectorShuffle %v2float %126 %126 0 1     ; RelaxedPrecision
        %124 =       OpExtInst %v2float %5 Atan2 %127 %128      ; RelaxedPrecision
        %129 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %130 =       OpLoad %v4float %129           ; RelaxedPrecision
        %131 =       OpVectorShuffle %v2float %130 %130 0 1     ; RelaxedPrecision
        %132 =       OpFOrdEqual %v2bool %124 %131
        %133 =       OpAll %bool %132
                     OpBranch %123

        %123 = OpLabel
        %134 =   OpPhi %bool %false %111 %133 %122
                 OpSelectionMerge %136 None
                 OpBranchConditional %134 %135 %136

        %135 =     OpLabel
        %138 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %139 =       OpLoad %v4float %138           ; RelaxedPrecision
        %140 =       OpVectorShuffle %v3float %139 %139 0 1 2   ; RelaxedPrecision
        %137 =       OpExtInst %v3float %5 Atan2 %140 %141      ; RelaxedPrecision
        %142 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %143 =       OpLoad %v4float %142           ; RelaxedPrecision
        %144 =       OpVectorShuffle %v3float %143 %143 0 1 2   ; RelaxedPrecision
        %145 =       OpFOrdEqual %v3bool %137 %144
        %146 =       OpAll %bool %145
                     OpBranch %136

        %136 = OpLabel
        %147 =   OpPhi %bool %false %123 %146 %135
                 OpSelectionMerge %149 None
                 OpBranchConditional %147 %148 %149

        %148 =     OpLabel
        %151 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %152 =       OpLoad %v4float %151           ; RelaxedPrecision
        %150 =       OpExtInst %v4float %5 Atan2 %152 %153  ; RelaxedPrecision
        %154 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %155 =       OpLoad %v4float %154           ; RelaxedPrecision
        %156 =       OpFOrdEqual %v4bool %150 %155
        %157 =       OpAll %bool %156
                     OpBranch %149

        %149 = OpLabel
        %158 =   OpPhi %bool %false %136 %157 %148
                 OpSelectionMerge %160 None
                 OpBranchConditional %158 %159 %160

        %159 =     OpLabel
        %161 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %162 =       OpLoad %v4float %161           ; RelaxedPrecision
        %163 =       OpCompositeExtract %float %162 0   ; RelaxedPrecision
        %164 =       OpFOrdEqual %bool %float_0 %163
                     OpBranch %160

        %160 = OpLabel
        %165 =   OpPhi %bool %false %149 %164 %159
                 OpSelectionMerge %167 None
                 OpBranchConditional %165 %166 %167

        %166 =     OpLabel
        %168 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %169 =       OpLoad %v4float %168           ; RelaxedPrecision
        %170 =       OpVectorShuffle %v2float %169 %169 0 1     ; RelaxedPrecision
        %171 =       OpFOrdEqual %v2bool %20 %170
        %172 =       OpAll %bool %171
                     OpBranch %167

        %167 = OpLabel
        %173 =   OpPhi %bool %false %160 %172 %166
                 OpSelectionMerge %175 None
                 OpBranchConditional %173 %174 %175

        %174 =     OpLabel
        %176 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %177 =       OpLoad %v4float %176           ; RelaxedPrecision
        %178 =       OpVectorShuffle %v3float %177 %177 0 1 2   ; RelaxedPrecision
        %179 =       OpFOrdEqual %v3bool %95 %178
        %180 =       OpAll %bool %179
                     OpBranch %175

        %175 = OpLabel
        %181 =   OpPhi %bool %false %167 %180 %174
                 OpSelectionMerge %183 None
                 OpBranchConditional %181 %182 %183

        %182 =     OpLabel
        %184 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %185 =       OpLoad %v4float %184           ; RelaxedPrecision
        %186 =       OpFOrdEqual %v4bool %104 %185
        %187 =       OpAll %bool %186
                     OpBranch %183

        %183 = OpLabel
        %188 =   OpPhi %bool %false %175 %187 %182
                 OpSelectionMerge %193 None
                 OpBranchConditional %188 %191 %192

        %191 =     OpLabel
        %194 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %196 =       OpLoad %v4float %194           ; RelaxedPrecision
                     OpStore %189 %196
                     OpBranch %193

        %192 =     OpLabel
        %197 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %199 =       OpLoad %v4float %197           ; RelaxedPrecision
                     OpStore %189 %199
                     OpBranch %193

        %193 = OpLabel
        %200 =   OpLoad %v4float %189               ; RelaxedPrecision
                 OpReturnValue %200
               OpFunctionEnd

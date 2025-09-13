               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %19
               OpName %_UniformBuffer "_UniformBuffer"  ; id %24
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %26
               OpName %inside_while_loop_b "inside_while_loop_b"    ; id %6
               OpName %inside_infinite_do_loop_b "inside_infinite_do_loop_b"    ; id %7
               OpName %inside_infinite_while_loop_b "inside_infinite_while_loop_b"  ; id %8
               OpName %after_do_loop_b "after_do_loop_b"                            ; id %9
               OpName %after_while_loop_b "after_while_loop_b"                      ; id %10
               OpName %switch_with_all_returns_b "switch_with_all_returns_b"        ; id %11
               OpName %switch_fallthrough_b "switch_fallthrough_b"                  ; id %12
               OpName %switch_fallthrough_twice_b "switch_fallthrough_twice_b"      ; id %13
               OpName %switch_with_break_in_loop_b "switch_with_break_in_loop_b"    ; id %14
               OpName %x "x"                                                        ; id %107
               OpName %switch_with_continue_in_loop_b "switch_with_continue_in_loop_b"  ; id %15
               OpName %x_0 "x"                                                          ; id %128
               OpName %switch_with_if_that_returns_b "switch_with_if_that_returns_b"    ; id %16
               OpName %switch_with_one_sided_if_then_fallthrough_b "switch_with_one_sided_if_then_fallthrough_b"    ; id %17
               OpName %main "main"                                                                                  ; id %18

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
               OpDecorate %_UniformBuffer Block
               OpDecorate %23 Binding 0
               OpDecorate %23 DescriptorSet 0
               OpDecorate %47 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %float     ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %23 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %28 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %32 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %37 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_2 = OpConstant %int 2
  %float_123 = OpConstant %float 123
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
      %int_1 = OpConstant %int 1
        %163 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %28

         %29 = OpLabel
         %33 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %33 %32
         %35 =   OpFunctionCall %v4float %main %33
                 OpStore %sk_FragColor %35
                 OpReturn
               OpFunctionEnd


               ; Function inside_while_loop_b
%inside_while_loop_b = OpFunction %bool None %37

         %38 = OpLabel
                 OpBranch %39

         %39 = OpLabel
                 OpLoopMerge %43 %42 None
                 OpBranch %40

         %40 =     OpLabel
         %44 =       OpAccessChain %_ptr_Uniform_float %23 %int_2
         %47 =       OpLoad %float %44              ; RelaxedPrecision
         %49 =       OpFOrdEqual %bool %47 %float_123
                     OpBranchConditional %49 %41 %43

         %41 =         OpLabel
                         OpReturnValue %false

         %42 =   OpLabel
                   OpBranch %39

         %43 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function inside_infinite_do_loop_b
%inside_infinite_do_loop_b = OpFunction %bool None %37

         %52 = OpLabel
                 OpBranch %53

         %53 = OpLabel
                 OpLoopMerge %57 %56 None
                 OpBranch %54

         %54 =     OpLabel
                     OpReturnValue %true

         %56 =   OpLabel
                   OpBranchConditional %true %53 %57

         %57 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function inside_infinite_while_loop_b
%inside_infinite_while_loop_b = OpFunction %bool None %37

         %58 = OpLabel
                 OpBranch %59

         %59 = OpLabel
                 OpLoopMerge %63 %62 None
                 OpBranch %60

         %60 =     OpLabel
                     OpBranchConditional %true %61 %63

         %61 =         OpLabel
                         OpReturnValue %true

         %62 =   OpLabel
                   OpBranch %59

         %63 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function after_do_loop_b
%after_do_loop_b = OpFunction %bool None %37

         %64 = OpLabel
                 OpBranch %65

         %65 = OpLabel
                 OpLoopMerge %69 %68 None
                 OpBranch %66

         %66 =     OpLabel
                     OpBranch %69

         %68 =   OpLabel
                   OpBranchConditional %true %65 %69

         %69 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function after_while_loop_b
%after_while_loop_b = OpFunction %bool None %37

         %70 = OpLabel
                 OpBranch %71

         %71 = OpLabel
                 OpLoopMerge %75 %74 None
                 OpBranch %72

         %72 =     OpLabel
                     OpBranchConditional %true %73 %75

         %73 =         OpLabel
                         OpBranch %75

         %74 =   OpLabel
                   OpBranch %71

         %75 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function switch_with_all_returns_b
%switch_with_all_returns_b = OpFunction %bool None %37

         %76 = OpLabel
         %77 =   OpAccessChain %_ptr_Uniform_float %23 %int_2
         %78 =   OpLoad %float %77                  ; RelaxedPrecision
         %79 =   OpConvertFToS %int %78
                 OpSelectionMerge %80 None
                 OpSwitch %79 %83 1 %81 2 %82

         %81 =     OpLabel
                     OpReturnValue %true

         %82 =     OpLabel
                     OpReturnValue %false

         %83 =     OpLabel
                     OpReturnValue %false

         %80 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function switch_fallthrough_b
%switch_fallthrough_b = OpFunction %bool None %37

         %84 = OpLabel
         %85 =   OpAccessChain %_ptr_Uniform_float %23 %int_2
         %86 =   OpLoad %float %85                  ; RelaxedPrecision
         %87 =   OpConvertFToS %int %86
                 OpSelectionMerge %88 None
                 OpSwitch %87 %91 1 %89 2 %91

         %89 =     OpLabel
                     OpReturnValue %true

         %91 =     OpLabel
                     OpReturnValue %false

         %88 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function switch_fallthrough_twice_b
%switch_fallthrough_twice_b = OpFunction %bool None %37

         %92 = OpLabel
         %93 =   OpAccessChain %_ptr_Uniform_float %23 %int_2
         %94 =   OpLoad %float %93                  ; RelaxedPrecision
         %95 =   OpConvertFToS %int %94
                 OpSelectionMerge %96 None
                 OpSwitch %95 %99 1 %99 2 %99

         %99 =     OpLabel
                     OpReturnValue %true

         %96 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function switch_with_break_in_loop_b
%switch_with_break_in_loop_b = OpFunction %bool None %37

        %100 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
        %101 =   OpAccessChain %_ptr_Uniform_float %23 %int_2
        %102 =   OpLoad %float %101                 ; RelaxedPrecision
        %103 =   OpConvertFToS %int %102
                 OpSelectionMerge %104 None
                 OpSwitch %103 %106 1 %105

        %105 =     OpLabel
                     OpStore %x %int_0
                     OpBranch %110

        %110 =     OpLabel
                     OpLoopMerge %114 %113 None
                     OpBranch %111

        %111 =         OpLabel
        %115 =           OpLoad %int %x
        %117 =           OpSLessThanEqual %bool %115 %int_10
                         OpBranchConditional %117 %112 %114

        %112 =             OpLabel
                             OpBranch %114

        %113 =       OpLabel
        %119 =         OpLoad %int %x
        %120 =         OpIAdd %int %119 %int_1
                       OpStore %x %120
                       OpBranch %110

        %114 =     OpLabel
                     OpBranch %106

        %106 =     OpLabel
                     OpReturnValue %true

        %104 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function switch_with_continue_in_loop_b
%switch_with_continue_in_loop_b = OpFunction %bool None %37

        %121 = OpLabel
        %x_0 =   OpVariable %_ptr_Function_int Function
        %122 =   OpAccessChain %_ptr_Uniform_float %23 %int_2
        %123 =   OpLoad %float %122                 ; RelaxedPrecision
        %124 =   OpConvertFToS %int %123
                 OpSelectionMerge %125 None
                 OpSwitch %124 %127 1 %126

        %126 =     OpLabel
                     OpStore %x_0 %int_0
                     OpBranch %129

        %129 =     OpLabel
                     OpLoopMerge %133 %132 None
                     OpBranch %130

        %130 =         OpLabel
        %134 =           OpLoad %int %x_0
        %135 =           OpSLessThanEqual %bool %134 %int_10
                         OpBranchConditional %135 %131 %133

        %131 =             OpLabel
                             OpBranch %132

        %132 =       OpLabel
        %136 =         OpLoad %int %x_0
        %137 =         OpIAdd %int %136 %int_1
                       OpStore %x_0 %137
                       OpBranch %129

        %133 =     OpLabel
                     OpBranch %127

        %127 =     OpLabel
                     OpReturnValue %true

        %125 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function switch_with_if_that_returns_b
%switch_with_if_that_returns_b = OpFunction %bool None %37

        %138 = OpLabel
        %139 =   OpAccessChain %_ptr_Uniform_float %23 %int_2
        %140 =   OpLoad %float %139                 ; RelaxedPrecision
        %141 =   OpConvertFToS %int %140
                 OpSelectionMerge %142 None
                 OpSwitch %141 %144 1 %143

        %143 =     OpLabel
        %145 =       OpAccessChain %_ptr_Uniform_float %23 %int_2
        %146 =       OpLoad %float %145             ; RelaxedPrecision
        %147 =       OpFOrdEqual %bool %146 %float_123
                     OpSelectionMerge %150 None
                     OpBranchConditional %147 %148 %149

        %148 =         OpLabel
                         OpReturnValue %false

        %149 =         OpLabel
                         OpReturnValue %true

        %150 =     OpLabel
                     OpBranch %144

        %144 =     OpLabel
                     OpReturnValue %true

        %142 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function switch_with_one_sided_if_then_fallthrough_b
%switch_with_one_sided_if_then_fallthrough_b = OpFunction %bool None %37

        %151 = OpLabel
        %152 =   OpAccessChain %_ptr_Uniform_float %23 %int_2
        %153 =   OpLoad %float %152                 ; RelaxedPrecision
        %154 =   OpConvertFToS %int %153
                 OpSelectionMerge %155 None
                 OpSwitch %154 %157 1 %156

        %156 =     OpLabel
        %158 =       OpAccessChain %_ptr_Uniform_float %23 %int_2
        %159 =       OpLoad %float %158             ; RelaxedPrecision
        %160 =       OpFOrdEqual %bool %159 %float_123
                     OpSelectionMerge %162 None
                     OpBranchConditional %160 %161 %162

        %161 =         OpLabel
                         OpReturnValue %false

        %162 =     OpLabel
                     OpBranch %157

        %157 =     OpLabel
                     OpReturnValue %true

        %155 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %163        ; RelaxedPrecision
        %164 = OpFunctionParameter %_ptr_Function_v2float

        %165 = OpLabel
        %211 =   OpVariable %_ptr_Function_v4float Function
        %166 =   OpFunctionCall %bool %inside_while_loop_b
                 OpSelectionMerge %168 None
                 OpBranchConditional %166 %167 %168

        %167 =     OpLabel
        %169 =       OpFunctionCall %bool %inside_infinite_do_loop_b
                     OpBranch %168

        %168 = OpLabel
        %170 =   OpPhi %bool %false %165 %169 %167
                 OpSelectionMerge %172 None
                 OpBranchConditional %170 %171 %172

        %171 =     OpLabel
        %173 =       OpFunctionCall %bool %inside_infinite_while_loop_b
                     OpBranch %172

        %172 = OpLabel
        %174 =   OpPhi %bool %false %168 %173 %171
                 OpSelectionMerge %176 None
                 OpBranchConditional %174 %175 %176

        %175 =     OpLabel
        %177 =       OpFunctionCall %bool %after_do_loop_b
                     OpBranch %176

        %176 = OpLabel
        %178 =   OpPhi %bool %false %172 %177 %175
                 OpSelectionMerge %180 None
                 OpBranchConditional %178 %179 %180

        %179 =     OpLabel
        %181 =       OpFunctionCall %bool %after_while_loop_b
                     OpBranch %180

        %180 = OpLabel
        %182 =   OpPhi %bool %false %176 %181 %179
                 OpSelectionMerge %184 None
                 OpBranchConditional %182 %183 %184

        %183 =     OpLabel
        %185 =       OpFunctionCall %bool %switch_with_all_returns_b
                     OpBranch %184

        %184 = OpLabel
        %186 =   OpPhi %bool %false %180 %185 %183
                 OpSelectionMerge %188 None
                 OpBranchConditional %186 %187 %188

        %187 =     OpLabel
        %189 =       OpFunctionCall %bool %switch_fallthrough_b
                     OpBranch %188

        %188 = OpLabel
        %190 =   OpPhi %bool %false %184 %189 %187
                 OpSelectionMerge %192 None
                 OpBranchConditional %190 %191 %192

        %191 =     OpLabel
        %193 =       OpFunctionCall %bool %switch_fallthrough_twice_b
                     OpBranch %192

        %192 = OpLabel
        %194 =   OpPhi %bool %false %188 %193 %191
                 OpSelectionMerge %196 None
                 OpBranchConditional %194 %195 %196

        %195 =     OpLabel
        %197 =       OpFunctionCall %bool %switch_with_break_in_loop_b
                     OpBranch %196

        %196 = OpLabel
        %198 =   OpPhi %bool %false %192 %197 %195
                 OpSelectionMerge %200 None
                 OpBranchConditional %198 %199 %200

        %199 =     OpLabel
        %201 =       OpFunctionCall %bool %switch_with_continue_in_loop_b
                     OpBranch %200

        %200 = OpLabel
        %202 =   OpPhi %bool %false %196 %201 %199
                 OpSelectionMerge %204 None
                 OpBranchConditional %202 %203 %204

        %203 =     OpLabel
        %205 =       OpFunctionCall %bool %switch_with_if_that_returns_b
                     OpBranch %204

        %204 = OpLabel
        %206 =   OpPhi %bool %false %200 %205 %203
                 OpSelectionMerge %208 None
                 OpBranchConditional %206 %207 %208

        %207 =     OpLabel
        %209 =       OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough_b
                     OpBranch %208

        %208 = OpLabel
        %210 =   OpPhi %bool %false %204 %209 %207
                 OpSelectionMerge %215 None
                 OpBranchConditional %210 %213 %214

        %213 =     OpLabel
        %216 =       OpAccessChain %_ptr_Uniform_v4float %23 %int_0
        %218 =       OpLoad %v4float %216           ; RelaxedPrecision
                     OpStore %211 %218
                     OpBranch %215

        %214 =     OpLabel
        %219 =       OpAccessChain %_ptr_Uniform_v4float %23 %int_1
        %220 =       OpLoad %v4float %219           ; RelaxedPrecision
                     OpStore %211 %220
                     OpBranch %215

        %215 = OpLabel
        %221 =   OpLoad %v4float %211               ; RelaxedPrecision
                 OpReturnValue %221
               OpFunctionEnd

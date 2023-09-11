               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %inside_while_loop_b "inside_while_loop_b"
               OpName %inside_infinite_do_loop_b "inside_infinite_do_loop_b"
               OpName %inside_infinite_while_loop_b "inside_infinite_while_loop_b"
               OpName %after_do_loop_b "after_do_loop_b"
               OpName %after_while_loop_b "after_while_loop_b"
               OpName %switch_with_all_returns_b "switch_with_all_returns_b"
               OpName %switch_fallthrough_b "switch_fallthrough_b"
               OpName %switch_fallthrough_twice_b "switch_fallthrough_twice_b"
               OpName %switch_with_break_in_loop_b "switch_with_break_in_loop_b"
               OpName %x "x"
               OpName %switch_with_continue_in_loop_b "switch_with_continue_in_loop_b"
               OpName %x_0 "x"
               OpName %switch_with_if_that_returns_b "switch_with_if_that_returns_b"
               OpName %switch_with_one_sided_if_then_fallthrough_b "switch_with_one_sided_if_then_fallthrough_b"
               OpName %main "main"
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
               OpDecorate %19 Binding 0
               OpDecorate %19 DescriptorSet 0
               OpDecorate %44 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %24 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %33 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
  %float_123 = OpConstant %float 123
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
      %int_1 = OpConstant %int 1
        %160 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %24
         %25 = OpLabel
         %29 = OpVariable %_ptr_Function_v2float Function
               OpStore %29 %28
         %31 = OpFunctionCall %v4float %main %29
               OpStore %sk_FragColor %31
               OpReturn
               OpFunctionEnd
%inside_while_loop_b = OpFunction %bool None %33
         %34 = OpLabel
               OpBranch %35
         %35 = OpLabel
               OpLoopMerge %39 %38 None
               OpBranch %36
         %36 = OpLabel
         %40 = OpAccessChain %_ptr_Uniform_float %19 %int_2
         %44 = OpLoad %float %40
         %46 = OpFOrdEqual %bool %44 %float_123
               OpBranchConditional %46 %37 %39
         %37 = OpLabel
               OpReturnValue %false
         %38 = OpLabel
               OpBranch %35
         %39 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%inside_infinite_do_loop_b = OpFunction %bool None %33
         %49 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpLoopMerge %54 %53 None
               OpBranch %51
         %51 = OpLabel
               OpReturnValue %true
         %53 = OpLabel
               OpBranchConditional %true %50 %54
         %54 = OpLabel
               OpUnreachable
               OpFunctionEnd
%inside_infinite_while_loop_b = OpFunction %bool None %33
         %55 = OpLabel
               OpBranch %56
         %56 = OpLabel
               OpLoopMerge %60 %59 None
               OpBranch %57
         %57 = OpLabel
               OpBranchConditional %true %58 %60
         %58 = OpLabel
               OpReturnValue %true
         %59 = OpLabel
               OpBranch %56
         %60 = OpLabel
               OpUnreachable
               OpFunctionEnd
%after_do_loop_b = OpFunction %bool None %33
         %61 = OpLabel
               OpBranch %62
         %62 = OpLabel
               OpLoopMerge %66 %65 None
               OpBranch %63
         %63 = OpLabel
               OpBranch %66
         %65 = OpLabel
               OpBranchConditional %true %62 %66
         %66 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%after_while_loop_b = OpFunction %bool None %33
         %67 = OpLabel
               OpBranch %68
         %68 = OpLabel
               OpLoopMerge %72 %71 None
               OpBranch %69
         %69 = OpLabel
               OpBranchConditional %true %70 %72
         %70 = OpLabel
               OpBranch %72
         %71 = OpLabel
               OpBranch %68
         %72 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%switch_with_all_returns_b = OpFunction %bool None %33
         %73 = OpLabel
         %74 = OpAccessChain %_ptr_Uniform_float %19 %int_2
         %75 = OpLoad %float %74
         %76 = OpConvertFToS %int %75
               OpSelectionMerge %77 None
               OpSwitch %76 %80 1 %78 2 %79
         %78 = OpLabel
               OpReturnValue %true
         %79 = OpLabel
               OpReturnValue %false
         %80 = OpLabel
               OpReturnValue %false
         %77 = OpLabel
               OpUnreachable
               OpFunctionEnd
%switch_fallthrough_b = OpFunction %bool None %33
         %81 = OpLabel
         %82 = OpAccessChain %_ptr_Uniform_float %19 %int_2
         %83 = OpLoad %float %82
         %84 = OpConvertFToS %int %83
               OpSelectionMerge %85 None
               OpSwitch %84 %88 1 %86 2 %88
         %86 = OpLabel
               OpReturnValue %true
         %88 = OpLabel
               OpReturnValue %false
         %85 = OpLabel
               OpUnreachable
               OpFunctionEnd
%switch_fallthrough_twice_b = OpFunction %bool None %33
         %89 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_float %19 %int_2
         %91 = OpLoad %float %90
         %92 = OpConvertFToS %int %91
               OpSelectionMerge %93 None
               OpSwitch %92 %96 1 %96 2 %96
         %96 = OpLabel
               OpReturnValue %true
         %93 = OpLabel
               OpUnreachable
               OpFunctionEnd
%switch_with_break_in_loop_b = OpFunction %bool None %33
         %97 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
         %98 = OpAccessChain %_ptr_Uniform_float %19 %int_2
         %99 = OpLoad %float %98
        %100 = OpConvertFToS %int %99
               OpSelectionMerge %101 None
               OpSwitch %100 %103 1 %102
        %102 = OpLabel
               OpStore %x %int_0
               OpBranch %107
        %107 = OpLabel
               OpLoopMerge %111 %110 None
               OpBranch %108
        %108 = OpLabel
        %112 = OpLoad %int %x
        %114 = OpSLessThanEqual %bool %112 %int_10
               OpBranchConditional %114 %109 %111
        %109 = OpLabel
               OpBranch %111
        %110 = OpLabel
        %116 = OpLoad %int %x
        %117 = OpIAdd %int %116 %int_1
               OpStore %x %117
               OpBranch %107
        %111 = OpLabel
               OpBranch %103
        %103 = OpLabel
               OpReturnValue %true
        %101 = OpLabel
               OpUnreachable
               OpFunctionEnd
%switch_with_continue_in_loop_b = OpFunction %bool None %33
        %118 = OpLabel
        %x_0 = OpVariable %_ptr_Function_int Function
        %119 = OpAccessChain %_ptr_Uniform_float %19 %int_2
        %120 = OpLoad %float %119
        %121 = OpConvertFToS %int %120
               OpSelectionMerge %122 None
               OpSwitch %121 %124 1 %123
        %123 = OpLabel
               OpStore %x_0 %int_0
               OpBranch %126
        %126 = OpLabel
               OpLoopMerge %130 %129 None
               OpBranch %127
        %127 = OpLabel
        %131 = OpLoad %int %x_0
        %132 = OpSLessThanEqual %bool %131 %int_10
               OpBranchConditional %132 %128 %130
        %128 = OpLabel
               OpBranch %129
        %129 = OpLabel
        %133 = OpLoad %int %x_0
        %134 = OpIAdd %int %133 %int_1
               OpStore %x_0 %134
               OpBranch %126
        %130 = OpLabel
               OpBranch %124
        %124 = OpLabel
               OpReturnValue %true
        %122 = OpLabel
               OpUnreachable
               OpFunctionEnd
%switch_with_if_that_returns_b = OpFunction %bool None %33
        %135 = OpLabel
        %136 = OpAccessChain %_ptr_Uniform_float %19 %int_2
        %137 = OpLoad %float %136
        %138 = OpConvertFToS %int %137
               OpSelectionMerge %139 None
               OpSwitch %138 %141 1 %140
        %140 = OpLabel
        %142 = OpAccessChain %_ptr_Uniform_float %19 %int_2
        %143 = OpLoad %float %142
        %144 = OpFOrdEqual %bool %143 %float_123
               OpSelectionMerge %147 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
               OpReturnValue %false
        %146 = OpLabel
               OpReturnValue %true
        %147 = OpLabel
               OpBranch %141
        %141 = OpLabel
               OpReturnValue %true
        %139 = OpLabel
               OpUnreachable
               OpFunctionEnd
%switch_with_one_sided_if_then_fallthrough_b = OpFunction %bool None %33
        %148 = OpLabel
        %149 = OpAccessChain %_ptr_Uniform_float %19 %int_2
        %150 = OpLoad %float %149
        %151 = OpConvertFToS %int %150
               OpSelectionMerge %152 None
               OpSwitch %151 %154 1 %153
        %153 = OpLabel
        %155 = OpAccessChain %_ptr_Uniform_float %19 %int_2
        %156 = OpLoad %float %155
        %157 = OpFOrdEqual %bool %156 %float_123
               OpSelectionMerge %159 None
               OpBranchConditional %157 %158 %159
        %158 = OpLabel
               OpReturnValue %false
        %159 = OpLabel
               OpBranch %154
        %154 = OpLabel
               OpReturnValue %true
        %152 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %v4float None %160
        %161 = OpFunctionParameter %_ptr_Function_v2float
        %162 = OpLabel
        %208 = OpVariable %_ptr_Function_v4float Function
        %163 = OpFunctionCall %bool %inside_while_loop_b
               OpSelectionMerge %165 None
               OpBranchConditional %163 %164 %165
        %164 = OpLabel
        %166 = OpFunctionCall %bool %inside_infinite_do_loop_b
               OpBranch %165
        %165 = OpLabel
        %167 = OpPhi %bool %false %162 %166 %164
               OpSelectionMerge %169 None
               OpBranchConditional %167 %168 %169
        %168 = OpLabel
        %170 = OpFunctionCall %bool %inside_infinite_while_loop_b
               OpBranch %169
        %169 = OpLabel
        %171 = OpPhi %bool %false %165 %170 %168
               OpSelectionMerge %173 None
               OpBranchConditional %171 %172 %173
        %172 = OpLabel
        %174 = OpFunctionCall %bool %after_do_loop_b
               OpBranch %173
        %173 = OpLabel
        %175 = OpPhi %bool %false %169 %174 %172
               OpSelectionMerge %177 None
               OpBranchConditional %175 %176 %177
        %176 = OpLabel
        %178 = OpFunctionCall %bool %after_while_loop_b
               OpBranch %177
        %177 = OpLabel
        %179 = OpPhi %bool %false %173 %178 %176
               OpSelectionMerge %181 None
               OpBranchConditional %179 %180 %181
        %180 = OpLabel
        %182 = OpFunctionCall %bool %switch_with_all_returns_b
               OpBranch %181
        %181 = OpLabel
        %183 = OpPhi %bool %false %177 %182 %180
               OpSelectionMerge %185 None
               OpBranchConditional %183 %184 %185
        %184 = OpLabel
        %186 = OpFunctionCall %bool %switch_fallthrough_b
               OpBranch %185
        %185 = OpLabel
        %187 = OpPhi %bool %false %181 %186 %184
               OpSelectionMerge %189 None
               OpBranchConditional %187 %188 %189
        %188 = OpLabel
        %190 = OpFunctionCall %bool %switch_fallthrough_twice_b
               OpBranch %189
        %189 = OpLabel
        %191 = OpPhi %bool %false %185 %190 %188
               OpSelectionMerge %193 None
               OpBranchConditional %191 %192 %193
        %192 = OpLabel
        %194 = OpFunctionCall %bool %switch_with_break_in_loop_b
               OpBranch %193
        %193 = OpLabel
        %195 = OpPhi %bool %false %189 %194 %192
               OpSelectionMerge %197 None
               OpBranchConditional %195 %196 %197
        %196 = OpLabel
        %198 = OpFunctionCall %bool %switch_with_continue_in_loop_b
               OpBranch %197
        %197 = OpLabel
        %199 = OpPhi %bool %false %193 %198 %196
               OpSelectionMerge %201 None
               OpBranchConditional %199 %200 %201
        %200 = OpLabel
        %202 = OpFunctionCall %bool %switch_with_if_that_returns_b
               OpBranch %201
        %201 = OpLabel
        %203 = OpPhi %bool %false %197 %202 %200
               OpSelectionMerge %205 None
               OpBranchConditional %203 %204 %205
        %204 = OpLabel
        %206 = OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough_b
               OpBranch %205
        %205 = OpLabel
        %207 = OpPhi %bool %false %201 %206 %204
               OpSelectionMerge %212 None
               OpBranchConditional %207 %210 %211
        %210 = OpLabel
        %213 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
        %215 = OpLoad %v4float %213
               OpStore %208 %215
               OpBranch %212
        %211 = OpLabel
        %216 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
        %217 = OpLoad %v4float %216
               OpStore %208 %217
               OpBranch %212
        %212 = OpLabel
        %218 = OpLoad %v4float %208
               OpReturnValue %218
               OpFunctionEnd

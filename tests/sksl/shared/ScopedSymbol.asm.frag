               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %glob "glob"                  ; id %11
               OpName %_UniformBuffer "_UniformBuffer"  ; id %15
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %17
               OpName %block_variable_hides_global_variable_b "block_variable_hides_global_variable_b"  ; id %2
               OpName %local_variable_hides_struct_b "local_variable_hides_struct_b"                    ; id %3
               OpName %S "S"                                                                            ; id %34
               OpName %local_struct_variable_hides_struct_type_b "local_struct_variable_hides_struct_type_b"    ; id %4
               OpName %S_0 "S"                                                                                  ; id %39
               OpMemberName %S_0 0 "i"
               OpName %S_1 "S"                      ; id %38
               OpName %local_variable_hides_global_variable_b "local_variable_hides_global_variable_b"  ; id %5
               OpName %glob_0 "glob"                                                                    ; id %49
               OpName %main "main"                                                                      ; id %6

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
               OpDecorate %14 Binding 0
               OpDecorate %14 DescriptorSet 0
               OpMemberDecorate %S_0 0 Offset 0
               OpDecorate %77 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
        %int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
       %glob = OpVariable %_ptr_Private_int Private
%_UniformBuffer = OpTypeStruct %v4float %v4float    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %28 = OpTypeFunction %bool
      %int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
        %S_0 = OpTypeStruct %int
%_ptr_Function_S_0 = OpTypePointer Function %S_0
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
         %50 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %19

         %20 = OpLabel
         %24 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %24 %23
         %26 =   OpFunctionCall %v4float %main %24
                 OpStore %sk_FragColor %26
                 OpReturn
               OpFunctionEnd


               ; Function block_variable_hides_global_variable_b
%block_variable_hides_global_variable_b = OpFunction %bool None %28

         %29 = OpLabel
         %30 =   OpLoad %int %glob
         %32 =   OpIEqual %bool %30 %int_2
                 OpReturnValue %32
               OpFunctionEnd


               ; Function local_variable_hides_struct_b
%local_variable_hides_struct_b = OpFunction %bool None %28

         %33 = OpLabel
          %S =   OpVariable %_ptr_Function_bool Function
                 OpStore %S %true
                 OpReturnValue %true
               OpFunctionEnd


               ; Function local_struct_variable_hides_struct_type_b
%local_struct_variable_hides_struct_type_b = OpFunction %bool None %28

         %37 = OpLabel
        %S_1 =   OpVariable %_ptr_Function_S_0 Function
         %42 =   OpCompositeConstruct %S_0 %int_1
                 OpStore %S_1 %42
         %44 =   OpAccessChain %_ptr_Function_int %S_1 %int_0
         %46 =   OpLoad %int %44
         %47 =   OpIEqual %bool %46 %int_1
                 OpReturnValue %47
               OpFunctionEnd


               ; Function local_variable_hides_global_variable_b
%local_variable_hides_global_variable_b = OpFunction %bool None %28

         %48 = OpLabel
     %glob_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %glob_0 %int_1
                 OpReturnValue %true
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %50         ; RelaxedPrecision
         %51 = OpFunctionParameter %_ptr_Function_v2float

         %52 = OpLabel
         %70 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %glob %int_2
                 OpSelectionMerge %55 None
                 OpBranchConditional %true %54 %55

         %54 =     OpLabel
         %56 =       OpFunctionCall %bool %block_variable_hides_global_variable_b
                     OpBranch %55

         %55 = OpLabel
         %57 =   OpPhi %bool %false %52 %56 %54
                 OpSelectionMerge %59 None
                 OpBranchConditional %57 %58 %59

         %58 =     OpLabel
         %60 =       OpFunctionCall %bool %local_variable_hides_struct_b
                     OpBranch %59

         %59 = OpLabel
         %61 =   OpPhi %bool %false %55 %60 %58
                 OpSelectionMerge %63 None
                 OpBranchConditional %61 %62 %63

         %62 =     OpLabel
         %64 =       OpFunctionCall %bool %local_struct_variable_hides_struct_type_b
                     OpBranch %63

         %63 = OpLabel
         %65 =   OpPhi %bool %false %59 %64 %62
                 OpSelectionMerge %67 None
                 OpBranchConditional %65 %66 %67

         %66 =     OpLabel
         %68 =       OpFunctionCall %bool %local_variable_hides_global_variable_b
                     OpBranch %67

         %67 = OpLabel
         %69 =   OpPhi %bool %false %63 %68 %66
                 OpSelectionMerge %74 None
                 OpBranchConditional %69 %72 %73

         %72 =     OpLabel
         %75 =       OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %77 =       OpLoad %v4float %75            ; RelaxedPrecision
                     OpStore %70 %77
                     OpBranch %74

         %73 =     OpLabel
         %78 =       OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %79 =       OpLoad %v4float %78            ; RelaxedPrecision
                     OpStore %70 %79
                     OpBranch %74

         %74 = OpLabel
         %80 =   OpLoad %v4float %70                ; RelaxedPrecision
                 OpReturnValue %80
               OpFunctionEnd

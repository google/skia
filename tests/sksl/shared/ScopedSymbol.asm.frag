               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %11
               OpName %glob "glob"                  ; id %15
               OpName %_UniformBuffer "_UniformBuffer"  ; id %18
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %20
               OpName %block_variable_hides_global_variable_b "block_variable_hides_global_variable_b"  ; id %6
               OpName %local_variable_hides_struct_b "local_variable_hides_struct_b"                    ; id %7
               OpName %S "S"                                                                            ; id %37
               OpName %local_struct_variable_hides_struct_type_b "local_struct_variable_hides_struct_type_b"    ; id %8
               OpName %S_0 "S"                                                                                  ; id %42
               OpMemberName %S_0 0 "i"
               OpName %S_1 "S"                      ; id %41
               OpName %local_variable_hides_global_variable_b "local_variable_hides_global_variable_b"  ; id %9
               OpName %glob_0 "glob"                                                                    ; id %52
               OpName %main "main"                                                                      ; id %10

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
               OpDecorate %17 Binding 0
               OpDecorate %17 DescriptorSet 0
               OpMemberDecorate %S_0 0 Offset 0
               OpDecorate %80 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_ptr_Private_int = OpTypePointer Private %int
       %glob = OpVariable %_ptr_Private_int Private
%_UniformBuffer = OpTypeStruct %v4float %v4float    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %17 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %22 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %26 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %31 = OpTypeFunction %bool
      %int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
        %S_0 = OpTypeStruct %int
%_ptr_Function_S_0 = OpTypePointer Function %S_0
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
         %53 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %22

         %23 = OpLabel
         %27 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %27 %26
         %29 =   OpFunctionCall %v4float %main %27
                 OpStore %sk_FragColor %29
                 OpReturn
               OpFunctionEnd


               ; Function block_variable_hides_global_variable_b
%block_variable_hides_global_variable_b = OpFunction %bool None %31

         %32 = OpLabel
         %33 =   OpLoad %int %glob
         %35 =   OpIEqual %bool %33 %int_2
                 OpReturnValue %35
               OpFunctionEnd


               ; Function local_variable_hides_struct_b
%local_variable_hides_struct_b = OpFunction %bool None %31

         %36 = OpLabel
          %S =   OpVariable %_ptr_Function_bool Function
                 OpStore %S %true
                 OpReturnValue %true
               OpFunctionEnd


               ; Function local_struct_variable_hides_struct_type_b
%local_struct_variable_hides_struct_type_b = OpFunction %bool None %31

         %40 = OpLabel
        %S_1 =   OpVariable %_ptr_Function_S_0 Function
         %45 =   OpCompositeConstruct %S_0 %int_1
                 OpStore %S_1 %45
         %47 =   OpAccessChain %_ptr_Function_int %S_1 %int_0
         %49 =   OpLoad %int %47
         %50 =   OpIEqual %bool %49 %int_1
                 OpReturnValue %50
               OpFunctionEnd


               ; Function local_variable_hides_global_variable_b
%local_variable_hides_global_variable_b = OpFunction %bool None %31

         %51 = OpLabel
     %glob_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %glob_0 %int_1
                 OpReturnValue %true
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %53         ; RelaxedPrecision
         %54 = OpFunctionParameter %_ptr_Function_v2float

         %55 = OpLabel
         %73 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %glob %int_2
                 OpSelectionMerge %58 None
                 OpBranchConditional %true %57 %58

         %57 =     OpLabel
         %59 =       OpFunctionCall %bool %block_variable_hides_global_variable_b
                     OpBranch %58

         %58 = OpLabel
         %60 =   OpPhi %bool %false %55 %59 %57
                 OpSelectionMerge %62 None
                 OpBranchConditional %60 %61 %62

         %61 =     OpLabel
         %63 =       OpFunctionCall %bool %local_variable_hides_struct_b
                     OpBranch %62

         %62 = OpLabel
         %64 =   OpPhi %bool %false %58 %63 %61
                 OpSelectionMerge %66 None
                 OpBranchConditional %64 %65 %66

         %65 =     OpLabel
         %67 =       OpFunctionCall %bool %local_struct_variable_hides_struct_type_b
                     OpBranch %66

         %66 = OpLabel
         %68 =   OpPhi %bool %false %62 %67 %65
                 OpSelectionMerge %70 None
                 OpBranchConditional %68 %69 %70

         %69 =     OpLabel
         %71 =       OpFunctionCall %bool %local_variable_hides_global_variable_b
                     OpBranch %70

         %70 = OpLabel
         %72 =   OpPhi %bool %false %66 %71 %69
                 OpSelectionMerge %77 None
                 OpBranchConditional %72 %75 %76

         %75 =     OpLabel
         %78 =       OpAccessChain %_ptr_Uniform_v4float %17 %int_0
         %80 =       OpLoad %v4float %78            ; RelaxedPrecision
                     OpStore %73 %80
                     OpBranch %77

         %76 =     OpLabel
         %81 =       OpAccessChain %_ptr_Uniform_v4float %17 %int_1
         %82 =       OpLoad %v4float %81            ; RelaxedPrecision
                     OpStore %73 %82
                     OpBranch %77

         %77 = OpLabel
         %83 =   OpLoad %v4float %73                ; RelaxedPrecision
                 OpReturnValue %83
               OpFunctionEnd

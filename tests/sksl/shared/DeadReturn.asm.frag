               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %11
               OpName %scratchVar "scratchVar"      ; id %15
               OpName %_UniformBuffer "_UniformBuffer"  ; id %19
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %21
               OpName %test_flat_b "test_flat_b"        ; id %6
               OpName %test_if_b "test_if_b"            ; id %7
               OpName %test_else_b "test_else_b"        ; id %8
               OpName %test_loop_if_b "test_loop_if_b"  ; id %9
               OpName %x "x"                            ; id %59
               OpName %main "main"                      ; id %10

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
               OpDecorate %18 Binding 0
               OpDecorate %18 DescriptorSet 0
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_ptr_Private_int = OpTypePointer Private %int
 %scratchVar = OpVariable %_ptr_Private_int Private
      %int_0 = OpConstant %int 0
%_UniformBuffer = OpTypeStruct %v4float %v4float    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %27 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %32 = OpTypeFunction %bool
       %true = OpConstantTrue %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
%_ptr_Function_int = OpTypePointer Function %int
         %79 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %23

         %24 = OpLabel
         %28 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %28 %27
         %30 =   OpFunctionCall %v4float %main %28
                 OpStore %sk_FragColor %30
                 OpReturn
               OpFunctionEnd


               ; Function test_flat_b
%test_flat_b = OpFunction %bool None %32

         %33 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function test_if_b
  %test_if_b = OpFunction %bool None %32

         %35 = OpLabel
         %36 =   OpAccessChain %_ptr_Uniform_v4float %18 %int_0
         %38 =   OpLoad %v4float %36                ; RelaxedPrecision
         %39 =   OpCompositeExtract %float %38 1    ; RelaxedPrecision
         %40 =   OpFOrdGreaterThan %bool %39 %float_0
                 OpSelectionMerge %43 None
                 OpBranchConditional %40 %41 %42

         %41 =     OpLabel
                     OpReturnValue %true

         %42 =     OpLabel
         %45 =       OpLoad %int %scratchVar
         %46 =       OpIAdd %int %45 %int_1
                     OpStore %scratchVar %46
                     OpBranch %43

         %43 = OpLabel
         %47 =   OpLoad %int %scratchVar
         %48 =   OpIAdd %int %47 %int_1
                 OpStore %scratchVar %48
                 OpReturnValue %false
               OpFunctionEnd


               ; Function test_else_b
%test_else_b = OpFunction %bool None %32

         %50 = OpLabel
         %51 =   OpAccessChain %_ptr_Uniform_v4float %18 %int_0
         %52 =   OpLoad %v4float %51                ; RelaxedPrecision
         %53 =   OpCompositeExtract %float %52 1    ; RelaxedPrecision
         %54 =   OpFOrdEqual %bool %53 %float_0
                 OpSelectionMerge %57 None
                 OpBranchConditional %54 %55 %56

         %55 =     OpLabel
                     OpReturnValue %false

         %56 =     OpLabel
                     OpReturnValue %true

         %57 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function test_loop_if_b
%test_loop_if_b = OpFunction %bool None %32

         %58 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
                 OpStore %x %int_0
                 OpBranch %61

         %61 = OpLabel
                 OpLoopMerge %65 %64 None
                 OpBranch %62

         %62 =     OpLabel
         %66 =       OpLoad %int %x
         %67 =       OpSLessThanEqual %bool %66 %int_1
                     OpBranchConditional %67 %63 %65

         %63 =         OpLabel
         %68 =           OpAccessChain %_ptr_Uniform_v4float %18 %int_0
         %69 =           OpLoad %v4float %68        ; RelaxedPrecision
         %70 =           OpCompositeExtract %float %69 1    ; RelaxedPrecision
         %71 =           OpFOrdEqual %bool %70 %float_0
                         OpSelectionMerge %74 None
                         OpBranchConditional %71 %72 %73

         %72 =             OpLabel
                             OpReturnValue %false

         %73 =             OpLabel
                             OpReturnValue %true

         %74 =         OpLabel
                         OpBranch %64

         %64 =   OpLabel
         %75 =     OpLoad %int %x
         %76 =     OpIAdd %int %75 %int_1
                   OpStore %x %76
                   OpBranch %61

         %65 = OpLabel
         %77 =   OpLoad %int %scratchVar
         %78 =   OpIAdd %int %77 %int_1
                 OpStore %scratchVar %78
                 OpReturnValue %true
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %79         ; RelaxedPrecision
         %80 = OpFunctionParameter %_ptr_Function_v2float

         %81 = OpLabel
         %95 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %scratchVar %int_0
         %82 =   OpFunctionCall %bool %test_flat_b
                 OpSelectionMerge %84 None
                 OpBranchConditional %82 %83 %84

         %83 =     OpLabel
         %85 =       OpFunctionCall %bool %test_if_b
                     OpBranch %84

         %84 = OpLabel
         %86 =   OpPhi %bool %false %81 %85 %83
                 OpSelectionMerge %88 None
                 OpBranchConditional %86 %87 %88

         %87 =     OpLabel
         %89 =       OpFunctionCall %bool %test_else_b
                     OpBranch %88

         %88 = OpLabel
         %90 =   OpPhi %bool %false %84 %89 %87
                 OpSelectionMerge %92 None
                 OpBranchConditional %90 %91 %92

         %91 =     OpLabel
         %93 =       OpFunctionCall %bool %test_loop_if_b
                     OpBranch %92

         %92 = OpLabel
         %94 =   OpPhi %bool %false %88 %93 %91
                 OpSelectionMerge %99 None
                 OpBranchConditional %94 %97 %98

         %97 =     OpLabel
        %100 =       OpAccessChain %_ptr_Uniform_v4float %18 %int_0
        %101 =       OpLoad %v4float %100           ; RelaxedPrecision
                     OpStore %95 %101
                     OpBranch %99

         %98 =     OpLabel
        %102 =       OpAccessChain %_ptr_Uniform_v4float %18 %int_1
        %103 =       OpLoad %v4float %102           ; RelaxedPrecision
                     OpStore %95 %103
                     OpBranch %99

         %99 = OpLabel
        %104 =   OpLoad %v4float %95                ; RelaxedPrecision
                 OpReturnValue %104
               OpFunctionEnd

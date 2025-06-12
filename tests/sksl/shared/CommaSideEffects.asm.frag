               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %4
               OpName %_UniformBuffer "_UniformBuffer"  ; id %9
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorWhite"
               OpMemberName %_UniformBuffer 3 "colorBlack"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %setToColorBlack_vh4 "setToColorBlack_vh4"    ; id %2
               OpName %main "main"                                  ; id %3
               OpName %a "a"                                        ; id %33
               OpName %b "b"                                        ; id %34
               OpName %c "c"                                        ; id %35
               OpName %d "d"                                        ; id %36

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
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %23 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %d RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %22 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
         %30 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v4bool = OpTypeVector %bool 4


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %13

         %14 = OpLabel
         %18 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %18 %17
         %20 =   OpFunctionCall %v4float %main %18
                 OpStore %sk_FragColor %20
                 OpReturn
               OpFunctionEnd


               ; Function setToColorBlack_vh4
%setToColorBlack_vh4 = OpFunction %void None %22
         %23 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %24 = OpLabel
         %25 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_3
         %29 =   OpLoad %v4float %25                ; RelaxedPrecision
                 OpStore %23 %29
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %30         ; RelaxedPrecision
         %31 = OpFunctionParameter %_ptr_Function_v2float

         %32 = OpLabel
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %c =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %d =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %43 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %81 =   OpVariable %_ptr_Function_v4float Function
         %37 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %39 =   OpLoad %v4float %37                ; RelaxedPrecision
                 OpStore %b %39
         %40 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %42 =   OpLoad %v4float %40                ; RelaxedPrecision
                 OpStore %c %42
         %44 =   OpFunctionCall %void %setToColorBlack_vh4 %43
         %45 =   OpLoad %v4float %43                ; RelaxedPrecision
                 OpStore %d %45
         %46 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_2
         %48 =   OpLoad %v4float %46                ; RelaxedPrecision
                 OpStore %a %48
         %49 =   OpFMul %v4float %48 %48            ; RelaxedPrecision
                 OpStore %a %49
         %50 =   OpFMul %v4float %39 %39            ; RelaxedPrecision
                 OpStore %b %50
         %51 =   OpFMul %v4float %42 %42            ; RelaxedPrecision
                 OpStore %c %51
         %52 =   OpFMul %v4float %45 %45            ; RelaxedPrecision
                 OpStore %d %52
         %55 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_2
         %56 =   OpLoad %v4float %55                ; RelaxedPrecision
         %57 =   OpFOrdEqual %v4bool %49 %56
         %59 =   OpAll %bool %57
                 OpSelectionMerge %61 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
         %62 =       OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %63 =       OpLoad %v4float %62            ; RelaxedPrecision
         %64 =       OpFOrdEqual %v4bool %50 %63
         %65 =       OpAll %bool %64
                     OpBranch %61

         %61 = OpLabel
         %66 =   OpPhi %bool %false %32 %65 %60
                 OpSelectionMerge %68 None
                 OpBranchConditional %66 %67 %68

         %67 =     OpLabel
         %69 =       OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %70 =       OpLoad %v4float %69            ; RelaxedPrecision
         %71 =       OpFOrdEqual %v4bool %51 %70
         %72 =       OpAll %bool %71
                     OpBranch %68

         %68 = OpLabel
         %73 =   OpPhi %bool %false %61 %72 %67
                 OpSelectionMerge %75 None
                 OpBranchConditional %73 %74 %75

         %74 =     OpLabel
         %76 =       OpAccessChain %_ptr_Uniform_v4float %8 %int_3
         %77 =       OpLoad %v4float %76            ; RelaxedPrecision
         %78 =       OpFOrdEqual %v4bool %52 %77
         %79 =       OpAll %bool %78
                     OpBranch %75

         %75 = OpLabel
         %80 =   OpPhi %bool %false %68 %79 %74
                 OpSelectionMerge %84 None
                 OpBranchConditional %80 %82 %83

         %82 =     OpLabel
         %85 =       OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %86 =       OpLoad %v4float %85            ; RelaxedPrecision
                     OpStore %81 %86
                     OpBranch %84

         %83 =     OpLabel
         %87 =       OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %88 =       OpLoad %v4float %87            ; RelaxedPrecision
                     OpStore %81 %88
                     OpBranch %84

         %84 = OpLabel
         %89 =   OpLoad %v4float %81                ; RelaxedPrecision
                 OpReturnValue %89
               OpFunctionEnd

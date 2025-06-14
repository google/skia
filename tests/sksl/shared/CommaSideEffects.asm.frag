               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorWhite"
               OpMemberName %_UniformBuffer 3 "colorBlack"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %setToColorBlack_vh4 "setToColorBlack_vh4"    ; id %6
               OpName %main "main"                                  ; id %7
               OpName %a "a"                                        ; id %36
               OpName %b "b"                                        ; id %37
               OpName %c "c"                                        ; id %38
               OpName %d "d"                                        ; id %39

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
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %27 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %d RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %26 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_3 = OpConstant %int 3
         %33 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v4bool = OpTypeVector %bool 4


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function setToColorBlack_vh4
%setToColorBlack_vh4 = OpFunction %void None %26
         %27 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %28 = OpLabel
         %29 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_3
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
                 OpStore %27 %32
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %33         ; RelaxedPrecision
         %34 = OpFunctionParameter %_ptr_Function_v2float

         %35 = OpLabel
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %c =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %d =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %46 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %84 =   OpVariable %_ptr_Function_v4float Function
         %40 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %42 =   OpLoad %v4float %40                ; RelaxedPrecision
                 OpStore %b %42
         %43 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %45 =   OpLoad %v4float %43                ; RelaxedPrecision
                 OpStore %c %45
         %47 =   OpFunctionCall %void %setToColorBlack_vh4 %46
         %48 =   OpLoad %v4float %46                ; RelaxedPrecision
                 OpStore %d %48
         %49 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_2
         %51 =   OpLoad %v4float %49                ; RelaxedPrecision
                 OpStore %a %51
         %52 =   OpFMul %v4float %51 %51            ; RelaxedPrecision
                 OpStore %a %52
         %53 =   OpFMul %v4float %42 %42            ; RelaxedPrecision
                 OpStore %b %53
         %54 =   OpFMul %v4float %45 %45            ; RelaxedPrecision
                 OpStore %c %54
         %55 =   OpFMul %v4float %48 %48            ; RelaxedPrecision
                 OpStore %d %55
         %58 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_2
         %59 =   OpLoad %v4float %58                ; RelaxedPrecision
         %60 =   OpFOrdEqual %v4bool %52 %59
         %62 =   OpAll %bool %60
                 OpSelectionMerge %64 None
                 OpBranchConditional %62 %63 %64

         %63 =     OpLabel
         %65 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %66 =       OpLoad %v4float %65            ; RelaxedPrecision
         %67 =       OpFOrdEqual %v4bool %53 %66
         %68 =       OpAll %bool %67
                     OpBranch %64

         %64 = OpLabel
         %69 =   OpPhi %bool %false %35 %68 %63
                 OpSelectionMerge %71 None
                 OpBranchConditional %69 %70 %71

         %70 =     OpLabel
         %72 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %73 =       OpLoad %v4float %72            ; RelaxedPrecision
         %74 =       OpFOrdEqual %v4bool %54 %73
         %75 =       OpAll %bool %74
                     OpBranch %71

         %71 = OpLabel
         %76 =   OpPhi %bool %false %64 %75 %70
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
         %79 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_3
         %80 =       OpLoad %v4float %79            ; RelaxedPrecision
         %81 =       OpFOrdEqual %v4bool %55 %80
         %82 =       OpAll %bool %81
                     OpBranch %78

         %78 = OpLabel
         %83 =   OpPhi %bool %false %71 %82 %77
                 OpSelectionMerge %87 None
                 OpBranchConditional %83 %85 %86

         %85 =     OpLabel
         %88 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %89 =       OpLoad %v4float %88            ; RelaxedPrecision
                     OpStore %84 %89
                     OpBranch %87

         %86 =     OpLabel
         %90 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %91 =       OpLoad %v4float %90            ; RelaxedPrecision
                     OpStore %84 %91
                     OpBranch %87

         %87 = OpLabel
         %92 =   OpLoad %v4float %84                ; RelaxedPrecision
                 OpReturnValue %92
               OpFunctionEnd

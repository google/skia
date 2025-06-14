               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorBlack"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %_0_v "_0_v"                      ; id %27
               OpName %_1_i "_1_i"                      ; id %33
               OpName %_2_x "_2_x"                      ; id %48
               OpName %_3_y "_3_y"                      ; id %52
               OpName %_4_z "_4_z"                      ; id %55
               OpName %_5_w "_5_w"                      ; id %58

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
               OpDecorate %_0_v RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %_2_x RelaxedPrecision
               OpDecorate %_3_y RelaxedPrecision
               OpDecorate %_4_z RelaxedPrecision
               OpDecorate %_5_w RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision

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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_n1_25 = OpConstant %float -1.25
         %63 = OpConstantComposite %v4float %float_n1_25 %float_n1_25 %float_n1_25 %float_0
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
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
       %_0_v =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
       %_1_i =   OpVariable %_ptr_Function_v4int Function
       %_2_x =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
       %_3_y =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
       %_4_z =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
       %_5_w =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %68 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
                 OpStore %_0_v %32
         %36 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %38 =   OpLoad %v4float %36                ; RelaxedPrecision
         %39 =   OpCompositeExtract %float %38 0    ; RelaxedPrecision
         %40 =   OpConvertFToS %int %39
         %41 =   OpCompositeExtract %float %38 1    ; RelaxedPrecision
         %42 =   OpConvertFToS %int %41
         %43 =   OpCompositeExtract %float %38 2    ; RelaxedPrecision
         %44 =   OpConvertFToS %int %43
         %45 =   OpCompositeExtract %float %38 3    ; RelaxedPrecision
         %46 =   OpConvertFToS %int %45
         %47 =   OpCompositeConstruct %v4int %40 %42 %44 %46
                 OpStore %_1_i %47
         %50 =   OpCompositeExtract %int %47 0
         %51 =   OpVectorExtractDynamic %float %32 %50
                 OpStore %_2_x %51
         %53 =   OpCompositeExtract %int %47 1
         %54 =   OpVectorExtractDynamic %float %32 %53
                 OpStore %_3_y %54
         %56 =   OpCompositeExtract %int %47 2
         %57 =   OpVectorExtractDynamic %float %32 %56
                 OpStore %_4_z %57
         %59 =   OpCompositeExtract %int %47 3
         %60 =   OpVectorExtractDynamic %float %32 %59
                 OpStore %_5_w %60
         %61 =   OpCompositeConstruct %v4float %51 %54 %57 %60  ; RelaxedPrecision
         %64 =   OpFOrdEqual %v4bool %61 %63
         %67 =   OpAll %bool %64
                 OpSelectionMerge %71 None
                 OpBranchConditional %67 %69 %70

         %69 =     OpLabel
         %72 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %74 =       OpLoad %v4float %72            ; RelaxedPrecision
                     OpStore %68 %74
                     OpBranch %71

         %70 =     OpLabel
         %75 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %77 =       OpLoad %v4float %75            ; RelaxedPrecision
                     OpStore %68 %77
                     OpBranch %71

         %71 = OpLabel
         %78 =   OpLoad %v4float %68                ; RelaxedPrecision
                 OpReturnValue %78
               OpFunctionEnd

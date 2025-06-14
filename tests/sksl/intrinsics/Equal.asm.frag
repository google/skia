               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %15
               OpMemberName %_UniformBuffer 0 "a"
               OpMemberName %_UniformBuffer 1 "b"
               OpMemberName %_UniformBuffer 2 "c"
               OpMemberName %_UniformBuffer 3 "d"
               OpMemberName %_UniformBuffer 4 "e"
               OpMemberName %_UniformBuffer 5 "f"
               OpName %main "main"                  ; id %6
               OpName %expectTTFF "expectTTFF"      ; id %20
               OpName %expectFFTT "expectFFTT"      ; id %27
               OpName %expectTTTT "expectTTTT"      ; id %29

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 3 Offset 40
               OpMemberDecorate %_UniformBuffer 4 Offset 48
               OpMemberDecorate %_UniformBuffer 5 Offset 64
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %35 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %v3int = OpTypeVector %int 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %v2uint %v2uint %v3int %v3int  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
         %26 = OpConstantComposite %v4bool %true %true %false %false
         %28 = OpConstantComposite %v4bool %false %false %true %true
         %30 = OpConstantComposite %v4bool %true %true %true %true
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Uniform_v2uint = OpTypePointer Uniform %v2uint
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
     %v2bool = OpTypeVector %bool 2
%_ptr_Uniform_v3int = OpTypePointer Uniform %v3int
      %int_4 = OpConstant %int 4
      %int_5 = OpConstant %int 5
     %v3bool = OpTypeVector %bool 3


               ; Function main
       %main = OpFunction %void None %18

         %19 = OpLabel
 %expectTTFF =   OpVariable %_ptr_Function_v4bool Function
 %expectFFTT =   OpVariable %_ptr_Function_v4bool Function
 %expectTTTT =   OpVariable %_ptr_Function_v4bool Function
                 OpStore %expectTTFF %26
                 OpStore %expectFFTT %28
                 OpStore %expectTTTT %30
         %32 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %35 =   OpLoad %v4float %32                ; RelaxedPrecision
         %36 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %38 =   OpLoad %v4float %36                ; RelaxedPrecision
         %31 =   OpFOrdEqual %v4bool %35 %38
         %39 =   OpCompositeExtract %bool %31 0
         %40 =   OpSelect %float %39 %float_1 %float_0  ; RelaxedPrecision
         %43 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %43 %40
         %46 =   OpAccessChain %_ptr_Uniform_v2uint %11 %int_2
         %49 =   OpLoad %v2uint %46
         %50 =   OpAccessChain %_ptr_Uniform_v2uint %11 %int_3
         %52 =   OpLoad %v2uint %50
         %45 =   OpIEqual %v2bool %49 %52
         %54 =   OpCompositeExtract %bool %45 1
         %55 =   OpSelect %float %54 %float_1 %float_0  ; RelaxedPrecision
         %56 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
                 OpStore %56 %55
         %58 =   OpAccessChain %_ptr_Uniform_v3int %11 %int_4
         %61 =   OpLoad %v3int %58
         %62 =   OpAccessChain %_ptr_Uniform_v3int %11 %int_5
         %64 =   OpLoad %v3int %62
         %57 =   OpIEqual %v3bool %61 %64
         %66 =   OpCompositeExtract %bool %57 2
         %67 =   OpSelect %float %66 %float_1 %float_0  ; RelaxedPrecision
         %68 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
                 OpStore %68 %67
         %70 =   OpLoad %v4bool %expectTTFF         ; RelaxedPrecision
         %69 =   OpAny %bool %70
                 OpSelectionMerge %72 None
                 OpBranchConditional %69 %72 %71

         %71 =     OpLabel
         %74 =       OpLoad %v4bool %expectFFTT     ; RelaxedPrecision
         %73 =       OpAny %bool %74
                     OpBranch %72

         %72 = OpLabel
         %75 =   OpPhi %bool %true %19 %73 %71
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %77 %76

         %76 =     OpLabel
         %79 =       OpLoad %v4bool %expectTTTT     ; RelaxedPrecision
         %78 =       OpAny %bool %79
                     OpBranch %77

         %77 = OpLabel
         %80 =   OpPhi %bool %true %72 %78 %76
         %81 =   OpSelect %float %80 %float_1 %float_0  ; RelaxedPrecision
         %82 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
                 OpStore %82 %81
                 OpReturn
               OpFunctionEnd

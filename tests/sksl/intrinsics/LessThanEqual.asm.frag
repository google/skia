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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision

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
                 OpStore %expectTTFF %26
                 OpStore %expectFFTT %28
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %33 =   OpLoad %v4float %30                ; RelaxedPrecision
         %34 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %36 =   OpLoad %v4float %34                ; RelaxedPrecision
         %29 =   OpFOrdLessThanEqual %v4bool %33 %36
         %37 =   OpCompositeExtract %bool %29 0
         %38 =   OpSelect %float %37 %float_1 %float_0  ; RelaxedPrecision
         %41 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %41 %38
         %44 =   OpAccessChain %_ptr_Uniform_v2uint %11 %int_2
         %47 =   OpLoad %v2uint %44
         %48 =   OpAccessChain %_ptr_Uniform_v2uint %11 %int_3
         %50 =   OpLoad %v2uint %48
         %43 =   OpULessThanEqual %v2bool %47 %50
         %52 =   OpCompositeExtract %bool %43 1
         %53 =   OpSelect %float %52 %float_1 %float_0  ; RelaxedPrecision
         %54 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
                 OpStore %54 %53
         %56 =   OpAccessChain %_ptr_Uniform_v3int %11 %int_4
         %59 =   OpLoad %v3int %56
         %60 =   OpAccessChain %_ptr_Uniform_v3int %11 %int_5
         %62 =   OpLoad %v3int %60
         %55 =   OpSLessThanEqual %v3bool %59 %62
         %64 =   OpCompositeExtract %bool %55 2
         %65 =   OpSelect %float %64 %float_1 %float_0  ; RelaxedPrecision
         %66 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
                 OpStore %66 %65
         %68 =   OpLoad %v4bool %expectTTFF         ; RelaxedPrecision
         %67 =   OpAny %bool %68
                 OpSelectionMerge %70 None
                 OpBranchConditional %67 %70 %69

         %69 =     OpLabel
         %72 =       OpLoad %v4bool %expectFFTT     ; RelaxedPrecision
         %71 =       OpAny %bool %72
                     OpBranch %70

         %70 = OpLabel
         %73 =   OpPhi %bool %true %19 %71 %69
         %74 =   OpSelect %float %73 %float_1 %float_0  ; RelaxedPrecision
         %75 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
                 OpStore %75 %74
                 OpReturn
               OpFunctionEnd

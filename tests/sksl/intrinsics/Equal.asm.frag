               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "a"
               OpMemberName %_UniformBuffer 1 "b"
               OpMemberName %_UniformBuffer 2 "c"
               OpMemberName %_UniformBuffer 3 "d"
               OpMemberName %_UniformBuffer 4 "e"
               OpMemberName %_UniformBuffer 5 "f"
               OpName %main "main"
               OpName %expectTTFF "expectTTFF"
               OpName %expectFFTT "expectFFTT"
               OpName %expectTTTT "expectTTTT"
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
        %int = OpTypeInt 32 1
      %v3int = OpTypeVector %int 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %v2uint %v2uint %v3int %v3int
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
         %23 = OpConstantComposite %v4bool %true %true %false %false
         %25 = OpConstantComposite %v4bool %false %false %true %true
         %27 = OpConstantComposite %v4bool %true %true %true %true
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
       %main = OpFunction %void None %15
         %16 = OpLabel
 %expectTTFF = OpVariable %_ptr_Function_v4bool Function
 %expectFFTT = OpVariable %_ptr_Function_v4bool Function
 %expectTTTT = OpVariable %_ptr_Function_v4bool Function
               OpStore %expectTTFF %23
               OpStore %expectFFTT %25
               OpStore %expectTTTT %27
         %29 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %32 = OpLoad %v4float %29
         %33 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %35 = OpLoad %v4float %33
         %28 = OpFOrdEqual %v4bool %32 %35
         %36 = OpCompositeExtract %bool %28 0
         %37 = OpSelect %float %36 %float_1 %float_0
         %40 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %40 %37
         %43 = OpAccessChain %_ptr_Uniform_v2uint %7 %int_2
         %46 = OpLoad %v2uint %43
         %47 = OpAccessChain %_ptr_Uniform_v2uint %7 %int_3
         %49 = OpLoad %v2uint %47
         %42 = OpIEqual %v2bool %46 %49
         %51 = OpCompositeExtract %bool %42 1
         %52 = OpSelect %float %51 %float_1 %float_0
         %53 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
               OpStore %53 %52
         %55 = OpAccessChain %_ptr_Uniform_v3int %7 %int_4
         %58 = OpLoad %v3int %55
         %59 = OpAccessChain %_ptr_Uniform_v3int %7 %int_5
         %61 = OpLoad %v3int %59
         %54 = OpIEqual %v3bool %58 %61
         %63 = OpCompositeExtract %bool %54 2
         %64 = OpSelect %float %63 %float_1 %float_0
         %65 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
               OpStore %65 %64
         %67 = OpLoad %v4bool %expectTTFF
         %66 = OpAny %bool %67
               OpSelectionMerge %69 None
               OpBranchConditional %66 %69 %68
         %68 = OpLabel
         %71 = OpLoad %v4bool %expectFFTT
         %70 = OpAny %bool %71
               OpBranch %69
         %69 = OpLabel
         %72 = OpPhi %bool %true %16 %70 %68
               OpSelectionMerge %74 None
               OpBranchConditional %72 %74 %73
         %73 = OpLabel
         %76 = OpLoad %v4bool %expectTTTT
         %75 = OpAny %bool %76
               OpBranch %74
         %74 = OpLabel
         %77 = OpPhi %bool %true %69 %75 %73
         %78 = OpSelect %float %77 %float_1 %float_0
         %79 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
               OpStore %79 %78
               OpReturn
               OpFunctionEnd

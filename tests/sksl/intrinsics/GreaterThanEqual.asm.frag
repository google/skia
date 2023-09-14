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
               OpName %expectFFTT "expectFFTT"
               OpName %expectTTFF "expectTTFF"
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
               OpDecorate %30 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
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
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %23 = OpConstantComposite %v4bool %false %false %true %true
         %25 = OpConstantComposite %v4bool %true %true %false %false
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
 %expectFFTT = OpVariable %_ptr_Function_v4bool Function
 %expectTTFF = OpVariable %_ptr_Function_v4bool Function
               OpStore %expectFFTT %23
               OpStore %expectTTFF %25
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %30 = OpLoad %v4float %27
         %31 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %33 = OpLoad %v4float %31
         %26 = OpFOrdGreaterThanEqual %v4bool %30 %33
         %34 = OpCompositeExtract %bool %26 0
         %35 = OpSelect %float %34 %float_1 %float_0
         %38 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %38 %35
         %41 = OpAccessChain %_ptr_Uniform_v2uint %7 %int_2
         %44 = OpLoad %v2uint %41
         %45 = OpAccessChain %_ptr_Uniform_v2uint %7 %int_3
         %47 = OpLoad %v2uint %45
         %40 = OpUGreaterThanEqual %v2bool %44 %47
         %49 = OpCompositeExtract %bool %40 1
         %50 = OpSelect %float %49 %float_1 %float_0
         %51 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
               OpStore %51 %50
         %53 = OpAccessChain %_ptr_Uniform_v3int %7 %int_4
         %56 = OpLoad %v3int %53
         %57 = OpAccessChain %_ptr_Uniform_v3int %7 %int_5
         %59 = OpLoad %v3int %57
         %52 = OpSGreaterThanEqual %v3bool %56 %59
         %61 = OpCompositeExtract %bool %52 2
         %62 = OpSelect %float %61 %float_1 %float_0
         %63 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
               OpStore %63 %62
         %65 = OpLoad %v4bool %expectTTFF
         %64 = OpAny %bool %65
               OpSelectionMerge %67 None
               OpBranchConditional %64 %67 %66
         %66 = OpLabel
         %69 = OpLoad %v4bool %expectFFTT
         %68 = OpAny %bool %69
               OpBranch %67
         %67 = OpLabel
         %70 = OpPhi %bool %true %16 %68 %66
         %71 = OpSelect %float %70 %float_1 %float_0
         %72 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
               OpStore %72 %71
               OpReturn
               OpFunctionEnd

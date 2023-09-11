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
               OpDecorate %36 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
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
         %26 = OpFOrdGreaterThan %v4bool %30 %33
         %34 = OpCompositeExtract %bool %26 0
         %35 = OpSelect %int %34 %int_1 %int_0
         %36 = OpConvertSToF %float %35
         %37 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %37 %36
         %40 = OpAccessChain %_ptr_Uniform_v2uint %7 %int_2
         %43 = OpLoad %v2uint %40
         %44 = OpAccessChain %_ptr_Uniform_v2uint %7 %int_3
         %46 = OpLoad %v2uint %44
         %39 = OpUGreaterThan %v2bool %43 %46
         %48 = OpCompositeExtract %bool %39 1
         %49 = OpSelect %int %48 %int_1 %int_0
         %50 = OpConvertSToF %float %49
         %51 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
               OpStore %51 %50
         %53 = OpAccessChain %_ptr_Uniform_v3int %7 %int_4
         %56 = OpLoad %v3int %53
         %57 = OpAccessChain %_ptr_Uniform_v3int %7 %int_5
         %59 = OpLoad %v3int %57
         %52 = OpSGreaterThan %v3bool %56 %59
         %61 = OpCompositeExtract %bool %52 2
         %62 = OpSelect %int %61 %int_1 %int_0
         %63 = OpConvertSToF %float %62
         %64 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
               OpStore %64 %63
         %66 = OpLoad %v4bool %expectTTFF
         %65 = OpAny %bool %66
               OpSelectionMerge %68 None
               OpBranchConditional %65 %68 %67
         %67 = OpLabel
         %70 = OpLoad %v4bool %expectFFTT
         %69 = OpAny %bool %70
               OpBranch %68
         %68 = OpLabel
         %71 = OpPhi %bool %true %16 %69 %67
         %72 = OpSelect %int %71 %int_1 %int_0
         %73 = OpConvertSToF %float %72
         %74 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
               OpStore %74 %73
               OpReturn
               OpFunctionEnd

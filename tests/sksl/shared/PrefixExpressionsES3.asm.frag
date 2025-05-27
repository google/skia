               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %ok "ok"
               OpName %val "val"
               OpName %mask "mask"
               OpName %imask "imask"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
      %false = OpConstantFalse %bool
     %uint_0 = OpConstant %uint 0
         %63 = OpConstantComposite %v2uint %uint_0 %uint_0
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
        %val = OpVariable %_ptr_Function_uint Function
       %mask = OpVariable %_ptr_Function_v2uint Function
      %imask = OpVariable %_ptr_Function_v2int Function
         %68 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
         %30 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %34 = OpLoad %v4float %30
         %35 = OpCompositeExtract %float %34 0
         %36 = OpConvertFToU %uint %35
               OpStore %val %36
         %40 = OpNot %uint %36
         %41 = OpCompositeConstruct %v2uint %36 %40
               OpStore %mask %41
         %45 = OpNot %v2uint %41
         %46 = OpCompositeExtract %uint %45 0
         %47 = OpBitcast %int %46
         %48 = OpCompositeExtract %uint %45 1
         %49 = OpBitcast %int %48
         %50 = OpCompositeConstruct %v2int %47 %49
               OpStore %imask %50
         %51 = OpNot %v2uint %41
         %52 = OpNot %v2int %50
         %53 = OpCompositeExtract %int %52 0
         %54 = OpBitcast %uint %53
         %55 = OpCompositeExtract %int %52 1
         %56 = OpBitcast %uint %55
         %57 = OpCompositeConstruct %v2uint %54 %56
         %58 = OpBitwiseAnd %v2uint %51 %57
               OpStore %mask %58
               OpSelectionMerge %61 None
               OpBranchConditional %true %60 %61
         %60 = OpLabel
         %64 = OpIEqual %v2bool %58 %63
         %66 = OpAll %bool %64
               OpBranch %61
         %61 = OpLabel
         %67 = OpPhi %bool %false %22 %66 %60
               OpStore %ok %67
               OpSelectionMerge %72 None
               OpBranchConditional %67 %70 %71
         %70 = OpLabel
         %73 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %74 = OpLoad %v4float %73
               OpStore %68 %74
               OpBranch %72
         %71 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %77 = OpLoad %v4float %75
               OpStore %68 %77
               OpBranch %72
         %72 = OpLabel
         %78 = OpLoad %v4float %68
               OpReturnValue %78
               OpFunctionEnd

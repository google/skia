               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
         %65 = OpConstantComposite %v2uint %uint_0 %uint_0
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
        %val = OpVariable %_ptr_Function_uint Function
       %mask = OpVariable %_ptr_Function_v2uint Function
      %imask = OpVariable %_ptr_Function_v2int Function
         %70 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
         %32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %36 = OpLoad %v4float %32
         %37 = OpCompositeExtract %float %36 0
         %38 = OpConvertFToU %uint %37
               OpStore %val %38
         %42 = OpNot %uint %38
         %43 = OpCompositeConstruct %v2uint %38 %42
               OpStore %mask %43
         %47 = OpNot %v2uint %43
         %48 = OpCompositeExtract %uint %47 0
         %49 = OpBitcast %int %48
         %50 = OpCompositeExtract %uint %47 1
         %51 = OpBitcast %int %50
         %52 = OpCompositeConstruct %v2int %49 %51
               OpStore %imask %52
         %53 = OpNot %v2uint %43
         %54 = OpNot %v2int %52
         %55 = OpCompositeExtract %int %54 0
         %56 = OpBitcast %uint %55
         %57 = OpCompositeExtract %int %54 1
         %58 = OpBitcast %uint %57
         %59 = OpCompositeConstruct %v2uint %56 %58
         %60 = OpBitwiseAnd %v2uint %53 %59
               OpStore %mask %60
               OpSelectionMerge %63 None
               OpBranchConditional %true %62 %63
         %62 = OpLabel
         %66 = OpIEqual %v2bool %60 %65
         %68 = OpAll %bool %66
               OpBranch %63
         %63 = OpLabel
         %69 = OpPhi %bool %false %25 %68 %62
               OpStore %ok %69
               OpSelectionMerge %74 None
               OpBranchConditional %69 %72 %73
         %72 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %76 = OpLoad %v4float %75
               OpStore %70 %76
               OpBranch %74
         %73 = OpLabel
         %77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %79 = OpLoad %v4float %77
               OpStore %70 %79
               OpBranch %74
         %74 = OpLabel
         %80 = OpLoad %v4float %70
               OpReturnValue %80
               OpFunctionEnd

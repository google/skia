               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "testMatrix4x4"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %main "main"                      ; id %6
               OpName %inputA "inputA"                  ; id %28
               OpName %inputB "inputB"                  ; id %36
               OpName %expected "expected"              ; id %41

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 64
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 80
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %91 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_5 = OpConstant %float 5
   %float_17 = OpConstant %float 17
   %float_38 = OpConstant %float 38
   %float_70 = OpConstant %float 70
         %46 = OpConstantComposite %v4float %float_5 %float_17 %float_38 %float_70
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %v3float = OpTypeVector %float 3
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %25         ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_Function_v2float

         %27 = OpLabel
     %inputA =   OpVariable %_ptr_Function_v4float Function
     %inputB =   OpVariable %_ptr_Function_v4float Function
   %expected =   OpVariable %_ptr_Function_v4float Function
         %86 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_mat4v4float %11 %int_0
         %33 =   OpAccessChain %_ptr_Uniform_v4float %30 %int_0
         %35 =   OpLoad %v4float %33
                 OpStore %inputA %35
         %37 =   OpAccessChain %_ptr_Uniform_mat4v4float %11 %int_0
         %39 =   OpAccessChain %_ptr_Uniform_v4float %37 %int_1
         %40 =   OpLoad %v4float %39
                 OpStore %inputB %40
                 OpStore %expected %46
         %50 =   OpCompositeExtract %float %35 0
         %51 =   OpCompositeExtract %float %40 0
         %49 =   OpFMul %float %50 %51
         %52 =   OpFOrdEqual %bool %49 %float_5
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
         %56 =       OpVectorShuffle %v2float %35 %35 0 1
         %57 =       OpVectorShuffle %v2float %40 %40 0 1
         %55 =       OpDot %float %56 %57
         %58 =       OpFOrdEqual %bool %55 %float_17
                     OpBranch %54

         %54 = OpLabel
         %59 =   OpPhi %bool %false %27 %58 %53
                 OpSelectionMerge %61 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
         %63 =       OpVectorShuffle %v3float %35 %35 0 1 2
         %65 =       OpVectorShuffle %v3float %40 %40 0 1 2
         %62 =       OpDot %float %63 %65
         %66 =       OpFOrdEqual %bool %62 %float_38
                     OpBranch %61

         %61 = OpLabel
         %67 =   OpPhi %bool %false %54 %66 %60
                 OpSelectionMerge %69 None
                 OpBranchConditional %67 %68 %69

         %68 =     OpLabel
         %70 =       OpDot %float %35 %40
         %71 =       OpFOrdEqual %bool %70 %float_70
                     OpBranch %69

         %69 = OpLabel
         %72 =   OpPhi %bool %false %61 %71 %68
                 OpSelectionMerge %74 None
                 OpBranchConditional %72 %73 %74

         %73 =     OpLabel
                     OpBranch %74

         %74 = OpLabel
         %76 =   OpPhi %bool %false %69 %true %73
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
                     OpBranch %78

         %78 = OpLabel
         %79 =   OpPhi %bool %false %74 %true %77
                 OpSelectionMerge %81 None
                 OpBranchConditional %79 %80 %81

         %80 =     OpLabel
                     OpBranch %81

         %81 = OpLabel
         %82 =   OpPhi %bool %false %78 %true %80
                 OpSelectionMerge %84 None
                 OpBranchConditional %82 %83 %84

         %83 =     OpLabel
                     OpBranch %84

         %84 = OpLabel
         %85 =   OpPhi %bool %false %81 %true %83
                 OpSelectionMerge %89 None
                 OpBranchConditional %85 %87 %88

         %87 =     OpLabel
         %90 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %91 =       OpLoad %v4float %90            ; RelaxedPrecision
                     OpStore %86 %91
                     OpBranch %89

         %88 =     OpLabel
         %92 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %94 =       OpLoad %v4float %92            ; RelaxedPrecision
                     OpStore %86 %94
                     OpBranch %89

         %89 = OpLabel
         %95 =   OpLoad %v4float %86                ; RelaxedPrecision
                 OpReturnValue %95
               OpFunctionEnd

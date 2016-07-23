# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Builds shaderc for the Vulkan backend
{
  'targets': [
    {
      'target_name': 'shaderc_combined',
      'type': 'static_library',
      'dependencies': [
        'libshaderc',
        'libshaderc_util',
        'liboglcompiler',
        'libspirv',
        'libglslangosdependent',
        'libglslang',
      ],
    },
    {
      'target_name': 'libshaderc',
      'type': 'static_library',
      'cflags': [
        '-w',
      ],
      'xcode_settings': {
        'WARNING_CFLAGS': [
          '-w'
        ],
      },
      'include_dirs': [
        '../third_party/externals/shaderc2/libshaderc/include',
        '../third_party/externals/shaderc2/libshaderc_util/include',
        '../third_party/externals/shaderc2/third_party/glslang',
      ],
      'sources': [
        '../third_party/externals/shaderc2/libshaderc/include/shaderc/shaderc.hpp',
        '../third_party/externals/shaderc2/libshaderc/src/shaderc.cc',
      ],
    },
    {
      'target_name': 'libshaderc_util',
      'type': 'static_library',
      'dependencies': [
        'libspirvtools'
      ],
      'cflags': [
        '-w',
      ],
      'xcode_settings': {
        'WARNING_CFLAGS': [
          '-w'
        ],
      },
      'include_dirs': [
        '../third_party/externals/shaderc2/libshaderc_util/include',
        '../third_party/externals/shaderc2/third_party/glslang',
        '../third_party/externals/shaderc2/third_party/spirv-tools/include',
      ],
      'sources': [
        '../third_party/externals/shaderc2/libshaderc_util/src/compiler.cc',
        '../third_party/externals/shaderc2/libshaderc_util/src/file_finder.cc',
        '../third_party/externals/shaderc2/libshaderc_util/src/io.cc',
        '../third_party/externals/shaderc2/libshaderc_util/src/message.cc',
        '../third_party/externals/shaderc2/libshaderc_util/src/resources.cc',
        '../third_party/externals/shaderc2/libshaderc_util/src/shader_stage.cc',
        '../third_party/externals/shaderc2/libshaderc_util/src/version_profile.cc',
      ],
    },
    {
      'target_name': 'libspirv',
      'type': 'static_library',
      'cflags': [
        '-w',
      ],
      'xcode_settings': {
        'WARNING_CFLAGS': [
          '-w'
        ],
      },
      'include_dirs': [
        '../third_party/externals/shaderc2/third_party/glslang',
      ],
      'sources': [
        '../third_party/externals/shaderc2/third_party/glslang/SPIRV/GlslangToSpv.cpp',
        '../third_party/externals/shaderc2/third_party/glslang/SPIRV/InReadableOrder.cpp',
        '../third_party/externals/shaderc2/third_party/glslang/SPIRV/SpvBuilder.cpp',
        '../third_party/externals/shaderc2/third_party/glslang/SPIRV/SPVRemapper.cpp',
        '../third_party/externals/shaderc2/third_party/glslang/SPIRV/doc.cpp',
        '../third_party/externals/shaderc2/third_party/glslang/SPIRV/disassemble.cpp',
      ],
    },
    {
      'target_name': 'liboglcompiler',
      'type': 'static_library',
      'cflags': [
        '-w',
      ],
      'xcode_settings': {
        'WARNING_CFLAGS': [
          '-w'
        ],
      },
      'include_dirs': [
        '../third_party/externals/shaderc2/third_party/glslang/OGLCompilersDLL',
      ],
      'sources': [
        '../third_party/externals/shaderc2/third_party/glslang/OGLCompilersDLL/InitializeDll.cpp',
      ],
    },
    {
      'target_name': 'libglslangosdependent',
      'type': 'static_library',
      'cflags': [
        '-w',
      ],
      'xcode_settings': {
        'WARNING_CFLAGS': [
          '-w'
        ],
      },
      'conditions': [
        ['skia_os == "win"', {
          'include_dirs': [
            '../third_party/externals/shaderc2/third_party/glslang/glslang/OSDependent/Windows',
          ],
          'sources': [
            '../third_party/externals/shaderc2/third_party/glslang/glslang/OSDependent/Windows/ossource.cpp',
          ],
        }, {
          'include_dirs': [
            '../third_party/externals/shaderc2/third_party/glslang/glslang/OSDependent/Unix',
          ],
          'sources': [
            '../third_party/externals/shaderc2/third_party/glslang/glslang/OSDependent/Unix/ossource.cpp',
          ],
        }],
      ],
    },
    {
      'target_name': 'libglslang',
      'type': 'static_library',
      'cflags': [
        '-w',
      ],
      'xcode_settings': {
        'WARNING_CFLAGS': [
          '-w'
        ],
      },
      'msvs_settings': {
        'VCCLCompilerTool': {
          'AdditionalOptions': [
            '/wd4800',
            '/wd4005',
            '/wd4189',
          ],
        },
      },
      'include_dirs': [
        '../third_party/externals/shaderc2/third_party/glslang/glslang/Include',
      ],
      'sources': [
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/glslang.y',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/glslang_tab.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/Constant.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/InfoSink.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/Initialize.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/IntermTraverse.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/Intermediate.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/ParseHelper.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/PoolAlloc.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/RemoveTree.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/Scan.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/ShaderLang.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/SymbolTable.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/Versions.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/intermOut.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/limits.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/linkValidate.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/parseConst.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/reflection.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/preprocessor/Pp.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/preprocessor/PpMemory.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/preprocessor/PpSymbols.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/GenericCodeGen/CodeGen.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/glslang/GenericCodeGen/Link.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/hlsl/hlslParseHelper.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/hlsl/hlslScanContext.cpp',
            '../third_party/externals/shaderc2/third_party/glslang/hlsl/hlslGrammar.cpp',
      ],
    },
    {
      'target_name': 'libspirvtools',
      'type': 'static_library',
      'dependencies': [
        'genspirvtools',
      ],
      'cflags': [
        '-w',
      ],
      'xcode_settings': {
        'WARNING_CFLAGS': [
          '-w'
        ],
      },
      'msvs_settings': {
        'VCCLCompilerTool': {
          'AdditionalOptions': [
            '/wd4800',
          ],
        },
      },
      'include_dirs': [
        '../third_party/externals/shaderc2/third_party/spirv-tools/include',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source',
      ],
      'sources': [
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/assembly_grammar.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/binary.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/diagnostic.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/disassemble.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/ext_inst.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/instruction.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/opcode.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/operand.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/print.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv_endian.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv_target_env.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/table.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/text.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/text_handler.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/validate.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/validate_cfg.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/validate_id.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/validate_instruction.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/validate_layout.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/validate_ssa.cpp',
        '../third_party/externals/shaderc2/third_party/spirv-tools/source/validate_types.cpp',
      ],
    },
    {
      'target_name': 'genspirvtools',
      'type': 'none',
      'actions': [
        {
          'action_name': 'produce_glsl-1-0',
          'inputs': [
            '../third_party/externals/shaderc2/third_party/spirv-tools/utils/generate_grammar_tables.py',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv-1-0.core.grammar.json',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/extinst-1-0.glsl.std.450.grammar.json',
          ],
          'outputs': [
             '../third_party/externals/shaderc2/third_party/spirv-tools/source/glsl.std.450.insts-1-0.inc',
          ],
          'action': [
            'python',
            '../third_party/externals/shaderc2/third_party/spirv-tools/utils/generate_grammar_tables.py',
            '--spirv-core-grammar',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv-1-0.core.grammar.json',
            '--extinst-glsl-grammar',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/extinst-1-0.glsl.std.450.grammar.json',
            '--glsl-insts-output',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/glsl.std.450.insts-1-0.inc',
          ],
        },
        {
          'action_name': 'produce_core_operand-1-0',
          'inputs': [
            '../third_party/externals/shaderc2/third_party/spirv-tools/utils/generate_grammar_tables.py',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv-1-0.core.grammar.json',
          ],
          'outputs': [
             '../third_party/externals/shaderc2/third_party/spirv-tools/source/core.insts-1-0.inc',
             '../third_party/externals/shaderc2/third_party/spirv-tools/source/operand.kinds-1-0.inc',
          ],
          'action': [
            'python',
            '../third_party/externals/shaderc2/third_party/spirv-tools/utils/generate_grammar_tables.py',
            '--spirv-core-grammar',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv-1-0.core.grammar.json',
            '--core-insts-output',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/core.insts-1-0.inc',
            '--operand-kinds-output',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/operand.kinds-1-0.inc'
          ],
        },
        {
          'action_name': 'produce_core_operand-1-1',
          'inputs': [
            '../third_party/externals/shaderc2/third_party/spirv-tools/utils/generate_grammar_tables.py',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv-1-1.core.grammar.json',
          ],
          'outputs': [
             '../third_party/externals/shaderc2/third_party/spirv-tools/source/core.insts-1-1.inc',
             '../third_party/externals/shaderc2/third_party/spirv-tools/source/operand.kinds-1-1.inc',
          ],
          'action': [
            'python',
            '../third_party/externals/shaderc2/third_party/spirv-tools/utils/generate_grammar_tables.py',
            '--spirv-core-grammar',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv-1-1.core.grammar.json',
            '--core-insts-output',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/core.insts-1-1.inc',
            '--operand-kinds-output',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/operand.kinds-1-1.inc'
          ],
        },
        {
          'action_name': 'produce_opencl-1-0',
          'inputs': [
            '../third_party/externals/shaderc2/third_party/spirv-tools/utils/generate_grammar_tables.py',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv-1-0.core.grammar.json',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/extinst-1-0.opencl.std.grammar.json',
          ],
          'outputs': [
             '../third_party/externals/shaderc2/third_party/spirv-tools/source/opencl.std.insts-1-0.inc',
          ],
          'action': [
            'python',
            '../third_party/externals/shaderc2/third_party/spirv-tools/utils/generate_grammar_tables.py',
            '--spirv-core-grammar',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/spirv-1-1.core.grammar.json',
            '--extinst-opencl-grammar',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/extinst-1-0.opencl.std.grammar.json',
            '--opencl-insts-output',
            '../third_party/externals/shaderc2/third_party/spirv-tools/source/opencl.std.insts-1-0.inc'
          ],
        },
      ],
    },
  ],
}

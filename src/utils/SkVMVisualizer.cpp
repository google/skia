/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/utils/SkVMVisualizer.h"
#include <sstream>
#include "src/core/SkStreamPriv.h"

namespace {

size_t get_addr(const char* str) {
    size_t addr;
    std::istringstream ss(str);
    ss >> std::hex >> addr;
    SkASSERT(!ss.fail());
    return addr;
}

}

namespace skvm::viz {

bool Instruction::operator == (const Instruction& o) const {
    return this->kind == o.kind &&
           this->startCode == o.startCode &&
           this->endCode == o.endCode &&
           this->instructionIndex == o.instructionIndex &&
           this->instruction == o.instruction &&
           this->duplicates == o.duplicates;
}

SkString Instruction::classes() const {
    SkString result((kind & InstructionFlags::kDead) ? "dead" : "normal");
    if (duplicates > 0) result += " origin";
    if (duplicates < 0) result += " deduped";
    return result;
}

uint32_t InstructionHash::operator()(const Instruction& i) const {
    uint32_t hash = 0;
    hash = SkOpts::hash_fn(&i.kind, sizeof(i.kind), hash);
    hash = SkOpts::hash_fn(&i.instructionIndex, sizeof(i.instructionIndex), hash);
    hash = SkOpts::hash_fn(&i.instruction, sizeof(i.instruction), hash);
    return hash;
}

void Visualizer::parseDisassembler(SkWStream* output, const char* code) {
    if (code == nullptr) {
        fAsmLine = 0;
        return;
    }
    // Read the disassembled code from <_skvm_jit> until
    // the last command that is attached to the byte code
    // We skip all the prelude (main loop organizing and such)
    // generate the main loop running on vector values (keeping hoisted commands in place)
    // and skip the tail loop (which is the same as the main, only on scalar values)
    // We stop after the last byte code.
    SkTArray<SkString> commands;
    SkStrSplit(code, "\n", kStrict_SkStrSplitMode, &commands);
    for (const SkString& line : commands) {
        ++fAsmLine;
        if (line.find("<_skvm_jit>") >= 0) {
            break;
        }
    }

    if (fAsmLine < commands.size()) {
        const SkString& line = commands[fAsmLine];
        SkTArray<SkString> tokens;
        SkStrSplit(line.c_str(), "\t", kStrict_SkStrSplitMode, &tokens);
        if (tokens.size() >= 2 && tokens[0].size() > 1) {
            fAsmStart = get_addr(tokens[0].c_str());
        }
    }

    fAsmEnd += fAsmStart;
    for (size_t i = fAsmLine; i < commands.size(); ++i) {
        const SkString& line = commands[i];
        SkTArray<SkString> tokens;
        SkStrSplit(line.c_str(), "\t", kStrict_SkStrSplitMode, &tokens);
        size_t addr = 0;
        if (tokens.size() >= 2 && tokens[0].size() > 1) {
            addr = get_addr(tokens[0].c_str());
        }
        if (addr > fAsmEnd) {
            break;
        }
        addr -= fAsmStart;
        if (!fAsm.empty()) {
            MachineCommand& prev = fAsm.back();
            if (prev.command.isEmpty()) {
                int len = addr - prev.address;
                prev.command.printf("{ align %d bytes }", len);
            }
        }
        SkString command;
        for (size_t t = 2; t < tokens.size(); ++t) {
            command += tokens[t];
        }
        fAsm.push_back({addr, tokens[0], command, tokens[1]});
    }
    if (!fAsm.empty()) {
        MachineCommand& prev = fAsm.back();
        if (prev.command.isEmpty()) {
            int len = fInstructions.back().endCode - prev.address;
            prev.command.printf("{ align %d bytes }", len);
        }
    }
    fAsmLine = 0;
}

void Visualizer::dump(SkWStream* output, const char* code) {
    SkDebugfStream stream;
    fOutput = output ? output : &stream;
    this->parseDisassembler(output, code);
    this->dumpHead();
    for (size_t id = 0ul; id < fInstructions.size(); ++id) {
        this->dumpInstruction(id);
    }
    this->dumpTail();
}

void Visualizer::markAsDeadCode(std::vector<bool>& live, const std::vector<int>& newIds) {
    for (size_t id = 0ul; id < fInstructions.size(); ++id) {
        Instruction& instruction = fInstructions[id];
        if (instruction.instructionIndex < 0) {
            // We skip commands that are duplicates of some other commands
            // They either will be dead or alive together with the origin
            continue;
        }
        SkASSERT(instruction.instructionIndex < (int)live.size());
        if (live[instruction.instructionIndex]) {
            instruction.instructionIndex = newIds[instruction.instructionIndex];
            fToDisassembler[instruction.instructionIndex] = id;
        } else {
            instruction.kind
                    = static_cast<InstructionFlags>(instruction.kind | InstructionFlags::kDead);
            fToDisassembler[instruction.instructionIndex] = -1;
            // Anything negative meaning the command is duplicate/dead
            instruction.instructionIndex = -2;
        }
    }
}

void Visualizer::addInstructions(std::vector<skvm::Instruction>& program) {
    for (Val id = 0; id < (Val)program.size(); id++) {
        skvm::Instruction& instr = program[id];
        auto isDuplicate = instr.op == Op::duplicate;
        if (isDuplicate) {
            this->markAsDuplicate(instr.immA, id);
            instr = program[instr.immA];
        }
        this->addInstruction({
            viz::InstructionFlags::kNormal,
            /*startCode=*/0, /*endCode=0*/0,
            id,
            isDuplicate ? -1 : 0,
            instr
        });
    }
}

void Visualizer::addInstruction(Instruction skvm) {
    if (!touches_varying_memory(skvm.instruction.op)) {
        if (auto found = fIndex.find(skvm)) {
            auto& instruction = fInstructions[*found];
            ++(instruction.duplicates);
            return;
        }
    }
    fIndex.set(skvm, fInstructions.size());
    fToDisassembler.set(skvm.instructionIndex, fInstructions.size());
    fInstructions.emplace_back(std::move(skvm));
}

void Visualizer::finalize(const std::vector<skvm::Instruction>& all,
                              const std::vector<skvm::OptimizedInstruction>& optimized) {
    for (Val id = 0; id < (Val)all.size(); id++) {
        if (optimized[id].can_hoist) {
            size_t found = fToDisassembler[id];
            Instruction& instruction = fInstructions[found];
            instruction.kind =
                    static_cast<InstructionFlags>(instruction.kind | InstructionFlags::kHoisted);
        }
    }
}

void Visualizer::addMachineCommands(int id, size_t start, size_t end) {
    size_t found = fToDisassembler[id];
    Instruction& instruction = fInstructions[found];
    instruction.startCode = start;
    instruction.endCode = end;
    fAsmEnd = std::max(fAsmEnd, end);
}

SkString Visualizer::V(int reg) const {
    if (reg == -1) {
        return SkString("{optimized}");
    } else if (reg == -2) {
        return SkString("{dead code}");
    } else {
        return SkStringPrintf("v%d", reg);
    }
}

void Visualizer::formatVV(const char* op, int v1, int v2) const {
    this->writeText("%s %s, %s", op, V(v1).c_str(), V(v2).c_str());
}
void Visualizer::formatPV(const char* op, int imm, int v1) const {
    this->writeText("%s Ptr%d, %s", op, imm, V(v1).c_str());
}
void Visualizer::formatPVV(const char* op, int imm, int v1, int v2) const {
    this->writeText("%s Ptr%d, %s, %s", op, imm, V(v1).c_str(), V(v2).c_str());
}
void Visualizer::formatPVVVV(const char* op, int imm, int v1, int v2, int v3, int v4) const {
    this->writeText("%s Ptr%d, %s, %s, %s, %s",
              op, imm, V(v1).c_str(), V(v2).c_str(), V(v3).c_str(), V(v4).c_str());
}
void Visualizer::formatA_(int id, const char* op) const {
    writeText("%s = %s", V(id).c_str(), op);
}
void Visualizer::formatA_P(int id, const char* op, int imm) const {
    this->writeText("%s = %s Ptr%d", V(id).c_str(), op, imm);
}
void Visualizer::formatA_PH(int id, const char* op, int immA, int immB) const {
    this->writeText("%s = %s Ptr%d, %x", V(id).c_str(), op, immA, immB);
}
void Visualizer::formatA_PHH(int id, const char* op, int immA, int immB, int immC) const {
    this->writeText("%s = %s Ptr%d, %x, %x", V(id).c_str(), op, immA, immB, immC);
}
void Visualizer::formatA_PHV(int id, const char* op, int immA, int immB, int v) const {
    this->writeText("%s = %s Ptr%d, %x, V%d", V(id).c_str(), op, immA, immB, V(v).c_str());
}
void Visualizer::formatA_S(int id, const char* op, int imm) const {
    float f;
    memcpy(&f, &imm, 4);
    char buffer[kSkStrAppendScalar_MaxSize];
    char* stop = SkStrAppendScalar(buffer, f);
    this->writeText("%s = %s %x (", V(id).c_str(), op, imm);
    fOutput->write(buffer, stop - buffer);
    this->writeText(")");
}
void Visualizer::formatA_V(int id, const char* op, int v) const {
    this->writeText("%s = %s %s", V(id).c_str(), op, V(v).c_str());
}
void Visualizer::formatA_VV(int id, const char* op, int v1, int v2) const {
    this->writeText("%s = %s %s, %s", V(id).c_str(), op, V(v1).c_str(), V(v2).c_str());
}
void Visualizer::formatA_VVV(int id, const char* op,  int v1, int v2, int v3) const {
    this->writeText(
            "%s = %s %s, %s, %s", V(id).c_str(), op, V(v1).c_str(), V(v2).c_str(), V(v3).c_str());
}
void Visualizer::formatA_VC(int id, const char* op,  int v, int imm) const {
    this->writeText("%s = %s %s, %d", V(id).c_str(), op, V(v).c_str(), imm);
}

void Visualizer::writeText(const char* format, ...) const {
    SkString message;
    va_list argp;
    va_start(argp, format);
    message.appendVAList(format, argp);
    va_end(argp);
    fOutput->writeText(message.c_str());
}

void Visualizer::dumpInstruction(int id0) const {
    const Instruction& instruction = fInstructions[id0];
    const int id = instruction.instructionIndex;
    const int x = instruction.instruction.x,
              y = instruction.instruction.y,
              z = instruction.instruction.z,
              w = instruction.instruction.w;
    const int immA = instruction.instruction.immA,
              immB = instruction.instruction.immB,
              immC = instruction.instruction.immC;
    if (instruction.instruction.op == skvm::Op::trace_line) {
        SkASSERT(fDebugInfo != nullptr);
        SkASSERT(immA >= 0 && immB <= (int)fDebugInfo->fSource.size());
        this->writeText("<tr class='source'><td class='mask'></td><td colspan=2>// %s</td></tr>\n",
                        fDebugInfo->fSource[immB].c_str());
        return;
    } else if (instruction.instruction.op == skvm::Op::trace_var ||
               instruction.instruction.op == skvm::Op::trace_scope) {
        // TODO: We can add some visualization here
        return;
    } else if (instruction.instruction.op == skvm::Op::trace_enter) {
        SkASSERT(fDebugInfo != nullptr);
        SkASSERT(immA >= 0 && immA <= (int)fDebugInfo->fFuncInfo.size());
        std::string& func = fDebugInfo->fFuncInfo[immA].name;
        SkString mask;
        mask.printf(immC == 1 ? "%s(-1)" : "%s", V(x).c_str());
        this->writeText(
                "<tr class='source'><td class='mask'>&#8618;%s</td><td colspan=2>%s</td></tr>\n",
                mask.c_str(),
                func.c_str());
        return;
    } else if (instruction.instruction.op == skvm::Op::trace_exit) {
        SkASSERT(fDebugInfo != nullptr);
        SkASSERT(immA >= 0 && immA <= (int)fDebugInfo->fFuncInfo.size());
        std::string& func = fDebugInfo->fFuncInfo[immA].name;
        SkString mask;
        mask.printf(immC == 1 ? "%s(-1)" : "%s", V(x).c_str());
        this->writeText(
                "<tr class='source'><td class='mask'>&#8617;%s</td><td colspan=2>%s</td></tr>\n",
                mask.c_str(),
                func.c_str());
        return;
    }
    // No label, to the operation
    SkString label;
    if ((instruction.kind & InstructionFlags::kHoisted) != 0) {
        label.set("&#8593;&#8593;&#8593; ");
    }
    if (instruction.duplicates > 0) {
        label.appendf("*%d", instruction.duplicates + 1);
    }
    SkString classes = instruction.classes();
    this->writeText("<tr class='%s'><td>%s</td><td>", classes.c_str(), label.c_str());
    // Operation
    switch (instruction.instruction.op) {
        case skvm::Op::assert_true:   formatVV("assert_true", x, y);                  break;
        case skvm::Op::store8:        formatPV("store8", immA, x);                    break;
        case skvm::Op::store16:       formatPV("store16", immA, x);                   break;
        case skvm::Op::store32:       formatPV("store32", immA, x);                   break;
        case skvm::Op::store64:       formatPVV("store64", immA, x, y);               break;
        case skvm::Op::store128:      formatPVVVV("store128", immA, x, y, z, w);      break;
        case skvm::Op::index:         formatA_(id, "index");                          break;
        case skvm::Op::load8:         formatA_P(id, "load8", immA);                   break;
        case skvm::Op::load16:        formatA_P(id, "load16", immA);                  break;
        case skvm::Op::load32:        formatA_P(id, "load32", immA);                  break;
        case skvm::Op::load64:        formatA_PH(id, "load64", immA, immB);           break;
        case skvm::Op::load128:       formatA_PH(id, "load128", immA, immB);          break;
        case skvm::Op::gather8:       formatA_PHV(id, "gather8", immA, immB, x);      break;
        case skvm::Op::gather16:      formatA_PHV(id, "gather16", immA, immB, x);     break;
        case skvm::Op::gather32:      formatA_PHV(id, "gather32", immA, immB, x);     break;
        case skvm::Op::uniform32:     formatA_PH(id, "uniform32", immA, immB);        break;
        case skvm::Op::array32:       formatA_PHH(id, "array32", immA, immB, immC);   break;
        case skvm::Op::splat:         formatA_S(id, "splat", immA);                   break;
        case skvm::Op:: add_f32:      formatA_VV(id, "add_f32", x, y);                break;
        case skvm::Op:: sub_f32:      formatA_VV(id, "sub_f32", x, y);                break;
        case skvm::Op:: mul_f32:      formatA_VV(id, "mul_f32", x, y);                break;
        case skvm::Op:: div_f32:      formatA_VV(id, "div_f32", x, y);                break;
        case skvm::Op:: min_f32:      formatA_VV(id, "min_f32", x, y);                break;
        case skvm::Op:: max_f32:      formatA_VV(id, "max_f32", x, y);                break;
        case skvm::Op:: fma_f32:      formatA_VVV(id, "fma_f32", x, y, z);            break;
        case skvm::Op:: fms_f32:      formatA_VVV(id, "fms_f32", x, y, z);            break;
        case skvm::Op::fnma_f32:      formatA_VVV(id, "fnma_f32", x, y, z);           break;
        case skvm::Op::sqrt_f32:      formatA_V(id, "sqrt_f32", x);                   break;
        case skvm::Op:: eq_f32:       formatA_VV(id, "eq_f32", x, y);                 break;
        case skvm::Op::neq_f32:       formatA_VV(id, "neq_f32", x, y);                break;
        case skvm::Op:: gt_f32:       formatA_VV(id, "gt_f32", x, y);                 break;
        case skvm::Op::gte_f32:       formatA_VV(id, "gte_f32", x, y);                break;
        case skvm::Op::add_i32:       formatA_VV(id, "add_i32", x, y);                break;
        case skvm::Op::sub_i32:       formatA_VV(id, "sub_i32", x, y);                break;
        case skvm::Op::mul_i32:       formatA_VV(id, "mul_i32", x, y);                break;
        case skvm::Op::shl_i32:       formatA_VC(id, "shl_i32", x, immA);             break;
        case skvm::Op::shr_i32:       formatA_VC(id, "shr_i32", x, immA);             break;
        case skvm::Op::sra_i32:       formatA_VC(id, "sra_i32", x, immA);             break;
        case skvm::Op::eq_i32:        formatA_VV(id, "eq_i32", x, y);                 break;
        case skvm::Op::gt_i32:        formatA_VV(id, "gt_i32", x, y);                 break;
        case skvm::Op::bit_and:       formatA_VV(id, "bit_and", x, y);                break;
        case skvm::Op::bit_or:        formatA_VV(id, "bit_or", x, y);                 break;
        case skvm::Op::bit_xor:       formatA_VV(id, "bit_xor", x, y);                break;
        case skvm::Op::bit_clear:     formatA_VV(id, "bit_clear", x, y);              break;
        case skvm::Op::select:        formatA_VVV(id, "select", x, y, z);             break;
        case skvm::Op::ceil:          formatA_V(id, "ceil", x);                       break;
        case skvm::Op::floor:         formatA_V(id, "floor", x);                      break;
        case skvm::Op::to_f32:        formatA_V(id, "to_f32", x);                     break;
        case skvm::Op::to_fp16:       formatA_V(id, "to_fp16", x);                    break;
        case skvm::Op::from_fp16:     formatA_V(id, "from_fp16", x);                  break;
        case skvm::Op::trunc:         formatA_V(id, "trunc", x);                      break;
        case skvm::Op::round:         formatA_V(id, "round", x);                      break;
        default: SkASSERT(false);
    }
    // Generation
    if ((instruction.kind & InstructionFlags::kDead) == 0) {
        struct Compare
        {
            bool operator() (const MachineCommand& c, std::pair<size_t, size_t> p) const
                            { return c.address < p.first; }
            bool operator() (std::pair<size_t, size_t> p, const MachineCommand& c) const
                            { return p.second <= c.address; }
        };

        std::pair<size_t, size_t> range(instruction.startCode, instruction.endCode);
        auto commands = std::equal_range(fAsm.begin(), fAsm.end(), range, Compare{ });
        for (const MachineCommand* line = commands.first; line != commands.second; ++line) {
            this->writeText("</td></tr>\n<tr class='machine'><td>%s</td><td colspan='2'>%s",
                            line->label.c_str(),
                            line->command.c_str());
        }
        fAsmLine = commands.second - fAsm.begin();
    }
    this->writeText("</td></tr>\n");
}

void Visualizer::dumpHead() const {
    this->writeText(
    "<html>\n"
    "<head>\n"
    "   <title>SkVM Disassembler Output</title>\n"
    "   <style>\n"
    "   button { border-style: none; font-size: 10px; background-color: lightpink; }\n"
    "   table { text-align: left; }\n"
    "   table th { background-color: lightgray; }\n"
    "   .dead, .dead1 { color: lightgray; text-decoration: line-through; }\n"
    "   .normal, .normal1 { }\n"
    "   .origin, .origin1 { font-weight: bold; }\n"
    "   .source, .source1 { color: darkblue; }\n"
    "   .mask, .mask1 { color: green; }\n"
    "   .comments, .comments1 { }\n"
    "   .machine, .machine1 { color: lightblue; }\n"
    "   </style>\n"
    "    <script>\n"
    "    function initializeButton(className) {\n"
    "      var btn = document.getElementById(className);\n"
    "      var elems = document.getElementsByClassName(className);\n"
    "      if (elems == undefined || elems.length == 0) {\n"
    "        btn.disabled = true;\n"
    "        btn.innerText = \"None\";\n"
    "        btn.style.background = \"lightgray\";\n"
    "        return;\n"
    "      }\n"
    "    };\n"
    "    function initialize() {\n"
    "      initializeButton('normal');\n"
    "      initializeButton('source');\n"
    "      initializeButton('dead');\n"
    "      initializeButton('machine');\n"
    "    };\n"
    "  </script>\n"
    "</head>\n"
    "<body onload='initialize();'>\n"
    "    <script>\n"
    "    function toggle(btn, className) {\n"
    "      var elems = document.getElementsByClassName(className);\n"
    "      for (var i = 0; i < elems.length; i++) {\n"
    "        var elem = elems.item(i);\n"
    "        if (elem.style.display === \"\") {\n"
    "            elem.style.display = \"none\";\n"
    "            btn.innerText = \"Show\";\n"
    "            btn.style.background = \"lightcyan\";\n"
    "        } else {\n"
    "            elem.style.display = \"\";\n"
    "            btn.innerText = \"Hide\";\n"
    "            btn.style.background = \"lightpink\";\n"
    "        }\n"
    "      }\n"
    "    };\n"
    "    </script>"
    "    <table border=\"0\" style='font-family:\"monospace\"; font-size: 10px;'>\n"
    "     <caption style='font-family:Roboto; font-size:15px; text-align:left;'>Legend</caption>\n"
    "     <tr>\n"
    "        <th style=\"min-width:100px;\"><u>Kind</u></th>\n"
    "        <th style=\"width:35%;\"><u>Example</u></th>\n"
    "        <th style=\"width: 5%; min-width:50px;\"><u></u></th>\n"
    "        <th style=\"width:60%;\"><u>Description</u></th>\n"
    "     </tr>\n"
    "      <tr class='normal1'>"
            "<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>"
            "<td>v1 = load32 Ptr1</td>"
            "<td><button id='normal' onclick=\"toggle(this, 'normal')\">Hide</button></td>"
            "<td>A regular SkVM command</td></tr>\n"
    "      <tr class='normal1 origin1'><td>*{N}</td>"
            "<td>v9 = gt_f32 v0, v1</td>"
            "<td><button id='dead' onclick=\"toggle(this, 'deduped')\">Hide</button></td>"
            "<td>A {N} times deduped SkVM command</td></tr>\n"
    "      <tr class='normal1'><td>&#8593;&#8593;&#8593; &nbsp;&nbsp;&nbsp;</td>"
            "<td>v22 = splat 3f800000 (1)</td><td></td>"
            "<td>A hoisted SkVM command</td></tr>\n"
    "      <tr class='source1'><td class='mask'>mask&#8618;v{N}(-1)</td>"
            "<td>// C++ source line</td><td></td>"
            "<td>Enter into the procedure with mask v{N} (which has a constant value -1)"
            "</td></tr>\n"
    "      <tr class='source1'><td class='mask'>mask&#8617;v{N}</td>"
            "<td>// C++ source line</td><td>"
            "</td><td>Exit the procedure with mask v{N}</td></tr>\n"
    "      <tr class='source1'><td class='mask'></td><td>// C++ source line</td>"
            "<td><button id='source' onclick=\"toggle(this, 'source')\">Hide</button></td>"
            "<td>Line trace back to C++ code</td></tr>\n"
    "      <tr class='dead1'><td></td><td>{dead code} = mul_f32 v1, v18</td>"
            "<td><button id='dead' onclick=\"toggle(this, 'dead')\">Hide</button></td>"
            "<td>An eliminated \"dead code\" SkVM command</td></tr>\n"
    "      <tr class='machine1'><td>{address}</td><td>vmovups (%rsi),%ymm0</td>"
            "<td><button id='machine' onclick=\"toggle(this, 'machine')\">Hide</button></td>"
            "<td>A disassembled machine command generated by SkVM command</td></tr>\n"
    "    </table>\n"
    "    <table border = \"0\"style='font-family:\"monospace\"; font-size: 10px;'>\n"
    "     <caption style='font-family:Roboto;font-size:15px;text-align:left;'>SkVM Code</caption>\n"
    "     <tr>\n"
    "        <th style=\"min-width:100px;\"><u>Kind</u></th>\n"
    "        <th style=\"width:40%;min-width:100px;\"><u>Command</u></th>\n"
    "        <th style=\"width:60%;\"><u>Comments</u></th>\n"
    "     </tr>");
}
void Visualizer::dumpTail() const {
    this->writeText(
    "      </table>\n"
            "</body>\n"
            "</html>"
    );
}
} // namespace skvm::viz

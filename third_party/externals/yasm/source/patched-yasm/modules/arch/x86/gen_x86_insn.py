#! /usr/bin/env python
# x86 instructions and prefixes data and code generation
#
#  Copyright (C) 2002-2007  Peter Johnson
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# NOTE: operands are arranged in NASM / Intel order (e.g. dest, src)

import os
import sys

from sys import stdout, version_info

scriptname = "gen_x86_insn.py"
scriptrev = "HEAD"

ordered_cpus = [
    "086", "186", "286", "386", "486", "586", "686", "K6", "Athlon", "P3",
    "P4", "IA64", "Hammer"]
ordered_cpu_features = [
    "FPU", "Cyrix", "AMD", "MMX", "3DNow", "SMM", "SSE", "SSE2",
    "SSE3", "SVM", "PadLock", "SSSE3", "SSE41", "SSE42", "SSE4a", "SSE5",
    "AVX", "FMA", "AES", "CLMUL", "MOVBE", "XOP", "FMA4", "F16C",
    "FSGSBASE", "RDRAND", "XSAVEOPT", "EPTVPID", "SMX", "AVX2", "BMI1",
    "BMI2", "INVPCID", "LZCNT"]
unordered_cpu_features = ["Priv", "Prot", "Undoc", "Obs"]

# Predefined VEX prefix field values
VEXW0 = 0xC0
VEXW1 = 0xC8
VEXL0 = 0xC0
VEXL1 = 0xC4
VEXpp = 0xC0    # OR with value

# Predefined XOP prefix field values
XOPW0 = 0x80
XOPW1 = 0x88
XOPL0 = 0x80
XOPL1 = 0x84
XOPpp = 0x80    # OR with value

def lprint(s, f = stdout, e = '\n') :
    f.write(s + e)

def cpu_lcd(cpu1, cpu2):
    """Find the lowest common denominator of two CPU sets."""
    retval = set()

    # CPU
    cpu1cpus = set(ordered_cpus) & set(cpu1)
    if not cpu1cpus:
        cpu1cpus.add("086")
    cpu1mincpu = min(ordered_cpus.index(x) for x in cpu1cpus)
    cpu2cpus = set(ordered_cpus) & set(cpu2)
    if not cpu2cpus:
        cpu2cpus.add("086")
    cpu2mincpu = min(ordered_cpus.index(x) for x in cpu1cpus)
    cpumin = ordered_cpus[min(cpu1mincpu, cpu2mincpu)]
    if cpumin == "086":
        cpumin = "Any"

    if cpumin != "Any":
        retval.add(cpumin)

    # Feature
    cpu1features = set(ordered_cpu_features) & set(cpu1)
    if not cpu1features:
        cpu1minfeature = -1
    else:
        cpu1minfeature = min(ordered_cpu_features.index(x)
                             for x in cpu1features)

    cpu2features = set(ordered_cpu_features) & set(cpu2)
    if not cpu2features:
        cpu2minfeature = -1
    else:
        cpu2minfeature = min(ordered_cpu_features.index(x)
                             for x in cpu2features)

    if cpu1minfeature != -1 and cpu2minfeature != -1:
        featuremin = ordered_cpu_features[min(cpu1minfeature, cpu2minfeature)]
        retval.add(featuremin)

    # Unordered features
    for feature in set(unordered_cpu_features) & set(cpu1) & set(cpu2):
        retval.add(feature)

    return retval

class Operand(object):
    def __init__(self, **kwargs):
        self.type = kwargs.pop("type")
        self.size = kwargs.pop("size", "Any")
        self.relaxed = kwargs.pop("relaxed", False)
        self.dest = kwargs.pop("dest", None)
        self.tmod = kwargs.pop("tmod", None)
        self.opt = kwargs.pop("opt", None)

        if kwargs:
            for arg in kwargs:
                lprint("Warning: unrecognized arg %s" % arg)

    def __str__(self):
        return "{"+ ", ".join(["OPT_%s" % self.type,
                               "OPS_%s" % self.size,
                               "%d" % self.relaxed,
                               self.dest == "EA64" and "1" or "0",
                               "OPTM_%s" % self.tmod,
                               "OPA_%s" % (self.dest == "EA64"
                                           and "EA" or self.dest),
                               "OPAP_%s" % self.opt]) + "}"

    def __eq__(self, other):
        return (self.type == other.type and
                self.size == other.size and
                self.relaxed == other.relaxed and
                self.dest == other.dest and
                self.tmod == other.tmod and
                self.opt == other.opt)

    def __ne__(self, other):
        return (self.type != other.type or
                self.size != other.size or
                self.relaxed != other.relaxed or
                self.dest != other.dest or
                self.tmod != other.tmod or
                self.opt != other.opt)

class GroupForm(object):
    def __init__(self, **kwargs):
        # Parsers
        self.parsers = set(kwargs.pop("parsers", ["gas", "nasm"]))

        # CPU feature flags initialization
        self.cpu = set(kwargs.pop("cpu", []))

        # Misc flags
        self.misc_flags = set(kwargs.pop("misc_flags", []))
        if kwargs.pop("only64", False):
            self.misc_flags.add("ONLY_64")
        if kwargs.pop("not64", False):
            self.misc_flags.add("NOT_64")
        if kwargs.pop("onlyavx", False):
            self.misc_flags.add("ONLY_AVX")
        if kwargs.pop("notavx", False):
            self.misc_flags.add("NOT_AVX")

        # Operation size
        self.opersize = kwargs.pop("opersize", 0)
        if self.opersize == 8:
            self.opersize = 0

        if self.opersize == 64:
            self.misc_flags.add("ONLY_64")
        elif self.opersize == 32 and "ONLY_64" not in self.misc_flags:
            self.cpu.add("386")

        # Default operation size in 64-bit mode
        self.def_opersize_64 = kwargs.pop("def_opersize_64", 0)

        # GAS suffix
        self.gen_suffix = kwargs.pop("gen_suffix", True)
        self.suffixes = kwargs.pop("suffixes", None)
        suffix = kwargs.pop("suffix", None)
        if suffix is not None:
            self.suffixes = [suffix]

        req_suffix = kwargs.pop("req_suffix", False)
        if not req_suffix:
            if self.suffixes is None:
                self.suffixes = ["Z"]
            else:
                self.suffixes.append("Z")

        if self.suffixes is not None:
            self.suffixes = set(x.upper() for x in self.suffixes)

        # Special instruction prefix
        self.special_prefix = "0"
        if "prefix" in kwargs:
            self.special_prefix = "0x%02X" % kwargs.pop("prefix")

        # VEX prefix
        if "vex" in kwargs:
            self.misc_flags.add("ONLY_AVX")
            vexW = kwargs.pop("vexw", 0)
            if vexW not in [0, 1]:
                raise ValueError("VEX.W must be 0 or 1")

            vexL = kwargs.pop("vex")
            if vexL == 128 or vexL == 0:
                vexL = 0
            elif vexL == 256:
                vexL = 1
            else:
                raise ValueError("VEX.L must be 128 or 256")

            if self.special_prefix in ["0", "0x00"]:
                vexpp = 0
            elif self.special_prefix == "0x66":
                vexpp = 1
            elif self.special_prefix == "0xF3":
                vexpp = 2
            elif self.special_prefix == "0xF2":
                vexpp = 3
            else:
                raise ValueError("Cannot combine VEX and special prefix %s"
                                 % self.special_prefix)

            self.special_prefix = "0x%02X" % (0xC0 + vexW*8 + vexL*4 + vexpp)

        # XOP prefix
        if "xop" in kwargs:
            xopW = kwargs.pop("xopw", 0)
            if xopW not in [0, 1]:
                raise ValueError("XOP.W must be 0 or 1")

            xopL = kwargs.pop("xop")
            if xopL == 128 or xopL == 0:
                xopL = 0
            elif xopL == 256:
                xopL = 1
            else:
                raise ValueError("XOP.L must be 128 or 256")

            # XOPpp is currently reserved (0)
            xoppp = 0
            if self.special_prefix not in ["0", "0x00"]:
                raise ValueError("Cannot combine XOP and special prefix %s"
                                 % self.special_prefix)
            self.special_prefix = "0x%02X" % (0x80 + xopW*8 + xopL*4 + xoppp)

        # Spare value
        self.spare = kwargs.pop("spare", 0)

        # Build opcodes string (C array initializer)
        if "opcode" in kwargs:
            # Usual case, just a single opcode
            self.opcode = kwargs.pop("opcode")
            self.opcode_len = len(self.opcode)
        elif "opcode1" in kwargs and "opcode2" in kwargs:
            # Two opcode case; the first opcode is the "optimized" opcode,
            # the second is the "relaxed" opcode.  For this to work, an
            # opt="foo" must be set for one of the operands.
            self.opcode1 = kwargs.pop("opcode1")
            self.opcode2 = kwargs.pop("opcode2")
            self.opcode_len = len(self.opcode1)
        else:
            raise KeyError("missing opcode")

        # Build operands string (C array initializer)
        self.operands = kwargs.pop("operands")
        for op in self.operands:
            if op.type in ["Reg", "RM", "Areg", "Creg", "Dreg"]:
                if op.size == 64:
                    self.misc_flags.add("ONLY_64")
                elif op.size == 32 and "ONLY_64" not in self.misc_flags:
                    self.cpu.add("386")
            if op.type in ["Imm", "ImmNotSegOff"]:
                if op.size == 64:
                    self.misc_flags.add("ONLY_64")
                elif op.size == 32 and "ONLY_64" not in self.misc_flags:
                    self.cpu.add("386")
            if op.type in ["FS", "GS"] and "ONLY_64" not in self.misc_flags:
                self.cpu.add("386")
            if op.type in ["CR4"] and "ONLY_64" not in self.misc_flags:
                self.cpu.add("586")
            if op.dest == "EA64":
                self.misc_flags.add("ONLY_64")

        # Modifiers
        self.modifiers = kwargs.pop("modifiers", [])

        # GAS flags
        self.gas_only = ("nasm" not in self.parsers)
        self.gas_illegal = ("gas" not in self.parsers)
        self.gas_no_rev = (kwargs.pop("gas_no_reverse", False) or
                           kwargs.pop("gas_no_rev", False))

        # CPU feature flags finalization
        # Remove redundancies
        maxcpu = -1
        maxcpu_set = self.cpu & set(ordered_cpus)
        if maxcpu_set:
            maxcpu = max(ordered_cpus.index(x) for x in maxcpu_set)
        if maxcpu != -1:
            for cpu in ordered_cpus[0:maxcpu]:
                self.cpu.discard(cpu)

        if kwargs:
            for arg in kwargs:
                lprint("Warning: unrecognized arg %s" % arg)

    def __str__(self):
        if hasattr(self, "opcode"):
            opcodes_str = ["0x%02X" % x for x in self.opcode]
        elif hasattr(self, "opcode1") and hasattr(self, "opcode2"):
            opcodes_str = ["0x%02X" % x for x in self.opcode1]
            opcodes_str.extend("0x%02X" % x for x in self.opcode2)
        # Ensure opcodes initializer string is 3 long
        opcodes_str.extend(["0", "0", "0"])
        opcodes_str = "{" + ', '.join(opcodes_str[0:3]) + "}"

        cpus_str = "|".join("CPU_%s" % x for x in sorted(self.cpu))

        if len(self.modifiers) > 3:
            raise ValueError("too many modifiers: %s" % (self.modifiers,))

        cpus_str = []
        if self.cpu is not None:
            if len(self.cpu) > 3:
                raise ValueError("too many CPUs: %s" % (self.cpu,))

            # Ensure CPUs initializer string is at least 3 long
            cpus_str.extend("CPU_%s" % x for x in sorted(self.cpu))

        # Ensure cpus initializer string is 3 long; 0=CPU_Any
        cpus_str.extend(["0", "0", "0"])


        mods = ["MOD_%s" % x for x in self.modifiers]
        # Ensure mods initializer string is 3 long
        mods.extend(["0", "0", "0"])
        mod_str = "{" + ', '.join(mods[0:3]) + "}"

        gas_flags = []
        if self.gas_only:
            gas_flags.append("GAS_ONLY")
        if self.gas_illegal:
            gas_flags.append("GAS_ILLEGAL")
        if self.gas_no_rev:
            gas_flags.append("GAS_NO_REV")
        if self.suffixes:
            gas_flags.extend("SUF_%s" % x for x in sorted(self.suffixes))
        gas_flags = "|".join(gas_flags)

        # Build instruction info structure initializer
        return "{ "+ ", ".join([gas_flags or "0",
                                "|".join(self.misc_flags) or "0",
                                cpus_str[0],
                                cpus_str[1],
                                cpus_str[2],
                                mod_str,
                                "%d" % (self.opersize or 0),
                                "%d" % (self.def_opersize_64 or 0),
                                self.special_prefix or "0",
                                "%d" % self.opcode_len,
                                opcodes_str,
                                "%d" % (self.spare or 0),
                                "%d" % len(self.operands),
                                "%d" % self.all_operands_index]) + " }"

groups = {}
groupnames_ordered = []
def add_group(name, **kwargs):
    forms = groups.setdefault(name, [])
    forms.append(GroupForm(**kwargs))
    groupnames_ordered.append(name)

class Insn(object):
    def __init__(self, groupname, suffix=None, parser=None, modifiers=None,
                 cpu=None, misc_flags=None, only64=False, not64=False,
                 avx=False):
        self.groupname = groupname
        if suffix is None:
            self.suffix = None
        else:
            self.suffix = suffix.upper()

        self.parsers = None
        if suffix is not None:
            self.parsers = set(["gas"])
        if parser is not None:
            self.parsers = set([parser])

        if modifiers is None:
            self.modifiers = []
        else:
            self.modifiers = modifiers
        if cpu is None:
            self.cpu = None
        else:
            self.cpu = set(cpu)

        if misc_flags is None:
            self.misc_flags = None
        else:
            self.misc_flags = set([x for x in misc_flags])

        if only64:
            if self.misc_flags is None:
                self.misc_flags = set()
            self.misc_flags.add("ONLY_64")
        if not64:
            if self.misc_flags is None:
                self.misc_flags = set()
            self.misc_flags.add("NOT_64")
        if avx:
            if self.misc_flags is None:
                self.misc_flags = set()
            self.misc_flags.add("ONLY_AVX")
            if self.cpu is None:
                self.cpu = set(["AVX"])

    def auto_cpu(self, parser):
        """Determine lowest common denominator CPU from group and suffix.
        Does nothing if CPU is already set."""
        if self.cpu is not None:
            return
        # Scan through group, matching parser and suffix
        for form in groups[self.groupname]:
            if parser not in form.parsers:
                continue
            if (self.suffix is not None and len(self.suffix) == 1 and
                (form.suffixes is None or self.suffix not in form.suffixes)):
                continue
            if self.cpu is None:
                self.cpu = set(form.cpu)
            else:
                self.cpu = cpu_lcd(self.cpu, form.cpu)

    def auto_misc_flags(self, parser):
        """Determine lowest common denominator flags from group and suffix.
        Does nothing if flags is already set."""
        if self.misc_flags is not None:
            return
        # Scan through group, matching parser and suffix
        for form in groups[self.groupname]:
            if parser not in form.parsers:
                continue
            if (self.suffix is not None and len(self.suffix) == 1 and
                (form.suffixes is None or self.suffix not in form.suffixes)):
                continue
            if self.misc_flags is None:
                self.misc_flags = set(form.misc_flags)
            else:
                self.misc_flags &= form.misc_flags

    def copy(self):
        """Return a shallow copy."""
        return Insn(self.groupname,
                    suffix=self.suffix,
                    modifiers=self.modifiers,
                    cpu=self.cpu,
                    misc_flags=self.misc_flags)

    def __str__(self):
        if self.suffix is None:
            suffix_str = "SUF_Z"
        elif len(self.suffix) == 1:
            suffix_str = "SUF_" + self.suffix
        else:
            suffix_str = self.suffix

        cpus_str = []
        if self.cpu is not None:
            if len(self.cpu) > 3:
                raise ValueError("too many CPUs: %s" % (self.cpu,))
            cpus_str.extend("CPU_%s" % x for x in sorted(self.cpu))

        # Ensure cpus initializer string is 3 long
        cpus_str.extend(["0", "0", "0"])

        if len(self.modifiers) > 3:
            raise ValueError("too many modifiers")
        mods_str = ["0x%02X" % x for x in self.modifiers]

        # Ensure modifiers is at least 3 long
        mods_str.extend(["0", "0", "0"])

        return ",\t".join(["%s_insn" % self.groupname,
                           "%d" % len(groups[self.groupname]),
                           suffix_str,
                           mods_str[0],
                           mods_str[1],
                           mods_str[2],
                           "|".join(self.misc_flags or []) or "0",
                           cpus_str[0],
                           cpus_str[1],
                           cpus_str[2]])

insns = {}
def add_insn(name, groupname, **kwargs):
    opts = insns.setdefault(name, [])
    opts.append(Insn(groupname, **kwargs))

class Prefix(object):
    def __init__(self, groupname, value, only64=False):
        self.groupname = groupname
        self.value = value
        self.only64 = only64

    def __str__(self):
        return ",\t".join(["NULL",
                           "X86_%s>>8" % self.groupname,
                           "0x%02X" % self.value,
                           "0",
                           "0",
                           "0",
                           self.only64 and "ONLY_64" or "0",
                           "0",
                           "0",
                           "0"])

gas_insns = {}
nasm_insns = {}

def add_prefix(name, groupname, value, parser=None, **kwargs):
    prefix = Prefix(groupname, value, **kwargs)
    if parser is None or parser == "gas":
        gas_insns[name] = prefix
    if parser is None or parser == "nasm":
        nasm_insns[name] = prefix

def finalize_insns():
    unused_groups = set(groups.keys())
    for name in insns:
        for insn in insns[name]:
            group = groups[insn.groupname]
            unused_groups.discard(insn.groupname)

            parsers = set()
            for form in group:
                parsers |= form.parsers
            if insn.parsers is not None:
                parsers &= insn.parsers

            if "gas" in parsers:
                suffixes = set()
                if insn.suffix is None:
                    for form in group:
                        if form.gen_suffix and form.suffixes is not None:
                            suffixes |= form.suffixes

                if not suffixes:
                    suffixes.add("Z")
                for suffix in suffixes:
                    if suffix == "Z":
                        keyword = name
                    else:
                        keyword = name+suffix
                    keyword = keyword.lower()
                    if keyword in gas_insns:
                        raise ValueError("duplicate gas instruction %s" %
                                         keyword)
                    newinsn = insn.copy()
                    if insn.suffix is None:
                        newinsn.suffix = suffix
                    newinsn.auto_cpu("gas")
                    newinsn.auto_misc_flags("gas")
                    gas_insns[keyword] = newinsn

            if "nasm" in parsers:
                keyword = name
                if keyword in nasm_insns:
                    raise ValueError("duplicate nasm instruction %s" % keyword)
                newinsn = insn.copy()
                newinsn.auto_cpu("nasm")
                newinsn.auto_misc_flags("nasm")
                nasm_insns[keyword] = newinsn

    unused_groups.discard("empty")
    unused_groups.discard("not64")
    if unused_groups:
        lprint("warning: unused groups: %s" % ", ".join(unused_groups))

def output_insns(f, parser, insns):
    lprint("/* Generated by %s r%s, do not edit */" % \
        (scriptname, scriptrev), f)
    lprint("""%%ignore-case
%%language=ANSI-C
%%compare-strncmp
%%readonly-tables
%%enum
%%struct-type
%%define hash-function-name insnprefix_%s_hash
%%define lookup-function-name insnprefix_%s_find
struct insnprefix_parse_data;
%%%%""" % (parser, parser), f)
    for keyword in sorted(insns):
        lprint("%s,\t%s" % (keyword.lower(), insns[keyword]), f)

def output_gas_insns(f):
    output_insns(f, "gas", gas_insns)

def output_nasm_insns(f):
    output_insns(f, "nasm", nasm_insns)

def output_groups(f):
    # Merge all operand lists into single list
    # Sort by number of operands to shorten output
    all_operands = []
    if version_info[0] == 2:
        gi = groups.itervalues()
    else:
        gi = groups.values()
    for form in sorted((form for g in gi for form in g),
                       key=lambda x:len(x.operands), reverse=True):
        num_operands = len(form.operands)
        for i in range(len(all_operands)):
            if all_operands[i:i+num_operands] == form.operands:
                form.all_operands_index = i
                break
        else:
            form.all_operands_index = len(all_operands)
            all_operands.extend(form.operands)

    # Output operands list
    lprint("/* Generated by %s r%s, do not edit */" % \
        (scriptname, scriptrev), f)
    lprint("static const x86_info_operand insn_operands[] = {", f)
    lprint("   ", f, '')
    lprint(",\n    ".join(str(x) for x in all_operands), f)
    lprint("};\n", f)

    # Output groups
    seen = set()
    for name in groupnames_ordered:
        if name in seen:
            continue
        seen.add(name)
        lprint("static const x86_insn_info %s_insn[] = {" % name, f)
        lprint("   ", f, '')
        lprint(",\n    ".join(str(x) for x in groups[name]), f)
        lprint("};\n", f)

#####################################################################
# General instruction groupings
#####################################################################

#
# Empty instruction
#
add_group("empty", opcode=[], operands=[])

#
# Placeholder for instructions invalid in 64-bit mode
#
add_group("not64", opcode=[], operands=[], not64=True)

#
# One byte opcode instructions with no operands
#
add_group("onebyte",
    modifiers=["Op0Add", "OpSizeR", "DOpS64R"],
    opcode=[0x00],
    operands=[])

#
# One byte opcode instructions with "special" prefix with no operands
#
add_group("onebyte_prefix",
    modifiers=["PreAdd", "Op0Add"],
    prefix=0x00,
    opcode=[0x00],
    operands=[])

#
# Two byte opcode instructions with no operands
#
add_group("twobyte",
    gen_suffix=False,
    suffixes=["l", "q"],
    modifiers=["Op0Add", "Op1Add"],
    opcode=[0x00, 0x00],
    operands=[])

#
# Three byte opcode instructions with no operands
#
add_group("threebyte",
    modifiers=["Op0Add", "Op1Add", "Op2Add"],
    opcode=[0x00, 0x00, 0x00],
    operands=[])

#
# One byte opcode instructions with general memory operand
#
add_group("onebytemem",
    gen_suffix=False,
    suffixes=["l", "q", "s"],
    modifiers=["SpAdd", "Op0Add"],
    opcode=[0x00],
    spare=0,
    operands=[Operand(type="Mem", dest="EA")])

#
# Two byte opcode instructions with general memory operand
#
add_group("twobytemem",
    gen_suffix=False,
    suffixes=["w", "l", "q", "s"],
    modifiers=["SpAdd", "Op0Add", "Op1Add"],
    opcode=[0x00, 0x00],
    spare=0,
    operands=[Operand(type="Mem", relaxed=True, dest="EA")])

#
# mov
#

# Absolute forms for non-64-bit mode
for sfx, sz in zip("bwl", [8, 16, 32]):
    add_group("mov",
        suffix=sfx,
        not64=True,
        opersize=sz,
        opcode=[0xA0+(sz!=8)],
        operands=[Operand(type="Areg", size=sz, dest=None),
                  Operand(type="MemOffs", size=sz, relaxed=True, dest="EA")])

for sfx, sz in zip("bwl", [8, 16, 32]):
    add_group("mov",
        suffix=sfx,
        not64=True,
        opersize=sz,
        opcode=[0xA2+(sz!=8)],
        operands=[Operand(type="MemOffs", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Areg", size=sz, dest=None)])

# 64-bit absolute forms for 64-bit mode.  Disabled for GAS, see movabs
for sz in (8, 16, 32, 64):
    add_group("mov",
        opersize=sz,
        opcode=[0xA0+(sz!=8)],
        only64=True,
        operands=[Operand(type="Areg", size=sz, dest=None),
                  Operand(type="MemOffs", size=sz, relaxed=True, dest="EA64")])

for sz in (8, 16, 32, 64):
    add_group("mov",
        only64=True,
        opersize=sz,
        opcode=[0xA2+(sz!=8)],
        operands=[Operand(type="MemOffs", size=sz, relaxed=True, dest="EA64"),
                  Operand(type="Areg", size=sz, dest=None)])

# General 32-bit forms using Areg / short absolute option
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("mov",
        suffix=sfx,
        opersize=sz,
        opcode1=[0x88+(sz!=8)],
        opcode2=[0xA2+(sz!=8)],
        operands=[
            Operand(type="RM", size=sz, relaxed=True, dest="EA", opt="ShortMov"),
            Operand(type="Areg", size=sz, dest="Spare")])

# General 32-bit forms
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("mov",
        suffix=sfx,
        opersize=sz,
        opcode=[0x88+(sz!=8)],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="Spare")])

# General 32-bit forms using Areg / short absolute option
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("mov",
        suffix=sfx,
        opersize=sz,
        opcode1=[0x8A+(sz!=8)],
        opcode2=[0xA0+(sz!=8)],
        operands=[Operand(type="Areg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA",
                          opt="ShortMov")])

# General 32-bit forms
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("mov",
        suffix=sfx,
        opersize=sz,
        opcode=[0x8A+(sz!=8)],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA")])

# Segment register forms
add_group("mov",
    suffix="w",
    opcode=[0x8C],
    operands=[Operand(type="Mem", size=16, relaxed=True, dest="EA"),
              Operand(type="SegReg", size=16, relaxed=True, dest="Spare")])
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("mov",
        suffix=sfx,
        opersize=sz,
        opcode=[0x8C],
        operands=[
            Operand(type="Reg", size=sz, dest="EA"),
            Operand(type="SegReg", size=16, relaxed=True, dest="Spare")])
add_group("mov",
    suffix="w",
    opcode=[0x8E],
    operands=[Operand(type="SegReg", size=16, relaxed=True, dest="Spare"),
              Operand(type="RM", size=16, relaxed=True, dest="EA")])
for sfx, sz in zip("lq", [32, 64]):
    add_group("mov",
        suffix=sfx,
        opcode=[0x8E],
        operands=[
            Operand(type="SegReg", size=16, relaxed=True, dest="Spare"),
            Operand(type="Reg", size=sz, dest="EA")])

# Immediate forms
add_group("mov",
    suffix="b",
    opcode=[0xB0],
    operands=[Operand(type="Reg", size=8, dest="Op0Add"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
for sfx, sz in zip("wl", [16, 32]):
    add_group("mov",
        suffix=sfx,
        opersize=sz,
        opcode=[0xB8],
        operands=[Operand(type="Reg", size=sz, dest="Op0Add"),
                  Operand(type="Imm", size=sz, relaxed=True, dest="Imm")])
# 64-bit forced size form
add_group("mov",
    parsers=["nasm"],
    opersize=64,
    opcode=[0xB8],
    operands=[Operand(type="Reg", size=64, dest="Op0Add"),
              Operand(type="Imm", size=64, dest="Imm")])
add_group("mov",
    suffix="q",
    opersize=64,
    opcode1=[0xB8],
    opcode2=[0xC7],
    operands=[Operand(type="Reg", size=64, dest="Op0Add"),
              Operand(type="Imm", size=64, relaxed=True, dest="Imm",
                      opt="SImm32Avail")])
# Need two sets here, one for strictness on left side, one for right.
for sfx, sz, immsz in zip("bwlq", [8, 16, 32, 64], [8, 16, 32, 32]):
    add_group("mov",
        suffix=sfx,
        opersize=sz,
        opcode=[0xC6+(sz!=8)],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Imm", size=immsz, dest="Imm")])
for sfx, sz, immsz in zip("bwlq", [8, 16, 32, 64], [8, 16, 32, 32]):
    add_group("mov",
            suffix=sfx,
            opersize=sz,
            opcode=[0xC6+(sz!=8)],
            operands=[Operand(type="RM", size=sz, dest="EA"),
                Operand(type="Imm", size=immsz, relaxed=True, dest="Imm")])

# CR forms
add_group("mov",
    suffix="l",
    not64=True,
    cpu=["Priv"],
    opcode=[0x0F, 0x22],
    operands=[Operand(type="CR4", size=32, dest="Spare"),
              Operand(type="Reg", size=32, dest="EA")])
add_group("mov",
    suffix="l",
    not64=True,
    cpu=["Priv"],
    opcode=[0x0F, 0x22],
    operands=[Operand(type="CRReg", size=32, dest="Spare"),
              Operand(type="Reg", size=32, dest="EA")])
add_group("mov",
    suffix="q",
    cpu=["Priv"],
    opcode=[0x0F, 0x22],
    operands=[Operand(type="CRReg", size=32, dest="Spare"),
              Operand(type="Reg", size=64, dest="EA")])
add_group("mov",
    suffix="l",
    not64=True,
    cpu=["Priv"],
    opcode=[0x0F, 0x20],
    operands=[Operand(type="Reg", size=32, dest="EA"),
              Operand(type="CR4", size=32, dest="Spare")])
add_group("mov",
    suffix="l",
    cpu=["Priv"],
    not64=True,
    opcode=[0x0F, 0x20],
    operands=[Operand(type="Reg", size=32, dest="EA"),
              Operand(type="CRReg", size=32, dest="Spare")])
add_group("mov",
    suffix="q",
    cpu=["Priv"],
    opcode=[0x0F, 0x20],
    operands=[Operand(type="Reg", size=64, dest="EA"),
              Operand(type="CRReg", size=32, dest="Spare")])

# DR forms
add_group("mov",
    suffix="l",
    not64=True,
    cpu=["Priv"],
    opcode=[0x0F, 0x23],
    operands=[Operand(type="DRReg", size=32, dest="Spare"),
              Operand(type="Reg", size=32, dest="EA")])
add_group("mov",
    suffix="q",
    cpu=["Priv"],
    opcode=[0x0F, 0x23],
    operands=[Operand(type="DRReg", size=32, dest="Spare"),
              Operand(type="Reg", size=64, dest="EA")])
add_group("mov",
    suffix="l",
    not64=True,
    cpu=["Priv"],
    opcode=[0x0F, 0x21],
    operands=[Operand(type="Reg", size=32, dest="EA"),
              Operand(type="DRReg", size=32, dest="Spare")])
add_group("mov",
    suffix="q",
    cpu=["Priv"],
    opcode=[0x0F, 0x21],
    operands=[Operand(type="Reg", size=64, dest="EA"),
              Operand(type="DRReg", size=32, dest="Spare")])

# MMX forms for GAS parser (copied from movq)
add_group("mov",
    suffix="q",
    cpu=["MMX"],
    parsers=["gas"],
    opcode=[0x0F, 0x6F],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])
add_group("mov",
    suffix="q",
    cpu=["MMX"],
    parsers=["gas"],
    opersize=64,
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="RM", size=64, relaxed=True, dest="EA")])
add_group("mov",
    suffix="q",
    cpu=["MMX"],
    parsers=["gas"],
    opcode=[0x0F, 0x7F],
    operands=[Operand(type="SIMDRM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=64, dest="Spare")])
add_group("mov",
    suffix="q",
    cpu=["MMX"],
    parsers=["gas"],
    opersize=64,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=64, dest="Spare")])

# SSE2 forms for GAS parser (copied from movq)
add_group("mov",
    suffix="q",
    cpu=["SSE2"],
    parsers=["gas"],
    prefix=0xF3,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("mov",
    suffix="q",
    cpu=["SSE2"],
    parsers=["gas"],
    prefix=0xF3,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])
add_group("mov",
    suffix="q",
    cpu=["SSE2"],
    parsers=["gas"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="RM", size=64, relaxed=True, dest="EA")])
add_group("mov",
    suffix="q",
    cpu=["SSE2"],
    parsers=["gas"],
    prefix=0x66,
    opcode=[0x0F, 0xD6],
    operands=[Operand(type="SIMDRM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("mov",
    suffix="q",
    cpu=["SSE2"],
    parsers=["gas"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])

add_insn("mov", "mov")

#
# 64-bit absolute move (for GAS).
# These are disabled for GAS for normal mov above.
#
add_group("movabs",
    suffix="b",
    only64=True,
    opcode=[0xA0],
    operands=[Operand(type="Areg", size=8, dest=None),
              Operand(type="MemOffs", size=8, relaxed=True, dest="EA64")])
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("movabs",
        only64=True,
        suffix=sfx,
        opersize=sz,
        opcode=[0xA1],
        operands=[Operand(type="Areg", size=sz, dest=None),
                  Operand(type="MemOffs", size=sz, relaxed=True,
                          dest="EA64")])

add_group("movabs",
    suffix="b",
    only64=True,
    opcode=[0xA2],
    operands=[Operand(type="MemOffs", size=8, relaxed=True, dest="EA64"),
              Operand(type="Areg", size=8, dest=None)])
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("movabs",
        suffix=sfx,
        only64=True,
        opersize=sz,
        opcode=[0xA3],
        operands=[Operand(type="MemOffs", size=sz, relaxed=True,
                          dest="EA64"),
                  Operand(type="Areg", size=sz, dest=None)])

# 64-bit immediate form
add_group("movabs",
    suffix="q",
    opersize=64,
    opcode=[0xB8],
    operands=[Operand(type="Reg", size=64, dest="Op0Add"),
              Operand(type="Imm", size=64, relaxed=True, dest="Imm")])

add_insn("movabs", "movabs", parser="gas")

#
# Move with sign/zero extend
#
add_group("movszx",
    suffix="b",
    cpu=["386"],
    modifiers=["Op1Add"],
    opersize=16,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=16, dest="Spare"),
              Operand(type="RM", size=8, relaxed=True, dest="EA")])
add_group("movszx",
    suffix="b",
    cpu=["386"],
    modifiers=["Op1Add"],
    opersize=32,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="RM", size=8, dest="EA")])
add_group("movszx",
    suffix="b",
    modifiers=["Op1Add"],
    opersize=64,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="RM", size=8, dest="EA")])
add_group("movszx",
    suffix="w",
    cpu=["386"],
    modifiers=["Op1Add"],
    opersize=32,
    opcode=[0x0F, 0x01],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="RM", size=16, dest="EA")])
add_group("movszx",
    suffix="w",
    modifiers=["Op1Add"],
    opersize=64,
    opcode=[0x0F, 0x01],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="RM", size=16, dest="EA")])

add_insn("movsbw", "movszx", suffix="b", modifiers=[0xBE])
add_insn("movsbl", "movszx", suffix="b", modifiers=[0xBE])
add_insn("movswl", "movszx", suffix="w", modifiers=[0xBE])
add_insn("movsbq", "movszx", suffix="b", modifiers=[0xBE], only64=True)
add_insn("movswq", "movszx", suffix="w", modifiers=[0xBE], only64=True)
add_insn("movsx", "movszx", modifiers=[0xBE])
add_insn("movzbw", "movszx", suffix="b", modifiers=[0xB6])
add_insn("movzbl", "movszx", suffix="b", modifiers=[0xB6])
add_insn("movzwl", "movszx", suffix="w", modifiers=[0xB6])
add_insn("movzbq", "movszx", suffix="b", modifiers=[0xB6], only64=True)
add_insn("movzwq", "movszx", suffix="w", modifiers=[0xB6], only64=True)
add_insn("movzx", "movszx", modifiers=[0xB6])

#
# Move with sign-extend doubleword (64-bit mode only)
#
add_group("movsxd",
    suffix="l",
    opersize=64,
    opcode=[0x63],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="RM", size=32, dest="EA")])

add_insn("movslq", "movsxd", suffix="l")
add_insn("movsxd", "movsxd", parser="nasm")

#
# Push instructions
#
add_group("push",
    def_opersize_64=64,
    opcode=[0x50],
    operands=[Operand(type="Reg", size="BITS", dest="Op0Add")])
add_group("push",
    suffix="w",
    opersize=16,
    def_opersize_64=64,
    opcode=[0x50],
    operands=[Operand(type="Reg", size=16, dest="Op0Add")])
add_group("push",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[0x50],
    operands=[Operand(type="Reg", size=32, dest="Op0Add")])
add_group("push",
    suffix="q",
    only64=True,
    def_opersize_64=64,
    opcode=[0x50],
    operands=[Operand(type="Reg", size=64, dest="Op0Add")])

add_group("push",
    def_opersize_64=64,
    opcode=[0xFF],
    spare=6,
    operands=[Operand(type="RM", size="BITS", dest="EA")])
add_group("push",
    suffix="w",
    opersize=16,
    def_opersize_64=64,
    opcode=[0xFF],
    spare=6,
    operands=[Operand(type="RM", size=16, dest="EA")])
add_group("push",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[0xFF],
    spare=6,
    operands=[Operand(type="RM", size=32, dest="EA")])
add_group("push",
    suffix="q",
    only64=True,
    def_opersize_64=64,
    opcode=[0xFF],
    spare=6,
    operands=[Operand(type="RM", size=64, dest="EA")])

add_group("push",
    cpu=["186"],
    parsers=["nasm"],
    def_opersize_64=64,
    opcode=[0x6A],
    operands=[Operand(type="Imm", size=8, dest="SImm")])
add_group("push",
    cpu=["186"],
    parsers=["gas"],
    def_opersize_64=64,
    opcode=[0x6A],
    operands=[Operand(type="Imm", size=8, relaxed=True, dest="SImm")])
add_group("push",
    suffix="q",
    only64=True,
    opersize=64,
    def_opersize_64=64,
    opcode1=[0x6A],
    opcode2=[0x68],
    operands=[Operand(type="Imm", size=32, relaxed=True, dest="SImm",
                      opt="SImm8")])
add_group("push",
    not64=True,
    cpu=["186"],
    parsers=["nasm"],
    opcode1=[0x6A],
    opcode2=[0x68],
    operands=[Operand(type="Imm", size="BITS", relaxed=True, dest="Imm",
                      opt="SImm8")])
add_group("push",
    suffix="w",
    cpu=["186"],
    opersize=16,
    def_opersize_64=64,
    opcode1=[0x6A],
    opcode2=[0x68],
    operands=[Operand(type="Imm", size=16, relaxed=True, dest="Imm",
                      opt="SImm8")])
add_group("push",
    suffix="l",
    not64=True,
    opersize=32,
    opcode1=[0x6A],
    opcode2=[0x68],
    operands=[Operand(type="Imm", size=32, relaxed=True, dest="Imm",
                      opt="SImm8")])
# Need these when we don't match the BITS size, but they need to be
# below the above line so the optimizer can kick in by default.
add_group("push",
    cpu=["186"],
    parsers=["nasm"],
    opersize=16,
    def_opersize_64=64,
    opcode=[0x68],
    operands=[Operand(type="Imm", size=16, dest="Imm")])
add_group("push",
    not64=True,
    parsers=["nasm"],
    opersize=32,
    opcode=[0x68],
    operands=[Operand(type="Imm", size=32, dest="Imm")])
add_group("push",
    only64=True,
    parsers=["nasm"],
    opersize=64,
    def_opersize_64=64,
    opcode=[0x68],
    operands=[Operand(type="Imm", size=32, dest="SImm")])
add_group("push",
    not64=True,
    opcode=[0x0E],
    operands=[Operand(type="CS", dest=None)])
add_group("push",
    suffix="w",
    not64=True,
    opersize=16,
    opcode=[0x0E],
    operands=[Operand(type="CS", size=16, dest=None)])
add_group("push",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[0x0E],
    operands=[Operand(type="CS", size=32, dest=None)])
add_group("push",
    not64=True,
    opcode=[0x16],
    operands=[Operand(type="SS", dest=None)])
add_group("push",
    suffix="w",
    not64=True,
    opersize=16,
    opcode=[0x16],
    operands=[Operand(type="SS", size=16, dest=None)])
add_group("push",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[0x16],
    operands=[Operand(type="SS", size=32, dest=None)])
add_group("push",
    not64=True,
    opcode=[0x1E],
    operands=[Operand(type="DS", dest=None)])
add_group("push",
    suffix="w",
    not64=True,
    opersize=16,
    opcode=[0x1E],
    operands=[Operand(type="DS", size=16, dest=None)])
add_group("push",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[0x1E],
    operands=[Operand(type="DS", size=32, dest=None)])
add_group("push",
    not64=True,
    opcode=[0x06],
    operands=[Operand(type="ES", dest=None)])
add_group("push",
    suffix="w",
    not64=True,
    opersize=16,
    opcode=[0x06],
    operands=[Operand(type="ES", size=16, dest=None)])
add_group("push",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[0x06],
    operands=[Operand(type="ES", size=32, dest=None)])
add_group("push",
    opcode=[0x0F, 0xA0],
    operands=[Operand(type="FS", dest=None)])
add_group("push",
    suffix="w",
    opersize=16,
    opcode=[0x0F, 0xA0],
    operands=[Operand(type="FS", size=16, dest=None)])
add_group("push",
    suffix="l",
    opersize=32,
    opcode=[0x0F, 0xA0],
    operands=[Operand(type="FS", size=32, dest=None)])
add_group("push",
    opcode=[0x0F, 0xA8],
    operands=[Operand(type="GS", dest=None)])
add_group("push",
    suffix="w",
    opersize=16,
    opcode=[0x0F, 0xA8],
    operands=[Operand(type="GS", size=16, dest=None)])
add_group("push",
    suffix="l",
    opersize=32,
    opcode=[0x0F, 0xA8],
    operands=[Operand(type="GS", size=32, dest=None)])

add_insn("push", "push")
add_insn("pusha", "onebyte", modifiers=[0x60, 0], cpu=["186"], not64=True)
add_insn("pushad", "onebyte", parser="nasm", modifiers=[0x60, 32],
         cpu=["386"], not64=True)
add_insn("pushal", "onebyte", parser="gas", modifiers=[0x60, 32],
         cpu=["386"], not64=True)
add_insn("pushaw", "onebyte", modifiers=[0x60, 16], cpu=["186"], not64=True)

#
# Pop instructions
#
add_group("pop",
    def_opersize_64=64,
    opcode=[0x58],
    operands=[Operand(type="Reg", size="BITS", dest="Op0Add")])
add_group("pop",
    suffix="w",
    opersize=16,
    def_opersize_64=64,
    opcode=[0x58],
    operands=[Operand(type="Reg", size=16, dest="Op0Add")])
add_group("pop",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[0x58],
    operands=[Operand(type="Reg", size=32, dest="Op0Add")])
add_group("pop",
    suffix="q",
    only64=True,
    def_opersize_64=64,
    opcode=[0x58],
    operands=[Operand(type="Reg", size=64, dest="Op0Add")])

add_group("pop",
    def_opersize_64=64,
    opcode=[0x8F],
    operands=[Operand(type="RM", size="BITS", dest="EA")])
add_group("pop",
    suffix="w",
    opersize=16,
    def_opersize_64=64,
    opcode=[0x8F],
    operands=[Operand(type="RM", size=16, dest="EA")])
add_group("pop",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[0x8F],
    operands=[Operand(type="RM", size=32, dest="EA")])
add_group("pop",
    suffix="q",
    only64=True,
    def_opersize_64=64,
    opcode=[0x8F],
    operands=[Operand(type="RM", size=64, dest="EA")])

# POP CS is debateably valid on the 8086, if obsolete and undocumented.
# We don't include it because it's VERY unlikely it will ever be used
# anywhere.  If someone really wants it they can db 0x0F it.
#add_group("pop",
#    cpu=["Undoc", "Obs"],
#    opcode=[0x0F],
#    operands=[Operand(type="CS", dest=None)])
add_group("pop",
    not64=True,
    opcode=[0x17],
    operands=[Operand(type="SS", dest=None)])
add_group("pop",
    not64=True,
    opersize=16,
    opcode=[0x17],
    operands=[Operand(type="SS", size=16, dest=None)])
add_group("pop",
    not64=True,
    opersize=32,
    opcode=[0x17],
    operands=[Operand(type="SS", size=32, dest=None)])
add_group("pop",
    not64=True,
    opcode=[0x1F],
    operands=[Operand(type="DS", dest=None)])
add_group("pop",
    not64=True,
    opersize=16,
    opcode=[0x1F],
    operands=[Operand(type="DS", size=16, dest=None)])
add_group("pop",
    not64=True,
    opersize=32,
    opcode=[0x1F],
    operands=[Operand(type="DS", size=32, dest=None)])
add_group("pop",
    not64=True,
    opcode=[0x07],
    operands=[Operand(type="ES", dest=None)])
add_group("pop",
    not64=True,
    opersize=16,
    opcode=[0x07],
    operands=[Operand(type="ES", size=16, dest=None)])
add_group("pop",
    not64=True,
    opersize=32,
    opcode=[0x07],
    operands=[Operand(type="ES", size=32, dest=None)])
add_group("pop",
    opcode=[0x0F, 0xA1],
    operands=[Operand(type="FS", dest=None)])
add_group("pop",
    opersize=16,
    opcode=[0x0F, 0xA1],
    operands=[Operand(type="FS", size=16, dest=None)])
add_group("pop",
    opersize=32,
    opcode=[0x0F, 0xA1],
    operands=[Operand(type="FS", size=32, dest=None)])
add_group("pop",
    opcode=[0x0F, 0xA9],
    operands=[Operand(type="GS", dest=None)])
add_group("pop",
    opersize=16,
    opcode=[0x0F, 0xA9],
    operands=[Operand(type="GS", size=16, dest=None)])
add_group("pop",
    opersize=32,
    opcode=[0x0F, 0xA9],
    operands=[Operand(type="GS", size=32, dest=None)])

add_insn("pop", "pop")
add_insn("popa", "onebyte", modifiers=[0x61, 0], cpu=["186"], not64=True)
add_insn("popad", "onebyte", parser="nasm", modifiers=[0x61, 32],
         cpu=["386"], not64=True)
add_insn("popal", "onebyte", parser="gas", modifiers=[0x61, 32],
         cpu=["386"], not64=True)
add_insn("popaw", "onebyte", modifiers=[0x61, 16], cpu=["186"], not64=True)

#
# Exchange instructions
#
add_group("xchg",
    suffix="b",
    opcode=[0x86],
    operands=[Operand(type="RM", size=8, relaxed=True, dest="EA"),
              Operand(type="Reg", size=8, dest="Spare")])
add_group("xchg",
    suffix="b",
    opcode=[0x86],
    operands=[Operand(type="Reg", size=8, dest="Spare"),
              Operand(type="RM", size=8, relaxed=True, dest="EA")])
# We could be extra-efficient in the 64-bit mode case here.
# XCHG AX, AX in 64-bit mode is a NOP, as it doesn't clear the
# high 48 bits of RAX. Thus we don't need the operand-size prefix.
# But this feels too clever, and probably not what the user really
# expects in the generated code, so we don't do it.
#add_group("xchg",
#    suffix="w",
#    only64=True,
#    opcode=[0x90],
#    operands=[Operand(type="Areg", size=16, dest=None),
#              Operand(type="AReg", size=16, dest="Op0Add")])
add_group("xchg",
    suffix="w",
    opersize=16,
    opcode=[0x90],
    operands=[Operand(type="Areg", size=16, dest=None),
              Operand(type="Reg", size=16, dest="Op0Add")])
add_group("xchg",
    suffix="w",
    opersize=16,
    opcode=[0x90],
    operands=[Operand(type="Reg", size=16, dest="Op0Add"),
              Operand(type="Areg", size=16, dest=None)])
add_group("xchg",
    suffix="w",
    opersize=16,
    opcode=[0x87],
    operands=[Operand(type="RM", size=16, relaxed=True, dest="EA"),
              Operand(type="Reg", size=16, dest="Spare")])
add_group("xchg",
    suffix="w",
    opersize=16,
    opcode=[0x87],
    operands=[Operand(type="Reg", size=16, dest="Spare"),
              Operand(type="RM", size=16, relaxed=True, dest="EA")])
# Be careful with XCHG EAX, EAX in 64-bit mode.  This needs to use
# the long form rather than the NOP form, as the long form clears
# the high 32 bits of RAX.  This makes all 32-bit forms in 64-bit
# mode have consistent operation.
#
# FIXME: due to a hard-to-fix bug in how we handle generating gas suffix CPU
# rules, this causes xchgl to be CPU_Any instead of CPU_386.  A hacky patch
# could fix it, but it's doubtful anyone will ever notice, so leave it.
add_group("xchg",
    suffix="l",
    only64=True,
    opersize=32,
    opcode=[0x87],
    operands=[Operand(type="Areg", size=32, dest="EA"),
              Operand(type="Areg", size=32, dest="Spare")])
add_group("xchg",
    suffix="l",
    opersize=32,
    opcode=[0x90],
    operands=[Operand(type="Areg", size=32, dest=None),
              Operand(type="Reg", size=32, dest="Op0Add")])
add_group("xchg",
    suffix="l",
    opersize=32,
    opcode=[0x90],
    operands=[Operand(type="Reg", size=32, dest="Op0Add"),
              Operand(type="Areg", size=32, dest=None)])
add_group("xchg",
    suffix="l",
    opersize=32,
    opcode=[0x87],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="Reg", size=32, dest="Spare")])
add_group("xchg",
    suffix="l",
    opersize=32,
    opcode=[0x87],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="RM", size=32, relaxed=True, dest="EA")])
# Be efficient with XCHG RAX, RAX.
# This is a NOP and thus doesn't need the REX prefix.
add_group("xchg",
    suffix="q",
    only64=True,
    opcode=[0x90],
    operands=[Operand(type="Areg", size=64, dest=None),
              Operand(type="Areg", size=64, dest="Op0Add")])
add_group("xchg",
    suffix="q",
    opersize=64,
    opcode=[0x90],
    operands=[Operand(type="Areg", size=64, dest=None),
              Operand(type="Reg", size=64, dest="Op0Add")])
add_group("xchg",
    suffix="q",
    opersize=64,
    opcode=[0x90],
    operands=[Operand(type="Reg", size=64, dest="Op0Add"),
              Operand(type="Areg", size=64, dest=None)])
add_group("xchg",
    suffix="q",
    opersize=64,
    opcode=[0x87],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="Reg", size=64, dest="Spare")])
add_group("xchg",
    suffix="q",
    opersize=64,
    opcode=[0x87],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="RM", size=64, relaxed=True, dest="EA")])

add_insn("xchg", "xchg")

#####################################################################
# In/out from ports
#####################################################################
add_group("in",
    suffix="b",
    opcode=[0xE4],
    operands=[Operand(type="Areg", size=8, dest=None),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
for sfx, sz in zip("wl", [16, 32]):
    add_group("in",
        suffix=sfx,
        opersize=sz,
        opcode=[0xE5],
        operands=[Operand(type="Areg", size=sz, dest=None),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("in",
    suffix="b",
    opcode=[0xEC],
    operands=[Operand(type="Areg", size=8, dest=None),
              Operand(type="Dreg", size=16, dest=None)])
for sfx, sz in zip("wl", [16, 32]):
    add_group("in",
        suffix=sfx,
        opersize=sz,
        opcode=[0xED],
        operands=[Operand(type="Areg", size=sz, dest=None),
                  Operand(type="Dreg", size=16, dest=None)])
# GAS-only variants (implicit accumulator register)
add_group("in",
    suffix="b",
    parsers=["gas"],
    opcode=[0xE4],
    operands=[Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
for sfx, sz in zip("wl", [16, 32]):
    add_group("in",
        suffix=sfx,
        parsers=["gas"],
        opersize=sz,
        opcode=[0xE5],
        operands=[Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("in",
    suffix="b",
    parsers=["gas"],
    opcode=[0xEC],
    operands=[Operand(type="Dreg", size=16, dest=None)])
add_group("in",
    suffix="w",
    parsers=["gas"],
    opersize=16,
    opcode=[0xED],
    operands=[Operand(type="Dreg", size=16, dest=None)])
add_group("in",
    suffix="l",
    cpu=["386"],
    parsers=["gas"],
    opersize=32,
    opcode=[0xED],
    operands=[Operand(type="Dreg", size=16, dest=None)])

add_insn("in", "in")

add_group("out",
    suffix="b",
    opcode=[0xE6],
    operands=[Operand(type="Imm", size=8, relaxed=True, dest="Imm"),
              Operand(type="Areg", size=8, dest=None)])
for sfx, sz in zip("wl", [16, 32]):
    add_group("out",
        suffix=sfx,
        opersize=sz,
        opcode=[0xE7],
        operands=[Operand(type="Imm", size=8, relaxed=True, dest="Imm"),
                  Operand(type="Areg", size=sz, dest=None)])
add_group("out",
    suffix="b",
    opcode=[0xEE],
    operands=[Operand(type="Dreg", size=16, dest=None),
              Operand(type="Areg", size=8, dest=None)])
for sfx, sz in zip("wl", [16, 32]):
    add_group("out",
        suffix=sfx,
        opersize=sz,
        opcode=[0xEF],
        operands=[Operand(type="Dreg", size=16, dest=None),
                  Operand(type="Areg", size=sz, dest=None)])
# GAS-only variants (implicit accumulator register)
add_group("out",
    suffix="b",
    parsers=["gas"],
    opcode=[0xE6],
    operands=[Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("out",
    suffix="w",
    parsers=["gas"],
    opersize=16,
    opcode=[0xE7],
    operands=[Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("out",
    suffix="l",
    cpu=["386"],
    parsers=["gas"],
    opersize=32,
    opcode=[0xE7],
    operands=[Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("out",
    suffix="b",
    parsers=["gas"],
    opcode=[0xEE],
    operands=[Operand(type="Dreg", size=16, dest=None)])
add_group("out",
    suffix="w",
    parsers=["gas"],
    opersize=16,
    opcode=[0xEF],
    operands=[Operand(type="Dreg", size=16, dest=None)])
add_group("out",
    suffix="l",
    cpu=["386"],
    parsers=["gas"],
    opersize=32,
    opcode=[0xEF],
    operands=[Operand(type="Dreg", size=16, dest=None)])

add_insn("out", "out")

#
# Load effective address
#
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("lea",
        suffix=sfx,
        opersize=sz,
        opcode=[0x8D],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="Mem", relaxed=True, dest="EA")])

add_insn("lea", "lea")

#
# Load segment registers from memory
#
for sfx, sz in zip("wl", [16, 32]):
    add_group("ldes",
        suffix=sfx,
        not64=True,
        modifiers=["Op0Add"],
        opersize=sz,
        opcode=[0x00],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="Mem", relaxed=True, dest="EA")])

add_insn("lds", "ldes", modifiers=[0xC5])
add_insn("les", "ldes", modifiers=[0xC4])

for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("lfgss",
        suffix=sfx,
        cpu=["386"],
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x00],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="Mem", relaxed=True, dest="EA")])

add_insn("lfs", "lfgss", modifiers=[0xB4])
add_insn("lgs", "lfgss", modifiers=[0xB5])
add_insn("lss", "lfgss", modifiers=[0xB2])

#
# Flags registers instructions
#
add_insn("clc", "onebyte", modifiers=[0xF8])
add_insn("cld", "onebyte", modifiers=[0xFC])
add_insn("cli", "onebyte", modifiers=[0xFA])
add_insn("clts", "twobyte", modifiers=[0x0F, 0x06], cpu=["286", "Priv"])
add_insn("cmc", "onebyte", modifiers=[0xF5])
add_insn("lahf", "onebyte", modifiers=[0x9F])
add_insn("sahf", "onebyte", modifiers=[0x9E])
add_insn("pushf", "onebyte", modifiers=[0x9C, 0, 64])
add_insn("pushfd", "onebyte", parser="nasm", modifiers=[0x9C, 32],
         cpu=["386"], not64=True)
add_insn("pushfl", "onebyte", parser="gas", modifiers=[0x9C, 32],
         cpu=["386"], not64=True)
add_insn("pushfw", "onebyte", modifiers=[0x9C, 16, 64])
add_insn("pushfq", "onebyte", modifiers=[0x9C, 64, 64], only64=True)
add_insn("popf", "onebyte", modifiers=[0x9D, 0, 64])
add_insn("popfd", "onebyte", parser="nasm", modifiers=[0x9D, 32],
         cpu=["386"], not64=True)
add_insn("popfl", "onebyte", parser="gas", modifiers=[0x9D, 32],
         cpu=["386"], not64=True)
add_insn("popfw", "onebyte", modifiers=[0x9D, 16, 64])
add_insn("popfq", "onebyte", modifiers=[0x9D, 64, 64], only64=True)
add_insn("stc", "onebyte", modifiers=[0xF9])
add_insn("std", "onebyte", modifiers=[0xFD])
add_insn("sti", "onebyte", modifiers=[0xFB])

#
# Arithmetic - general
#
add_group("arith",
    suffix="b",
    modifiers=["Op0Add"],
    opcode=[0x04],
    operands=[Operand(type="Areg", size=8, dest=None),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
for sfx, sz, immsz in zip("wlq", [16, 32, 64], [16, 32, 32]):
    add_group("arith",
        suffix=sfx,
        modifiers=["Op2Add", "Op1AddSp"],
        opersize=sz,
        opcode1=[0x83, 0xC0],
        opcode2=[0x05],
        operands=[Operand(type="Areg", size=sz, dest=None),
                  Operand(type="Imm", size=immsz, relaxed=True, dest="Imm",
                          opt="SImm8")])

add_group("arith",
    suffix="b",
    modifiers=["Gap", "SpAdd"],
    opcode=[0x80],
    spare=0,
    operands=[Operand(type="RM", size=8, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("arith",
    suffix="b",
    modifiers=["Gap", "SpAdd"],
    opcode=[0x80],
    spare=0,
    operands=[Operand(type="RM", size=8, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, dest="Imm")])

add_group("arith",
    suffix="w",
    modifiers=["Gap", "SpAdd"],
    opersize=16,
    opcode=[0x83],
    spare=0,
    operands=[Operand(type="RM", size=16, dest="EA"),
              Operand(type="Imm", size=8, dest="SImm")])
add_group("arith",
    parsers=["nasm"],
    modifiers=["Gap", "SpAdd"],
    opersize=16,
    opcode1=[0x83],
    opcode2=[0x81],
    spare=0,
    operands=[Operand(type="RM", size=16, relaxed=True, dest="EA"),
              Operand(type="Imm", size=16, dest="Imm", opt="SImm8")])
add_group("arith",
    suffix="w",
    modifiers=["Gap", "SpAdd"],
    opersize=16,
    opcode1=[0x83],
    opcode2=[0x81],
    spare=0,
    operands=[
        Operand(type="RM", size=16, dest="EA"),
        Operand(type="Imm", size=16, relaxed=True, dest="Imm", opt="SImm8")])

add_group("arith",
    suffix="l",
    modifiers=["Gap", "SpAdd"],
    opersize=32,
    opcode=[0x83],
    spare=0,
    operands=[Operand(type="RM", size=32, dest="EA"),
              Operand(type="Imm", size=8, dest="SImm")])
# Not64 because we can't tell if add [], dword in 64-bit mode is supposed
# to be a qword destination or a dword destination.
add_group("arith",
    not64=True,
    parsers=["nasm"],
    modifiers=["Gap", "SpAdd"],
    opersize=32,
    opcode1=[0x83],
    opcode2=[0x81],
    spare=0,
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="Imm", size=32, dest="Imm", opt="SImm8")])
add_group("arith",
    suffix="l",
    modifiers=["Gap", "SpAdd"],
    opersize=32,
    opcode1=[0x83],
    opcode2=[0x81],
    spare=0,
    operands=[
        Operand(type="RM", size=32, dest="EA"),
        Operand(type="Imm", size=32, relaxed=True, dest="Imm", opt="SImm8")])

# No relaxed-RM mode for 64-bit destinations; see above Not64 comment.
add_group("arith",
    suffix="q",
    modifiers=["Gap", "SpAdd"],
    opersize=64,
    opcode=[0x83],
    spare=0,
    operands=[Operand(type="RM", size=64, dest="EA"),
              Operand(type="Imm", size=8, dest="SImm")])
add_group("arith",
    suffix="q",
    modifiers=["Gap", "SpAdd"],
    opersize=64,
    opcode1=[0x83],
    opcode2=[0x81],
    spare=0,
    operands=[
        Operand(type="RM", size=64, dest="EA"),
        Operand(type="Imm", size=32, relaxed=True, dest="Imm", opt="SImm8")])

for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("arith",
        suffix=sfx,
        modifiers=["Op0Add"],
        opersize=sz,
        opcode=[0x00+(sz!=8)],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="Spare")])
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("arith",
        suffix=sfx,
        modifiers=["Op0Add"],
        opersize=sz,
        opcode=[0x02+(sz!=8)],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA")])

add_insn("add", "arith", modifiers=[0x00, 0])
add_insn("or",  "arith", modifiers=[0x08, 1])
add_insn("adc", "arith", modifiers=[0x10, 2])
add_insn("sbb", "arith", modifiers=[0x18, 3])
add_insn("and", "arith", modifiers=[0x20, 4])
add_insn("sub", "arith", modifiers=[0x28, 5])
add_insn("xor", "arith", modifiers=[0x30, 6])
add_insn("cmp", "arith", modifiers=[0x38, 7])

#
# Arithmetic - inc/dec
#
add_group("incdec",
    suffix="b",
    modifiers=["Gap", "SpAdd"],
    opcode=[0xFE],
    spare=0,
    operands=[Operand(type="RM", size=8, dest="EA")])
for sfx, sz in zip("wl", [16, 32]):
    add_group("incdec",
        suffix=sfx,
        not64=True,
        modifiers=["Op0Add"],
        opersize=sz,
        opcode=[0x00],
        operands=[Operand(type="Reg", size=sz, dest="Op0Add")])
    add_group("incdec",
        suffix=sfx,
        modifiers=["Gap", "SpAdd"],
        opersize=sz,
        opcode=[0xFF],
        spare=0,
        operands=[Operand(type="RM", size=sz, dest="EA")])
add_group("incdec",
    suffix="q",
    modifiers=["Gap", "SpAdd"],
    opersize=64,
    opcode=[0xFF],
    spare=0,
    operands=[Operand(type="RM", size=64, dest="EA")])

add_insn("inc", "incdec", modifiers=[0x40, 0])
add_insn("dec", "incdec", modifiers=[0x48, 1])

#
# Arithmetic - mul/neg/not F6 opcodes
#
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("f6",
        suffix=sfx,
        modifiers=["SpAdd"],
        opersize=sz,
        opcode=[0xF6+(sz!=8)],
        spare=0,
        operands=[Operand(type="RM", size=sz, dest="EA")])

add_insn("not", "f6", modifiers=[2])
add_insn("neg", "f6", modifiers=[3])
add_insn("mul", "f6", modifiers=[4])

#
# Arithmetic - div/idiv F6 opcodes
# These allow explicit accumulator in GAS mode.
#
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("div",
        suffix=sfx,
        modifiers=["SpAdd"],
        opersize=sz,
        opcode=[0xF6+(sz!=8)],
        spare=0,
        operands=[Operand(type="RM", size=sz, dest="EA")])
# Versions with explicit accumulator
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("div",
        suffix=sfx,
        modifiers=["SpAdd"],
        opersize=sz,
        opcode=[0xF6+(sz!=8)],
        spare=0,
        operands=[Operand(type="Areg", size=sz, dest=None),
                  Operand(type="RM", size=sz, dest="EA")])

add_insn("div", "div", modifiers=[6])
add_insn("idiv", "div", modifiers=[7])

#
# Arithmetic - test instruction
#
for sfx, sz, immsz in zip("bwlq", [8, 16, 32, 64], [8, 16, 32, 32]):
    add_group("test",
        suffix=sfx,
        opersize=sz,
        opcode=[0xA8+(sz!=8)],
        operands=[Operand(type="Areg", size=sz, dest=None),
                  Operand(type="Imm", size=immsz, relaxed=True, dest="Imm")])

for sfx, sz, immsz in zip("bwlq", [8, 16, 32, 64], [8, 16, 32, 32]):
    add_group("test",
        suffix=sfx,
        opersize=sz,
        opcode=[0xF6+(sz!=8)],
        operands=[Operand(type="RM", size=sz, dest="EA"),
                  Operand(type="Imm", size=immsz, relaxed=True, dest="Imm")])
    add_group("test",
        suffix=sfx,
        opersize=sz,
        opcode=[0xF6+(sz!=8)],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Imm", size=immsz, dest="Imm")])

for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("test",
        suffix=sfx,
        opersize=sz,
        opcode=[0x84+(sz!=8)],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="Spare")])
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("test",
        suffix=sfx,
        opersize=sz,
        opcode=[0x84+(sz!=8)],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA")])

add_insn("test", "test")

#
# Arithmetic - aad/aam
#
add_group("aadm",
    modifiers=["Op0Add"],
    opcode=[0xD4, 0x0A],
    operands=[])
add_group("aadm",
    modifiers=["Op0Add"],
    opcode=[0xD4],
    operands=[Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("aaa", "onebyte", modifiers=[0x37], not64=True)
add_insn("aas", "onebyte", modifiers=[0x3F], not64=True)
add_insn("daa", "onebyte", modifiers=[0x27], not64=True)
add_insn("das", "onebyte", modifiers=[0x2F], not64=True)
add_insn("aad", "aadm", modifiers=[0x01], not64=True)
add_insn("aam", "aadm", modifiers=[0x00], not64=True)

#
# Conversion instructions
#
add_insn("cbw", "onebyte", modifiers=[0x98, 16])
add_insn("cwde", "onebyte", modifiers=[0x98, 32], cpu=["386"])
add_insn("cdqe", "onebyte", modifiers=[0x98, 64], only64=True)
add_insn("cwd", "onebyte", modifiers=[0x99, 16])
add_insn("cdq", "onebyte", modifiers=[0x99, 32], cpu=["386"])
add_insn("cqo", "onebyte", modifiers=[0x99, 64], only64=True)

#
# Conversion instructions - GAS / AT&T naming
#
add_insn("cbtw", "onebyte", parser="gas", modifiers=[0x98, 16])
add_insn("cwtl", "onebyte", parser="gas", modifiers=[0x98, 32], cpu=["386"])
add_insn("cltq", "onebyte", parser="gas", modifiers=[0x98, 64], only64=True)
add_insn("cwtd", "onebyte", parser="gas", modifiers=[0x99, 16])
add_insn("cltd", "onebyte", parser="gas", modifiers=[0x99, 32], cpu=["386"])
add_insn("cqto", "onebyte", parser="gas", modifiers=[0x99, 64], only64=True)

#
# Arithmetic - imul
#
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("imul",
        suffix=sfx,
        opersize=sz,
        opcode=[0xF6+(sz!=8)],
        spare=5,
        operands=[Operand(type="RM", size=sz, dest="EA")])
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("imul",
        suffix=sfx,
        cpu=["386"],
        opersize=sz,
        opcode=[0x0F, 0xAF],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA")])
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("imul",
        suffix=sfx,
        cpu=["186"],
        opersize=sz,
        opcode=[0x6B],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Imm", size=8, dest="SImm")])
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("imul",
        suffix=sfx,
        cpu=["186"],
        opersize=sz,
        opcode=[0x6B],
        operands=[Operand(type="Reg", size=sz, dest="SpareEA"),
                  Operand(type="Imm", size=8, dest="SImm")])
for sfx, sz, immsz in zip("wlq", [16, 32, 64], [16, 32, 32]):
    add_group("imul",
        suffix=sfx,
        cpu=["186"],
        opersize=sz,
        opcode1=[0x6B],
        opcode2=[0x69],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Imm", size=immsz, relaxed=True, dest="SImm",
                          opt="SImm8")])
for sfx, sz, immsz in zip("wlq", [16, 32, 64], [16, 32, 32]):
    add_group("imul",
        suffix=sfx,
        cpu=["186"],
        opersize=sz,
        opcode1=[0x6B],
        opcode2=[0x69],
        operands=[Operand(type="Reg", size=sz, dest="SpareEA"),
                  Operand(type="Imm", size=immsz, relaxed=True, dest="SImm",
                          opt="SImm8")])

add_insn("imul", "imul")

#
# Shifts - standard
#
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("shift",
        suffix=sfx,
        modifiers=["SpAdd"],
        opersize=sz,
        opcode=[0xD2+(sz!=8)],
        spare=0,
        operands=[Operand(type="RM", size=sz, dest="EA"),
                  Operand(type="Creg", size=8, dest=None)])
    add_group("shift",
        suffix=sfx,
        modifiers=["SpAdd"],
        opersize=sz,
        opcode=[0xD0+(sz!=8)],
        spare=0,
        operands=[Operand(type="RM", size=sz, dest="EA"),
                  Operand(type="Imm1", size=8, relaxed=True, dest=None)])
    add_group("shift",
        suffix=sfx,
        cpu=["186"],
        modifiers=["SpAdd"],
        opersize=sz,
        opcode=[0xC0+(sz!=8)],
        spare=0,
        operands=[Operand(type="RM", size=sz, dest="EA"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
# In GAS mode, single operands are equivalent to shifting by 1 forms
for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("shift",
        suffix=sfx,
        parsers=["gas"],
        modifiers=["SpAdd"],
        opersize=sz,
        opcode=[0xD0+(sz!=8)],
        spare=0,
        operands=[Operand(type="RM", size=sz, dest="EA")])

add_insn("rol", "shift", modifiers=[0])
add_insn("ror", "shift", modifiers=[1])
add_insn("rcl", "shift", modifiers=[2])
add_insn("rcr", "shift", modifiers=[3])
add_insn("sal", "shift", modifiers=[4])
add_insn("shl", "shift", modifiers=[4])
add_insn("shr", "shift", modifiers=[5])
add_insn("sar", "shift", modifiers=[7])

#
# Shifts - doubleword
#
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("shlrd",
        suffix=sfx,
        cpu=["386"],
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x00],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
    add_group("shlrd",
        suffix=sfx,
        cpu=["386"],
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x01],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="Creg", size=8, dest=None)])
# GAS parser supports two-operand form for shift with CL count
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("shlrd",
        suffix=sfx,
        cpu=["386"],
        parsers=["gas"],
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x01],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="Spare")])

add_insn("shld", "shlrd", modifiers=[0xA4])
add_insn("shrd", "shlrd", modifiers=[0xAC])

#####################################################################
# Control transfer instructions (unconditional)
#####################################################################
#
# call
#
add_group("call",
    opcode=[],
    operands=[Operand(type="ImmNotSegOff", dest="JmpRel")])
add_group("call",
    suffix="w",
    opersize=16,
    opcode=[],
    operands=[Operand(type="ImmNotSegOff", size=16, dest="JmpRel")])
add_group("call",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[],
    operands=[Operand(type="ImmNotSegOff", size=32, dest="JmpRel")])
add_group("call",
    suffixes=["l", "q"],
    only64=True,
    opersize=64,
    opcode=[],
    operands=[Operand(type="ImmNotSegOff", size=32, dest="JmpRel")])

add_group("call",
    opersize=16,
    def_opersize_64=64,
    opcode=[0xE8],
    operands=[Operand(type="Imm", size=16, tmod="Near", dest="JmpRel")])
add_group("call",
    not64=True,
    opersize=32,
    opcode=[0xE8],
    operands=[Operand(type="Imm", size=32, tmod="Near", dest="JmpRel")])
add_group("call",
    only64=True,
    opersize=64,
    def_opersize_64=64,
    opcode=[0xE8],
    operands=[Operand(type="Imm", size=32, tmod="Near", dest="JmpRel")])
add_group("call",
    def_opersize_64=64,
    opcode=[0xE8],
    operands=[Operand(type="Imm", tmod="Near", dest="JmpRel")])

add_group("call",
    suffix="w",
    req_suffix=True,
    opersize=16,
    opcode=[0xFF],
    spare=2,
    operands=[Operand(type="RM", size=16, dest="EA")])
add_group("call",
    suffix="l",
    req_suffix=True,
    not64=True,
    opersize=32,
    opcode=[0xFF],
    spare=2,
    operands=[Operand(type="RM", size=32, dest="EA")])
add_group("call",
    suffix="q",
    req_suffix=True,
    opersize=64,
    def_opersize_64=64,
    opcode=[0xFF],
    spare=2,
    operands=[Operand(type="RM", size=64, dest="EA")])
add_group("call",
    parsers=["gas"],
    def_opersize_64=64,
    opcode=[0xFF],
    spare=2,
    operands=[Operand(type="Reg", size="BITS", dest="EA")])
add_group("call",
    def_opersize_64=64,
    opcode=[0xFF],
    spare=2,
    operands=[Operand(type="Mem", dest="EA")])
add_group("call",
    parsers=["nasm"],
    opersize=16,
    def_opersize_64=64,
    opcode=[0xFF],
    spare=2,
    operands=[Operand(type="RM", size=16, tmod="Near", dest="EA")])
add_group("call",
    parsers=["nasm"],
    not64=True,
    opersize=32,
    opcode=[0xFF],
    spare=2,
    operands=[Operand(type="RM", size=32, tmod="Near", dest="EA")])
add_group("call",
    parsers=["nasm"],
    opersize=64,
    def_opersize_64=64,
    opcode=[0xFF],
    spare=2,
    operands=[Operand(type="RM", size=64, tmod="Near", dest="EA")])
add_group("call",
    parsers=["nasm"],
    def_opersize_64=64,
    opcode=[0xFF],
    spare=2,
    operands=[Operand(type="Mem", tmod="Near", dest="EA")])

# Far indirect (through memory).  Needs explicit FAR override (NASM only)
for sz in [16, 32, 64]:
    add_group("call",
        parsers=["nasm"],
        opersize=sz,
        opcode=[0xFF],
        spare=3,
        operands=[Operand(type="Mem", size=sz, tmod="Far", dest="EA")])
add_group("call",
    parsers=["nasm"],
    opcode=[0xFF],
    spare=3,
    operands=[Operand(type="Mem", tmod="Far", dest="EA")])

# With explicit FAR override
for sz in [16, 32]:
    add_group("call",
        parsers=["nasm"],
        not64=True,
        opersize=sz,
        opcode=[0x9A],
        operands=[Operand(type="Imm", size=sz, tmod="Far", dest="JmpFar")])
add_group("call",
    parsers=["nasm"],
    not64=True,
    opcode=[0x9A],
    operands=[Operand(type="Imm", tmod="Far", dest="JmpFar")])

# Since not caught by first ImmNotSegOff group, implicitly FAR (in NASM).
for sz in [16, 32]:
    add_group("call",
        parsers=["nasm"],
        not64=True,
        opersize=sz,
        opcode=[0x9A],
        operands=[Operand(type="Imm", size=sz, dest="JmpFar")])
add_group("call",
    parsers=["nasm"],
    not64=True,
    opcode=[0x9A],
    operands=[Operand(type="Imm", dest="JmpFar")])

# Two-operand FAR (GAS only)
for sfx, sz in zip("wl", [16, 32]):
    add_group("call",
        suffix=sfx,
        req_suffix=True,
        parsers=["gas"],
        not64=True,
        gas_no_reverse=True,
        opersize=sz,
        opcode=[0x9A],
        operands=[Operand(type="Imm", size=16, relaxed=True, dest="JmpFar"),
                  Operand(type="Imm", size=sz, relaxed=True, dest="JmpFar")])
add_group("call",
    parsers=["gas"],
    not64=True,
    gas_no_reverse=True,
    opcode=[0x9A],
    operands=[Operand(type="Imm", size=16, relaxed=True, dest="JmpFar"),
              Operand(type="Imm", size="BITS", relaxed=True, dest="JmpFar")])

add_insn("call", "call")

#
# jmp
#
add_group("jmp",
    opcode=[],
    operands=[Operand(type="ImmNotSegOff", dest="JmpRel")])
add_group("jmp",
    suffix="w",
    opersize=16,
    opcode=[],
    operands=[Operand(type="ImmNotSegOff", size=16, dest="JmpRel")])
add_group("jmp",
    suffix="l",
    not64=True,
    opersize=32,
    opcode=[0x00],
    operands=[Operand(type="ImmNotSegOff", size=32, dest="JmpRel")])
add_group("jmp",
    suffixes=["l", "q"],
    only64=True,
    opersize=64,
    opcode=[0x00],
    operands=[Operand(type="ImmNotSegOff", size=32, dest="JmpRel")])

add_group("jmp",
    def_opersize_64=64,
    opcode=[0xEB],
    operands=[Operand(type="Imm", tmod="Short", dest="JmpRel")])
add_group("jmp",
    opersize=16,
    def_opersize_64=64,
    opcode=[0xE9],
    operands=[Operand(type="Imm", size=16, tmod="Near", dest="JmpRel")])
add_group("jmp",
    not64=True,
    cpu=["386"],
    opersize=32,
    opcode=[0xE9],
    operands=[Operand(type="Imm", size=32, tmod="Near", dest="JmpRel")])
add_group("jmp",
    only64=True,
    opersize=64,
    def_opersize_64=64,
    opcode=[0xE9],
    operands=[Operand(type="Imm", size=32, tmod="Near", dest="JmpRel")])
add_group("jmp",
    def_opersize_64=64,
    opcode=[0xE9],
    operands=[Operand(type="Imm", tmod="Near", dest="JmpRel")])

add_group("jmp",
    suffix="w",
    req_suffix=True,
    opersize=16,
    def_opersize_64=64,
    opcode=[0xFF],
    spare=4,
    operands=[Operand(type="RM", size=16, dest="EA")])
add_group("jmp",
    suffix="l",
    req_suffix=True,
    not64=True,
    opersize=32,
    opcode=[0xFF],
    spare=4,
    operands=[Operand(type="RM", size=32, dest="EA")])
add_group("jmp",
    suffix="q",
    req_suffix=True,
    opersize=64,
    def_opersize_64=64,
    opcode=[0xFF],
    spare=4,
    operands=[Operand(type="RM", size=64, dest="EA")])
add_group("jmp",
    parsers=["gas"],
    def_opersize_64=64,
    opcode=[0xFF],
    spare=4,
    operands=[Operand(type="Reg", size="BITS", dest="EA")])
add_group("jmp",
    def_opersize_64=64,
    opcode=[0xFF],
    spare=4,
    operands=[Operand(type="Mem", dest="EA")])
add_group("jmp",
    parsers=["nasm"],
    opersize=16,
    def_opersize_64=64,
    opcode=[0xFF],
    spare=4,
    operands=[Operand(type="RM", size=16, tmod="Near", dest="EA")])
add_group("jmp",
    parsers=["nasm"],
    not64=True,
    cpu=["386"],
    opersize=32,
    opcode=[0xFF],
    spare=4,
    operands=[Operand(type="RM", size=32, tmod="Near", dest="EA")])
add_group("jmp",
    parsers=["nasm"],
    opersize=64,
    def_opersize_64=64,
    opcode=[0xFF],
    spare=4,
    operands=[Operand(type="RM", size=64, tmod="Near", dest="EA")])
add_group("jmp",
    parsers=["nasm"],
    def_opersize_64=64,
    opcode=[0xFF],
    spare=4,
    operands=[Operand(type="Mem", tmod="Near", dest="EA")])

# Far indirect (through memory).  Needs explicit FAR override.
for sz in [16, 32, 64]:
    add_group("jmp",
        opersize=sz,
        opcode=[0xFF],
        spare=5,
        operands=[Operand(type="Mem", size=sz, tmod="Far", dest="EA")])
add_group("jmp",
    opcode=[0xFF],
    spare=5,
    operands=[Operand(type="Mem", tmod="Far", dest="EA")])

# With explicit FAR override
for sz in [16, 32]:
    add_group("jmp",
        not64=True,
        opersize=sz,
        opcode=[0xEA],
        operands=[Operand(type="Imm", size=sz, tmod="Far", dest="JmpFar")])
add_group("jmp",
    not64=True,
    opcode=[0xEA],
    operands=[Operand(type="Imm", tmod="Far", dest="JmpFar")])

# Since not caught by first ImmNotSegOff group, implicitly FAR (in NASM).
for sz in [16, 32]:
    add_group("jmp",
        parsers=["nasm"],
        not64=True,
        opersize=sz,
        opcode=[0xEA],
        operands=[Operand(type="Imm", size=sz, dest="JmpFar")])
add_group("jmp",
    parsers=["nasm"],
    not64=True,
    opcode=[0xEA],
    operands=[Operand(type="Imm", dest="JmpFar")])

# Two-operand FAR (GAS only)
for sfx, sz in zip("wl", [16, 32]):
    add_group("jmp",
        parsers=["gas"],
        suffix=sfx,
        req_suffix=True,
        not64=True,
        gas_no_reverse=True,
        opersize=sz,
        opcode=[0xEA],
        operands=[Operand(type="Imm", size=16, relaxed=True, dest="JmpFar"),
                  Operand(type="Imm", size=sz, relaxed=True, dest="JmpFar")])
add_group("jmp",
    parsers=["gas"],
    not64=True,
    gas_no_reverse=True,
    opcode=[0xEA],
    operands=[Operand(type="Imm", size=16, relaxed=True, dest="JmpFar"),
              Operand(type="Imm", size="BITS", relaxed=True, dest="JmpFar")])

add_insn("jmp", "jmp")

#
# GAS far calls/jumps
#

# Far indirect (through memory)
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("ljmpcall",
        suffix=sfx,
        req_suffix=True,
        opersize=sz,
        modifiers=["SpAdd"],
        opcode=[0xFF],
        spare=0,
        operands=[Operand(type="Mem", size=sz, relaxed=True, dest="EA")])
add_group("ljmpcall",
    modifiers=["SpAdd"],
    opcode=[0xFF],
    spare=0,
    operands=[Operand(type="Mem", size="BITS", relaxed=True, dest="EA")])

# Two-operand far
for sfx, sz in zip("wl", [16, 32]):
    add_group("ljmpcall",
        not64=True,
        gas_no_reverse=True,
        suffix=sfx,
        req_suffix=True,
        opersize=sz,
        modifiers=["Gap", "Op0Add"],
        opcode=[0x00],
        operands=[Operand(type="Imm", size=16, relaxed=True, dest="JmpFar"),
                  Operand(type="Imm", size=sz, relaxed=True, dest="JmpFar")])
add_group("ljmpcall",
    not64=True,
    gas_no_reverse=True,
    modifiers=["Gap", "Op0Add"],
    opcode=[0x00],
    operands=[Operand(type="Imm", size=16, relaxed=True, dest="JmpFar"),
              Operand(type="Imm", size="BITS", relaxed=True, dest="JmpFar")])

add_insn("ljmp", "ljmpcall", parser="gas", modifiers=[5, 0xEA])
add_insn("lcall", "ljmpcall", parser="gas", modifiers=[3, 0x9A])

#
# ret
#
add_group("retnf",
    not64=True,
    modifiers=["Op0Add"],
    opcode=[0x01],
    operands=[])
add_group("retnf",
    not64=True,
    modifiers=["Op0Add"],
    opcode=[0x00],
    operands=[Operand(type="Imm", size=16, relaxed=True, dest="Imm")])
add_group("retnf",
    only64=True,
    modifiers=["Op0Add", "OpSizeR"],
    opcode=[0x01],
    operands=[])
add_group("retnf",
    only64=True,
    modifiers=["Op0Add", "OpSizeR"],
    opcode=[0x00],
    operands=[Operand(type="Imm", size=16, relaxed=True, dest="Imm")])
add_group("retnf",
    gen_suffix=False,
    suffixes=["w", "l", "q"],
    modifiers=["Op0Add", "OpSizeR"],
    opcode=[0x01],
    operands=[])
# GAS suffix versions
add_group("retnf",
    gen_suffix=False,
    suffixes=["w", "l", "q"],
    modifiers=["Op0Add", "OpSizeR"],
    opcode=[0x00],
    operands=[Operand(type="Imm", size=16, relaxed=True, dest="Imm")])

add_insn("ret", "retnf", modifiers=[0xC2])
add_insn("retw", "retnf", parser="gas", modifiers=[0xC2, 16])
add_insn("retl", "retnf", parser="gas", modifiers=[0xC2], not64=True)
add_insn("retq", "retnf", parser="gas", modifiers=[0xC2], only64=True)
add_insn("retn", "retnf", parser="nasm", modifiers=[0xC2])
add_insn("retf", "retnf", parser="nasm", modifiers=[0xCA, 64])
add_insn("lret", "retnf", parser="gas", modifiers=[0xCA], suffix="z")
add_insn("lretw", "retnf", parser="gas", modifiers=[0xCA, 16], suffix="w")
add_insn("lretl", "retnf", parser="gas", modifiers=[0xCA], suffix="l")
add_insn("lretq", "retnf", parser="gas", modifiers=[0xCA, 64], only64=True,
         suffix="q")

#
# enter
#
add_group("enter",
    suffix="l",
    not64=True,
    cpu=["186"],
    gas_no_reverse=True,
    opcode=[0xC8],
    operands=[
        Operand(type="Imm", size=16, relaxed=True, dest="EA", opt="A16"),
        Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("enter",
    suffix="q",
    only64=True,
    cpu=["186"],
    gas_no_reverse=True,
    opersize=64,
    def_opersize_64=64,
    opcode=[0xC8],
    operands=[
        Operand(type="Imm", size=16, relaxed=True, dest="EA", opt="A16"),
        Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
# GAS suffix version
add_group("enter",
    suffix="w",
    cpu=["186"],
    parsers=["gas"],
    gas_no_reverse=True,
    opersize=16,
    opcode=[0xC8],
    operands=[
        Operand(type="Imm", size=16, relaxed=True, dest="EA", opt="A16"),
        Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("enter", "enter")

#
# leave
#
add_insn("leave", "onebyte", modifiers=[0xC9, 0, 64], cpu=["186"])
add_insn("leavew", "onebyte", parser="gas", modifiers=[0xC9, 16, 0],
         cpu=["186"])
add_insn("leavel", "onebyte", parser="gas", modifiers=[0xC9, 0, 64],
         cpu=["186"])
add_insn("leaveq", "onebyte", parser="gas", modifiers=[0xC9, 0, 64],
         only64=True)

#####################################################################
# Conditional jumps
#####################################################################
add_group("jcc",
    opcode=[],
    operands=[Operand(type="Imm", dest="JmpRel")])
add_group("jcc",
    opersize=16,
    opcode=[],
    operands=[Operand(type="Imm", size=16, dest="JmpRel")])
add_group("jcc",
    not64=True,
    opersize=32,
    opcode=[],
    operands=[Operand(type="Imm", size=32, dest="JmpRel")])
add_group("jcc",
    only64=True,
    opersize=64,
    opcode=[],
    operands=[Operand(type="Imm", size=32, dest="JmpRel")])

add_group("jcc",
    modifiers=["Op0Add"],
    def_opersize_64=64,
    opcode=[0x70],
    operands=[Operand(type="Imm", tmod="Short", dest="JmpRel")])
add_group("jcc",
    cpu=["186"],
    modifiers=["Op1Add"],
    opersize=16,
    def_opersize_64=64,
    opcode=[0x0F, 0x80],
    operands=[Operand(type="Imm", size=16, tmod="Near", dest="JmpRel")])
add_group("jcc",
    not64=True,
    cpu=["386"],
    modifiers=["Op1Add"],
    opersize=32,
    opcode=[0x0F, 0x80],
    operands=[Operand(type="Imm", size=32, tmod="Near", dest="JmpRel")])
add_group("jcc",
    only64=True,
    modifiers=["Op1Add"],
    opersize=64,
    def_opersize_64=64,
    opcode=[0x0F, 0x80],
    operands=[Operand(type="Imm", size=32, tmod="Near", dest="JmpRel")])
add_group("jcc",
    cpu=["186"],
    modifiers=["Op1Add"],
    def_opersize_64=64,
    opcode=[0x0F, 0x80],
    operands=[Operand(type="Imm", tmod="Near", dest="JmpRel")])

add_insn("jo", "jcc", modifiers=[0x00])
add_insn("jno", "jcc", modifiers=[0x01])
add_insn("jb", "jcc", modifiers=[0x02])
add_insn("jc", "jcc", modifiers=[0x02])
add_insn("jnae", "jcc", modifiers=[0x02])
add_insn("jnb", "jcc", modifiers=[0x03])
add_insn("jnc", "jcc", modifiers=[0x03])
add_insn("jae", "jcc", modifiers=[0x03])
add_insn("je", "jcc", modifiers=[0x04])
add_insn("jz", "jcc", modifiers=[0x04])
add_insn("jne", "jcc", modifiers=[0x05])
add_insn("jnz", "jcc", modifiers=[0x05])
add_insn("jbe", "jcc", modifiers=[0x06])
add_insn("jna", "jcc", modifiers=[0x06])
add_insn("jnbe", "jcc", modifiers=[0x07])
add_insn("ja", "jcc", modifiers=[0x07])
add_insn("js", "jcc", modifiers=[0x08])
add_insn("jns", "jcc", modifiers=[0x09])
add_insn("jp", "jcc", modifiers=[0x0A])
add_insn("jpe", "jcc", modifiers=[0x0A])
add_insn("jnp", "jcc", modifiers=[0x0B])
add_insn("jpo", "jcc", modifiers=[0x0B])
add_insn("jl", "jcc", modifiers=[0x0C])
add_insn("jnge", "jcc", modifiers=[0x0C])
add_insn("jnl", "jcc", modifiers=[0x0D])
add_insn("jge", "jcc", modifiers=[0x0D])
add_insn("jle", "jcc", modifiers=[0x0E])
add_insn("jng", "jcc", modifiers=[0x0E])
add_insn("jnle", "jcc", modifiers=[0x0F])
add_insn("jg", "jcc", modifiers=[0x0F])

#
# jcxz
#
add_group("jcxz",
    modifiers=["AdSizeR"],
    opcode=[],
    operands=[Operand(type="Imm", dest="JmpRel")])
add_group("jcxz",
    modifiers=["AdSizeR"],
    def_opersize_64=64,
    opcode=[0xE3],
    operands=[Operand(type="Imm", tmod="Short", dest="JmpRel")])

add_insn("jcxz", "jcxz", modifiers=[16])
add_insn("jecxz", "jcxz", modifiers=[32], cpu=["386"])
add_insn("jrcxz", "jcxz", modifiers=[64], only64=True)

#####################################################################
# Loop instructions
#####################################################################
add_group("loop",
    opcode=[],
    operands=[Operand(type="Imm", dest="JmpRel")])
add_group("loop",
    not64=True,
    opcode=[],
    operands=[Operand(type="Imm", dest="JmpRel"),
              Operand(type="Creg", size=16, dest="AdSizeR")])
add_group("loop",
    def_opersize_64=64,
    opcode=[],
    operands=[Operand(type="Imm", dest="JmpRel"),
              Operand(type="Creg", size=32, dest="AdSizeR")])
add_group("loop",
    def_opersize_64=64,
    opcode=[],
    operands=[Operand(type="Imm", dest="JmpRel"),
              Operand(type="Creg", size=64, dest="AdSizeR")])

add_group("loop",
    not64=True,
    modifiers=["Op0Add"],
    opcode=[0xE0],
    operands=[Operand(type="Imm", tmod="Short", dest="JmpRel")])
for sz in [16, 32, 64]:
    add_group("loop",
        modifiers=["Op0Add"],
        def_opersize_64=64,
        opcode=[0xE0],
        operands=[Operand(type="Imm", tmod="Short", dest="JmpRel"),
                  Operand(type="Creg", size=sz, dest="AdSizeR")])

add_insn("loop", "loop", modifiers=[2])
add_insn("loopz", "loop", modifiers=[1])
add_insn("loope", "loop", modifiers=[1])
add_insn("loopnz", "loop", modifiers=[0])
add_insn("loopne", "loop", modifiers=[0])

# GAS w/l/q suffixes have to set addrsize via modifiers
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("loop"+sfx,
        not64=(sz == 16),
        only64=(sz == 64),
        modifiers=["Gap", "AdSizeR"],
        def_opersize_64=64,
        opcode=[],
        operands=[Operand(type="Imm", dest="JmpRel")])
    add_group("loop"+sfx,
        not64=(sz == 16),
        only64=(sz == 64),
        modifiers=["Op0Add", "AdSizeR"],
        def_opersize_64=64,
        opcode=[0xE0],
        operands=[Operand(type="Imm", tmod="Short", dest="JmpRel")])

    add_group("loop"+sfx,
        not64=(sz == 16),
        only64=(sz == 64),
        def_opersize_64=64,
        opcode=[],
        operands=[Operand(type="Imm", dest="JmpRel"),
                  Operand(type="Creg", size=sz, dest="AdSizeR")])
    add_group("loop"+sfx,
        not64=(sz == 16),
        only64=(sz == 64),
        modifiers=["Op0Add"],
        def_opersize_64=64,
        opcode=[0xE0],
        operands=[Operand(type="Imm", tmod="Short", dest="JmpRel"),
                  Operand(type="Creg", size=sz, dest="AdSizeR")])

    add_insn("loop"+sfx, "loop"+sfx, parser="gas", modifiers=[2, sz])
    add_insn("loopz"+sfx, "loop"+sfx, parser="gas", modifiers=[1, sz])
    add_insn("loope"+sfx, "loop"+sfx, parser="gas", modifiers=[1, sz])
    add_insn("loopnz"+sfx, "loop"+sfx, parser="gas", modifiers=[0, sz])
    add_insn("loopne"+sfx, "loop"+sfx, parser="gas", modifiers=[0, sz])

#####################################################################
# Set byte on flag instructions
#####################################################################
add_group("setcc",
    suffix="b",
    cpu=["386"],
    modifiers=["Op1Add"],
    opcode=[0x0F, 0x90],
    spare=2,
    operands=[Operand(type="RM", size=8, relaxed=True, dest="EA")])

add_insn("seto", "setcc", modifiers=[0x00])
add_insn("setno", "setcc", modifiers=[0x01])
add_insn("setb", "setcc", modifiers=[0x02])
add_insn("setc", "setcc", modifiers=[0x02])
add_insn("setnae", "setcc", modifiers=[0x02])
add_insn("setnb", "setcc", modifiers=[0x03])
add_insn("setnc", "setcc", modifiers=[0x03])
add_insn("setae", "setcc", modifiers=[0x03])
add_insn("sete", "setcc", modifiers=[0x04])
add_insn("setz", "setcc", modifiers=[0x04])
add_insn("setne", "setcc", modifiers=[0x05])
add_insn("setnz", "setcc", modifiers=[0x05])
add_insn("setbe", "setcc", modifiers=[0x06])
add_insn("setna", "setcc", modifiers=[0x06])
add_insn("setnbe", "setcc", modifiers=[0x07])
add_insn("seta", "setcc", modifiers=[0x07])
add_insn("sets", "setcc", modifiers=[0x08])
add_insn("setns", "setcc", modifiers=[0x09])
add_insn("setp", "setcc", modifiers=[0x0A])
add_insn("setpe", "setcc", modifiers=[0x0A])
add_insn("setnp", "setcc", modifiers=[0x0B])
add_insn("setpo", "setcc", modifiers=[0x0B])
add_insn("setl", "setcc", modifiers=[0x0C])
add_insn("setnge", "setcc", modifiers=[0x0C])
add_insn("setnl", "setcc", modifiers=[0x0D])
add_insn("setge", "setcc", modifiers=[0x0D])
add_insn("setle", "setcc", modifiers=[0x0E])
add_insn("setng", "setcc", modifiers=[0x0E])
add_insn("setnle", "setcc", modifiers=[0x0F])
add_insn("setg", "setcc", modifiers=[0x0F])

#####################################################################
# String instructions
#####################################################################
add_insn("cmpsb", "onebyte", modifiers=[0xA6, 0])
add_insn("cmpsw", "onebyte", modifiers=[0xA7, 16])

# cmpsd has to be non-onebyte for SSE2 forms below
add_group("cmpsd",
    parsers=["nasm"],
    notavx=True,
    opersize=32,
    opcode=[0xA7],
    operands=[])

add_insn("cmpsd", "cmpsd", cpu=[])

add_insn("cmpsl", "onebyte", parser="gas", modifiers=[0xA7, 32], cpu=["386"])
add_insn("cmpsq", "onebyte", modifiers=[0xA7, 64], only64=True)
add_insn("insb", "onebyte", modifiers=[0x6C, 0])
add_insn("insw", "onebyte", modifiers=[0x6D, 16])
add_insn("insd", "onebyte", parser="nasm", modifiers=[0x6D, 32], cpu=["386"])
add_insn("insl", "onebyte", parser="gas", modifiers=[0x6D, 32], cpu=["386"])
add_insn("outsb", "onebyte", modifiers=[0x6E, 0])
add_insn("outsw", "onebyte", modifiers=[0x6F, 16])
add_insn("outsd", "onebyte", parser="nasm", modifiers=[0x6F, 32],
         cpu=["386"])
add_insn("outsl", "onebyte", parser="gas", modifiers=[0x6F, 32], cpu=["386"])
add_insn("lodsb", "onebyte", modifiers=[0xAC, 0])
add_insn("lodsw", "onebyte", modifiers=[0xAD, 16])
add_insn("lodsd", "onebyte", parser="nasm", modifiers=[0xAD, 32],
         cpu=["386"])
add_insn("lodsl", "onebyte", parser="gas", modifiers=[0xAD, 32], cpu=["386"])
add_insn("lodsq", "onebyte", modifiers=[0xAD, 64], only64=True)
add_insn("movsb", "onebyte", modifiers=[0xA4, 0])
add_insn("movsw", "onebyte", modifiers=[0xA5, 16])

# movsd has to be non-onebyte for SSE2 forms below
add_group("movsd",
    parsers=["nasm", "gas"],
    notavx=True,
    opersize=32,
    opcode=[0xA5],
    operands=[])

add_insn("movsd", "movsd", cpu=["386"])

add_insn("movsl", "onebyte", parser="gas", modifiers=[0xA5, 32], cpu=["386"])
add_insn("movsq", "onebyte", modifiers=[0xA5, 64], only64=True)
# smov alias for movs in GAS mode
add_insn("smovb", "onebyte", parser="gas", modifiers=[0xA4, 0])
add_insn("smovw", "onebyte", parser="gas", modifiers=[0xA5, 16])
add_insn("smovl", "onebyte", parser="gas", modifiers=[0xA5, 32], cpu=["386"])
add_insn("smovq", "onebyte", parser="gas", modifiers=[0xA5, 64], only64=True)
add_insn("scasb", "onebyte", modifiers=[0xAE, 0])
add_insn("scasw", "onebyte", modifiers=[0xAF, 16])
add_insn("scasd", "onebyte", parser="nasm", modifiers=[0xAF, 32],
         cpu=["386"])
add_insn("scasl", "onebyte", parser="gas", modifiers=[0xAF, 32], cpu=["386"])
add_insn("scasq", "onebyte", modifiers=[0xAF, 64], only64=True)
# ssca alias for scas in GAS mode
add_insn("sscab", "onebyte", parser="gas", modifiers=[0xAE, 0])
add_insn("sscaw", "onebyte", parser="gas", modifiers=[0xAF, 16])
add_insn("sscal", "onebyte", parser="gas", modifiers=[0xAF, 32], cpu=["386"])
add_insn("sscaq", "onebyte", parser="gas", modifiers=[0xAF, 64], only64=True)
add_insn("stosb", "onebyte", modifiers=[0xAA, 0])
add_insn("stosw", "onebyte", modifiers=[0xAB, 16])
add_insn("stosd", "onebyte", parser="nasm", modifiers=[0xAB, 32],
         cpu=["386"])
add_insn("stosl", "onebyte", parser="gas", modifiers=[0xAB, 32], cpu=["386"])
add_insn("stosq", "onebyte", modifiers=[0xAB, 64], only64=True)
add_insn("xlatb", "onebyte", modifiers=[0xD7, 0])

#####################################################################
# Bit manipulation
#####################################################################

#
# bit tests
#
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("bittest",
        suffix=sfx,
        cpu=["386"],
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x00],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="Spare")])
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("bittest",
        suffix=sfx,
        cpu=["386"],
        modifiers=["Gap", "SpAdd"],
        opersize=sz,
        opcode=[0x0F, 0xBA],
        spare=0,
        operands=[Operand(type="RM", size=sz, dest="EA"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("bt",  "bittest", modifiers=[0xA3, 4])
add_insn("bts", "bittest", modifiers=[0xAB, 5])
add_insn("btr", "bittest", modifiers=[0xB3, 6])
add_insn("btc", "bittest", modifiers=[0xBB, 7])

#
# bit scans - also used for lar/lsl
#
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("bsfr",
        suffix=sfx,
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x00],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA")])

add_insn("bsf", "bsfr", modifiers=[0xBC], cpu=["386"])
add_insn("bsr", "bsfr", modifiers=[0xBD], cpu=["386"])

#####################################################################
# Interrupts and operating system instructions
#####################################################################
add_group("int",
    opcode=[0xCD],
    operands=[Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("int", "int")
add_insn("int3", "onebyte", modifiers=[0xCC])
add_insn("int03", "onebyte", parser="nasm", modifiers=[0xCC])
add_insn("into", "onebyte", modifiers=[0xCE], not64=True)
add_insn("iret", "onebyte", modifiers=[0xCF])
add_insn("iretw", "onebyte", modifiers=[0xCF, 16])
add_insn("iretd", "onebyte", parser="nasm", modifiers=[0xCF, 32],
         cpu=["386"])
add_insn("iretl", "onebyte", parser="gas", modifiers=[0xCF, 32], cpu=["386"])
add_insn("iretq", "onebyte", modifiers=[0xCF, 64], only64=True)
add_insn("rsm", "twobyte", modifiers=[0x0F, 0xAA], cpu=["586", "SMM"])

for sfx, sz in zip("wl", [16, 32]):
    add_group("bound",
        suffix=sfx,
        cpu=["186"],
        not64=True,
        opersize=sz,
        opcode=[0x62],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="Mem", size=sz, relaxed=True, dest="EA")])

add_insn("bound", "bound")
add_insn("hlt", "onebyte", modifiers=[0xF4], cpu=["Priv"])
add_insn("nop", "onebyte", modifiers=[0x90])

#
# Protection control
#
for sfx, sz, sz2 in zip("wlq", [16, 32, 64], [16, 32, 32]):
    add_group("larlsl",
        suffix=sfx,
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x00],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="Reg", size=sz2, dest="EA")])
    add_group("larlsl",
        suffix=sfx,
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x00],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=16, relaxed=True, dest="EA")])

add_insn("lar", "larlsl", modifiers=[0x02], cpu=["286", "Prot"])
add_insn("lsl", "larlsl", modifiers=[0x03], cpu=["286", "Prot"])

add_group("arpl",
    suffix="w",
    cpu=["Prot", "286"],
    not64=True,
    opcode=[0x63],
    operands=[Operand(type="RM", size=16, relaxed=True, dest="EA"),
              Operand(type="Reg", size=16, dest="Spare")])

add_insn("arpl", "arpl")

for sfx in [None, "w", "l", "q"]:
    add_insn("lgdt"+(sfx or ""), "twobytemem", suffix=sfx,
             modifiers=[2, 0x0F, 0x01], cpu=["286", "Priv"])
    add_insn("lidt"+(sfx or ""), "twobytemem", suffix=sfx,
             modifiers=[3, 0x0F, 0x01], cpu=["286", "Priv"])
    add_insn("sgdt"+(sfx or ""), "twobytemem", suffix=sfx,
             modifiers=[0, 0x0F, 0x01], cpu=["286", "Priv"])
    add_insn("sidt"+(sfx or ""), "twobytemem", suffix=sfx,
             modifiers=[1, 0x0F, 0x01], cpu=["286", "Priv"])

for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("str",
        suffix=sfx,
        cpu=["Prot", "286"],
        opersize=sz,
        opcode=[0x0F, 0x00],
        spare=1,
        operands=[Operand(type="Reg", size=sz, dest="EA")])
add_group("str",
    suffixes=["w", "l"],
    cpu=["Prot", "286"],
    opcode=[0x0F, 0x00],
    spare=1,
    operands=[Operand(type="RM", size=16, relaxed=True, dest="EA")])

add_insn("str", "str")

add_group("prot286",
    suffix="w",
    cpu=["286"],
    modifiers=["SpAdd", "Op1Add"],
    opcode=[0x0F, 0x00],
    spare=0,
    operands=[Operand(type="RM", size=16, relaxed=True, dest="EA")])

add_insn("lldt", "prot286", modifiers=[2, 0], cpu=["286", "Prot", "Priv"])
add_insn("ltr", "prot286", modifiers=[3, 0], cpu=["286", "Prot", "Priv"])
add_insn("verr", "prot286", modifiers=[4, 0], cpu=["286", "Prot"])
add_insn("verw", "prot286", modifiers=[5, 0], cpu=["286", "Prot"])
add_insn("lmsw", "prot286", modifiers=[6, 1], cpu=["286", "Priv"])

for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("sldtmsw",
        suffix=sfx,
        only64=(sz==64),
        cpu=[(sz==32) and "386" or "286"],
        modifiers=["SpAdd", "Op1Add"],
        opcode=[0x0F, 0x00],
        spare=0,
        operands=[Operand(type="Mem", size=sz, relaxed=True, dest="EA")])
for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("sldtmsw",
        suffix=sfx,
        cpu=["286"],
        modifiers=["SpAdd", "Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x00],
        spare=0,
        operands=[Operand(type="Reg", size=sz, dest="EA")])

add_insn("sldt", "sldtmsw", modifiers=[0, 0])
add_insn("smsw", "sldtmsw", modifiers=[4, 1])

#####################################################################
# Floating point instructions
#####################################################################
add_insn("fcompp",  "twobyte", modifiers=[0xDE, 0xD9], cpu=["FPU"])
add_insn("fucompp", "twobyte", modifiers=[0xDA, 0xE9], cpu=["286", "FPU"])
add_insn("ftst",    "twobyte", modifiers=[0xD9, 0xE4], cpu=["FPU"])
add_insn("fxam",    "twobyte", modifiers=[0xD9, 0xE5], cpu=["FPU"])
add_insn("fld1",    "twobyte", modifiers=[0xD9, 0xE8], cpu=["FPU"])
add_insn("fldl2t",  "twobyte", modifiers=[0xD9, 0xE9], cpu=["FPU"])
add_insn("fldl2e",  "twobyte", modifiers=[0xD9, 0xEA], cpu=["FPU"])
add_insn("fldpi",   "twobyte", modifiers=[0xD9, 0xEB], cpu=["FPU"])
add_insn("fldlg2",  "twobyte", modifiers=[0xD9, 0xEC], cpu=["FPU"])
add_insn("fldln2",  "twobyte", modifiers=[0xD9, 0xED], cpu=["FPU"])
add_insn("fldz",    "twobyte", modifiers=[0xD9, 0xEE], cpu=["FPU"])
add_insn("f2xm1",   "twobyte", modifiers=[0xD9, 0xF0], cpu=["FPU"])
add_insn("fyl2x",   "twobyte", modifiers=[0xD9, 0xF1], cpu=["FPU"])
add_insn("fptan",   "twobyte", modifiers=[0xD9, 0xF2], cpu=["FPU"])
add_insn("fpatan",  "twobyte", modifiers=[0xD9, 0xF3], cpu=["FPU"])
add_insn("fxtract", "twobyte", modifiers=[0xD9, 0xF4], cpu=["FPU"])
add_insn("fprem1",  "twobyte", modifiers=[0xD9, 0xF5], cpu=["286", "FPU"])
add_insn("fdecstp", "twobyte", modifiers=[0xD9, 0xF6], cpu=["FPU"])
add_insn("fincstp", "twobyte", modifiers=[0xD9, 0xF7], cpu=["FPU"])
add_insn("fprem",   "twobyte", modifiers=[0xD9, 0xF8], cpu=["FPU"])
add_insn("fyl2xp1", "twobyte", modifiers=[0xD9, 0xF9], cpu=["FPU"])
add_insn("fsqrt",   "twobyte", modifiers=[0xD9, 0xFA], cpu=["FPU"])
add_insn("fsincos", "twobyte", modifiers=[0xD9, 0xFB], cpu=["286", "FPU"])
add_insn("frndint", "twobyte", modifiers=[0xD9, 0xFC], cpu=["FPU"])
add_insn("fscale",  "twobyte", modifiers=[0xD9, 0xFD], cpu=["FPU"])
add_insn("fsin",    "twobyte", modifiers=[0xD9, 0xFE], cpu=["286", "FPU"])
add_insn("fcos",    "twobyte", modifiers=[0xD9, 0xFF], cpu=["286", "FPU"])
add_insn("fchs",    "twobyte", modifiers=[0xD9, 0xE0], cpu=["FPU"])
add_insn("fabs",    "twobyte", modifiers=[0xD9, 0xE1], cpu=["FPU"])
add_insn("fninit",  "twobyte", modifiers=[0xDB, 0xE3], cpu=["FPU"])
add_insn("finit", "threebyte", modifiers=[0x9B, 0xDB, 0xE3], cpu=["FPU"])
add_insn("fnclex",  "twobyte", modifiers=[0xDB, 0xE2], cpu=["FPU"])
add_insn("fclex", "threebyte", modifiers=[0x9B, 0xDB, 0xE2], cpu=["FPU"])
for sfx in [None, "l", "s"]:
    add_insn("fnstenv"+(sfx or ""), "onebytemem", suffix=sfx,
             modifiers=[6, 0xD9], cpu=["FPU"])
    add_insn("fstenv"+(sfx or ""),  "twobytemem", suffix=sfx,
             modifiers=[6, 0x9B, 0xD9], cpu=["FPU"])
    add_insn("fldenv"+(sfx or ""),  "onebytemem", suffix=sfx,
             modifiers=[4, 0xD9], cpu=["FPU"])
    add_insn("fnsave"+(sfx or ""),  "onebytemem", suffix=sfx,
             modifiers=[6, 0xDD], cpu=["FPU"])
    add_insn("fsave"+(sfx or ""),   "twobytemem", suffix=sfx,
             modifiers=[6, 0x9B, 0xDD], cpu=["FPU"])
    add_insn("frstor"+(sfx or ""),  "onebytemem", suffix=sfx,
             modifiers=[4, 0xDD], cpu=["FPU"])
add_insn("fnop",    "twobyte", modifiers=[0xD9, 0xD0], cpu=["FPU"])
add_insn("fwait",   "onebyte", modifiers=[0x9B], cpu=["FPU"])
# Prefixes; should the others be here too? should wait be a prefix?
add_insn("wait",    "onebyte", modifiers=[0x9B])

#
# load/store with pop (integer and normal)
#
add_group("fld",
    suffix="s",
    cpu=["FPU"],
    opcode=[0xD9],
    operands=[Operand(type="Mem", size=32, dest="EA")])
add_group("fld",
    suffix="l",
    cpu=["FPU"],
    opcode=[0xDD],
    operands=[Operand(type="Mem", size=64, dest="EA")])
add_group("fld",
    cpu=["FPU"],
    opcode=[0xDB],
    spare=5,
    operands=[Operand(type="Mem", size=80, dest="EA")])
add_group("fld",
    cpu=["FPU"],
    opcode=[0xD9, 0xC0],
    operands=[Operand(type="Reg", size=80, dest="Op1Add")])

add_insn("fld", "fld")

add_group("fstp",
    suffix="s",
    cpu=["FPU"],
    opcode=[0xD9],
    spare=3,
    operands=[Operand(type="Mem", size=32, dest="EA")])
add_group("fstp",
    suffix="l",
    cpu=["FPU"],
    opcode=[0xDD],
    spare=3,
    operands=[Operand(type="Mem", size=64, dest="EA")])
add_group("fstp",
    cpu=["FPU"],
    opcode=[0xDB],
    spare=7,
    operands=[Operand(type="Mem", size=80, dest="EA")])
add_group("fstp",
    cpu=["FPU"],
    opcode=[0xDD, 0xD8],
    operands=[Operand(type="Reg", size=80, dest="Op1Add")])

add_insn("fstp", "fstp")

#
# Long memory version of floating point load/store for GAS
#
add_group("fldstpt",
    cpu=["FPU"],
    modifiers=["SpAdd"],
    opcode=[0xDB],
    spare=0,
    operands=[Operand(type="Mem", size=80, relaxed=True, dest="EA")])

add_insn("fldt", "fldstpt", modifiers=[5])
add_insn("fstpt", "fldstpt", modifiers=[7])

add_group("fildstp",
    suffix="s",
    cpu=["FPU"],
    modifiers=["SpAdd"],
    opcode=[0xDF],
    spare=0,
    operands=[Operand(type="Mem", size=16, dest="EA")])
add_group("fildstp",
    suffix="l",
    cpu=["FPU"],
    modifiers=["SpAdd"],
    opcode=[0xDB],
    spare=0,
    operands=[Operand(type="Mem", size=32, dest="EA")])
add_group("fildstp",
    suffix="q",
    cpu=["FPU"],
    modifiers=["Gap", "Op0Add", "SpAdd"],
    opcode=[0xDD],
    spare=0,
    operands=[Operand(type="Mem", size=64, dest="EA")])
# No-suffix alias for memory for GAS compat -> "s" version generated
add_group("fildstp",
    cpu=["FPU"],
    parsers=["gas"],
    modifiers=["SpAdd"],
    opcode=[0xDF],
    spare=0,
    operands=[Operand(type="Mem", size=16, relaxed=True, dest="EA")])

add_insn("fild", "fildstp", modifiers=[0, 2, 5])
add_insn("fistp", "fildstp", modifiers=[3, 2, 7])

add_group("fbldstp",
    cpu=["FPU"],
    modifiers=["SpAdd"],
    opcode=[0xDF],
    spare=0,
    operands=[Operand(type="Mem", size=80, relaxed=True, dest="EA")])

add_insn("fbld", "fbldstp", modifiers=[4])
add_insn("fildll", "fbldstp", parser="gas", modifiers=[5])
add_insn("fbstp", "fbldstp", modifiers=[6])
add_insn("fistpll", "fbldstp", parser="gas", modifiers=[7])

#
# store (normal)
#
add_group("fst",
    suffix="s",
    cpu=["FPU"],
    opcode=[0xD9],
    spare=2,
    operands=[Operand(type="Mem", size=32, dest="EA")])
add_group("fst",
    suffix="l",
    cpu=["FPU"],
    opcode=[0xDD],
    spare=2,
    operands=[Operand(type="Mem", size=64, dest="EA")])
add_group("fst",
    cpu=["FPU"],
    opcode=[0xDD, 0xD0],
    operands=[Operand(type="Reg", size=80, dest="Op1Add")])

add_insn("fst", "fst")

#
# exchange (with ST0)
#
add_group("fxch",
    cpu=["FPU"],
    opcode=[0xD9, 0xC8],
    operands=[Operand(type="Reg", size=80, dest="Op1Add")])
add_group("fxch",
    cpu=["FPU"],
    opcode=[0xD9, 0xC8],
    operands=[Operand(type="ST0", size=80, dest=None),
              Operand(type="Reg", size=80, dest="Op1Add")])
add_group("fxch",
    cpu=["FPU"],
    opcode=[0xD9, 0xC8],
    operands=[Operand(type="Reg", size=80, dest="Op1Add"),
              Operand(type="ST0", size=80, dest=None)])
add_group("fxch",
    cpu=["FPU"],
    opcode=[0xD9, 0xC9],
    operands=[])

add_insn("fxch", "fxch")

#
# comparisons
#
add_group("fcom",
    suffix="s",
    cpu=["FPU"],
    modifiers=["Gap", "SpAdd"],
    opcode=[0xD8],
    spare=0,
    operands=[Operand(type="Mem", size=32, dest="EA")])
add_group("fcom",
    suffix="l",
    cpu=["FPU"],
    modifiers=["Gap", "SpAdd"],
    opcode=[0xDC],
    spare=0,
    operands=[Operand(type="Mem", size=64, dest="EA")])
add_group("fcom",
    cpu=["FPU"],
    modifiers=["Op1Add"],
    opcode=[0xD8, 0x00],
    operands=[Operand(type="Reg", size=80, dest="Op1Add")])
# No-suffix alias for memory for GAS compat -> "s" version generated
add_group("fcom",
    cpu=["FPU"],
    parsers=["gas"],
    modifiers=["Gap", "SpAdd"],
    opcode=[0xD8],
    spare=0,
    operands=[Operand(type="Mem", size=32, relaxed=True, dest="EA")])
# Alias for fcom %st(1) for GAS compat
add_group("fcom",
    cpu=["FPU"],
    parsers=["gas"],
    modifiers=["Op1Add"],
    opcode=[0xD8, 0x01],
    operands=[])
add_group("fcom",
    cpu=["FPU"],
    parsers=["nasm"],
    modifiers=["Op1Add"],
    opcode=[0xD8, 0x00],
    operands=[Operand(type="ST0", size=80, dest=None),
              Operand(type="Reg", size=80, dest="Op1Add")])

add_insn("fcom", "fcom", modifiers=[0xD0, 2])
add_insn("fcomp", "fcom", modifiers=[0xD8, 3])

#
# extended comparisons
#
add_group("fcom2",
    cpu=["FPU", "286"],
    modifiers=["Op0Add", "Op1Add"],
    opcode=[0x00, 0x00],
    operands=[Operand(type="Reg", size=80, dest="Op1Add")])
add_group("fcom2",
    cpu=["FPU", "286"],
    modifiers=["Op0Add", "Op1Add"],
    opcode=[0x00, 0x00],
    operands=[Operand(type="ST0", size=80, dest=None),
              Operand(type="Reg", size=80, dest="Op1Add")])

add_insn("fucom", "fcom2", modifiers=[0xDD, 0xE0])
add_insn("fucomp", "fcom2", modifiers=[0xDD, 0xE8])

#
# arithmetic
#
add_group("farith",
    suffix="s",
    cpu=["FPU"],
    modifiers=["Gap", "Gap", "SpAdd"],
    opcode=[0xD8],
    spare=0,
    operands=[Operand(type="Mem", size=32, dest="EA")])
add_group("farith",
    suffix="l",
    cpu=["FPU"],
    modifiers=["Gap", "Gap", "SpAdd"],
    opcode=[0xDC],
    spare=0,
    operands=[Operand(type="Mem", size=64, dest="EA")])
add_group("farith",
    cpu=["FPU"],
    modifiers=["Gap", "Op1Add"],
    opcode=[0xD8, 0x00],
    operands=[Operand(type="Reg", size=80, dest="Op1Add")])
add_group("farith",
    cpu=["FPU"],
    modifiers=["Gap", "Op1Add"],
    opcode=[0xD8, 0x00],
    operands=[Operand(type="ST0", size=80, dest=None),
              Operand(type="Reg", size=80, dest="Op1Add")])
add_group("farith",
    cpu=["FPU"],
    modifiers=["Op1Add"],
    opcode=[0xDC, 0x00],
    operands=[Operand(type="Reg", size=80, tmod="To", dest="Op1Add")])
add_group("farith",
    cpu=["FPU"],
    parsers=["nasm"],
    modifiers=["Op1Add"],
    opcode=[0xDC, 0x00],
    operands=[Operand(type="Reg", size=80, dest="Op1Add"),
              Operand(type="ST0", size=80, dest=None)])
add_group("farith",
    cpu=["FPU"],
    parsers=["gas"],
    modifiers=["Gap", "Op1Add"],
    opcode=[0xDC, 0x00],
    operands=[Operand(type="Reg", size=80, dest="Op1Add"),
              Operand(type="ST0", size=80, dest=None)])

add_insn("fadd", "farith", modifiers=[0xC0, 0xC0, 0])
add_insn("fsub", "farith", modifiers=[0xE8, 0xE0, 4])
add_insn("fsubr", "farith", modifiers=[0xE0, 0xE8, 5])
add_insn("fmul", "farith", modifiers=[0xC8, 0xC8, 1])
add_insn("fdiv", "farith", modifiers=[0xF8, 0xF0, 6])
add_insn("fdivr", "farith", modifiers=[0xF0, 0xF8, 7])

add_group("farithp",
    cpu=["FPU"],
    modifiers=["Op1Add"],
    opcode=[0xDE, 0x01],
    operands=[])
add_group("farithp",
    cpu=["FPU"],
    modifiers=["Op1Add"],
    opcode=[0xDE, 0x00],
    operands=[Operand(type="Reg", size=80, dest="Op1Add")])
add_group("farithp",
    cpu=["FPU"],
    modifiers=["Op1Add"],
    opcode=[0xDE, 0x00],
    operands=[Operand(type="Reg", size=80, dest="Op1Add"),
              Operand(type="ST0", size=80, dest=None)])

add_insn("faddp", "farithp", modifiers=[0xC0])
add_insn("fsubp", "farithp", parser="nasm", modifiers=[0xE8])
add_insn("fsubp", "farithp", parser="gas", modifiers=[0xE0])
add_insn("fsubrp", "farithp", parser="nasm", modifiers=[0xE0])
add_insn("fsubrp", "farithp", parser="gas", modifiers=[0xE8])
add_insn("fmulp", "farithp", modifiers=[0xC8])
add_insn("fdivp", "farithp", parser="nasm", modifiers=[0xF8])
add_insn("fdivp", "farithp", parser="gas", modifiers=[0xF0])
add_insn("fdivrp", "farithp", parser="nasm", modifiers=[0xF0])
add_insn("fdivrp", "farithp", parser="gas", modifiers=[0xF8])

#
# integer arith/store wo pop/compare
#
add_group("fiarith",
    suffix="s",
    cpu=["FPU"],
    modifiers=["SpAdd", "Op0Add"],
    opcode=[0x04],
    spare=0,
    operands=[Operand(type="Mem", size=16, dest="EA")])
add_group("fiarith",
    suffix="l",
    cpu=["FPU"],
    modifiers=["SpAdd", "Op0Add"],
    opcode=[0x00],
    spare=0,
    operands=[Operand(type="Mem", size=32, dest="EA")])

add_insn("fist",   "fiarith", modifiers=[2, 0xDB])
add_insn("ficom",  "fiarith", modifiers=[2, 0xDA])
add_insn("ficomp", "fiarith", modifiers=[3, 0xDA])
add_insn("fiadd",  "fiarith", modifiers=[0, 0xDA])
add_insn("fisub",  "fiarith", modifiers=[4, 0xDA])
add_insn("fisubr", "fiarith", modifiers=[5, 0xDA])
add_insn("fimul",  "fiarith", modifiers=[1, 0xDA])
add_insn("fidiv",  "fiarith", modifiers=[6, 0xDA])
add_insn("fidivr", "fiarith", modifiers=[7, 0xDA])

#
# processor control
#
add_group("fldnstcw",
    suffix="w",
    cpu=["FPU"],
    modifiers=["SpAdd"],
    opcode=[0xD9],
    spare=0,
    operands=[Operand(type="Mem", size=16, relaxed=True, dest="EA")])

add_insn("fldcw", "fldnstcw", modifiers=[5])
add_insn("fnstcw", "fldnstcw", modifiers=[7])

add_group("fstcw",
    suffix="w",
    cpu=["FPU"],
    opcode=[0x9B, 0xD9],
    spare=7,
    operands=[Operand(type="Mem", size=16, relaxed=True, dest="EA")])

add_insn("fstcw", "fstcw")

add_group("fnstsw",
    suffix="w",
    cpu=["FPU"],
    opcode=[0xDD],
    spare=7,
    operands=[Operand(type="Mem", size=16, relaxed=True, dest="EA")])
add_group("fnstsw",
    suffix="w",
    cpu=["FPU"],
    opcode=[0xDF, 0xE0],
    operands=[Operand(type="Areg", size=16, dest=None)])

add_insn("fnstsw", "fnstsw")

add_group("fstsw",
    suffix="w",
    cpu=["FPU"],
    opcode=[0x9B, 0xDD],
    spare=7,
    operands=[Operand(type="Mem", size=16, relaxed=True, dest="EA")])
add_group("fstsw",
    suffix="w",
    cpu=["FPU"],
    opcode=[0x9B, 0xDF, 0xE0],
    operands=[Operand(type="Areg", size=16, dest=None)])

add_insn("fstsw", "fstsw")

add_group("ffree",
    cpu=["FPU"],
    modifiers=["Op0Add"],
    opcode=[0x00, 0xC0],
    operands=[Operand(type="Reg", size=80, dest="Op1Add")])

add_insn("ffree", "ffree", modifiers=[0xDD])
add_insn("ffreep", "ffree", modifiers=[0xDF], cpu=["686", "FPU", "Undoc"])

#####################################################################
# 486 extensions
#####################################################################
add_group("bswap",
    suffix="l",
    cpu=["486"],
    opersize=32,
    opcode=[0x0F, 0xC8],
    operands=[Operand(type="Reg", size=32, dest="Op1Add")])
add_group("bswap",
    suffix="q",
    opersize=64,
    opcode=[0x0F, 0xC8],
    operands=[Operand(type="Reg", size=64, dest="Op1Add")])

add_insn("bswap", "bswap")

for sfx, sz in zip("bwlq", [8, 16, 32, 64]):
    add_group("cmpxchgxadd",
        suffix=sfx,
        cpu=["486"],
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x00+(sz!=8)],
        operands=[Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="Spare")])

add_insn("xadd", "cmpxchgxadd", modifiers=[0xC0])
add_insn("cmpxchg", "cmpxchgxadd", modifiers=[0xB0])
add_insn("cmpxchg486", "cmpxchgxadd", parser="nasm", modifiers=[0xA6],
         cpu=["486", "Undoc"])

add_insn("invd", "twobyte", modifiers=[0x0F, 0x08], cpu=["486", "Priv"])
add_insn("wbinvd", "twobyte", modifiers=[0x0F, 0x09], cpu=["486", "Priv"])
add_insn("invlpg", "twobytemem", modifiers=[7, 0x0F, 0x01],
         cpu=["486", "Priv"])

#####################################################################
# 586+ and late 486 extensions
#####################################################################
add_insn("cpuid", "twobyte", modifiers=[0x0F, 0xA2], cpu=["486"])

#####################################################################
# Pentium extensions
#####################################################################
add_insn("wrmsr", "twobyte", modifiers=[0x0F, 0x30], cpu=["586", "Priv"])
add_insn("rdtsc", "twobyte", modifiers=[0x0F, 0x31], cpu=["586"])
add_insn("rdmsr", "twobyte", modifiers=[0x0F, 0x32], cpu=["586", "Priv"])

add_group("cmpxchg8b",
    suffix="q",
    cpu=["586"],
    opcode=[0x0F, 0xC7],
    spare=1,
    operands=[Operand(type="Mem", size=64, relaxed=True, dest="EA")])

add_insn("cmpxchg8b", "cmpxchg8b")

#####################################################################
# Pentium II/Pentium Pro extensions
#####################################################################
add_insn("sysenter", "twobyte", modifiers=[0x0F, 0x34], cpu=["686"],
         not64=True)
add_insn("sysexit",  "twobyte", modifiers=[0x0F, 0x35], cpu=["686", "Priv"],
         not64=True)
for sfx in [None, "q"]:
    add_insn("fxsave"+(sfx or ""),  "twobytemem", suffix=sfx,
             modifiers=[0, 0x0F, 0xAE], cpu=["686", "FPU"])
    add_insn("fxrstor"+(sfx or ""), "twobytemem", suffix=sfx,
             modifiers=[1, 0x0F, 0xAE], cpu=["686", "FPU"])
add_insn("rdpmc", "twobyte", modifiers=[0x0F, 0x33], cpu=["686"])
add_insn("ud2",   "twobyte", modifiers=[0x0F, 0x0B], cpu=["286"])
add_insn("ud1",   "twobyte", modifiers=[0x0F, 0xB9], cpu=["286", "Undoc"])

for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("cmovcc",
        suffix=sfx,
        cpu=["686"],
        modifiers=["Op1Add"],
        opersize=sz,
        opcode=[0x0F, 0x40],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA")])

add_insn("cmovo", "cmovcc", modifiers=[0x00])
add_insn("cmovno", "cmovcc", modifiers=[0x01])
add_insn("cmovb", "cmovcc", modifiers=[0x02])
add_insn("cmovc", "cmovcc", modifiers=[0x02])
add_insn("cmovnae", "cmovcc", modifiers=[0x02])
add_insn("cmovnb", "cmovcc", modifiers=[0x03])
add_insn("cmovnc", "cmovcc", modifiers=[0x03])
add_insn("cmovae", "cmovcc", modifiers=[0x03])
add_insn("cmove", "cmovcc", modifiers=[0x04])
add_insn("cmovz", "cmovcc", modifiers=[0x04])
add_insn("cmovne", "cmovcc", modifiers=[0x05])
add_insn("cmovnz", "cmovcc", modifiers=[0x05])
add_insn("cmovbe", "cmovcc", modifiers=[0x06])
add_insn("cmovna", "cmovcc", modifiers=[0x06])
add_insn("cmovnbe", "cmovcc", modifiers=[0x07])
add_insn("cmova", "cmovcc", modifiers=[0x07])
add_insn("cmovs", "cmovcc", modifiers=[0x08])
add_insn("cmovns", "cmovcc", modifiers=[0x09])
add_insn("cmovp", "cmovcc", modifiers=[0x0A])
add_insn("cmovpe", "cmovcc", modifiers=[0x0A])
add_insn("cmovnp", "cmovcc", modifiers=[0x0B])
add_insn("cmovpo", "cmovcc", modifiers=[0x0B])
add_insn("cmovl", "cmovcc", modifiers=[0x0C])
add_insn("cmovnge", "cmovcc", modifiers=[0x0C])
add_insn("cmovnl", "cmovcc", modifiers=[0x0D])
add_insn("cmovge", "cmovcc", modifiers=[0x0D])
add_insn("cmovle", "cmovcc", modifiers=[0x0E])
add_insn("cmovng", "cmovcc", modifiers=[0x0E])
add_insn("cmovnle", "cmovcc", modifiers=[0x0F])
add_insn("cmovg", "cmovcc", modifiers=[0x0F])

add_group("fcmovcc",
    cpu=["FPU", "686"],
    modifiers=["Op0Add", "Op1Add"],
    opcode=[0x00, 0x00],
    operands=[Operand(type="ST0", size=80, dest=None),
              Operand(type="Reg", size=80, dest="Op1Add")])

add_insn("fcmovb",   "fcmovcc", modifiers=[0xDA, 0xC0])
add_insn("fcmove",   "fcmovcc", modifiers=[0xDA, 0xC8])
add_insn("fcmovbe",  "fcmovcc", modifiers=[0xDA, 0xD0])
add_insn("fcmovu",   "fcmovcc", modifiers=[0xDA, 0xD8])
add_insn("fcmovnb",  "fcmovcc", modifiers=[0xDB, 0xC0])
add_insn("fcmovne",  "fcmovcc", modifiers=[0xDB, 0xC8])
add_insn("fcmovnbe", "fcmovcc", modifiers=[0xDB, 0xD0])
add_insn("fcmovnu",  "fcmovcc", modifiers=[0xDB, 0xD8])

add_insn("fcomi", "fcom2", modifiers=[0xDB, 0xF0], cpu=["686", "FPU"])
add_insn("fucomi", "fcom2", modifiers=[0xDB, 0xE8], cpu=["686", "FPU"])
add_insn("fcomip", "fcom2", modifiers=[0xDF, 0xF0], cpu=["686", "FPU"])
add_insn("fucomip", "fcom2", modifiers=[0xDF, 0xE8], cpu=["686", "FPU"])

#####################################################################
# Pentium4 extensions
#####################################################################
add_group("movnti",
    suffix="l",
    cpu=["P4"],
    opcode=[0x0F, 0xC3],
    operands=[Operand(type="Mem", size=32, relaxed=True, dest="EA"),
              Operand(type="Reg", size=32, dest="Spare")])
add_group("movnti",
    suffix="q",
    cpu=["P4"],
    opersize=64,
    opcode=[0x0F, 0xC3],
    operands=[Operand(type="Mem", size=64, relaxed=True, dest="EA"),
              Operand(type="Reg", size=64, dest="Spare")])

add_insn("movnti", "movnti")

add_group("clflush",
    cpu=["P3"],
    opcode=[0x0F, 0xAE],
    spare=7,
    operands=[Operand(type="Mem", size=8, relaxed=True, dest="EA")])

add_insn("clflush", "clflush")

add_insn("lfence", "threebyte", modifiers=[0x0F, 0xAE, 0xE8], cpu=["P3"])
add_insn("mfence", "threebyte", modifiers=[0x0F, 0xAE, 0xF0], cpu=["P3"])
add_insn("pause", "onebyte_prefix", modifiers=[0xF3, 0x90], cpu=["P4"])

#####################################################################
# MMX/SSE2 instructions
#####################################################################

add_insn("emms", "twobyte", modifiers=[0x0F, 0x77], cpu=["MMX"])

#
# movd
#
add_group("movd",
    cpu=["MMX"],
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="RM", size=32, relaxed=True, dest="EA")])
add_group("movd",
    cpu=["MMX"],
    opersize=64,
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="RM", size=64, relaxed=True, dest="EA")])
add_group("movd",
    cpu=["MMX"],
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=64, dest="Spare")])
add_group("movd",
    cpu=["MMX"],
    opersize=64,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=64, dest="Spare")])
add_group("movd",
    cpu=["SSE2"],
    prefix=0x66,
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="RM", size=32, relaxed=True, dest="EA")])
add_group("movd",
    cpu=["SSE2"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="RM", size=64, relaxed=True, dest="EA")])
add_group("movd",
    cpu=["SSE2"],
    prefix=0x66,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("movd",
    cpu=["SSE2"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])

add_insn("movd", "movd")

#
# movq
#

# MMX forms
add_group("movq",
    cpu=["MMX"],
    parsers=["nasm"],
    opcode=[0x0F, 0x6F],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])
add_group("movq",
    cpu=["MMX"],
    parsers=["nasm"],
    opersize=64,
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="RM", size=64, relaxed=True, dest="EA")])
add_group("movq",
    cpu=["MMX"],
    parsers=["nasm"],
    opcode=[0x0F, 0x7F],
    operands=[Operand(type="SIMDRM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=64, dest="Spare")])
add_group("movq",
    cpu=["MMX"],
    parsers=["nasm"],
    opersize=64,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=64, dest="Spare")])

# SSE2 forms
add_group("movq",
    cpu=["SSE2"],
    parsers=["nasm"],
    prefix=0xF3,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("movq",
    cpu=["SSE2"],
    parsers=["nasm"],
    prefix=0xF3,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])
add_group("movq",
    cpu=["SSE2"],
    parsers=["nasm"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="RM", size=64, relaxed=True, dest="EA")])
add_group("movq",
    cpu=["SSE2"],
    parsers=["nasm"],
    prefix=0x66,
    opcode=[0x0F, 0xD6],
    operands=[Operand(type="SIMDRM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("movq",
    cpu=["SSE2"],
    parsers=["nasm"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])

add_insn("movq", "movq")

add_group("mmxsse2",
    cpu=["MMX"],
    modifiers=["Op1Add"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])
add_group("mmxsse2",
    cpu=["SSE2"],
    modifiers=["Op1Add"],
    prefix=0x66,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])

add_insn("packssdw",  "mmxsse2", modifiers=[0x6B])
add_insn("packsswb",  "mmxsse2", modifiers=[0x63])
add_insn("packuswb",  "mmxsse2", modifiers=[0x67])
add_insn("paddb",     "mmxsse2", modifiers=[0xFC])
add_insn("paddw",     "mmxsse2", modifiers=[0xFD])
add_insn("paddd",     "mmxsse2", modifiers=[0xFE])
add_insn("paddq",     "mmxsse2", modifiers=[0xD4])
add_insn("paddsb",    "mmxsse2", modifiers=[0xEC])
add_insn("paddsw",    "mmxsse2", modifiers=[0xED])
add_insn("paddusb",   "mmxsse2", modifiers=[0xDC])
add_insn("paddusw",   "mmxsse2", modifiers=[0xDD])
add_insn("pand",      "mmxsse2", modifiers=[0xDB])
add_insn("pandn",     "mmxsse2", modifiers=[0xDF])
add_insn("pcmpeqb",   "mmxsse2", modifiers=[0x74])
add_insn("pcmpeqw",   "mmxsse2", modifiers=[0x75])
add_insn("pcmpeqd",   "mmxsse2", modifiers=[0x76])
add_insn("pcmpgtb",   "mmxsse2", modifiers=[0x64])
add_insn("pcmpgtw",   "mmxsse2", modifiers=[0x65])
add_insn("pcmpgtd",   "mmxsse2", modifiers=[0x66])
add_insn("pmaddwd",   "mmxsse2", modifiers=[0xF5])
add_insn("pmulhw",    "mmxsse2", modifiers=[0xE5])
add_insn("pmullw",    "mmxsse2", modifiers=[0xD5])
add_insn("por",       "mmxsse2", modifiers=[0xEB])
add_insn("psubb",     "mmxsse2", modifiers=[0xF8])
add_insn("psubw",     "mmxsse2", modifiers=[0xF9])
add_insn("psubd",     "mmxsse2", modifiers=[0xFA])
add_insn("psubq",     "mmxsse2", modifiers=[0xFB])
add_insn("psubsb",    "mmxsse2", modifiers=[0xE8])
add_insn("psubsw",    "mmxsse2", modifiers=[0xE9])
add_insn("psubusb",   "mmxsse2", modifiers=[0xD8])
add_insn("psubusw",   "mmxsse2", modifiers=[0xD9])
add_insn("punpckhbw", "mmxsse2", modifiers=[0x68])
add_insn("punpckhwd", "mmxsse2", modifiers=[0x69])
add_insn("punpckhdq", "mmxsse2", modifiers=[0x6A])
add_insn("punpcklbw", "mmxsse2", modifiers=[0x60])
add_insn("punpcklwd", "mmxsse2", modifiers=[0x61])
add_insn("punpckldq", "mmxsse2", modifiers=[0x62])
add_insn("pxor",      "mmxsse2", modifiers=[0xEF])

# AVX versions don't support the MMX registers
add_insn("vpackssdw",  "xmm_xmm128_256avx2", modifiers=[0x66, 0x6B, VEXL0], avx=True)
add_insn("vpacksswb",  "xmm_xmm128_256avx2", modifiers=[0x66, 0x63, VEXL0], avx=True)
add_insn("vpackuswb",  "xmm_xmm128_256avx2", modifiers=[0x66, 0x67, VEXL0], avx=True)
add_insn("vpaddb",     "xmm_xmm128_256avx2", modifiers=[0x66, 0xFC, VEXL0], avx=True)
add_insn("vpaddw",     "xmm_xmm128_256avx2", modifiers=[0x66, 0xFD, VEXL0], avx=True)
add_insn("vpaddd",     "xmm_xmm128_256avx2", modifiers=[0x66, 0xFE, VEXL0], avx=True)
add_insn("vpaddq",     "xmm_xmm128_256avx2", modifiers=[0x66, 0xD4, VEXL0], avx=True)
add_insn("vpaddsb",    "xmm_xmm128_256avx2", modifiers=[0x66, 0xEC, VEXL0], avx=True)
add_insn("vpaddsw",    "xmm_xmm128_256avx2", modifiers=[0x66, 0xED, VEXL0], avx=True)
add_insn("vpaddusb",   "xmm_xmm128_256avx2", modifiers=[0x66, 0xDC, VEXL0], avx=True)
add_insn("vpaddusw",   "xmm_xmm128_256avx2", modifiers=[0x66, 0xDD, VEXL0], avx=True)
add_insn("vpand",      "xmm_xmm128_256avx2", modifiers=[0x66, 0xDB, VEXL0], avx=True)
add_insn("vpandn",     "xmm_xmm128_256avx2", modifiers=[0x66, 0xDF, VEXL0], avx=True)
add_insn("vpcmpeqb",   "xmm_xmm128_256avx2", modifiers=[0x66, 0x74, VEXL0], avx=True)
add_insn("vpcmpeqw",   "xmm_xmm128_256avx2", modifiers=[0x66, 0x75, VEXL0], avx=True)
add_insn("vpcmpeqd",   "xmm_xmm128_256avx2", modifiers=[0x66, 0x76, VEXL0], avx=True)
add_insn("vpcmpgtb",   "xmm_xmm128_256avx2", modifiers=[0x66, 0x64, VEXL0], avx=True)
add_insn("vpcmpgtw",   "xmm_xmm128_256avx2", modifiers=[0x66, 0x65, VEXL0], avx=True)
add_insn("vpcmpgtd",   "xmm_xmm128_256avx2", modifiers=[0x66, 0x66, VEXL0], avx=True)
add_insn("vpmaddwd",   "xmm_xmm128_256avx2", modifiers=[0x66, 0xF5, VEXL0], avx=True)
add_insn("vpmulhw",    "xmm_xmm128_256avx2", modifiers=[0x66, 0xE5, VEXL0], avx=True)
add_insn("vpmullw",    "xmm_xmm128_256avx2", modifiers=[0x66, 0xD5, VEXL0], avx=True)
add_insn("vpor",       "xmm_xmm128_256avx2", modifiers=[0x66, 0xEB, VEXL0], avx=True)
add_insn("vpsubb",     "xmm_xmm128_256avx2", modifiers=[0x66, 0xF8, VEXL0], avx=True)
add_insn("vpsubw",     "xmm_xmm128_256avx2", modifiers=[0x66, 0xF9, VEXL0], avx=True)
add_insn("vpsubd",     "xmm_xmm128_256avx2", modifiers=[0x66, 0xFA, VEXL0], avx=True)
add_insn("vpsubq",     "xmm_xmm128_256avx2", modifiers=[0x66, 0xFB, VEXL0], avx=True)
add_insn("vpsubsb",    "xmm_xmm128_256avx2", modifiers=[0x66, 0xE8, VEXL0], avx=True)
add_insn("vpsubsw",    "xmm_xmm128_256avx2", modifiers=[0x66, 0xE9, VEXL0], avx=True)
add_insn("vpsubusb",   "xmm_xmm128_256avx2", modifiers=[0x66, 0xD8, VEXL0], avx=True)
add_insn("vpsubusw",   "xmm_xmm128_256avx2", modifiers=[0x66, 0xD9, VEXL0], avx=True)
add_insn("vpunpckhbw", "xmm_xmm128_256avx2", modifiers=[0x66, 0x68, VEXL0], avx=True)
add_insn("vpunpckhwd", "xmm_xmm128_256avx2", modifiers=[0x66, 0x69, VEXL0], avx=True)
add_insn("vpunpckhdq", "xmm_xmm128_256avx2", modifiers=[0x66, 0x6A, VEXL0], avx=True)
add_insn("vpunpcklbw", "xmm_xmm128_256avx2", modifiers=[0x66, 0x60, VEXL0], avx=True)
add_insn("vpunpcklwd", "xmm_xmm128_256avx2", modifiers=[0x66, 0x61, VEXL0], avx=True)
add_insn("vpunpckldq", "xmm_xmm128_256avx2", modifiers=[0x66, 0x62, VEXL0], avx=True)
add_insn("vpxor",      "xmm_xmm128_256avx2", modifiers=[0x66, 0xEF, VEXL0], avx=True)

add_group("pshift",
    cpu=["MMX"],
    modifiers=["Op1Add"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])
add_group("pshift",
    cpu=["MMX"],
    modifiers=["Gap", "Op1Add", "SpAdd"],
    opcode=[0x0F, 0x00],
    spare=0,
    operands=[Operand(type="SIMDReg", size=64, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pshift",
    cpu=["SSE2"],
    modifiers=["Op1Add"],
    prefix=0x66,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("pshift",
    cpu=["SSE2"],
    modifiers=["Gap", "Op1Add", "SpAdd"],
    prefix=0x66,
    opcode=[0x0F, 0x00],
    spare=0,
    operands=[Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("psllw", "pshift", modifiers=[0xF1, 0x71, 6])
add_insn("pslld", "pshift", modifiers=[0xF2, 0x72, 6])
add_insn("psllq", "pshift", modifiers=[0xF3, 0x73, 6])
add_insn("psraw", "pshift", modifiers=[0xE1, 0x71, 4])
add_insn("psrad", "pshift", modifiers=[0xE2, 0x72, 4])
add_insn("psrlw", "pshift", modifiers=[0xD1, 0x71, 2])
add_insn("psrld", "pshift", modifiers=[0xD2, 0x72, 2])
add_insn("psrlq", "pshift", modifiers=[0xD3, 0x73, 2])

# Ran out of modifiers, so AVX has to be separate
for cpu, sz in zip(["AVX", "AVX2"], [128, 256]):
    add_group("vpshift",
        cpu=[cpu],
        modifiers=["Op1Add"],
        vex=sz,
        prefix=0x66,
        opcode=[0x0F, 0x00],
        operands=[Operand(type="SIMDReg", size=sz, dest="SpareVEX"),
                  Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
    add_group("vpshift",
        cpu=[cpu],
        modifiers=["Gap", "Op1Add", "SpAdd"],
        vex=sz,
        prefix=0x66,
        opcode=[0x0F, 0x00],
        spare=0,
        operands=[Operand(type="SIMDReg", size=sz, dest="EAVEX"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
    add_group("vpshift",
        cpu=[cpu],
        modifiers=["Op1Add"],
        vex=sz,
        prefix=0x66,
        opcode=[0x0F, 0x00],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDReg", size=sz, dest="VEX"),
                  Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
    add_group("vpshift",
        cpu=[cpu],
        modifiers=["Gap", "Op1Add", "SpAdd"],
        vex=sz,
        prefix=0x66,
        opcode=[0x0F, 0x00],
        spare=0,
        operands=[Operand(type="SIMDReg", size=sz, dest="VEX"),
                  Operand(type="SIMDReg", size=sz, dest="EA"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vpsllw", "vpshift", modifiers=[0xF1, 0x71, 6])
add_insn("vpslld", "vpshift", modifiers=[0xF2, 0x72, 6])
add_insn("vpsllq", "vpshift", modifiers=[0xF3, 0x73, 6])
add_insn("vpsraw", "vpshift", modifiers=[0xE1, 0x71, 4])
add_insn("vpsrad", "vpshift", modifiers=[0xE2, 0x72, 4])
add_insn("vpsrlw", "vpshift", modifiers=[0xD1, 0x71, 2])
add_insn("vpsrld", "vpshift", modifiers=[0xD2, 0x72, 2])
add_insn("vpsrlq", "vpshift", modifiers=[0xD3, 0x73, 2])

#
# PIII (Katmai) new instructions / SIMD instructions
#
add_insn("pavgb",   "mmxsse2", modifiers=[0xE0], cpu=["P3", "MMX"])
add_insn("pavgw",   "mmxsse2", modifiers=[0xE3], cpu=["P3", "MMX"])
add_insn("pmaxsw",  "mmxsse2", modifiers=[0xEE], cpu=["P3", "MMX"])
add_insn("pmaxub",  "mmxsse2", modifiers=[0xDE], cpu=["P3", "MMX"])
add_insn("pminsw",  "mmxsse2", modifiers=[0xEA], cpu=["P3", "MMX"])
add_insn("pminub",  "mmxsse2", modifiers=[0xDA], cpu=["P3", "MMX"])
add_insn("pmulhuw", "mmxsse2", modifiers=[0xE4], cpu=["P3", "MMX"])
add_insn("psadbw",  "mmxsse2", modifiers=[0xF6], cpu=["P3", "MMX"])

# AVX versions don't support MMX register
add_insn("vpavgb",   "xmm_xmm128_256avx2", modifiers=[0x66, 0xE0, VEXL0], avx=True)
add_insn("vpavgw",   "xmm_xmm128_256avx2", modifiers=[0x66, 0xE3, VEXL0], avx=True)
add_insn("vpmaxsw",  "xmm_xmm128_256avx2", modifiers=[0x66, 0xEE, VEXL0], avx=True)
add_insn("vpmaxub",  "xmm_xmm128_256avx2", modifiers=[0x66, 0xDE, VEXL0], avx=True)
add_insn("vpminsw",  "xmm_xmm128_256avx2", modifiers=[0x66, 0xEA, VEXL0], avx=True)
add_insn("vpminub",  "xmm_xmm128_256avx2", modifiers=[0x66, 0xDA, VEXL0], avx=True)
add_insn("vpmulhuw", "xmm_xmm128_256avx2", modifiers=[0x66, 0xE4, VEXL0], avx=True)
add_insn("vpsadbw",  "xmm_xmm128_256avx2", modifiers=[0x66, 0xF6, VEXL0], avx=True)

add_insn("prefetchnta", "twobytemem", modifiers=[0, 0x0F, 0x18], cpu=["P3"])
add_insn("prefetcht0", "twobytemem", modifiers=[1, 0x0F, 0x18], cpu=["P3"])
add_insn("prefetcht1", "twobytemem", modifiers=[2, 0x0F, 0x18], cpu=["P3"])
add_insn("prefetcht2", "twobytemem", modifiers=[3, 0x0F, 0x18], cpu=["P3"])

add_insn("sfence", "threebyte", modifiers=[0x0F, 0xAE, 0xF8], cpu=["P3"])

add_group("xmm_xmm128_256",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("xmm_xmm128_256",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("xmm_xmm128_256",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="SpareVEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])
add_group("xmm_xmm128_256",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

# Same as above, except 256-bit version only available in AVX2
add_group("xmm_xmm128_256avx2",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("xmm_xmm128_256avx2",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("xmm_xmm128_256avx2",
    cpu=["AVX2"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="SpareVEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])
add_group("xmm_xmm128_256avx2",
    cpu=["AVX2"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

# Version that does not allow YMM registers
add_group("xmm_xmm128",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("xmm_xmm128",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])

add_insn("addps",    "xmm_xmm128", modifiers=[0, 0x58])
add_insn("andnps",   "xmm_xmm128", modifiers=[0, 0x55])
add_insn("andps",    "xmm_xmm128", modifiers=[0, 0x54])
add_insn("divps",    "xmm_xmm128", modifiers=[0, 0x5E])
add_insn("maxps",    "xmm_xmm128", modifiers=[0, 0x5F])
add_insn("minps",    "xmm_xmm128", modifiers=[0, 0x5D])
add_insn("mulps",    "xmm_xmm128", modifiers=[0, 0x59])
add_insn("orps",     "xmm_xmm128", modifiers=[0, 0x56])
add_insn("rcpps",    "xmm_xmm128", modifiers=[0, 0x53])
add_insn("rsqrtps",  "xmm_xmm128", modifiers=[0, 0x52])
add_insn("sqrtps",   "xmm_xmm128", modifiers=[0, 0x51])
add_insn("subps",    "xmm_xmm128", modifiers=[0, 0x5C])
add_insn("unpckhps", "xmm_xmm128", modifiers=[0, 0x15])
add_insn("unpcklps", "xmm_xmm128", modifiers=[0, 0x14])
add_insn("xorps",    "xmm_xmm128", modifiers=[0, 0x57])

add_insn("vaddps",    "xmm_xmm128_256", modifiers=[0, 0x58, VEXL0], avx=True)
add_insn("vandnps",   "xmm_xmm128_256", modifiers=[0, 0x55, VEXL0], avx=True)
add_insn("vandps",    "xmm_xmm128_256", modifiers=[0, 0x54, VEXL0], avx=True)
add_insn("vdivps",    "xmm_xmm128_256", modifiers=[0, 0x5E, VEXL0], avx=True)
add_insn("vmaxps",    "xmm_xmm128_256", modifiers=[0, 0x5F, VEXL0], avx=True)
add_insn("vminps",    "xmm_xmm128_256", modifiers=[0, 0x5D, VEXL0], avx=True)
add_insn("vmulps",    "xmm_xmm128_256", modifiers=[0, 0x59, VEXL0], avx=True)
add_insn("vorps",     "xmm_xmm128_256", modifiers=[0, 0x56, VEXL0], avx=True)
# vrcpps, vrsqrtps, and vsqrtps don't add third operand
add_insn("vsubps",    "xmm_xmm128_256", modifiers=[0, 0x5C, VEXL0], avx=True)
add_insn("vunpckhps", "xmm_xmm128_256", modifiers=[0, 0x15, VEXL0], avx=True)
add_insn("vunpcklps", "xmm_xmm128_256", modifiers=[0, 0x14, VEXL0], avx=True)
add_insn("vxorps",    "xmm_xmm128_256", modifiers=[0, 0x57, VEXL0], avx=True)

add_group("cvt_rx_xmm32",
    suffix="l",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("cvt_rx_xmm32",
    suffix="l",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])
# REX
add_group("cvt_rx_xmm32",
    suffix="q",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    opersize=64,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("cvt_rx_xmm32",
    suffix="q",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    opersize=64,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])

add_insn("cvtss2si", "cvt_rx_xmm32", modifiers=[0xF3, 0x2D])
add_insn("cvttss2si", "cvt_rx_xmm32", modifiers=[0xF3, 0x2C])
add_insn("vcvtss2si", "cvt_rx_xmm32", modifiers=[0xF3, 0x2D, VEXL0], avx=True)
add_insn("vcvttss2si", "cvt_rx_xmm32", modifiers=[0xF3, 0x2C, VEXL0], avx=True)

add_group("cvt_mm_xmm64",
    cpu=["SSE"],
    modifiers=["Op1Add"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("cvt_mm_xmm64",
    cpu=["SSE"],
    modifiers=["Op1Add"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])

add_insn("cvtps2pi", "cvt_mm_xmm64", modifiers=[0x2D])
add_insn("cvttps2pi", "cvt_mm_xmm64", modifiers=[0x2C])

add_group("cvt_xmm_mm_ps",
    cpu=["SSE"],
    modifiers=["Op1Add"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])

add_insn("cvtpi2ps", "cvt_xmm_mm_ps", modifiers=[0x2A])

# Memory size can be relaxed only in BITS=32 case, where there's no
# ambiguity.
add_group("cvt_xmm_rmx",
    suffix="l",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="RM", size=32, dest="EA")])
add_group("cvt_xmm_rmx",
    suffix="l",
    cpu=["SSE"],
    not64=True,
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="RM", size=32, relaxed=True, dest="EA")])
# REX
add_group("cvt_xmm_rmx",
    suffix="q",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    opersize=64,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="RM", size=64, dest="EA")])
add_group("cvt_xmm_rmx",
    suffix="l",
    cpu=["AVX"],
    not64=True,
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="RM", size=32, relaxed=True, dest="EA")])
add_group("cvt_xmm_rmx",
    suffix="l",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="RM", size=32, dest="EA")])
add_group("cvt_xmm_rmx",
    suffix="q",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    opersize=64,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="RM", size=64, dest="EA")])

add_insn("cvtsi2ss", "cvt_xmm_rmx", modifiers=[0xF3, 0x2A])
add_insn("vcvtsi2ss", "cvt_xmm_rmx", modifiers=[0xF3, 0x2A, VEXL0], avx=True)

add_group("xmm_xmm32",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("xmm_xmm32",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])
add_group("xmm_xmm32",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("xmm_xmm32",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])

add_insn("addss",   "xmm_xmm32", modifiers=[0xF3, 0x58])
add_insn("comiss",  "xmm_xmm32", modifiers=[0, 0x2F])
add_insn("divss",   "xmm_xmm32", modifiers=[0xF3, 0x5E])
add_insn("maxss",   "xmm_xmm32", modifiers=[0xF3, 0x5F])
add_insn("minss",   "xmm_xmm32", modifiers=[0xF3, 0x5D])
add_insn("mulss",   "xmm_xmm32", modifiers=[0xF3, 0x59])
add_insn("rcpss",   "xmm_xmm32", modifiers=[0xF3, 0x53])
add_insn("rsqrtss", "xmm_xmm32", modifiers=[0xF3, 0x52])
add_insn("sqrtss",  "xmm_xmm32", modifiers=[0xF3, 0x51])
add_insn("subss",   "xmm_xmm32", modifiers=[0xF3, 0x5C])
add_insn("ucomiss", "xmm_xmm32", modifiers=[0, 0x2E])

add_insn("vaddss",   "xmm_xmm32", modifiers=[0xF3, 0x58, VEXL0], avx=True)
# vcomiss and vucomiss are only two operand
add_insn("vdivss",   "xmm_xmm32", modifiers=[0xF3, 0x5E, VEXL0], avx=True)
add_insn("vmaxss",   "xmm_xmm32", modifiers=[0xF3, 0x5F, VEXL0], avx=True)
add_insn("vminss",   "xmm_xmm32", modifiers=[0xF3, 0x5D, VEXL0], avx=True)
add_insn("vmulss",   "xmm_xmm32", modifiers=[0xF3, 0x59, VEXL0], avx=True)
add_insn("vrcpss",   "xmm_xmm32", modifiers=[0xF3, 0x53, VEXL0], avx=True)
add_insn("vrsqrtss", "xmm_xmm32", modifiers=[0xF3, 0x52, VEXL0], avx=True)
add_insn("vsqrtss",  "xmm_xmm32", modifiers=[0xF3, 0x51, VEXL0], avx=True)
add_insn("vsubss",   "xmm_xmm32", modifiers=[0xF3, 0x5C, VEXL0], avx=True)

add_group("ssecmp_128",
    cpu=["SSE"],
    modifiers=["Imm8", "PreAdd", "SetVEX"],
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("ssecmp_128",
    cpu=["AVX"],
    modifiers=["Imm8", "PreAdd"],
    vex=128,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("ssecmp_128",
    cpu=["AVX"],
    modifiers=["Imm8", "PreAdd"],
    vex=256,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_group("ssecmp_32",
    cpu=["SSE"],
    modifiers=["Imm8", "PreAdd", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("ssecmp_32",
    cpu=["SSE"],
    modifiers=["Imm8", "PreAdd", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])
add_group("ssecmp_32",
    cpu=["AVX"],
    modifiers=["Imm8", "PreAdd"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("ssecmp_32",
    cpu=["AVX"],
    modifiers=["Imm8", "PreAdd"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])

ssecoms = [(0x0, "eq"),
           (0x1, "lt"),
           (0x2, "le"),
           (0x3, "unord"),
           (0x4, "neq"),
           (0x5, "nlt"),
           (0x6, "nle"),
           (0x7, "ord")]
for ib, cc in ssecoms:
    add_insn("cmp"+cc+"ps", "ssecmp_128", modifiers=[ib])
    add_insn("cmp"+cc+"ss", "ssecmp_32", modifiers=[ib, 0xF3])

avxcoms = [(0x00, "eq"),
           (0x01, "lt"),
           (0x02, "le"),
           (0x03, "unord"),
           (0x04, "neq"),
           (0x05, "nlt"),
           (0x06, "nle"),
           (0x07, "ord"),
           (0x08, "eq_uq"),
           (0x09, "nge"),
           (0x0a, "ngt"),
           (0x0b, "false"),
           (0x0c, "neq_oq"),
           (0x0d, "ge"),
           (0x0e, "gt"),
           (0x0f, "true"),
           (0x10, "eq_os"),
           (0x11, "lt_oq"),
           (0x12, "le_oq"),
           (0x13, "unord_s"),
           (0x14, "neq_us"),
           (0x15, "nlt_uq"),
           (0x16, "nle_uq"),
           (0x17, "ord_s"),
           (0x18, "eq_us"),
           (0x19, "nge_uq"),
           (0x1a, "ngt_uq"),
           (0x1b, "false_os"),
           (0x1c, "neq_os"),
           (0x1d, "ge_oq"),
           (0x1e, "gt_oq"),
           (0x1f, "true_us")]
for ib, cc in avxcoms:
    add_insn("vcmp"+cc+"ps", "ssecmp_128", modifiers=[ib, 0, VEXL0], avx=True)
    add_insn("vcmp"+cc+"ss", "ssecmp_32", modifiers=[ib, 0xF3, VEXL0], avx=True)

add_group("xmm_xmm128_imm",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("cmpps", "xmm_xmm128_imm", modifiers=[0, 0xC2])
add_insn("shufps", "xmm_xmm128_imm", modifiers=[0, 0xC6])

# YMM register AVX2 version of above
add_group("xmm_xmm128_imm_256avx2",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("xmm_xmm128_imm_256avx2",
    cpu=["AVX2"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

# YMM register and 4-operand version of above
add_group("xmm_xmm128_imm_256",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("xmm_xmm128_imm_256",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("xmm_xmm128_imm_256",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vcmpps", "xmm_xmm128_imm_256", modifiers=[0, 0xC2, VEXL0], avx=True)
add_insn("vshufps", "xmm_xmm128_imm_256", modifiers=[0, 0xC6, VEXL0], avx=True)

add_group("xmm_xmm32_imm",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("xmm_xmm32_imm",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("xmm_xmm32_imm",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("xmm_xmm32_imm",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("cmpss", "xmm_xmm32_imm", modifiers=[0xF3, 0xC2])
add_insn("vcmpss", "xmm_xmm32_imm", modifiers=[0xF3, 0xC2, VEXL0], avx=True)

add_group("ldstmxcsr",
    cpu=["SSE"],
    modifiers=["SpAdd", "SetVEX"],
    opcode=[0x0F, 0xAE],
    spare=0,
    operands=[Operand(type="Mem", size=32, relaxed=True, dest="EA")])

add_insn("ldmxcsr", "ldstmxcsr", modifiers=[2])
add_insn("stmxcsr", "ldstmxcsr", modifiers=[3])
add_insn("vldmxcsr", "ldstmxcsr", modifiers=[2, VEXL0], avx=True)
add_insn("vstmxcsr", "ldstmxcsr", modifiers=[3, VEXL0], avx=True)

add_group("maskmovq",
    cpu=["MMX", "P3"],
    opcode=[0x0F, 0xF7],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=64, dest="EA")])

add_insn("maskmovq", "maskmovq")

# Too many modifiers, so can't reuse first two cases for AVX version
# Just repeat and disable first two with noavx.
add_group("movau",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add"],
    notavx=True,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("movau",
    cpu=["SSE"],
    notavx=True,
    modifiers=["PreAdd", "Op1Add", "Op1Add"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("movau",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("movau",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add", "Op1Add"],
    vex=128,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("movau",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])
add_group("movau",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add", "Op1Add"],
    vex=256,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="Spare")])

add_insn("movaps", "movau", modifiers=[0, 0x28, 0x01])
add_insn("movups", "movau", modifiers=[0, 0x10, 0x01])
add_insn("vmovaps", "movau", modifiers=[0, 0x28, 0x01], avx=True)
add_insn("vmovups", "movau", modifiers=[0, 0x10, 0x01], avx=True)

add_group("movhllhps",
    cpu=["SSE"],
    modifiers=["Op1Add", "SetVEX"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("movhllhps",
    cpu=["AVX"],
    modifiers=["Op1Add"],
    vex=128,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_insn("movhlps", "movhllhps", modifiers=[0x12])
add_insn("movlhps", "movhllhps", modifiers=[0x16])
add_insn("vmovhlps", "movhllhps", modifiers=[0x12, VEXL0], avx=True)
add_insn("vmovlhps", "movhllhps", modifiers=[0x16, VEXL0], avx=True)

add_group("movhlp",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_group("movhlp",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x01],
    operands=[Operand(type="Mem", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("movhlp",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])

add_insn("movhps", "movhlp", modifiers=[0, 0x16])
add_insn("movlps", "movhlp", modifiers=[0, 0x12])
add_insn("vmovhps", "movhlp", modifiers=[0, 0x16, VEXL0], avx=True)
add_insn("vmovlps", "movhlp", modifiers=[0, 0x12, VEXL0], avx=True)

add_group("movmsk",
    suffix="l",
    cpu=["SSE"],
    modifiers=["PreAdd", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x50],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("movmsk",
    suffix="q",
    cpu=["SSE"],
    modifiers=["PreAdd", "SetVEX"],
    prefix=0x00,
    opersize=64,
    opcode=[0x0F, 0x50],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("movmsk",
    suffix="l",
    cpu=["AVX"],
    modifiers=["PreAdd"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x50],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="EA")])
add_group("movmsk",
    suffix="q",
    cpu=["SSE"],
    modifiers=["PreAdd"],
    vex=256,
    prefix=0x00,
    opersize=64,
    opcode=[0x0F, 0x50],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="EA")])

add_insn("movmskps", "movmsk")
add_insn("vmovmskps", "movmsk", modifiers=[0, VEXL0], avx=True)

add_group("movnt",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Mem", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("movnt",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Mem", size=256, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="Spare")])

add_insn("movntps", "movnt", modifiers=[0, 0x2B])
add_insn("vmovntps", "movnt", modifiers=[0, 0x2B, VEXL0], avx=True)

add_group("movntq",
    cpu=["SSE"],
    opcode=[0x0F, 0xE7],
    operands=[Operand(type="Mem", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=64, dest="Spare")])

add_insn("movntq", "movntq")

add_group("movss",
    cpu=["SSE"],
    modifiers=["SetVEX"],
    prefix=0xF3,
    opcode=[0x0F, 0x10],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("movss",
    cpu=["SSE"],
    modifiers=["SetVEX"],
    prefix=0xF3,
    opcode=[0x0F, 0x10],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])
add_group("movss",
    cpu=["SSE"],
    modifiers=["SetVEX"],
    prefix=0xF3,
    opcode=[0x0F, 0x11],
    operands=[Operand(type="Mem", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("movss",
    cpu=["AVX"],
    vex=128,
    prefix=0xF3,
    opcode=[0x0F, 0x10],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_insn("movss", "movss")
add_insn("vmovss", "movss", modifiers=[VEXL0], avx=True)

add_group("pextrw",
    suffix="l",
    cpu=["MMX", "P3"],
    notavx=True,
    opcode=[0x0F, 0xC5],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="SIMDReg", size=64, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pextrw",
    suffix="l",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0xC5],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pextrw",
    suffix="q",
    cpu=["MMX", "P3"],
    notavx=True,
    opersize=64,
    opcode=[0x0F, 0xC5],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=64, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pextrw",
    suffix="q",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0xC5],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
# SSE41 instructions
add_group("pextrw",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x15],
    operands=[Operand(type="Mem", size=16, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pextrw",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    opersize=32,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x15],
    operands=[Operand(type="Reg", size=32, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pextrw",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x15],
    operands=[Operand(type="Reg", size=64, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pextrw", "pextrw")
add_insn("vpextrw", "pextrw", modifiers=[VEXL0], avx=True)

add_group("pinsrw",
    suffix="l",
    cpu=["MMX", "P3"],
    notavx=True,
    opcode=[0x0F, 0xC4],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="Reg", size=32, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrw",
    suffix="q",
    cpu=["MMX", "P3"],
    notavx=True,
    def_opersize_64=64,
    opersize=64,
    opcode=[0x0F, 0xC4],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="Reg", size=64, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrw",
    suffix="l",
    cpu=["MMX", "P3"],
    notavx=True,
    opcode=[0x0F, 0xC4],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="Mem", size=16, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrw",
    suffix="l",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0xC4],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Reg", size=32, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrw",
    suffix="q",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    def_opersize_64=64,
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0xC4],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Reg", size=64, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrw",
    suffix="l",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0xC4],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=16, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrw",
    suffix="l",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0xC4],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Reg", size=32, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrw",
    suffix="q",
    cpu=["AVX"],
    vex=128,
    def_opersize_64=64,
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0xC4],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Reg", size=64, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrw",
    suffix="l",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0xC4],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=16, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pinsrw", "pinsrw")
add_insn("vpinsrw", "pinsrw", modifiers=[VEXL0], avx=True)

add_group("pmovmskb",
    suffix="l",
    cpu=["MMX", "P3"],
    notavx=True,
    opcode=[0x0F, 0xD7],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="SIMDReg", size=64, dest="EA")])
add_group("pmovmskb",
    suffix="l",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0xD7],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("pmovmskb",
    suffix="l",
    cpu=["AVX2"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0xD7],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="EA")])
add_group("pmovmskb",
    suffix="q",
    cpu=["MMX", "P3"],
    notavx=True,
    opersize=64,
    def_opersize_64=64,
    opcode=[0x0F, 0xD7],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=64, dest="EA")])
add_group("pmovmskb",
    suffix="q",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    opersize=64,
    def_opersize_64=64,
    prefix=0x66,
    opcode=[0x0F, 0xD7],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("pmovmskb",
    suffix="q",
    cpu=["SSE2"],
    vex=256,
    opersize=64,
    def_opersize_64=64,
    prefix=0x66,
    opcode=[0x0F, 0xD7],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="EA")])

add_insn("pmovmskb", "pmovmskb")
add_insn("vpmovmskb", "pmovmskb", modifiers=[VEXL0], avx=True)

add_group("pshufw",
    cpu=["MMX", "P3"],
    opcode=[0x0F, 0x70],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pshufw", "pshufw")

#####################################################################
# SSE2 instructions
#####################################################################
add_group("xmm_xmm64",
    cpu=["SSE2"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("xmm_xmm64",
    cpu=["SSE2"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_group("xmm_xmm64",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("xmm_xmm64",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])

add_insn("addsd",    "xmm_xmm64", modifiers=[0xF2, 0x58])
add_insn("comisd",   "xmm_xmm64", modifiers=[0x66, 0x2F])
add_insn("cvtdq2pd", "xmm_xmm64", modifiers=[0xF3, 0xE6])
add_insn("cvtps2pd", "xmm_xmm64", modifiers=[0, 0x5A])
add_insn("cvtsd2ss", "xmm_xmm64", modifiers=[0xF2, 0x5A])
add_insn("divsd",    "xmm_xmm64", modifiers=[0xF2, 0x5E])
add_insn("maxsd",    "xmm_xmm64", modifiers=[0xF2, 0x5F])
add_insn("minsd",    "xmm_xmm64", modifiers=[0xF2, 0x5D])
add_insn("mulsd",    "xmm_xmm64", modifiers=[0xF2, 0x59])
add_insn("subsd",    "xmm_xmm64", modifiers=[0xF2, 0x5C])
add_insn("sqrtsd",   "xmm_xmm64", modifiers=[0xF2, 0x51])
add_insn("ucomisd",  "xmm_xmm64", modifiers=[0x66, 0x2E])

add_insn("vaddsd",    "xmm_xmm64", modifiers=[0xF2, 0x58, VEXL0], avx=True)
# vcomisd and vucomisd are only two operand
# vcvtdq2pd and vcvtps2pd can take ymm, xmm version
add_insn("vcvtsd2ss", "xmm_xmm64", modifiers=[0xF2, 0x5A, VEXL0], avx=True)
add_insn("vdivsd",    "xmm_xmm64", modifiers=[0xF2, 0x5E, VEXL0], avx=True)
add_insn("vmaxsd",    "xmm_xmm64", modifiers=[0xF2, 0x5F, VEXL0], avx=True)
add_insn("vminsd",    "xmm_xmm64", modifiers=[0xF2, 0x5D, VEXL0], avx=True)
add_insn("vmulsd",    "xmm_xmm64", modifiers=[0xF2, 0x59, VEXL0], avx=True)
add_insn("vsubsd",    "xmm_xmm64", modifiers=[0xF2, 0x5C, VEXL0], avx=True)
add_insn("vsqrtsd",   "xmm_xmm64", modifiers=[0xF2, 0x51, VEXL0], avx=True)

add_insn("addpd",    "xmm_xmm128", modifiers=[0x66, 0x58], cpu=["SSE2"])
add_insn("andnpd",   "xmm_xmm128", modifiers=[0x66, 0x55], cpu=["SSE2"])
add_insn("andpd",    "xmm_xmm128", modifiers=[0x66, 0x54], cpu=["SSE2"])
add_insn("cvtdq2ps", "xmm_xmm128", modifiers=[0, 0x5B], cpu=["SSE2"])
add_insn("cvtpd2dq", "xmm_xmm128", modifiers=[0xF2, 0xE6], cpu=["SSE2"])
add_insn("cvtpd2ps", "xmm_xmm128", modifiers=[0x66, 0x5A], cpu=["SSE2"])
add_insn("cvtps2dq", "xmm_xmm128", modifiers=[0x66, 0x5B], cpu=["SSE2"])
add_insn("divpd",    "xmm_xmm128", modifiers=[0x66, 0x5E], cpu=["SSE2"])
add_insn("maxpd",    "xmm_xmm128", modifiers=[0x66, 0x5F], cpu=["SSE2"])
add_insn("minpd",    "xmm_xmm128", modifiers=[0x66, 0x5D], cpu=["SSE2"])
add_insn("mulpd",    "xmm_xmm128", modifiers=[0x66, 0x59], cpu=["SSE2"])
add_insn("orpd",     "xmm_xmm128", modifiers=[0x66, 0x56], cpu=["SSE2"])
add_insn("sqrtpd",   "xmm_xmm128", modifiers=[0x66, 0x51], cpu=["SSE2"])
add_insn("subpd",    "xmm_xmm128", modifiers=[0x66, 0x5C], cpu=["SSE2"])
add_insn("unpckhpd", "xmm_xmm128", modifiers=[0x66, 0x15], cpu=["SSE2"])
add_insn("unpcklpd", "xmm_xmm128", modifiers=[0x66, 0x14], cpu=["SSE2"])
add_insn("xorpd",    "xmm_xmm128", modifiers=[0x66, 0x57], cpu=["SSE2"])

add_insn("vaddpd",    "xmm_xmm128_256", modifiers=[0x66, 0x58, VEXL0], avx=True)
add_insn("vandnpd",   "xmm_xmm128_256", modifiers=[0x66, 0x55, VEXL0], avx=True)
add_insn("vandpd",    "xmm_xmm128_256", modifiers=[0x66, 0x54, VEXL0], avx=True)
# vcvtdq2ps and vcvtps2dq are 2-operand, YMM capable
# vcvtpd2dq and vcvtpd2ps take xmm, ymm combination
add_insn("vdivpd",    "xmm_xmm128_256", modifiers=[0x66, 0x5E, VEXL0], avx=True)
add_insn("vmaxpd",    "xmm_xmm128_256", modifiers=[0x66, 0x5F, VEXL0], avx=True)
add_insn("vminpd",    "xmm_xmm128_256", modifiers=[0x66, 0x5D, VEXL0], avx=True)
add_insn("vmulpd",    "xmm_xmm128_256", modifiers=[0x66, 0x59, VEXL0], avx=True)
add_insn("vorpd",     "xmm_xmm128_256", modifiers=[0x66, 0x56, VEXL0], avx=True)
# vsqrtpd doesn't add third operand
add_insn("vsubpd",    "xmm_xmm128_256", modifiers=[0x66, 0x5C, VEXL0], avx=True)
add_insn("vunpckhpd", "xmm_xmm128_256", modifiers=[0x66, 0x15, VEXL0], avx=True)
add_insn("vunpcklpd", "xmm_xmm128_256", modifiers=[0x66, 0x14, VEXL0], avx=True)
add_insn("vxorpd",    "xmm_xmm128_256", modifiers=[0x66, 0x57, VEXL0], avx=True)

add_group("ssecmp_64",
    cpu=["SSE2"],
    modifiers=["Imm8", "PreAdd", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("ssecmp_64",
    cpu=["SSE2"],
    modifiers=["Imm8", "PreAdd", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_group("ssecmp_64",
    cpu=["AVX"],
    modifiers=["Imm8", "PreAdd"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("ssecmp_64",
    cpu=["AVX"],
    modifiers=["Imm8", "PreAdd"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])

for ib, cc in ssecoms:
    add_insn("cmp"+cc+"sd", "ssecmp_64", modifiers=[ib, 0xF2])
    add_insn("cmp"+cc+"pd", "ssecmp_128", modifiers=[ib, 0x66])

for ib, cc in avxcoms:
    add_insn("vcmp"+cc+"sd", "ssecmp_64", modifiers=[ib, 0xF2, VEXL0], avx=True)
    add_insn("vcmp"+cc+"pd", "ssecmp_128", modifiers=[ib, 0x66, VEXL0], avx=True)

add_insn("cmppd",  "xmm_xmm128_imm", modifiers=[0x66, 0xC2], cpu=["SSE2"])
add_insn("shufpd", "xmm_xmm128_imm", modifiers=[0x66, 0xC6], cpu=["SSE2"])
add_insn("vcmppd",  "xmm_xmm128_imm_256", modifiers=[0x66, 0xC2, VEXL0], avx=True)
add_insn("vshufpd", "xmm_xmm128_imm_256", modifiers=[0x66, 0xC6, VEXL0], avx=True)

add_insn("cvtsi2sd", "cvt_xmm_rmx", modifiers=[0xF2, 0x2A], cpu=["SSE2"])
add_insn("vcvtsi2sd", "cvt_xmm_rmx", modifiers=[0xF2, 0x2A, VEXL0], avx=True)

add_group("cvt_rx_xmm64",
    suffix="l",
    cpu=["SSE2"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("cvt_rx_xmm64",
    suffix="l",
    cpu=["SSE2"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
# REX
add_group("cvt_rx_xmm64",
    suffix="q",
    cpu=["SSE2"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    opersize=64,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("cvt_rx_xmm64",
    suffix="q",
    cpu=["SSE2"],
    modifiers=["PreAdd", "Op1Add", "SetVEX"],
    opersize=64,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])

add_insn("cvtsd2si", "cvt_rx_xmm64", modifiers=[0xF2, 0x2D])
add_insn("vcvtsd2si", "cvt_rx_xmm64", modifiers=[0xF2, 0x2D, VEXL0], avx=True)

add_group("cvt_mm_xmm",
    cpu=["SSE2"],
    modifiers=["PreAdd", "Op1Add"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])

add_insn("cvtpd2pi", "cvt_mm_xmm", modifiers=[0x66, 0x2D], cpu=["SSE2"])

add_group("cvt_xmm_mm_ss",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])

add_insn("cvtpi2pd", "cvt_xmm_mm_ss", modifiers=[0x66, 0x2A], cpu=["SSE2"])

# cmpsd SSE2 form
add_group("cmpsd",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0xF2,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("cmpsd",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0xF2,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("cmpsd",
    cpu=["AVX"],
    vex=128,
    prefix=0xF2,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("cmpsd",
    cpu=["AVX"],
    vex=128,
    prefix=0xF2,
    opcode=[0x0F, 0xC2],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

# cmpsd is added in string instructions above, so don't re-add_insn()
add_insn("vcmpsd", "cmpsd", modifiers=[VEXL0], avx=True)

add_insn("movapd", "movau", modifiers=[0x66, 0x28, 0x01], cpu=["SSE2"])
add_insn("movupd", "movau", modifiers=[0x66, 0x10, 0x01], cpu=["SSE2"])
add_insn("vmovapd", "movau", modifiers=[0x66, 0x28, 0x01], avx=True)
add_insn("vmovupd", "movau", modifiers=[0x66, 0x10, 0x01], avx=True)

add_insn("movhpd", "movhlp", modifiers=[0x66, 0x16], cpu=["SSE2"])
add_insn("movlpd", "movhlp", modifiers=[0x66, 0x12], cpu=["SSE2"])
add_insn("vmovhpd", "movhlp", modifiers=[0x66, 0x16, VEXL0], avx=True)
add_insn("vmovlpd", "movhlp", modifiers=[0x66, 0x12, VEXL0], avx=True)

add_insn("movmskpd", "movmsk", modifiers=[0x66], cpu=["SSE2"])
add_insn("vmovmskpd", "movmsk", modifiers=[0x66, VEXL0], avx=True)

add_insn("movntpd", "movnt", modifiers=[0x66, 0x2B], cpu=["SSE2"])
add_insn("movntdq", "movnt", modifiers=[0x66, 0xE7], cpu=["SSE2"])
add_insn("vmovntpd", "movnt", modifiers=[0x66, 0x2B, VEXL0], avx=True)
add_insn("vmovntdq", "movnt", modifiers=[0x66, 0xE7, VEXL0], avx=True)

# movsd SSE2 forms
add_group("movsd",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0xF2,
    opcode=[0x0F, 0x10],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("movsd",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0xF2,
    opcode=[0x0F, 0x10],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_group("movsd",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0xF2,
    opcode=[0x0F, 0x11],
    operands=[Operand(type="Mem", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("movsd",
    cpu=["AVX"],
    vex=128,
    prefix=0xF2,
    opcode=[0x0F, 0x10],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])
# movsd is added in string instructions above, so don't re-add_insn()
add_insn("vmovsd", "movsd", modifiers=[VEXL0], avx=True)

#####################################################################
# P4 VMX Instructions
#####################################################################

add_group("eptvpid",
    modifiers=["Op2Add"],
    suffix="l",
    not64=True,
    cpu=["EPTVPID"],
    opersize=32,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x80],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="Mem", size=128, relaxed=True, dest="EA")])
add_group("eptvpid",
    modifiers=["Op2Add"],
    suffix="q",
    cpu=["EPTVPID"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x80],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="Mem", size=128, relaxed=True, dest="EA")])
add_insn("invept", "eptvpid", modifiers=[0])
add_insn("invvpid", "eptvpid", modifiers=[1])

add_insn("vmcall", "threebyte", modifiers=[0x0F, 0x01, 0xC1], cpu=["P4"])
add_insn("vmlaunch", "threebyte", modifiers=[0x0F, 0x01, 0xC2], cpu=["P4"])
add_insn("vmresume", "threebyte", modifiers=[0x0F, 0x01, 0xC3], cpu=["P4"])
add_insn("vmxoff", "threebyte", modifiers=[0x0F, 0x01, 0xC4], cpu=["P4"])

add_group("vmxmemrd",
    suffix="l",
    not64=True,
    cpu=["P4"],
    opersize=32,
    opcode=[0x0F, 0x78],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="Reg", size=32, dest="Spare")])
add_group("vmxmemrd",
    suffix="q",
    cpu=["P4"],
    opersize=64,
    def_opersize_64=64,
    opcode=[0x0F, 0x78],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="Reg", size=64, dest="Spare")])
add_insn("vmread", "vmxmemrd")

add_group("vmxmemwr",
    suffix="l",
    not64=True,
    cpu=["P4"],
    opersize=32,
    opcode=[0x0F, 0x79],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="RM", size=32, relaxed=True, dest="EA")])
add_group("vmxmemwr",
    suffix="q",
    cpu=["P4"],
    opersize=64,
    def_opersize_64=64,
    opcode=[0x0F, 0x79],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="RM", size=64, relaxed=True, dest="EA")])
add_insn("vmwrite", "vmxmemwr")

add_group("vmxtwobytemem",
    modifiers=["SpAdd"],
    cpu=["P4"],
    opcode=[0x0F, 0xC7],
    spare=0,
    operands=[Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_insn("vmptrld", "vmxtwobytemem", modifiers=[6])
add_insn("vmptrst", "vmxtwobytemem", modifiers=[7])

add_group("vmxthreebytemem",
    modifiers=["PreAdd"],
    cpu=["P4"],
    prefix=0x00,
    opcode=[0x0F, 0xC7],
    spare=6,
    operands=[Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_insn("vmclear", "vmxthreebytemem", modifiers=[0x66])
add_insn("vmxon", "vmxthreebytemem", modifiers=[0xF3])

#####################################################################
# Intel SMX Instructions
#####################################################################
add_insn("getsec", "twobyte", modifiers=[0x0F, 0x37], cpu=["SMX"])

add_insn("cvttpd2pi", "cvt_mm_xmm", modifiers=[0x66, 0x2C], cpu=["SSE2"])
add_insn("cvttsd2si", "cvt_rx_xmm64", modifiers=[0xF2, 0x2C], cpu=["SSE2"])
add_insn("cvttpd2dq", "xmm_xmm128", modifiers=[0x66, 0xE6], cpu=["SSE2"])
add_insn("cvttps2dq", "xmm_xmm128", modifiers=[0xF3, 0x5B], cpu=["SSE2"])
add_insn("pmuludq", "mmxsse2", modifiers=[0xF4], cpu=["SSE2"])
add_insn("pshufd", "xmm_xmm128_imm", modifiers=[0x66, 0x70], cpu=["SSE2"])
add_insn("pshufhw", "xmm_xmm128_imm", modifiers=[0xF3, 0x70], cpu=["SSE2"])
add_insn("pshuflw", "xmm_xmm128_imm", modifiers=[0xF2, 0x70], cpu=["SSE2"])
add_insn("punpckhqdq", "xmm_xmm128", modifiers=[0x66, 0x6D], cpu=["SSE2"])
add_insn("punpcklqdq", "xmm_xmm128", modifiers=[0x66, 0x6C], cpu=["SSE2"])

add_insn("vcvttsd2si", "cvt_rx_xmm64", modifiers=[0xF2, 0x2C, VEXL0], avx=True)
# vcvttpd2dq takes xmm, ymm combination
# vcvttps2dq is two-operand
add_insn("vpmuludq", "xmm_xmm128_256avx2", modifiers=[0x66, 0xF4, VEXL0], avx=True)
add_insn("vpshufd", "xmm_xmm128_imm_256avx2", modifiers=[0x66, 0x70, VEXL0], avx=True)
add_insn("vpshufhw", "xmm_xmm128_imm_256avx2", modifiers=[0xF3, 0x70, VEXL0], avx=True)
add_insn("vpshuflw", "xmm_xmm128_imm_256avx2", modifiers=[0xF2, 0x70, VEXL0], avx=True)
add_insn("vpunpckhqdq", "xmm_xmm128_256avx2", modifiers=[0x66, 0x6D, VEXL0], avx=True)
add_insn("vpunpcklqdq", "xmm_xmm128_256avx2", modifiers=[0x66, 0x6C, VEXL0], avx=True)

add_insn("cvtss2sd", "xmm_xmm32", modifiers=[0xF3, 0x5A], cpu=["SSE2"])
add_insn("vcvtss2sd", "xmm_xmm32", modifiers=[0xF3, 0x5A, VEXL0], avx=True)

add_group("maskmovdqu",
    cpu=["SSE2"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0xF7],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_insn("maskmovdqu", "maskmovdqu")
add_insn("vmaskmovdqu", "maskmovdqu", modifiers=[VEXL0], avx=True)

add_insn("movdqa", "movau", modifiers=[0x66, 0x6F, 0x10], cpu=["SSE2"])
add_insn("movdqu", "movau", modifiers=[0xF3, 0x6F, 0x10], cpu=["SSE2"])
add_insn("vmovdqa", "movau", modifiers=[0x66, 0x6F, 0x10], avx=True)
add_insn("vmovdqu", "movau", modifiers=[0xF3, 0x6F, 0x10], avx=True)

add_group("movdq2q",
    cpu=["SSE2"],
    prefix=0xF2,
    opcode=[0x0F, 0xD6],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_insn("movdq2q", "movdq2q")

add_group("movq2dq",
    cpu=["SSE2"],
    prefix=0xF3,
    opcode=[0x0F, 0xD6],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=64, dest="EA")])

add_insn("movq2dq", "movq2dq")

add_group("pslrldq",
    cpu=["SSE2"],
    modifiers=["SpAdd", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x73],
    spare=0,
    operands=[Operand(type="SIMDReg", size=128, dest="EAVEX"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pslrldq",
    cpu=["SSE2"],
    modifiers=["SpAdd", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x73],
    spare=0,
    operands=[Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pslrldq",
    cpu=["AVX2"],
    modifiers=["SpAdd"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x73],
    spare=0,
    operands=[Operand(type="SIMDReg", size=256, dest="EAVEX"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pslrldq",
    cpu=["AVX2"],
    modifiers=["SpAdd"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x73],
    spare=0,
    operands=[Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDReg", size=256, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pslldq", "pslrldq", modifiers=[7])
add_insn("psrldq", "pslrldq", modifiers=[3])
add_insn("vpslldq", "pslrldq", modifiers=[7, VEXL0], avx=True)
add_insn("vpsrldq", "pslrldq", modifiers=[3, VEXL0], avx=True)

#####################################################################
# SSE3 / PNI Prescott New Instructions instructions
#####################################################################
add_insn("addsubpd", "xmm_xmm128", modifiers=[0x66, 0xD0], cpu=["SSE3"])
add_insn("addsubps", "xmm_xmm128", modifiers=[0xF2, 0xD0], cpu=["SSE3"])
add_insn("haddpd",   "xmm_xmm128", modifiers=[0x66, 0x7C], cpu=["SSE3"])
add_insn("haddps",   "xmm_xmm128", modifiers=[0xF2, 0x7C], cpu=["SSE3"])
add_insn("hsubpd",   "xmm_xmm128", modifiers=[0x66, 0x7D], cpu=["SSE3"])
add_insn("hsubps",   "xmm_xmm128", modifiers=[0xF2, 0x7D], cpu=["SSE3"])

add_insn("vaddsubpd", "xmm_xmm128_256", modifiers=[0x66, 0xD0, VEXL0], avx=True)
add_insn("vaddsubps", "xmm_xmm128_256", modifiers=[0xF2, 0xD0, VEXL0], avx=True)
add_insn("vhaddpd",   "xmm_xmm128_256", modifiers=[0x66, 0x7C, VEXL0], avx=True)
add_insn("vhaddps",   "xmm_xmm128_256", modifiers=[0xF2, 0x7C, VEXL0], avx=True)
add_insn("vhsubpd",   "xmm_xmm128_256", modifiers=[0x66, 0x7D, VEXL0], avx=True)
add_insn("vhsubps",   "xmm_xmm128_256", modifiers=[0xF2, 0x7D, VEXL0], avx=True)

add_insn("movshdup", "xmm_xmm128", modifiers=[0xF3, 0x16], cpu=["SSE3"])
add_insn("movsldup", "xmm_xmm128", modifiers=[0xF3, 0x12], cpu=["SSE3"])
add_insn("fisttp",   "fildstp", modifiers=[1, 0, 1], cpu=["SSE3"])
add_insn("fisttpll", "fildstp", suffix="q", modifiers=[7], cpu=["SSE3"])
add_insn("movddup", "xmm_xmm64", modifiers=[0xF2, 0x12], cpu=["SSE3"])
add_insn("monitor", "threebyte", modifiers=[0x0F, 0x01, 0xC8], cpu=["SSE3"])
add_insn("mwait",   "threebyte", modifiers=[0x0F, 0x01, 0xC9], cpu=["SSE3"])

add_group("lddqu",
    cpu=["SSE3"],
    modifiers=["SetVEX"],
    prefix=0xF2,
    opcode=[0x0F, 0xF0],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=128, relaxed=True, dest="EA")])
add_group("lddqu",
    cpu=["AVX"],
    vex=256,
    prefix=0xF2,
    opcode=[0x0F, 0xF0],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="Mem", size=256, relaxed=True, dest="EA")])

add_insn("lddqu", "lddqu")
add_insn("vlddqu", "lddqu", modifiers=[VEXL0], avx=True)

#####################################################################
# SSSE3 / TNI Tejas New Intructions instructions
#####################################################################

add_group("ssse3",
    cpu=["SSSE3"],
    notavx=True,
    modifiers=["Op2Add"],
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])
add_group("ssse3",
    cpu=["SSSE3"],
    modifiers=["Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("ssse3",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("ssse3",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="SpareVEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])
add_group("ssse3",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_insn("pshufb",    "ssse3", modifiers=[0x00])
add_insn("phaddw",    "ssse3", modifiers=[0x01])
add_insn("phaddd",    "ssse3", modifiers=[0x02])
add_insn("phaddsw",   "ssse3", modifiers=[0x03])
add_insn("pmaddubsw", "ssse3", modifiers=[0x04])
add_insn("phsubw",    "ssse3", modifiers=[0x05])
add_insn("phsubd",    "ssse3", modifiers=[0x06])
add_insn("phsubsw",   "ssse3", modifiers=[0x07])
add_insn("psignb",    "ssse3", modifiers=[0x08])
add_insn("psignw",    "ssse3", modifiers=[0x09])
add_insn("psignd",    "ssse3", modifiers=[0x0A])
add_insn("pmulhrsw",  "ssse3", modifiers=[0x0B])
add_insn("pabsb",     "ssse3", modifiers=[0x1C])
add_insn("pabsw",     "ssse3", modifiers=[0x1D])
add_insn("pabsd",     "ssse3", modifiers=[0x1E])

add_insn("vpshufb",    "ssse3", modifiers=[0x00, VEXL0], avx=True)
add_insn("vphaddw",    "ssse3", modifiers=[0x01, VEXL0], avx=True)
add_insn("vphaddd",    "ssse3", modifiers=[0x02, VEXL0], avx=True)
add_insn("vphaddsw",   "ssse3", modifiers=[0x03, VEXL0], avx=True)
add_insn("vpmaddubsw", "ssse3", modifiers=[0x04, VEXL0], avx=True)
add_insn("vphsubw",    "ssse3", modifiers=[0x05, VEXL0], avx=True)
add_insn("vphsubd",    "ssse3", modifiers=[0x06, VEXL0], avx=True)
add_insn("vphsubsw",   "ssse3", modifiers=[0x07, VEXL0], avx=True)
add_insn("vpsignb",    "ssse3", modifiers=[0x08, VEXL0], avx=True)
add_insn("vpsignw",    "ssse3", modifiers=[0x09, VEXL0], avx=True)
add_insn("vpsignd",    "ssse3", modifiers=[0x0A, VEXL0], avx=True)
add_insn("vpmulhrsw",  "ssse3", modifiers=[0x0B, VEXL0], avx=True)
# vpabsb/vpabsw/vpabsd are 2 operand only

add_group("ssse3imm",
    cpu=["SSSE3"],
    modifiers=["Op2Add"],
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("ssse3imm",
    cpu=["SSSE3"],
    modifiers=["Op2Add"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("palignr", "ssse3imm", modifiers=[0x0F])
add_insn("vpalignr", "sse4imm_256avx2", modifiers=[0x0F, VEXL0], avx=True)

#####################################################################
# SSE4.1 / SSE4.2 instructions
#####################################################################

add_group("sse4",
    cpu=["SSE41"],
    modifiers=["Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("sse4",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_insn("packusdw",   "sse4", modifiers=[0x2B])
add_insn("pcmpeqq",    "sse4", modifiers=[0x29])
add_insn("pcmpgtq",    "sse4", modifiers=[0x37])
add_insn("phminposuw", "sse4", modifiers=[0x41])
add_insn("pmaxsb",     "sse4", modifiers=[0x3C])
add_insn("pmaxsd",     "sse4", modifiers=[0x3D])
add_insn("pmaxud",     "sse4", modifiers=[0x3F])
add_insn("pmaxuw",     "sse4", modifiers=[0x3E])
add_insn("pminsb",     "sse4", modifiers=[0x38])
add_insn("pminsd",     "sse4", modifiers=[0x39])
add_insn("pminud",     "sse4", modifiers=[0x3B])
add_insn("pminuw",     "sse4", modifiers=[0x3A])
add_insn("pmuldq",     "sse4", modifiers=[0x28])
add_insn("pmulld",     "sse4", modifiers=[0x40])
add_insn("ptest",      "sse4", modifiers=[0x17])

# AVX versions use ssse3, and disable MMX version, as they're 3-operand
add_insn("vpackusdw",   "ssse3", modifiers=[0x2B, VEXL0], avx=True)
add_insn("vpcmpeqq",    "ssse3", modifiers=[0x29, VEXL0], avx=True)
add_insn("vpcmpgtq",    "ssse3", modifiers=[0x37, VEXL0], avx=True)
# vphminposuw is 2 operand only
add_insn("vpmaxsb",     "ssse3", modifiers=[0x3C, VEXL0], avx=True)
add_insn("vpmaxsd",     "ssse3", modifiers=[0x3D, VEXL0], avx=True)
add_insn("vpmaxud",     "ssse3", modifiers=[0x3F, VEXL0], avx=True)
add_insn("vpmaxuw",     "ssse3", modifiers=[0x3E, VEXL0], avx=True)
add_insn("vpminsb",     "ssse3", modifiers=[0x38, VEXL0], avx=True)
add_insn("vpminsd",     "ssse3", modifiers=[0x39, VEXL0], avx=True)
add_insn("vpminud",     "ssse3", modifiers=[0x3B, VEXL0], avx=True)
add_insn("vpminuw",     "ssse3", modifiers=[0x3A, VEXL0], avx=True)
add_insn("vpmuldq",     "ssse3", modifiers=[0x28, VEXL0], avx=True)
add_insn("vpmulld",     "ssse3", modifiers=[0x40, VEXL0], avx=True)
# vptest uses SSE4 style (2 operand only), and takes 256-bit operands
add_insn("vptest", "sse4", modifiers=[0x17, VEXL0], avx=True)

add_group("sse4imm_256",
    cpu=["SSE41"],
    modifiers=["Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("sse4imm_256",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("sse4imm_256",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="SpareVEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("sse4imm_256",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

# Same as above except AVX2 required for 256-bit.
add_group("sse4imm_256avx2",
    cpu=["SSE41"],
    modifiers=["Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("sse4imm_256avx2",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("sse4imm_256avx2",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="SpareVEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("sse4imm_256avx2",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

# Version that does not allow YMM registers
add_group("sse4imm",
    cpu=["SSE41"],
    modifiers=["Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("sse4imm",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

for sz in [32, 64]:
    add_group("sse4m%dimm" % sz,
        cpu=["SSE41"],
        modifiers=["Op2Add", "SetVEX"],
        prefix=0x66,
        opcode=[0x0F, 0x3A, 0x00],
        operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
                  Operand(type="SIMDReg", size=128, dest="EA"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
    add_group("sse4m%dimm" % sz,
        cpu=["SSE41"],
        modifiers=["Op2Add", "SetVEX"],
        prefix=0x66,
        opcode=[0x0F, 0x3A, 0x00],
        operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
                  Operand(type="Mem", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
    add_group("sse4m%dimm" % sz,
        cpu=["AVX"],
        modifiers=["Op2Add"],
        vex=128,
        prefix=0x66,
        opcode=[0x0F, 0x3A, 0x00],
        operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
                  Operand(type="SIMDReg", size=128, dest="VEX"),
                  Operand(type="SIMDReg", size=128, dest="EA"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
    add_group("sse4m%dimm" % sz,
        cpu=["AVX"],
        modifiers=["Op2Add"],
        vex=128,
        prefix=0x66,
        opcode=[0x0F, 0x3A, 0x00],
        operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
                  Operand(type="SIMDReg", size=128, dest="VEX"),
                  Operand(type="Mem", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("blendpd", "sse4imm", modifiers=[0x0D])
add_insn("blendps", "sse4imm", modifiers=[0x0C])
add_insn("dppd",    "sse4imm", modifiers=[0x41])
add_insn("dpps",    "sse4imm", modifiers=[0x40])
add_insn("mpsadbw", "sse4imm", modifiers=[0x42])
add_insn("pblendw", "sse4imm", modifiers=[0x0E])
add_insn("roundpd", "sse4imm", modifiers=[0x09])
add_insn("roundps", "sse4imm", modifiers=[0x08])
add_insn("roundsd", "sse4m64imm", modifiers=[0x0B])
add_insn("roundss", "sse4m32imm", modifiers=[0x0A])

# vdppd does not allow YMM registers
# vmpsadbw and vpblendw do not allow YMM registers unless AVX2
add_insn("vblendpd", "sse4imm_256", modifiers=[0x0D, VEXL0], avx=True)
add_insn("vblendps", "sse4imm_256", modifiers=[0x0C, VEXL0], avx=True)
add_insn("vdppd",    "sse4imm", modifiers=[0x41, VEXL0], avx=True)
add_insn("vdpps",    "sse4imm_256", modifiers=[0x40, VEXL0], avx=True)
add_insn("vmpsadbw", "sse4imm_256avx2", modifiers=[0x42, VEXL0], avx=True)
add_insn("vpblendw", "sse4imm_256avx2", modifiers=[0x0E, VEXL0], avx=True)
# vroundpd and vroundps don't add another register operand
add_insn("vroundsd", "sse4m64imm", modifiers=[0x0B, VEXL0], avx=True)
add_insn("vroundss", "sse4m32imm", modifiers=[0x0A, VEXL0], avx=True)

add_group("sse4xmm0",
    cpu=["SSE41"],
    modifiers=["Op2Add"],
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("sse4xmm0",
    cpu=["SSE41"],
    modifiers=["Op2Add"],
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="XMM0", size=128, dest=None)])

add_insn("blendvpd", "sse4xmm0", modifiers=[0x15])
add_insn("blendvps", "sse4xmm0", modifiers=[0x14])
add_insn("pblendvb", "sse4xmm0", modifiers=[0x10])

# implicit XMM0 can't be VEX-encoded
add_group("avx_sse4xmm0",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEXImmSrc")])
add_group("avx_sse4xmm0",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="VEXImmSrc")])

add_insn("vblendvpd", "avx_sse4xmm0", modifiers=[0x4B])
add_insn("vblendvps", "avx_sse4xmm0", modifiers=[0x4A])

# vpblendvb didn't have a 256-bit form until AVX2
add_group("avx2_sse4xmm0",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEXImmSrc")])
add_group("avx2_sse4xmm0",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="VEXImmSrc")])

add_insn("vpblendvb", "avx2_sse4xmm0", modifiers=[0x4C])

for sfx, sz in zip("bwl", [8, 16, 32]):
    add_group("crc32",
        suffix=sfx,
        cpu=["SSE42"],
        opersize=sz,
        prefix=0xF2,
        opcode=[0x0F, 0x38, 0xF0+(sz!=8)],
        operands=[Operand(type="Reg", size=32, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=(sz==32), dest="EA")])
for sfx, sz in zip("bq", [8, 64]):
    add_group("crc32",
        suffix=sfx,
        cpu=["SSE42"],
        opersize=64,
        prefix=0xF2,
        opcode=[0x0F, 0x38, 0xF0+(sz!=8)],
        operands=[Operand(type="Reg", size=64, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=(sz==64), dest="EA")])

add_insn("crc32", "crc32")

add_group("extractps",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x17],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("extractps",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x17],
    operands=[Operand(type="Reg", size=64, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("extractps", "extractps")
add_insn("vextractps", "extractps", modifiers=[VEXL0], avx=True)

add_group("insertps",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x21],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("insertps",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x21],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("insertps",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x21],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("insertps",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x21],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("insertps", "insertps")
add_insn("vinsertps", "insertps", modifiers=[VEXL0], avx=True)

add_group("movntdqa",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x2A],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=128, relaxed=True, dest="EA")])
add_group("movntdqa",
    cpu=["AVX2"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x2A],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="Mem", size=256, relaxed=True, dest="EA")])

add_insn("movntdqa", "movntdqa")
add_insn("vmovntdqa", "movntdqa", modifiers=[VEXL0], avx=True)

add_group("sse4pcmpstr",
    cpu=["SSE42"],
    modifiers=["Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pcmpestri", "sse4pcmpstr", modifiers=[0x61])
add_insn("pcmpestrm", "sse4pcmpstr", modifiers=[0x60])
add_insn("pcmpistri", "sse4pcmpstr", modifiers=[0x63])
add_insn("pcmpistrm", "sse4pcmpstr", modifiers=[0x62])

add_insn("vpcmpestri", "sse4pcmpstr", modifiers=[0x61, VEXL0], avx=True)
add_insn("vpcmpestrm", "sse4pcmpstr", modifiers=[0x60, VEXL0], avx=True)
add_insn("vpcmpistri", "sse4pcmpstr", modifiers=[0x63, VEXL0], avx=True)
add_insn("vpcmpistrm", "sse4pcmpstr", modifiers=[0x62, VEXL0], avx=True)

add_group("pextrb",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x14],
    operands=[Operand(type="Mem", size=8, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pextrb",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x14],
    operands=[Operand(type="Reg", size=32, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pextrb",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x14],
    operands=[Operand(type="Reg", size=64, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pextrb", "pextrb")
add_insn("vpextrb", "pextrb", modifiers=[VEXL0], avx=True)

add_group("pextrd",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x16],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pextrd", "pextrd")
add_insn("vpextrd", "pextrd", modifiers=[VEXL0], avx=True)

add_group("pextrq",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x16],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pextrq", "pextrq")
add_insn("vpextrq", "pextrq", modifiers=[VEXL0], avx=True)

add_group("pinsrb",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x20],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Mem", size=8, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrb",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x20],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="Reg", size=32, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrb",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x20],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=8, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrb",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x20],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Reg", size=32, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pinsrb", "pinsrb")
add_insn("vpinsrb", "pinsrb", modifiers=[VEXL0], avx=True)

add_group("pinsrd",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x22],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrd",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x22],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pinsrd", "pinsrd")
add_insn("vpinsrd", "pinsrd", modifiers=[VEXL0], avx=True)

add_group("pinsrq",
    cpu=["SSE41"],
    modifiers=["SetVEX"],
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x22],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pinsrq",
    cpu=["AVX"],
    vex=128,
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x22],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("pinsrq", "pinsrq")
add_insn("vpinsrq", "pinsrq", modifiers=[VEXL0], avx=True)

for sz in [16, 32, 64]:
    add_group("sse4m%d" % sz,
        cpu=["SSE41"],
        modifiers=["Op2Add", "SetVEX"],
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x00],
        operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
                  Operand(type="Mem", size=sz, relaxed=True, dest="EA")])
    add_group("sse4m%d" % sz,
        cpu=["SSE41"],
        modifiers=["Op2Add", "SetVEX"],
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x00],
        operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
                  Operand(type="SIMDReg", size=128, dest="EA")])
    add_group("sse4m%d" % sz,
        cpu=["AVX2"],
        modifiers=["Op2Add"],
        vex=256,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x00],
        operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
                  Operand(type="Mem", size=sz*2, relaxed=True, dest="EA")])
    add_group("sse4m%d" % sz,
        cpu=["AVX2"],
        modifiers=["Op2Add"],
        vex=256,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x00],
        operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
                  Operand(type="SIMDReg", size=128, dest="EA")])

add_insn("pmovsxbw", "sse4m64", modifiers=[0x20])
add_insn("pmovsxwd", "sse4m64", modifiers=[0x23])
add_insn("pmovsxdq", "sse4m64", modifiers=[0x25])
add_insn("pmovzxbw", "sse4m64", modifiers=[0x30])
add_insn("pmovzxwd", "sse4m64", modifiers=[0x33])
add_insn("pmovzxdq", "sse4m64", modifiers=[0x35])

add_insn("vpmovsxbw", "sse4m64", modifiers=[0x20, VEXL0], avx=True)
add_insn("vpmovsxwd", "sse4m64", modifiers=[0x23, VEXL0], avx=True)
add_insn("vpmovsxdq", "sse4m64", modifiers=[0x25, VEXL0], avx=True)
add_insn("vpmovzxbw", "sse4m64", modifiers=[0x30, VEXL0], avx=True)
add_insn("vpmovzxwd", "sse4m64", modifiers=[0x33, VEXL0], avx=True)
add_insn("vpmovzxdq", "sse4m64", modifiers=[0x35, VEXL0], avx=True)

add_insn("pmovsxbd", "sse4m32", modifiers=[0x21])
add_insn("pmovsxwq", "sse4m32", modifiers=[0x24])
add_insn("pmovzxbd", "sse4m32", modifiers=[0x31])
add_insn("pmovzxwq", "sse4m32", modifiers=[0x34])

add_insn("vpmovsxbd", "sse4m32", modifiers=[0x21, VEXL0], avx=True)
add_insn("vpmovsxwq", "sse4m32", modifiers=[0x24, VEXL0], avx=True)
add_insn("vpmovzxbd", "sse4m32", modifiers=[0x31, VEXL0], avx=True)
add_insn("vpmovzxwq", "sse4m32", modifiers=[0x34, VEXL0], avx=True)

add_insn("pmovsxbq", "sse4m16", modifiers=[0x22])
add_insn("pmovzxbq", "sse4m16", modifiers=[0x32])

add_insn("vpmovsxbq", "sse4m16", modifiers=[0x22, VEXL0], avx=True)
add_insn("vpmovzxbq", "sse4m16", modifiers=[0x32, VEXL0], avx=True)

for sfx, sz in zip("wlq", [16, 32, 64]):
    add_group("cnt",
        suffix=sfx,
        modifiers=["Op1Add"],
        opersize=sz,
        prefix=0xF3,
        opcode=[0x0F, 0x00],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA")])

add_insn("popcnt", "cnt", modifiers=[0xB8], cpu=["SSE42"])

#####################################################################
# Intel AVX instructions
#####################################################################

# Most AVX instructions are mixed in with above SSEx groups.
# Some make more sense to have separate groups due to naming conflicts
# that the v-named versions don't have to deal with.
add_group("vmovd",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="RM", size=32, relaxed=True, dest="EA")])
add_group("vmovd",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])

add_insn("vmovd", "vmovd")

add_group("vmovq",
    cpu=["AVX"],
    vex=128,
    prefix=0xF3,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("vmovq",
    cpu=["AVX"],
    vex=128,
    prefix=0xF3,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_group("vmovq",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0xD6],
    operands=[Operand(type="Mem", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("vmovq",
    cpu=["AVX"],
    vex=128,
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x6E],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="RM", size=64, relaxed=True, dest="EA")])
add_group("vmovq",
    cpu=["AVX"],
    vex=128,
    opersize=64,
    prefix=0x66,
    opcode=[0x0F, 0x7E],
    operands=[Operand(type="RM", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])

add_insn("vmovq", "vmovq")

# Some AVX variants don't add third operand
add_group("avx_xmm_xmm128",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("avx_xmm_xmm128",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_insn("vmovshdup", "avx_xmm_xmm128", modifiers=[0xF3, 0x16])
add_insn("vmovsldup", "avx_xmm_xmm128", modifiers=[0xF3, 0x12])
add_insn("vrcpps",    "avx_xmm_xmm128", modifiers=[0, 0x53])
add_insn("vrsqrtps",  "avx_xmm_xmm128", modifiers=[0, 0x52])
add_insn("vsqrtps",   "avx_xmm_xmm128", modifiers=[0, 0x51])
add_insn("vsqrtpd",   "avx_xmm_xmm128", modifiers=[0x66, 0x51])
add_insn("vcvtdq2ps", "avx_xmm_xmm128", modifiers=[0, 0x5B])
add_insn("vcvtps2dq", "avx_xmm_xmm128", modifiers=[0x66, 0x5B])
add_insn("vcvttps2dq", "avx_xmm_xmm128", modifiers=[0xF3, 0x5B])

add_group("avx_sse4imm",
    cpu=["SSE41"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("avx_sse4imm",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("avx_sse4imm",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vroundpd", "avx_sse4imm", modifiers=[0x09])
add_insn("vroundps", "avx_sse4imm", modifiers=[0x08])

add_group("vmovddup",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("vmovddup",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_group("vmovddup",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_insn("vmovddup",  "vmovddup", modifiers=[0xF2, 0x12])

# Some xmm_xmm64 combinations only take two operands in AVX
# (VEX.vvvv must be 1111b)
add_group("avx_xmm_xmm64",
    cpu=["SSE2"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("avx_xmm_xmm64",
    cpu=["SSE2"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])

add_insn("vcomisd",  "avx_xmm_xmm64", modifiers=[0x66, 0x2F], avx=True)
add_insn("vucomisd", "avx_xmm_xmm64", modifiers=[0x66, 0x2E], avx=True)

# Some xmm_xmm64 combinations only take two operands in AVX
# (VEX.vvvv must be 1111b)
add_group("avx_xmm_xmm32",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("avx_xmm_xmm32",
    cpu=["SSE"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])

add_insn("vcomiss",  "avx_xmm_xmm32", modifiers=[0, 0x2F], avx=True)
add_insn("vucomiss", "avx_xmm_xmm32", modifiers=[0, 0x2E], avx=True)

# Some conversion functions take ymm, xmm combination
add_group("avx_cvt_xmm64",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("avx_cvt_xmm64",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_group("avx_cvt_xmm64",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])

add_insn("vcvtdq2pd", "avx_cvt_xmm64", modifiers=[0xF3, 0xE6])
add_insn("vcvtps2pd", "avx_cvt_xmm64", modifiers=[0, 0x5A])

# Some SSE3 opcodes are only two operand in AVX
# (VEX.vvvv must be 1111b)
add_group("avx_ssse3_2op",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_insn("vphminposuw", "avx_ssse3_2op", modifiers=[0x41], avx=True)

# VPABS* are extended to 256-bit in AVX2
for cpu, sz in zip(["AVX", "AVX2"], [128, 256]):
    add_group("avx2_ssse3_2op",
        cpu=[cpu],
        modifiers=["Op2Add"],
        vex=sz,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x00],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDRM", size=sz, relaxed=True, dest="EA")])
add_insn("vpabsb",     "avx2_ssse3_2op", modifiers=[0x1C], avx=True)
add_insn("vpabsw",     "avx2_ssse3_2op", modifiers=[0x1D], avx=True)
add_insn("vpabsd",     "avx2_ssse3_2op", modifiers=[0x1E], avx=True)

# Some conversion functions take xmm, ymm combination
# Need separate x and y versions for gas mode
add_group("avx_cvt_xmm128_x",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("avx_cvt_xmm128_y",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_group("avx_cvt_xmm128",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=128,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, dest="EA")])
add_group("avx_cvt_xmm128",
    cpu=["AVX"],
    modifiers=["PreAdd", "Op1Add"],
    vex=256,
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=256, dest="EA")])

add_insn("vcvtpd2dqx", "avx_cvt_xmm128_x", modifiers=[0xF2, 0xE6], parser="gas")
add_insn("vcvtpd2dqy", "avx_cvt_xmm128_y", modifiers=[0xF2, 0xE6], parser="gas")
add_insn("vcvtpd2dq", "avx_cvt_xmm128", modifiers=[0xF2, 0xE6])

add_insn("vcvtpd2psx", "avx_cvt_xmm128_x", modifiers=[0x66, 0x5A], parser="gas")
add_insn("vcvtpd2psy", "avx_cvt_xmm128_y", modifiers=[0x66, 0x5A], parser="gas")
add_insn("vcvtpd2ps", "avx_cvt_xmm128", modifiers=[0x66, 0x5A])

add_insn("vcvttpd2dqx", "avx_cvt_xmm128_x", modifiers=[0x66, 0xE6], parser="gas")
add_insn("vcvttpd2dqy", "avx_cvt_xmm128_y", modifiers=[0x66, 0xE6], parser="gas")
add_insn("vcvttpd2dq", "avx_cvt_xmm128", modifiers=[0x66, 0xE6])

# Instructions new to AVX
add_insn("vtestps", "sse4", modifiers=[0x0E, VEXL0], avx=True)
add_insn("vtestpd", "sse4", modifiers=[0x0F, VEXL0], avx=True)

add_group("vbroadcastss",
    cpu=["AVX"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x18],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])
add_group("vbroadcastss",
    cpu=["AVX"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x18],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])
add_group("vbroadcastss",
    cpu=["AVX2"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x18],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("vbroadcastss",
    cpu=["AVX2"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x18],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_insn("vbroadcastss", "vbroadcastss")

add_group("vbroadcastsd",
    cpu=["AVX"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x19],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_group("vbroadcastsd",
    cpu=["AVX2"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x19],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_insn("vbroadcastsd", "vbroadcastsd")

add_group("vbroadcastif128",
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="Mem", size=128, relaxed=True, dest="EA")])

add_insn("vbroadcastf128", "vbroadcastif128", modifiers=[0x1A], cpu=["AVX"])
add_insn("vbroadcasti128", "vbroadcastif128", modifiers=[0x5A], cpu=["AVX2"])

add_group("vextractif128",
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vextractf128", "vextractif128", modifiers=[0x19], cpu=["AVX"])
add_insn("vextracti128", "vextractif128", modifiers=[0x39], cpu=["AVX2"])

add_group("vinsertif128",
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vinsertf128", "vinsertif128", modifiers=[0x18], cpu=["AVX"])
add_insn("vinserti128", "vinsertif128", modifiers=[0x38], cpu=["AVX2"])

add_group("vzero",
    cpu=["AVX"],
    modifiers=["SetVEX"],
    opcode=[0x0F, 0x77],
    operands=[])

add_insn("vzeroall", "vzero", modifiers=[VEXL1])
add_insn("vzeroupper", "vzero", modifiers=[VEXL0])

add_group("vmaskmov",
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("vmaskmov",
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])
add_group("vmaskmov",
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x02],
    operands=[Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="Spare")])
add_group("vmaskmov",
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x02],
    operands=[Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDReg", size=256, dest="Spare")])

add_insn("vmaskmovps", "vmaskmov", modifiers=[0x2C], cpu=["AVX"])
add_insn("vmaskmovpd", "vmaskmov", modifiers=[0x2D], cpu=["AVX"])

add_group("vpermil",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x08],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("vpermil",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x08],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])
add_group("vpermil",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("vpermil",
    cpu=["AVX"],
    modifiers=["Op2Add"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vpermilpd", "vpermil", modifiers=[0x05])
add_insn("vpermilps", "vpermil", modifiers=[0x04])

add_group("vperm2f128",
    cpu=["AVX"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x06],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vperm2f128", "vperm2f128")

#####################################################################
# Intel AVX2 instructions
#####################################################################

# Most AVX2 instructions are mixed in with above SSEx/AVX groups.
# Some make more sense to have separate groups.

# vex.vvvv=1111b
add_group("vperm_var_avx2",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=0,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_insn("vpermd",     "vperm_var_avx2", modifiers=[0x36])
add_insn("vpermps",    "vperm_var_avx2", modifiers=[0x16])

# vex.vvvv=1111b
add_group("vperm_imm_avx2",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=1,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vpermq",     "vperm_imm_avx2", modifiers=[0x00])
add_insn("vpermpd",    "vperm_imm_avx2", modifiers=[0x01])

add_group("vperm2i128_avx2",
    cpu=["AVX2"],
    vex=256,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x46],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vperm2i128", "vperm2i128_avx2")

# vex.vvvv=1111b
for sz in [128, 256]:
    add_group("vpbroadcastb_avx2",
        cpu=["AVX2"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x78],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDReg", size=128, relaxed=True, dest="EA")])
# vex.vvvv=1111b
for sz in [128, 256]:
    add_group("vpbroadcastb_avx2",
        cpu=["AVX2"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x78],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="RM", size=8, relaxed=True, dest="EA")])

add_insn("vpbroadcastb", "vpbroadcastb_avx2")

# vex.vvvv=1111b
for sz in [128, 256]:
    add_group("vpbroadcastw_avx2",
        cpu=["AVX2"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x79],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDReg", size=128, relaxed=True, dest="EA")])
# vex.vvvv=1111b
for sz in [128, 256]:
    add_group("vpbroadcastw_avx2",
        cpu=["AVX2"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x79],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="RM", size=16, relaxed=True, dest="EA")])

add_insn("vpbroadcastw", "vpbroadcastw_avx2")

# vex.vvvv=1111b
for sz in [128, 256]:
    add_group("vpbroadcastd_avx2",
        cpu=["AVX2"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x58],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDReg", size=128, relaxed=True, dest="EA")])
# vex.vvvv=1111b
for sz in [128, 256]:
    add_group("vpbroadcastd_avx2",
        cpu=["AVX2"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x58],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="RM", size=32, relaxed=True, dest="EA")])

add_insn("vpbroadcastd", "vpbroadcastd_avx2")

# vex.vvvv=1111b
for sz in [128, 256]:
    add_group("vpbroadcastq_avx2",
        cpu=["AVX2"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x59],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDReg", size=128, relaxed=True, dest="EA")])
# vex.vvvv=1111b
for sz in [128, 256]:
    add_group("vpbroadcastq_avx2",
        cpu=["AVX2"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x59],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="RM", size=64, relaxed=True, dest="EA")])

add_insn("vpbroadcastq", "vpbroadcastq_avx2")

for sz in [128, 256]:
    add_group("vpshiftv_vexw0_avx2",
        cpu=["AVX2"],
        modifiers=["Op2Add"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x00],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDReg", size=sz, dest="VEX"),
                  Operand(type="SIMDRM", size=sz, relaxed=True, dest="EA")])

for sz in [128, 256]:
    add_group("vpshiftv_vexw1_avx2",
        cpu=["AVX2"],
        modifiers=["Op2Add"],
        vex=sz,
        vexw=1,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x00],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDReg", size=sz, dest="VEX"),
                  Operand(type="SIMDRM", size=sz, relaxed=True, dest="EA")])

add_insn("vpsrlvd", "vpshiftv_vexw0_avx2", modifiers=[0x45])
add_insn("vpsrlvq", "vpshiftv_vexw1_avx2", modifiers=[0x45])
add_insn("vpsravd", "vpshiftv_vexw0_avx2", modifiers=[0x46])

add_insn("vpsllvd", "vpshiftv_vexw0_avx2", modifiers=[0x47])
add_insn("vpsllvq", "vpshiftv_vexw1_avx2", modifiers=[0x47])

add_insn("vpmaskmovd", "vmaskmov", modifiers=[0x8C], cpu=["AVX2"])

# vex.vvvv=1111b
for sz in [128, 256]:
    add_group("vmaskmov_vexw1_avx2",
        cpu=["AVX2"],
        modifiers=["Op2Add"],
        vex=sz,
        vexw=1,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x00],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDReg", size=sz, dest="VEX"),
                  Operand(type="SIMDRM", size=sz, relaxed=True, dest="EA")])
 
for sz in [128, 256]:
    add_group("vmaskmov_vexw1_avx2",
        cpu=["AVX2"],
        modifiers=["Op2Add"],
        vex=sz,
        vexw=1,
        prefix=0x66,
        opcode=[0x0F, 0x38, 0x02],
        operands=[Operand(type="SIMDRM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="SIMDReg", size=sz, dest="VEX"),
                  Operand(type="SIMDReg", size=sz, dest="Spare")])
 
add_insn("vpmaskmovq", "vmaskmov_vexw1_avx2", modifiers=[0x8C])

for sz in [128, 256]:
    add_group("vex_66_0F3A_imm8_avx2",
        cpu=["AVX2"],
        modifiers=["Op2Add"],
        vex=sz,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x3A, 0x00],
        operands=[Operand(type="SIMDReg", size=sz, dest="Spare"),
                  Operand(type="SIMDReg", size=sz, dest="VEX"),
                  Operand(type="SIMDRM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("vpblendd", "vex_66_0F3A_imm8_avx2", modifiers=[0x02]) 

# Vector register in EA.
add_group("gather_64x_64x",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=1,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="MemXMMIndex", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEX")])
add_group("gather_64x_64x",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=1,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="MemXMMIndex", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="VEX")])
add_insn("vgatherdpd", "gather_64x_64x", modifiers=[0x92])
add_insn("vpgatherdq", "gather_64x_64x", modifiers=[0x90])

add_group("gather_64x_64y",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=1,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="MemXMMIndex", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEX")])
add_group("gather_64x_64y",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=1,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="MemYMMIndex", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="VEX")])
add_insn("vgatherqpd", "gather_64x_64y", modifiers=[0x93])
add_insn("vpgatherqq", "gather_64x_64y", modifiers=[0x91])

add_group("gather_32x_32y",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=0,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="MemXMMIndex", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEX")])
add_group("gather_32x_32y",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=0,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="MemYMMIndex", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="VEX")])
add_insn("vgatherdps", "gather_32x_32y", modifiers=[0x92])
add_insn("vpgatherdd", "gather_32x_32y", modifiers=[0x90])

add_group("gather_32x_32y_128",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=0,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="MemXMMIndex", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEX")])
add_group("gather_32x_32y_128",
    cpu=["AVX2"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=0,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="MemYMMIndex", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEX")])
add_insn("vgatherqps", "gather_32x_32y_128", modifiers=[0x93])
add_insn("vpgatherqd", "gather_32x_32y_128", modifiers=[0x91])

#####################################################################
# Intel FMA instructions
#####################################################################

### 128/256b FMA PS
add_group("vfma_ps",
    cpu=["FMA"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=0, # single precision
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("vfma_ps",
    cpu=["FMA"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=0, # single precision
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

### 128/256b FMA PD(W=1)
add_group("vfma_pd",
    cpu=["FMA"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=1, # double precision
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("vfma_pd",
    cpu=["FMA"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=1, # double precision
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_group("vfma_ss",
    cpu=["FMA"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=0,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_group("vfma_ss",
    cpu=["FMA"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=0,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])

add_group("vfma_sd",
    cpu=["FMA"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=1,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_group("vfma_sd",
    cpu=["FMA"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=1,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])

for orderval, order in enumerate(["132", "213", "231"]):
    ov = orderval << 4
    for combval, comb in enumerate(["ps", "pd", "ss", "sd"]):
        cv = combval >> 1
        add_insn("vfmadd"+order+comb, "vfma_"+comb, modifiers=[0x98+ov+cv])
        add_insn("vfmsub"+order+comb, "vfma_"+comb, modifiers=[0x9A+ov+cv])
        add_insn("vfnmsub"+order+comb, "vfma_"+comb, modifiers=[0x9E+ov+cv])
        add_insn("vfnmadd"+order+comb, "vfma_"+comb, modifiers=[0x9C+ov+cv])

    # no ss/sd for these
    for comb in ["ps", "pd"]:
        add_insn("vfmaddsub"+order+comb, "vfma_"+comb, modifiers=[0x96+ov])
        add_insn("vfmsubadd"+order+comb, "vfma_"+comb, modifiers=[0x97+ov])

#####################################################################
# Intel AES instructions
#####################################################################

add_group("aes",
    cpu=["AES"],
    modifiers=["Op1Add", "Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x00, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("aes",
    cpu=["AES", "AVX"],
    modifiers=["Op1Add", "Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x00, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])


add_insn("aesenc", "aes", modifiers=[0x38, 0xDC])
add_insn("aesenclast", "aes", modifiers=[0x38, 0xDD])
add_insn("aesdec", "aes", modifiers=[0x38, 0xDE])
add_insn("aesdeclast", "aes", modifiers=[0x38, 0xDF])

add_insn("vaesenc", "aes", modifiers=[0x38, 0xDC, VEXL0], avx=True)
add_insn("vaesenclast", "aes", modifiers=[0x38, 0xDD, VEXL0], avx=True)
add_insn("vaesdec", "aes", modifiers=[0x38, 0xDE, VEXL0], avx=True)
add_insn("vaesdeclast", "aes", modifiers=[0x38, 0xDF, VEXL0], avx=True)

add_group("aesimc",
    cpu=["AES"],
    modifiers=["Op1Add", "Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x00, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])

add_insn("aesimc", "aesimc", modifiers=[0x38, 0xDB])
add_insn("vaesimc", "aesimc", modifiers=[0x38, 0xDB, VEXL0], avx=True)


add_group("aes_imm",
    cpu=["AES"],
    modifiers=["Op1Add", "Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x00, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("aeskeygenassist", "aes_imm", modifiers=[0x3A, 0xDF])
add_insn("vaeskeygenassist", "aes_imm", modifiers=[0x3A, 0xDF, VEXL0],
         avx=True)

#####################################################################
# Intel PCLMULQDQ instruction
#####################################################################

add_group("pclmulqdq",
    cpu=["CLMUL"],
    modifiers=["Op1Add", "Op2Add", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x00, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("pclmulqdq",
    cpu=["CLMUL", "AVX"],
    modifiers=["Op1Add", "Op2Add"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x00, 0x00],
    operands=[Operand(type="SIMDReg", size=128,               dest="Spare"),
              Operand(type="SIMDReg", size=128,               dest="VEX"),
              Operand(type="SIMDRM",  size=128, relaxed=True, dest="EA"),
              Operand(type="Imm",     size=8,   relaxed=True, dest="Imm")])

add_insn("pclmulqdq", "pclmulqdq", modifiers=[0x3A, 0x44])
add_insn("vpclmulqdq", "pclmulqdq", modifiers=[0x3A, 0x44, VEXL0], avx=True)

add_group("pclmulqdq_fixed",
    cpu=["CLMUL"],
    modifiers=["Imm8", "SetVEX"],
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x44],
    operands=[Operand(type="SIMDReg", size=128, dest="SpareVEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("pclmulqdq_fixed",
    cpu=["CLMUL", "AVX"],
    modifiers=["Imm8"],
    vex=128,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x44],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM",  size=128, relaxed=True, dest="EA")])

for comb, combval in zip(["lql","hql","lqh","hqh"], [0x00,0x01,0x10,0x11]):
    add_insn("pclmul"+comb+"qdq", "pclmulqdq_fixed", modifiers=[combval])
    add_insn("vpclmul"+comb+"qdq", "pclmulqdq_fixed",
             modifiers=[combval, VEXL0], avx=True)

#####################################################################
# AVX Post-32nm instructions
#####################################################################

# RDRAND
add_group("rdrand",
    cpu=["RDRAND"],
    opersize=16,
    opcode=[0x0F, 0xC7],
    spare=6,
    operands=[Operand(type="Reg", size=16, dest="EA")])
add_group("rdrand",
    #suffix="l",
    cpu=["RDRAND"],
    opersize=32,
    opcode=[0x0F, 0xC7],
    spare=6,
    operands=[Operand(type="Reg", size=32, dest="EA")])
add_group("rdrand",
    cpu=["RDRAND"],
    opersize=64,
    opcode=[0x0F, 0xC7],
    spare=6,
    operands=[Operand(type="Reg", size=64, dest="EA")])
add_insn("rdrand", "rdrand")

# FSGSBASE instructions
add_group("fs_gs_base",
    only64=True,
    cpu=["FSGSBASE"],
    modifiers=['SpAdd'],
    opersize=32,
    prefix=0xF3,
    opcode=[0x0F, 0xAE],
    operands=[Operand(type="Reg", size=32, dest="EA")])
add_group("fs_gs_base",
    only64=True,
    cpu=["FSGSBASE"],
    opersize=64,
    modifiers=['SpAdd'],
    prefix=0xF3,
    opcode=[0x0F, 0xAE],
    operands=[Operand(type="Reg", size=64, dest="EA")])

add_insn("rdfsbase", "fs_gs_base", modifiers=[0], only64=True)
add_insn("rdgsbase", "fs_gs_base", modifiers=[1], only64=True)
add_insn("wrfsbase", "fs_gs_base", modifiers=[2], only64=True)
add_insn("wrgsbase", "fs_gs_base", modifiers=[3], only64=True)

# Float-16 conversion instructions
for g in ['ps2ph', 'ph2ps']:
    operands1=[]
    operands1.append(Operand(type="SIMDReg", size=128, dest="EA"))
    operands1.append(Operand(type="SIMDReg", size=128, dest="Spare"))

    operands2=[]
    operands2.append(Operand(type="Mem", size=64, dest="EA"))
    operands2.append(Operand(type="SIMDReg", size=128, dest="Spare"))

    operands3=[]
    operands3.append(Operand(type="SIMDReg", size=128, dest="EA"))
    operands3.append(Operand(type="SIMDReg", size=256, dest="Spare"))

    operands4=[]
    operands4.append(Operand(type="Mem", size=128, dest="EA"))
    operands4.append(Operand(type="SIMDReg", size=256, dest="Spare"))

    if g == 'ph2ps':
        operands1.reverse()
        operands2.reverse()
        operands3.reverse()
        operands4.reverse()
        map = 0x38
    elif g == 'ps2ph':
        immop = Operand(type="Imm", size=8, relaxed=True, dest="Imm")
        operands1.append(immop)
        operands2.append(immop)
        operands3.append(immop)
        operands4.append(immop)
        map = 0x3A

    add_group("avx_cvt" + g,
        cpu=["F16C", "AVX"],
        modifiers=["PreAdd", "Op2Add"],
        vex=128,
        prefix=0x00,
        opcode=[0x0F, map, 0x00],
        operands=operands1)

    add_group("avx_cvt" + g,
        cpu=["F16C", "AVX"],
        modifiers=["PreAdd", "Op2Add"],
        vex=128,
        prefix=0x00,
        opcode=[0x0F, map, 0x00],
        operands=operands2)

    add_group("avx_cvt" + g,
        cpu=["F16C", "AVX"],
        modifiers=["PreAdd", "Op2Add"],
        vex=256,
        prefix=0x00,
        opcode=[0x0F, map, 0x00],
        operands=operands3)

    add_group("avx_cvt" + g,
        cpu=["F16C", "AVX"],
        modifiers=["PreAdd", "Op2Add"],
        vex=256,
        prefix=0x00,
        opcode=[0x0F, map, 0x00],
        operands=operands4)

add_insn("vcvtps2ph", "avx_cvtps2ph", modifiers=[0x66, 0x1D], avx=True)
add_insn("vcvtph2ps", "avx_cvtph2ps", modifiers=[0x66, 0x13], avx=True)

#####################################################################
# AMD SSE4a instructions
#####################################################################

add_group("extrq",
    cpu=["SSE4a"],
    prefix=0x66,
    opcode=[0x0F, 0x78],
    operands=[Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("extrq",
    cpu=["SSE4a"],
    prefix=0x66,
    opcode=[0x0F, 0x79],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_insn("extrq", "extrq")

add_group("insertq",
    cpu=["SSE4a"],
    prefix=0xF2,
    opcode=[0x0F, 0x78],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
add_group("insertq",
    cpu=["SSE4a"],
    prefix=0xF2,
    opcode=[0x0F, 0x79],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])

add_insn("insertq", "insertq")

add_group("movntsd",
    cpu=["SSE4a"],
    prefix=0xF2,
    opcode=[0x0F, 0x2B],
    operands=[Operand(type="Mem", size=64, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])

add_insn("movntsd", "movntsd")

add_group("movntss",
    cpu=["SSE4a"],
    prefix=0xF3,
    opcode=[0x0F, 0x2B],
    operands=[Operand(type="Mem", size=32, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="Spare")])

add_insn("movntss", "movntss")

#####################################################################
# AMD XOP instructions
#####################################################################

add_group("vfrc_pdps",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=128,
    opcode=[0x09, 0x80],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("vfrc_pdps",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=256,
    opcode=[0x09, 0x80],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])
add_insn("vfrczpd", "vfrc_pdps", modifiers=[0x01])
add_insn("vfrczps", "vfrc_pdps", modifiers=[0x00])

add_group("vfrczsd",
    cpu=["XOP"],
    xop=128,
    opcode=[0x09, 0x83],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("vfrczsd",
    cpu=["XOP"],
    xop=128,
    opcode=[0x09, 0x83],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])
add_insn("vfrczsd", "vfrczsd")

add_group("vfrczss",
    cpu=["XOP"],
    xop=128,
    opcode=[0x09, 0x82],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="EA")])
add_group("vfrczss",
    cpu=["XOP"],
    xop=128,
    opcode=[0x09, 0x82],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])
add_insn("vfrczss", "vfrczss")

add_group("vpcmov",
    cpu=["XOP"],
    xop=128,
    xopw=0,
    opcode=[0x08, 0xA2],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEXImmSrc")])
add_group("vpcmov",
    cpu=["XOP"],
    xop=128,
    xopw=1,
    opcode=[0x08, 0xA2],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="VEXImmSrc"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("vpcmov",
    cpu=["XOP"],
    xop=256,
    xopw=0,
    opcode=[0x08, 0xA2],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="VEXImmSrc")])
add_group("vpcmov",
    cpu=["XOP"],
    xop=256,
    xopw=1,
    opcode=[0x08, 0xA2],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDReg", size=256, dest="VEXImmSrc"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_insn("vpcmov", "vpcmov")

add_group("vpcom",
    cpu=["XOP"],
    modifiers=["Op1Add", "Imm8"],
    xop=128,
    opcode=[0x08, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("vpcom_imm",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=128,
    opcode=[0x08, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

for opc, sfx in [(0xCC, "b"),
                 (0xCE, "d"),
                 (0xCD, "w"),
                 (0xCF, "q"),
                 (0xEC, "ub"),
                 (0xEE, "ud"),
                 (0xEF, "uq"),
                 (0xED, "uw")]:
    add_insn("vpcom"+sfx, "vpcom_imm", modifiers=[opc])
    for ib, cc in enumerate(["lt", "le", "gt", "ge",
                             "eq", "neq", "false", "true"]):
        add_insn("vpcom"+cc+sfx, "vpcom", modifiers=[opc, ib])
    # ne alias for neq
    add_insn("vpcomne"+sfx, "vpcom", modifiers=[opc, 5])

add_group("vphaddsub",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=128,
    opcode=[0x09, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])

add_insn("vphaddbw", "vphaddsub", modifiers=[0xC1])
add_insn("vphaddbd", "vphaddsub", modifiers=[0xC2])
add_insn("vphaddbq", "vphaddsub", modifiers=[0xC3])
add_insn("vphaddwd", "vphaddsub", modifiers=[0xC6])
add_insn("vphaddwq", "vphaddsub", modifiers=[0xC7])
add_insn("vphadddq", "vphaddsub", modifiers=[0xCB])

add_insn("vphaddubw", "vphaddsub", modifiers=[0xD1])
add_insn("vphaddubd", "vphaddsub", modifiers=[0xD2])
add_insn("vphaddubq", "vphaddsub", modifiers=[0xD3])
add_insn("vphadduwd", "vphaddsub", modifiers=[0xD6])
add_insn("vphadduwq", "vphaddsub", modifiers=[0xD7])
add_insn("vphaddudq", "vphaddsub", modifiers=[0xD8])

add_insn("vphsubbw", "vphaddsub", modifiers=[0xE1])
add_insn("vphsubwd", "vphaddsub", modifiers=[0xE2])
add_insn("vphsubdq", "vphaddsub", modifiers=[0xE3])

add_group("vpma",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=128,
    opcode=[0x08, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEXImmSrc")])

add_insn("vpmacsdd", "vpma", modifiers=[0x9E])
add_insn("vpmacsdqh", "vpma", modifiers=[0x9F])
add_insn("vpmacsdql", "vpma", modifiers=[0x97])
add_insn("vpmacssdd", "vpma", modifiers=[0x8E])
add_insn("vpmacssdqh", "vpma", modifiers=[0x8F])
add_insn("vpmacssdql", "vpma", modifiers=[0x87])
add_insn("vpmacsswd", "vpma", modifiers=[0x86])
add_insn("vpmacssww", "vpma", modifiers=[0x85])
add_insn("vpmacswd", "vpma", modifiers=[0x96])
add_insn("vpmacsww", "vpma", modifiers=[0x95])
add_insn("vpmadcsswd", "vpma", modifiers=[0xA6])
add_insn("vpmadcswd", "vpma", modifiers=[0xB6])

add_group("vpperm",
    cpu=["XOP"],
    xop=128,
    xopw=0,
    opcode=[0x08, 0xA3],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEXImmSrc")])
add_group("vpperm",
    cpu=["XOP"],
    xop=128,
    xopw=1,
    opcode=[0x08, 0xA3],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="VEXImmSrc"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_insn("vpperm", "vpperm")

add_group("vprot",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=128,
    xopw=0,
    opcode=[0x09, 0x90],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEX")])
add_group("vprot",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=128,
    xopw=1,
    opcode=[0x09, 0x90],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("vprot",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=128,
    opcode=[0x08, 0xC0],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="Imm", size=8, relaxed=True, dest="Imm")])
for opc, sfx in enumerate(["b", "w", "d", "q"]):
    add_insn("vprot"+sfx, "vprot", modifiers=[opc])

add_group("amd_vpshift",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=128,
    xopw=0,
    opcode=[0x09, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEX")])
add_group("amd_vpshift",
    cpu=["XOP"],
    modifiers=["Op1Add"],
    xop=128,
    xopw=1,
    opcode=[0x09, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
for opc, sfx in enumerate(["b", "w", "d", "q"]):
    add_insn("vpsha"+sfx, "amd_vpshift", modifiers=[0x98+opc])
    add_insn("vpshl"+sfx, "amd_vpshift", modifiers=[0x94+opc])

#####################################################################
# AMD FMA4 instructions (same as original Intel FMA instructions)
#####################################################################

add_group("fma_128_256",
    cpu=["FMA4"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=0,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=128, dest="VEXImmSrc")])
add_group("fma_128_256",
    cpu=["FMA4"],
    modifiers=["Op2Add"],
    vex=128,
    vexw=1,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
              Operand(type="SIMDReg", size=128, dest="VEX"),
              Operand(type="SIMDReg", size=128, dest="VEXImmSrc"),
              Operand(type="SIMDRM", size=128, relaxed=True, dest="EA")])
add_group("fma_128_256",
    cpu=["FMA4"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=0,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA"),
              Operand(type="SIMDReg", size=256, dest="VEXImmSrc")])
add_group("fma_128_256",
    cpu=["FMA4"],
    modifiers=["Op2Add"],
    vex=256,
    vexw=1,
    prefix=0x66,
    opcode=[0x0F, 0x3A, 0x00],
    operands=[Operand(type="SIMDReg", size=256, dest="Spare"),
              Operand(type="SIMDReg", size=256, dest="VEX"),
              Operand(type="SIMDReg", size=256, dest="VEXImmSrc"),
              Operand(type="SIMDRM", size=256, relaxed=True, dest="EA")])

add_insn("vfmaddpd", "fma_128_256", modifiers=[0x69])
add_insn("vfmaddps", "fma_128_256", modifiers=[0x68])
add_insn("vfmaddsubpd", "fma_128_256", modifiers=[0x5D])
add_insn("vfmaddsubps", "fma_128_256", modifiers=[0x5C])
add_insn("vfmsubaddpd", "fma_128_256", modifiers=[0x5F])
add_insn("vfmsubaddps", "fma_128_256", modifiers=[0x5E])
add_insn("vfmsubpd", "fma_128_256", modifiers=[0x6D])
add_insn("vfmsubps", "fma_128_256", modifiers=[0x6C])
add_insn("vfnmaddpd", "fma_128_256", modifiers=[0x79])
add_insn("vfnmaddps", "fma_128_256", modifiers=[0x78])
add_insn("vfnmsubpd", "fma_128_256", modifiers=[0x7D])
add_insn("vfnmsubps", "fma_128_256", modifiers=[0x7C])

for sz in [32, 64]:
    add_group("fma_128_m%d" % sz,
        cpu=["FMA4"],
        modifiers=["Op2Add"],
        vex=128,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x3A, 0x00],
        operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
                  Operand(type="SIMDReg", size=128, dest="VEX"),
                  Operand(type="SIMDReg", size=128, dest="EA"),
                  Operand(type="SIMDReg", size=128, dest="VEXImmSrc")])
    add_group("fma_128_m%d" % sz,
        cpu=["FMA4"],
        modifiers=["Op2Add"],
        vex=128,
        vexw=0,
        prefix=0x66,
        opcode=[0x0F, 0x3A, 0x00],
        operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
                  Operand(type="SIMDReg", size=128, dest="VEX"),
                  Operand(type="Mem", size=sz, relaxed=True, dest="EA"),
                  Operand(type="SIMDReg", size=128, dest="VEXImmSrc")])
    add_group("fma_128_m%d" % sz,
        cpu=["FMA4"],
        modifiers=["Op2Add"],
        vex=128,
        vexw=1,
        prefix=0x66,
        opcode=[0x0F, 0x3A, 0x00],
        operands=[Operand(type="SIMDReg", size=128, dest="Spare"),
                  Operand(type="SIMDReg", size=128, dest="VEX"),
                  Operand(type="SIMDReg", size=128, dest="VEXImmSrc"),
                  Operand(type="Mem", size=sz, relaxed=True, dest="EA")])

add_insn("vfmaddsd", "fma_128_m64", modifiers=[0x6B])
add_insn("vfmaddss", "fma_128_m32", modifiers=[0x6A])
add_insn("vfmsubsd", "fma_128_m64", modifiers=[0x6F])
add_insn("vfmsubss", "fma_128_m32", modifiers=[0x6E])
add_insn("vfnmaddsd", "fma_128_m64", modifiers=[0x7B])
add_insn("vfnmaddss", "fma_128_m32", modifiers=[0x7A])
add_insn("vfnmsubsd", "fma_128_m64", modifiers=[0x7F])
add_insn("vfnmsubss", "fma_128_m32", modifiers=[0x7E])

#####################################################################
# Intel XSAVE and XSAVEOPT instructions
#####################################################################
add_insn("xgetbv", "threebyte", modifiers=[0x0F, 0x01, 0xD0],
         cpu=["XSAVE", "386"])
add_insn("xsetbv", "threebyte", modifiers=[0x0F, 0x01, 0xD1],
         cpu=["XSAVE", "386", "Priv"])
add_insn("xsave", "twobytemem", modifiers=[4, 0x0F, 0xAE],
         cpu=["XSAVE", "386"])
add_insn("xrstor", "twobytemem", modifiers=[5, 0x0F, 0xAE],
         cpu=["XSAVE", "386"])

add_insn("xsaveopt", "twobytemem", modifiers=[6, 0x0F, 0xAE],
         cpu=["XSAVEOPT"])

add_group("xsaveopt64",
    modifiers=["SpAdd", "Op0Add", "Op1Add"],
    opcode=[0x00, 0x00],
    spare=0,
    opersize=64,
    operands=[Operand(type="Mem", relaxed=True, dest="EA")])
add_insn("xsaveopt64", "xsaveopt64", modifiers=[6, 0x0F, 0xAE],
         cpu=["XSAVEOPT"], only64=True)

#####################################################################
# Intel MOVBE instruction
#####################################################################
for sz in (16, 32, 64):
    add_group("movbe",
        cpu=["MOVBE"],
        opersize=sz,
        opcode=[0x0F, 0x38, 0xF0],
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="Mem", size=sz, relaxed=True, dest="EA")])
    add_group("movbe",
        cpu=["MOVBE"],
        opersize=sz,
        opcode=[0x0F, 0x38, 0xF1],
        operands=[Operand(type="Mem", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="Spare")])
add_insn("movbe", "movbe")

#####################################################################
# Intel advanced bit manipulations (BMI1/2)
#####################################################################

add_insn("tzcnt", "cnt", modifiers=[0xBC], cpu=["BMI1"])
# LZCNT is present as AMD ext

for sfx, sz in zip("wlq", [32, 64]):
    add_group("vex_gpr_ndd_rm_0F38_regext",
        suffix=sfx,
        modifiers=["PreAdd", "Op2Add", "SpAdd" ],
        opersize=sz,
        prefix=0x00,
        opcode=[0x0F, 0x38, 0x00],
        vex=0, ## VEX.L=0
        operands=[Operand(type="Reg", size=sz, dest="VEX"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA")])


add_insn("blsr",   "vex_gpr_ndd_rm_0F38_regext", modifiers=[0x00, 0xF3, 1],
         cpu=["BMI1"])
add_insn("blsmsk", "vex_gpr_ndd_rm_0F38_regext", modifiers=[0x00, 0xF3, 2],
         cpu=["BMI1"])
add_insn("blsi",   "vex_gpr_ndd_rm_0F38_regext", modifiers=[0x00, 0xF3, 3],
         cpu=["BMI1"])

for sfx, sz in zip("wlq", [32, 64]):
    add_group("vex_gpr_reg_rm_0F_imm8",
        suffix=sfx,
        modifiers=["PreAdd", "Op1Add", "Op2Add"],
        opersize=sz,
        prefix=0x00,
        opcode=[0x0F, 0x00, 0x00],
        vex=0, ## VEX.L=0
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Imm", size=8, relaxed=True, dest="Imm")])

add_insn("rorx", "vex_gpr_reg_rm_0F_imm8", modifiers=[0xF2, 0x3A, 0xF0],
         cpu=["BMI2"])

for sfx, sz in zip("lq", [32, 64]):  # no 16-bit forms
    add_group("vex_gpr_reg_nds_rm_0F",
        suffix=sfx,
        modifiers=["PreAdd", "Op1Add", "Op2Add"],
        opersize=sz,
        prefix=0x00,
        opcode=[0x0F, 0x00, 0x00],
        vex=0,
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="Reg", size=sz, dest="VEX"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA")])

add_insn("andn", "vex_gpr_reg_nds_rm_0F", modifiers=[0x00, 0x38, 0xF2],
         cpu=["BMI1"])

add_insn("pdep", "vex_gpr_reg_nds_rm_0F", modifiers=[0xF2, 0x38, 0xF5],
         cpu=["BMI2"])
add_insn("pext", "vex_gpr_reg_nds_rm_0F", modifiers=[0xF3, 0x38, 0xF5],
         cpu=["BMI2"])

for sfx, sz in zip("lq", [32, 64]):  # no 16-bit forms
    add_group("vex_gpr_reg_rm_nds_0F",
        suffix=sfx,
        modifiers=["PreAdd", "Op1Add", "Op2Add"],
        opersize=sz,
        prefix=0x00,
        opcode=[0x0F, 0x00, 0x00],
        vex=0,
        operands=[Operand(type="Reg", size=sz, dest="Spare"),
                  Operand(type="RM", size=sz, relaxed=True, dest="EA"),
                  Operand(type="Reg", size=sz, dest="VEX")])

add_insn("bzhi", "vex_gpr_reg_rm_nds_0F", modifiers=[0x00, 0x38, 0xF5],
         cpu=["BMI2"])
add_insn("bextr","vex_gpr_reg_rm_nds_0F", modifiers=[0x00, 0x38, 0xF7],
         cpu=["BMI1"])
add_insn("shlx", "vex_gpr_reg_rm_nds_0F", modifiers=[0x66, 0x38, 0xF7],
         cpu=["BMI2"])
add_insn("shrx", "vex_gpr_reg_rm_nds_0F", modifiers=[0xF2, 0x38, 0xF7],
         cpu=["BMI2"])
add_insn("sarx", "vex_gpr_reg_rm_nds_0F", modifiers=[0xF3, 0x38, 0xF7],
         cpu=["BMI2"])

add_insn("mulx", "vex_gpr_reg_nds_rm_0F", modifiers=[0xF2, 0x38, 0xF6],
         cpu=["BMI2"])



#####################################################################
# Intel INVPCID instruction
#####################################################################
add_group("invpcid",
    cpu=["INVPCID", "Priv"],
    not64=True,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x82],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="Mem", size=128, relaxed=True, dest="EA")])
add_group("invpcid",
    cpu=["INVPCID", "Priv"],
    only64=True,
    def_opersize_64=64,
    prefix=0x66,
    opcode=[0x0F, 0x38, 0x82],
    operands=[Operand(type="Reg", size=64, dest="Spare"),
              Operand(type="Mem", size=128, relaxed=True, dest="EA")])
add_insn("invpcid", "invpcid")

#####################################################################
# AMD 3DNow! instructions
#####################################################################

add_insn("prefetch", "twobytemem", modifiers=[0x00, 0x0F, 0x0D], cpu=["3DNow"])
add_insn("prefetchw", "twobytemem", modifiers=[0x01, 0x0F, 0x0D], cpu=["3DNow"])
add_insn("femms", "twobyte", modifiers=[0x0F, 0x0E], cpu=["3DNow"])

add_group("now3d",
    cpu=["3DNow"],
    modifiers=["Imm8"],
    opcode=[0x0F, 0x0F],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])

add_insn("pavgusb", "now3d", modifiers=[0xBF])
add_insn("pf2id", "now3d", modifiers=[0x1D])
add_insn("pf2iw", "now3d", modifiers=[0x1C], cpu=["Athlon", "3DNow"])
add_insn("pfacc", "now3d", modifiers=[0xAE])
add_insn("pfadd", "now3d", modifiers=[0x9E])
add_insn("pfcmpeq", "now3d", modifiers=[0xB0])
add_insn("pfcmpge", "now3d", modifiers=[0x90])
add_insn("pfcmpgt", "now3d", modifiers=[0xA0])
add_insn("pfmax", "now3d", modifiers=[0xA4])
add_insn("pfmin", "now3d", modifiers=[0x94])
add_insn("pfmul", "now3d", modifiers=[0xB4])
add_insn("pfnacc", "now3d", modifiers=[0x8A], cpu=["Athlon", "3DNow"])
add_insn("pfpnacc", "now3d", modifiers=[0x8E], cpu=["Athlon", "3DNow"])
add_insn("pfrcp", "now3d", modifiers=[0x96])
add_insn("pfrcpit1", "now3d", modifiers=[0xA6])
add_insn("pfrcpit2", "now3d", modifiers=[0xB6])
add_insn("pfrsqit1", "now3d", modifiers=[0xA7])
add_insn("pfrsqrt", "now3d", modifiers=[0x97])
add_insn("pfsub", "now3d", modifiers=[0x9A])
add_insn("pfsubr", "now3d", modifiers=[0xAA])
add_insn("pi2fd", "now3d", modifiers=[0x0D])
add_insn("pi2fw", "now3d", modifiers=[0x0C], cpu=["Athlon", "3DNow"])
add_insn("pmulhrwa", "now3d", modifiers=[0xB7])
add_insn("pswapd", "now3d", modifiers=[0xBB], cpu=["Athlon", "3DNow"])

#####################################################################
# AMD extensions
#####################################################################

add_insn("syscall", "twobyte", modifiers=[0x0F, 0x05], cpu=["686", "AMD"])
for sfx in [None, "l", "q"]:
    add_insn("sysret"+(sfx or ""), "twobyte", suffix=sfx, modifiers=[0x0F, 0x07],
             cpu=["686", "AMD", "Priv"])
add_insn("lzcnt", "cnt", modifiers=[0xBD], cpu=["LZCNT"])

#####################################################################
# AMD x86-64 extensions
#####################################################################

add_insn("swapgs", "threebyte", modifiers=[0x0F, 0x01, 0xF8], only64=True)
add_insn("rdtscp", "threebyte", modifiers=[0x0F, 0x01, 0xF9],
         cpu=["686", "AMD", "Priv"])

add_group("cmpxchg16b",
    only64=True,
    opersize=64,
    opcode=[0x0F, 0xC7],
    spare=1,
    operands=[Operand(type="Mem", size=128, relaxed=True, dest="EA")])

add_insn("cmpxchg16b", "cmpxchg16b")

#####################################################################
# AMD Pacifica SVM instructions
#####################################################################

add_insn("clgi", "threebyte", modifiers=[0x0F, 0x01, 0xDD], cpu=["SVM"])
add_insn("stgi", "threebyte", modifiers=[0x0F, 0x01, 0xDC], cpu=["SVM"])
add_insn("vmmcall", "threebyte", modifiers=[0x0F, 0x01, 0xD9], cpu=["SVM"])

add_group("invlpga",
    cpu=["SVM"],
    opcode=[0x0F, 0x01, 0xDF],
    operands=[])
add_group("invlpga",
    cpu=["SVM"],
    opcode=[0x0F, 0x01, 0xDF],
    operands=[Operand(type="MemrAX", dest="AdSizeEA"),
              Operand(type="Creg", size=32, dest=None)])

add_insn("invlpga", "invlpga")

add_group("skinit",
    cpu=["SVM"],
    opcode=[0x0F, 0x01, 0xDE],
    operands=[])
add_group("skinit",
    cpu=["SVM"],
    opcode=[0x0F, 0x01, 0xDE],
    operands=[Operand(type="MemEAX", dest=None)])

add_insn("skinit", "skinit")

add_group("svm_rax",
    cpu=["SVM"],
    modifiers=["Op2Add"],
    opcode=[0x0F, 0x01, 0x00],
    operands=[])
add_group("svm_rax",
    cpu=["SVM"],
    modifiers=["Op2Add"],
    opcode=[0x0F, 0x01, 0x00],
    operands=[Operand(type="MemrAX", dest="AdSizeEA")])

add_insn("vmload", "svm_rax", modifiers=[0xDA])
add_insn("vmrun", "svm_rax", modifiers=[0xD8])
add_insn("vmsave", "svm_rax", modifiers=[0xDB])

#####################################################################
# VIA PadLock instructions
#####################################################################

add_group("padlock",
    cpu=["PadLock"],
    modifiers=["Imm8", "PreAdd", "Op1Add"],
    prefix=0x00,
    opcode=[0x0F, 0x00],
    operands=[])

add_insn("xstore", "padlock", modifiers=[0xC0, 0x00, 0xA7])
add_insn("xstorerng", "padlock", modifiers=[0xC0, 0x00, 0xA7])
add_insn("xcryptecb", "padlock", modifiers=[0xC8, 0xF3, 0xA7])
add_insn("xcryptcbc", "padlock", modifiers=[0xD0, 0xF3, 0xA7])
add_insn("xcryptctr", "padlock", modifiers=[0xD8, 0xF3, 0xA7])
add_insn("xcryptcfb", "padlock", modifiers=[0xE0, 0xF3, 0xA7])
add_insn("xcryptofb", "padlock", modifiers=[0xE8, 0xF3, 0xA7])
add_insn("montmul", "padlock", modifiers=[0xC0, 0xF3, 0xA6])
add_insn("xsha1", "padlock", modifiers=[0xC8, 0xF3, 0xA6])
add_insn("xsha256", "padlock", modifiers=[0xD0, 0xF3, 0xA6])

#####################################################################
# Cyrix MMX instructions
#####################################################################

add_group("cyrixmmx",
    cpu=["MMX", "Cyrix"],
    modifiers=["Op1Add"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="SIMDRM", size=64, relaxed=True, dest="EA")])

add_insn("paddsiw", "cyrixmmx", modifiers=[0x51])
add_insn("paveb", "cyrixmmx", modifiers=[0x50])
add_insn("pdistib", "cyrixmmx", modifiers=[0x54])
add_insn("pmagw", "cyrixmmx", modifiers=[0x52])
add_insn("pmulhriw", "cyrixmmx", modifiers=[0x5D])
add_insn("pmulhrwc", "cyrixmmx", modifiers=[0x59])
add_insn("pmvgezb", "cyrixmmx", modifiers=[0x5C])
add_insn("pmvlzb", "cyrixmmx", modifiers=[0x5B])
add_insn("pmvnzb", "cyrixmmx", modifiers=[0x5A])
add_insn("pmvzb", "cyrixmmx", modifiers=[0x58])
add_insn("psubsiw", "cyrixmmx", modifiers=[0x55])

add_group("pmachriw",
    cpu=["MMX", "Cyrix"],
    opcode=[0x0F, 0x5E],
    operands=[Operand(type="SIMDReg", size=64, dest="Spare"),
              Operand(type="Mem", size=64, relaxed=True, dest="EA")])

add_insn("pmachriw", "pmachriw")

#####################################################################
# Cyrix extensions
#####################################################################

add_insn("smint", "twobyte", modifiers=[0x0F, 0x38], cpu=["686", "Cyrix"])
add_insn("smintold", "twobyte", modifiers=[0x0F, 0x7E], cpu=["486", "Cyrix", "Obs"])

add_group("rdwrshr",
    cpu=["Cyrix", "SMM", "686"],
    modifiers=["Op1Add"],
    opcode=[0x0F, 0x36],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA")])

add_insn("rdshr", "rdwrshr", modifiers=[0x00])
add_insn("wrshr", "rdwrshr", modifiers=[0x01])

add_group("rsdc",
    cpu=["Cyrix", "SMM", "486"],
    opcode=[0x0F, 0x79],
    operands=[Operand(type="SegReg", size=16, relaxed=True, dest="Spare"),
              Operand(type="Mem", size=80, relaxed=True, dest="EA")])

add_insn("rsdc", "rsdc")

add_group("cyrixsmm",
    cpu=["Cyrix", "SMM", "486"],
    modifiers=["Op1Add"],
    opcode=[0x0F, 0x00],
    operands=[Operand(type="Mem", size=80, relaxed=True, dest="EA")])

add_insn("rsldt", "cyrixsmm", modifiers=[0x7B])
add_insn("rsts", "cyrixsmm", modifiers=[0x7D])
add_insn("svldt", "cyrixsmm", modifiers=[0x7A])
add_insn("svts", "cyrixsmm", modifiers=[0x7C])

add_group("svdc",
    cpu=["Cyrix", "SMM", "486"],
    opcode=[0x0F, 0x78],
    operands=[Operand(type="Mem", size=80, relaxed=True, dest="EA"),
              Operand(type="SegReg", size=16, relaxed=True, dest="Spare")])

add_insn("svdc", "svdc")

#####################################################################
# Obsolete/undocumented instructions
#####################################################################

add_insn("fsetpm", "twobyte", modifiers=[0xDB, 0xE4], cpu=["286", "FPU", "Obs"])
add_insn("loadall", "twobyte", modifiers=[0x0F, 0x07], cpu=["386", "Undoc"])
add_insn("loadall286", "twobyte", modifiers=[0x0F, 0x05], cpu=["286", "Undoc"])
add_insn("salc", "onebyte", modifiers=[0xD6], cpu=["Undoc"], not64=True)
add_insn("smi", "onebyte", modifiers=[0xF1], cpu=["386", "Undoc"])

add_group("ibts",
    cpu=["Undoc", "Obs", "386"],
    opersize=16,
    opcode=[0x0F, 0xA7],
    operands=[Operand(type="RM", size=16, relaxed=True, dest="EA"),
              Operand(type="Reg", size=16, dest="Spare")])
add_group("ibts",
    cpu=["Undoc", "Obs", "386"],
    opersize=32,
    opcode=[0x0F, 0xA7],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="Reg", size=32, dest="Spare")])

add_insn("ibts", "ibts")

add_group("umov",
    cpu=["Undoc", "386"],
    opcode=[0x0F, 0x10],
    operands=[Operand(type="RM", size=8, relaxed=True, dest="EA"),
              Operand(type="Reg", size=8, dest="Spare")])
add_group("umov",
    cpu=["Undoc", "386"],
    opersize=16,
    opcode=[0x0F, 0x11],
    operands=[Operand(type="RM", size=16, relaxed=True, dest="EA"),
              Operand(type="Reg", size=16, dest="Spare")])
add_group("umov",
    cpu=["Undoc", "386"],
    opersize=32,
    opcode=[0x0F, 0x11],
    operands=[Operand(type="RM", size=32, relaxed=True, dest="EA"),
              Operand(type="Reg", size=32, dest="Spare")])
add_group("umov",
    cpu=["Undoc", "386"],
    opcode=[0x0F, 0x12],
    operands=[Operand(type="Reg", size=8, dest="Spare"),
              Operand(type="RM", size=8, relaxed=True, dest="EA")])
add_group("umov",
    cpu=["Undoc", "386"],
    opersize=16,
    opcode=[0x0F, 0x13],
    operands=[Operand(type="Reg", size=16, dest="Spare"),
              Operand(type="RM", size=16, relaxed=True, dest="EA")])
add_group("umov",
    cpu=["Undoc", "386"],
    opersize=32,
    opcode=[0x0F, 0x13],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="RM", size=32, relaxed=True, dest="EA")])

add_insn("umov", "umov")

add_group("xbts",
    cpu=["Undoc", "Obs", "386"],
    opersize=16,
    opcode=[0x0F, 0xA6],
    operands=[Operand(type="Reg", size=16, dest="Spare"),
              Operand(type="Mem", size=16, relaxed=True, dest="EA")])
add_group("xbts",
    cpu=["Undoc", "Obs", "386"],
    opersize=32,
    opcode=[0x0F, 0xA6],
    operands=[Operand(type="Reg", size=32, dest="Spare"),
              Operand(type="Mem", size=32, relaxed=True, dest="EA")])

add_insn("xbts", "xbts")

finalize_insns()

#####################################################################
# Prefixes
#####################################################################
# operand size overrides
for sz in [16, 32, 64]:
    add_prefix("o%d" % sz, "OPERSIZE", sz, parser="nasm", only64=(sz==64))
    add_prefix("data%d" % sz, "OPERSIZE", sz, parser="gas", only64=(sz==64))
add_prefix("word",      "OPERSIZE", 16, parser="gas")
add_prefix("dword",     "OPERSIZE", 32, parser="gas")
add_prefix("qword",     "OPERSIZE", 64, parser="gas", only64=True)

# address size overrides
for sz in [16, 32, 64]:
    add_prefix("a%d" % sz, "ADDRSIZE", sz, parser="nasm", only64=(sz==64))
    add_prefix("addr%d" % sz, "ADDRSIZE", sz, parser="gas", only64=(sz==64))
add_prefix("aword",     "ADDRSIZE", 16, parser="gas")
add_prefix("adword",    "ADDRSIZE", 32, parser="gas")
add_prefix("aqword",    "ADDRSIZE", 64, parser="gas", only64=True)

# instruction prefixes
add_prefix("lock",      "LOCKREP",  0xF0)
add_prefix("repne",     "LOCKREP",  0xF2)
add_prefix("repnz",     "LOCKREP",  0xF2)
add_prefix("rep",       "LOCKREP",  0xF3)
add_prefix("repe",      "LOCKREP",  0xF3)
add_prefix("repz",      "LOCKREP",  0xF3)

# other prefixes, limited to GAS-only at the moment
# Hint taken/not taken for jumps
add_prefix("ht",        "SEGREG",   0x3E, parser="gas")
add_prefix("hnt",       "SEGREG",   0x2E, parser="gas")

# REX byte explicit prefixes
for val, suf in enumerate(["", "z", "y", "yz", "x", "xz", "xy", "xyz"]):
    add_prefix("rex" + suf, "REX", 0x40+val, parser="gas", only64=True)
    add_prefix("rex64" + suf, "REX", 0x48+val, parser="gas", only64=True)

#####################################################################
# Output generation
#####################################################################

out_dir = ""
if len(sys.argv) > 1:
  out_dir = sys.argv[1]

output_groups(file(os.path.join(out_dir, "x86insns.c"), "wt"))
output_gas_insns(file(os.path.join(out_dir, "x86insn_gas.gperf"), "wt"))
output_nasm_insns(file(os.path.join(out_dir, "x86insn_nasm.gperf"), "wt"))


#!/usr/bin/env python3
#
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This uses nm to identify all SkRasterPipeline symbols in the binary
# (using stage names from SkRasterPipelineOpList.h), then objdump to
# extract the (cleaned) assembly, and llvm-mca to analyse the steady-state
# runtime of those functions, creating or appending to a .tsv spreadsheet.

import argparse
import subprocess
import re
import os
import csv
import sys
import multiprocessing
from typing import Any, Dict, List, Optional, Set, Tuple

def get_stage_names(skia_root: str) -> List[str]:
    """Parses SkRasterPipelineOpList.h for all op names."""
    header_path = os.path.join(skia_root, "src/core/SkRasterPipelineOpList.h")
    if not os.path.exists(header_path):
        print(f"Error: Could not find {header_path}")
        sys.exit(1)

    ops: Set[str] = set()
    with open(header_path, 'r') as f:
        for line in f:
            # Skip macro definitions and enum declarations that might contain "M(op)"
            if "#define M(op)" in line or "enum class" in line or "static constexpr" in line:
                continue
            matches = re.findall(r'M\((\w+)\)', line)
            for m in matches:
                if m != "op":
                    ops.add(m)

    return sorted(list(ops))

# Known Raster Pipeline namespaces to avoid capturing unrelated symbols with similar names (e.g. 'clear')
RP_NAMESPACES = {
    'ml3', 'ml4', 'avx', 'avx2', 'sse2', 'sse41', 'sse3', 'ssse3', 'neon', 'scalar', 'lsx', 'lasx',
}

def find_symbols_in_binary(bin_path: str, ops: List[str]) -> Dict[str, Dict[str, Any]]:
    """Uses nm -S -C to find symbols matching the ops efficiently in one pass."""
    # -S: Print size, -C: Demangle, --defined-only: Only symbols in the binary
    cmd = ["nm", "-S", "-C", "--defined-only", bin_path]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running nm: {e.stderr}")
        sys.exit(1)
    except Exception as e:
        print(f"Error starting nm: {e}")
        sys.exit(1)

    found_symbols = {}
    ops_set = set(ops)
    # Example output from nm:
    # 00000000004b1f80 000000000000027a t ml3::gray_to_RGB1(unsigned int*, unsigned char const*, int)
    # 00000000004b0e90 0000000000000352 t ml3::RGBA_to_bgrA(unsigned int*, unsigned int const*, int)
    # 00000000004b11f0 000000000000026b t ml3::RGBA_to_BGRA(unsigned int*, unsigned int const*, int)
    # 00000000004b0b30 0000000000000352 t ml3::RGBA_to_rgbA(unsigned int*, unsigned int const*, int)
    # 00000000004b1770 00000000000001ad t ml3::grayA_to_rgbA(unsigned int*, unsigned char const*, int)
    # 00000000004b1460 0000000000000302 t ml3::grayA_to_RGBA(unsigned int*, unsigned char const*, int)
    # 0000000000886790 000000000000018e t ml3::blit_row_color32(unsigned int*, int, unsigned int)
    #
    # Regex to match: [Address] [Size] [Type] [Name]([Args])
    # We want to capture Namespace, optional lowp, and the Op name.
    stage_pattern = re.compile(
        r'^(?P<addr>[0-9a-fA-F]+)\s+'
        r'(?P<size>[0-9a-fA-F]+)\s+'
        r'[tT]\s+'
        r'(?P<full_name>(?P<ns>\w+)::(?:(?P<lowp>lowp)::)?(?P<op>\w+))'
        r'\(.*$'
    )

    for line in result.stdout.splitlines():
        match = stage_pattern.match(line)
        if match:
            addr = int(match.group('addr'), 16)
            size = int(match.group('size'), 16)
            full_name = match.group('full_name')
            namespace = match.group('ns')
            is_lowp = match.group('lowp') is not None
            op_name = match.group('op')

            if op_name in ops_set and namespace.lower() in RP_NAMESPACES:
                found_symbols[full_name] = {
                    "start": addr,
                    "size": size,
                    "full_demangled": line.split(' ', 3)[-1].strip(),
                    "op": op_name,
                    "namespace": namespace,
                    "precision": "lowp" if is_lowp else "highp"
                }

    return found_symbols

def extract_and_clean_asm(bin_path: str, symbol_info: Dict[str, Any], output_path: str) -> bool:
    """Extracts assembly for a symbol and cleans it for llvm-mca

        The raw objdump looks something like:
00000000006269a0 <_ZN3ml4L8negate_xEP21SkRasterPipelineStagemmPSt4byteDv16_fS4_S4_S4_S4_S4_S4_S4_>:
6269a0:       mov    %rdi,%rax
6269a3:       vxorps 0x69bc63(%rip){1to16},%zmm0,%zmm0        # cc2610 <_IO_stdin_used+0x610>
6269ad:       add    $0x10,%rdi
6269b1:       jmp    *0x10(%rax)

        The desired output for llvm-mca analysis is like:
.L6269a0:
  mov    %rdi,%rax
.L6269a3:
  vxorps 0x69bc63(%rip){1to16},%zmm0,%zmm0        # cc2610
.L6269ad:
  add    $0x10,%rdi
.L6269b1:
  jmp    *0x10(%rax)

        llvm-mca doesn't handle absolute jumps well, so we add in labels
        for each instruction and then rewrite jumps to use labels instead
        of addresses.

    """
    start = symbol_info["start"]
    stop = start + symbol_info["size"]

    try:
        # We need the raw instructions with addresses to resolve jump targets
        cmd = [
            "objdump", "-d", "--no-show-raw-insn", bin_path,
            f"--start-address=0x{start:x}",
            f"--stop-address=0x{stop:x}"
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        output = result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error disassembling {symbol_info['full_demangled']}: {e.stderr}")
        return False
    except Exception as e:
        print(f"Error running objdump: {e}")
        return False

    lines: List[Tuple[str, str]] = []
    addresses: Set[str] = set()

    # First pass: identify all instruction addresses
    for line in output.splitlines():
        if ':' not in line:
            continue
        parts = line.split(':', 1)
        try:
            addr = parts[0].strip()
            # Canonicalize address (remove leading zeros for matching)
            addr_val = int(addr, 16)
            addr_str = f"{addr_val:x}"
            addresses.add(addr_str)
            lines.append((addr_str, parts[1].strip()))
        except (ValueError, IndexError):
            continue

    with open(output_path, 'w') as f:
        for addr, content in lines:
            # content looks like: "je     605e55 <symbol+offset>"
            # Strip the <...> part
            clean_content = re.sub(r'<.*?>', '', content).strip()

            # Identify if this instruction is a jump/call
            # and if it targets one of our internal addresses.
            # We look for a hex number in the clean_content.
            # e.g. "je     605e55"
            match = re.search(r'\b(?P<target>[0-9a-f]{4,})\b', clean_content)
            if match:
                target_addr = match.group('target')
                if target_addr in addresses:
                    # Replace the hex address with a label name
                    clean_content = clean_content.replace(target_addr, f".L{target_addr}")

            # Output the line with its own label
            f.write(f".L{addr}:\n")
            f.write(f"  {clean_content}\n")
    return True

def run_mca(mca_path: str, mcpu: str, asm_path: str) -> Optional[Dict[str, float]]:
    """Runs llvm-mca to get steady-state runtime stats."""
    try:
        # Use -skip-unsupported-instructions=parse-failure to gracefully handle complex branches/calls
        cmd = [
            mca_path,
            f"-mcpu={mcpu}",
            # We choose 100 iterations here to simulate the effect of running
            # this stage many times in a row. That's not exactly how things will
            # be in practice, since a pipeline is made up of many stages, but it
            # should be more like a real-world scenario than --iterations=1.
            "-iterations=100",
            "-skip-unsupported-instructions=parse-failure",
            asm_path
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        output = result.stdout
    except subprocess.CalledProcessError as e:
        error_msg = e.stdout + e.stderr
        if "error: no assembly instructions found" in error_msg:
            return None
        # If it still fails, it's a more serious issue
        print(f"Error running llvm-mca on {asm_path}: {error_msg}")
        return None
    except Exception as e:
        print(f"Error running llvm-mca: {e}")
        return None

    # Example output for llvm-mca-19:
    # Iterations:        100
    # Instructions:      2100
    # Total Cycles:      1337
    # Total uOps:        3300

    # Dispatch Width:    6
    # uOps Per Cycle:    2.47
    # IPC:               1.57
    # Block RThroughput: 12.0
    # ...

    metrics: Dict[str, float] = {}
    # Parse Summary Table
    total_cycles_match = re.search(r'Total Cycles:\s+(?P<cycles>\d+)', output)
    if total_cycles_match:
        metrics["Cycles/Iter"] = float(total_cycles_match.group('cycles')) / 100.0

    ipc_match = re.search(r'IPC:\s+(?P<ipc>[\d.]+)', output)
    if ipc_match:
        metrics["IPC"] = float(ipc_match.group('ipc'))

    rthroughput_match = re.search(r'Block RThroughput:\s+(?P<rthp>[\d.]+)', output)
    if rthroughput_match:
        metrics["RThroughput"] = float(rthroughput_match.group('rthp'))

    return metrics

def process_symbol(args_tuple: Tuple[str, Dict[str, Any], str, str, str, str]) -> Tuple[str, Optional[Dict[str, float]]]:
    """Worker function for parallel processing."""
    name, info, bin_path, version_dir, mca_path, mcpu = args_tuple
    asm_path = os.path.join(version_dir, f"{name.replace('::', '_')}.s")
    if extract_and_clean_asm(bin_path, info, asm_path):
        metrics = run_mca(mca_path, mcpu, asm_path)
        if metrics:
            return name, metrics
    return name, None

def main() -> None:
    parser = argparse.ArgumentParser(description="Analyze SkRasterPipeline stage performance.")
    parser.add_argument("--bin", required=True, help="Path to the binary to analyze.")
    parser.add_argument("--name", required=True, help="Name for this run (e.g. control, experiment).")
    parser.add_argument("--work_dir", required=True, help="Directory to store assembly and results.")
    parser.add_argument("--skia_root", default=".", help="Path to Skia root directory.")
    parser.add_argument("--mcpu", default="x86-64-v4", help="CPU model for llvm-mca.")
    parser.add_argument("--mca_path", default="/usr/bin/llvm-mca-19", help="Path to llvm-mca.")
    parser.add_argument("--list-stages", action="store_true", help="Just list found stages and exit.")
    parser.add_argument("--reset-experiments", action="store_true", help="Remove all columns except the first (typically the control) before adding the new results.")

    args = parser.parse_args()

    if not os.path.exists(args.bin):
        print(f"Error: Binary {args.bin} does not exist.")
        sys.exit(1)

    if not os.path.exists(args.work_dir):
        os.makedirs(args.work_dir)

    # Ensure version subfolder exists
    version_dir = os.path.join(args.work_dir, args.name)
    if not os.path.exists(version_dir):
        os.makedirs(version_dir)

    print(f"Scanning {args.skia_root} for RP stages...")
    ops = get_stage_names(args.skia_root)
    print(f"Found {len(ops)} potential stages.")

    print(f"Discovering symbols in {args.bin}...")
    symbols = find_symbols_in_binary(args.bin, ops)
    print(f"Found {len(symbols)} compiled stages.")

    if args.list_stages:
        for s in sorted(symbols.keys()):
            print(f"  {s}")
        return

    results: Dict[str, Dict[str, float]] = {}
    total = len(symbols)
    print(f"Analyzing {total} stages in parallel...")

    # Prepare arguments for workers
    worker_args = [
        (name, info, args.bin, version_dir, args.mca_path, args.mcpu)
        for name, info in symbols.items()
    ]

    count = 0
    # Run the analysis in parallel or it takes a long time with 1000+ stages
    with multiprocessing.Pool() as pool:
        for name, metrics in pool.imap_unordered(process_symbol, worker_args):
            count += 1
            if metrics:
                results[name] = metrics

            # Progress bar
            percent = (count * 100) // total
            bar_len = 40
            filled_len = (count * bar_len) // total
            bar = '#' * filled_len + '-' * (bar_len - filled_len)
            sys.stdout.write(f"\r[{bar}] {percent}% ({count}/{total}) {name[:30]:<30}")
            sys.stdout.flush()

    print("\nAnalysis complete.")

    # Manage spreadsheet
    tsv_path = os.path.join(args.work_dir, "analysis.tsv")
    all_data: Dict[str, Dict[str, str]] = {}
    versions: List[str] = []

    if os.path.exists(tsv_path):
        with open(tsv_path, 'r', newline='') as f:
            reader = csv.DictReader(f, delimiter='\t')
            # Extract version names from headers like "control Cycles/Iter"
            if reader.fieldnames:
                for field in reader.fieldnames:
                    if " Cycles/Iter" in field:
                        versions.append(field.replace(" Cycles/Iter", ""))

            for row in reader:
                symbol = row['Symbol']
                all_data[symbol] = row

    if args.reset_experiments and versions:
        versions = [versions[0]]

    if args.name not in versions:
        versions.append(args.name)

    # Update all_data with new results
    for symbol, metrics in results.items():
        info = symbols[symbol]
        if symbol not in all_data:
            all_data[symbol] = {
                'Symbol': symbol,
                'Op': info['op'],
                'Namespace': info['namespace'],
                'Precision': info['precision']
            }

        # Use friendly headers directly as internal keys
        all_data[symbol][f"{args.name} Cycles/Iter"] = f"{metrics['Cycles/Iter']:.2f}"
        all_data[symbol][f"{args.name} Inst/Cycle (IPC)"] = f"{metrics['IPC']:.2f}"
        all_data[symbol][f"{args.name} Throughput (RThp)"] = f"{metrics['RThroughput']:.2f}"

        # Calculate improvement if there's a control (the first version)
        if len(versions) > 1:
            control = versions[0]
            ctrl_key = f"{control} Cycles/Iter"
            if ctrl_key in all_data[symbol] and all_data[symbol][ctrl_key]:
                try:
                    ctrl_cycles = float(all_data[symbol][ctrl_key])
                    curr_cycles = metrics['Cycles/Iter']
                    if ctrl_cycles > 0:
                        diff = (ctrl_cycles - curr_cycles) / ctrl_cycles * 100
                        all_data[symbol][f"{args.name} % Improv"] = f"{diff:+.2f}"
                except ValueError:
                    pass

    # Write back to TSV
    fieldnames = ['Op', 'Namespace', 'Precision', 'Symbol']
    for v in versions:
        fieldnames.append(f"{v} Cycles/Iter")
        fieldnames.append(f"{v} Inst/Cycle (IPC)")
        fieldnames.append(f"{v} Throughput (RThp)")
        if v != versions[0]:
            fieldnames.append(f"{v} % Improv")

    with open(tsv_path, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter='\t', extrasaction='ignore')
        writer.writeheader()
        for symbol in sorted(all_data.keys()):
            writer.writerow(all_data[symbol])

    print(f"Results written to {tsv_path}")

if __name__ == "__main__":
    main()

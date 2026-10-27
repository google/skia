# Skia `nanobench` Tool Usage

`nanobench` is Skia's performance benchmarking tool. It measures the execution time of GMs, benches, and other sources. **Always use a Release build for benchmarking.**

## Basic Usage
```bash
out/Release/nanobench --match <pattern>
```

## Performance Baselines
To accurately measure the impact of a change, it is recommended to keep a "control" version of the benchmark binary:
1. Build the baseline (e.g., in `out/Release`).
2. Copy the binary to a stable location: `cp out/Release/nanobench ./nanobench_control`.
3. Apply your changes and rebuild `out/Release/nanobench`.
4. Run both and compare: `./nanobench_control ...` vs `out/Release/nanobench ...`.

## Common Flags

### Matching/Filtering
- `--match` (or `-m`): Filter benchmarks by name (case-sensitive substring).
- `--config <names>`: Configs to run (e.g., `8888`, `gl`, `vk`). Default: `8888 gl nonrendering`.
- `--benchType <type>`: Filter by benchmark type (e.g., `micro`, `recording`).

### Benchmarking Parameters
- `--samples <n>`: Number of samples per benchmark (default 10).
- `--ms <n>`: If >0, run each bench for this many ms instead of obeying `--samples`.
- `--loops <n>`: Number of times to run each bench. Set to 0 (default) to auto-tune.
- `--verbose` (or `-v`): Enable verbose output.

### Profiling
Requires `skia_use_pprof=true` in GN args.
- `--cpuprofile <file>`: Write pprof CPU profile to `<file>`.
- `--memprofile <file>`: Write pprof heap profile to `<file>`.

## Example Workflows

### Run benchmarks matching "gradient"
```bash
out/Release/nanobench --config 8888 --match gradient --samples 5 --ms 100
```

### Profile a specific benchmark
```bash
# Ensure GN args have skia_use_pprof=true
out/Release/nanobench --config 8888 --match alphagradients --cpuprofile profile.pprof
```

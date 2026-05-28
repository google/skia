# Skia `nanobench` Tool Usage

`nanobench` is Skia's performance benchmarking tool. It measures the execution time of GMs, benches, and other sources.

## Basic Usage
```bash
out/Debug/nanobench --match <pattern>
```

## Common Flags

### Matching/Filtering
- `--match <pattern>`: Case-insensitive glob on benchmark names. Substring matches often need careful capitalization or specific patterns.
- `--config <names>`: Configs to run (e.g., `8888`, `gpu`, `vk`).

### Benchmarking Parameters
- `--samples <n>`: Number of samples to take (default is 10).
- `--ms <n>`: Minimum milliseconds to spend on each bench (default is 250).
- `--verbose` (or `-v`): Enable verbose output.

### Profiling
Requires `skia_use_pprof=true` in GN args.
- `--cpuprofile <file>`: Write CPU profile to `<file>`.
- `--memprofile <file>`: Write memory profile to `<file>`.

## Example Workflows

### Run benchmarks matching "gradient"
```bash
out/Debug/nanobench --config 8888 --match gradient --samples 5 --ms 100
```

### Profile a specific benchmark
```bash
# Ensure GN args have skia_use_pprof=true
out/Release/nanobench --config 8888 --match alphagradients --cpuprofile profile.pprof
```

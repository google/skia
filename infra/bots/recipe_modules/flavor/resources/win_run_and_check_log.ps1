# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Usage: win_run_and_check_log.ps1 <cmd> <args>
# Runs the given command, prints the exit code, and prints any application
# crashes found in the Windows Application Event Log. If the command fails,
# return exit code 1; if any crashes, return exit code 2; otherwise, return exit
# code 0.

$ErrorActionPreference = 'Stop'

$begin = Get-Date

# Run the command passed to the script.
& $args[0] $args[1..($args.length - 1)]

$res = $LastExitCode

Write-Host "$($args[0]) exited with status $res"

# dm sometimes exits with negative exit codes, e.g. -1073741569, which in some
# cases cmd treats as 0. Ensure any non-zero are OK for cmd.
if ($res -ne 0) {
  $res = 1
}

$end = Get-Date

# "-InstanceId 1000" means "Application Error".
# We use "-ErrorAction SilentlyContinue" because Get-EventLog raises an error if
# no logs are found.
$faults = Get-EventLog -LogName Application -InstanceId 1000 -EntryType Error `
  -After $begin -Before $end -ErrorAction SilentlyContinue

if ($faults) {
  Write-Host 'If the message below is unrelated to this run, `
    please file a bug and assign to dogben.'
  Write-Host $faults[0].Message
  if ($res -eq 0) {
    $res = 2
  }
}

# https://stackoverflow.com/a/50202663
if ($res -ne 0) {
  $host.SetShouldExit($res)
  exit $res
}

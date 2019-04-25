# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Usage: win_run_and_check_log.ps1 <cmd> <args>
# Runs the given command. If it fails, return the exit code of the command. If
# it succeeds, check the Windows Application Event Log for application crashes;
# if any are found, print info about the crash and return exit code 17.

$ErrorActionPreference = 'Stop'

$begin = Get-Date

# Run the command passed to the script.
& $args[0] $args[1..($args.length - 1)]

$end = Get-Date

# "-InstanceId 1000" means "Application Error".
# We use "-ErrorAction SilentlyContinue" because Get-EventLog raises an error if
# no logs are found.
$faults = Get-EventLog -LogName Application -InstanceId 1000 -EntryType Error `
  -After $begin -Before $end -ErrorAction SilentlyContinue

if ($faults) {
  $host.SetShouldExit(17);
  Write-Host 'If the message below is unrelated to this run, `
    please file a bug and assign to dogben.'
  Write-Host $faults[0].Message
}

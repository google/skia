G3 Canary Production Manual
===========================

General information about canaries is available in
[go/autoroller-canary-bots](https://goto.google.com/autoroller-canary-bots).

Alerts
======

g3_canary_infra_failures
------------------------

Happens when the skia_try_service in G3 returns an exception.
Check the skia_try_service's error logs in [go/skia-borg-jobs](go/skia-borg-jobs)

For errors that do not seem to be transient, restarting the borg job has worked
in the past:
```
borg --borg=${BORG_CELL} --user=skia --name=skia_try_service --avoid_parent restarttask 0
```

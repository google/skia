Infra Trooper Documentation
===========================

### Contents ###

*   [What does an Infra trooper do?](#what_is_a_trooper)
*   [View current and upcoming troopers](#view_current_upcoming_troopers)
*   [How to swap trooper shifts](#how_to_swap)
*   [Tips for troopers](#tips)


<a name="what_is_a_trooper"></a>
What does an Infra trooper do?
------------------------------

The trooper has two main jobs:

1) Keep an eye on Infra alerts emails (sent to infra-alerts@skia.org). The alerts are also available [here](https://alerts.skia.org/infra).

2) Resolve the above alerts as they come in.

<a name="view_current_upcoming_troopers"></a>
View current and upcoming troopers
----------------------------------

The list of troopers is specified in the [skia-tree-status web app](http://skia-tree-status.appspot.com/trooper). The current trooper is highlighted in green.
The banner on the top of the [status page](https://status.skia.org) also displays the current trooper.


<a name="how_to_swap"></a>
How to swap trooper shifts
--------------------------

If you need to swap shifts with someone (because you are out sick or on vacation), please get approval from the person you want to swap with. Then send an email to skiabot@google.com and cc rmistry@.


<a name="tips"></a>
Tips for troopers
-----------------

- Make sure you are a member of
  [MDB group chrome-skia-ninja](https://ganpati.corp.google.com/#Group_Info?name=chrome-skia-ninja@prod.google.com).
  Valentine passwords and Chrome Golo access are based on membership in this
  group.

- These alerts generally auto-dismiss once the criteria for the alert is no
  longer met:
  - Monitoring alerts, including prober, collectd, and others
  - Disconnected build slaves

- These alerts generally do not auto-dismiss ([issue here](https://bug.skia.org/4292)):
  - Build slaves that failed a step
  - Disconnected devices (these are detected as the "wait for device" step failing)

- "Failed to execute query" may show a different query than the failing one;
  dismiss the alert to get a new alert showing the query that is actually
  failing. (All "failed to execute query" alerts are lumped into a single alert,
  which is why the failed query which initially triggered the alert may not be
  failing any more but the alert is still active because another query is
  failing.)

- Where machines are located:
  - Machine name like "skia-vm-NNN" -> GCE
  - Machine name ends with "a3", "a4", "m3" -> Chrome Golo
  - Machine name starts with "skiabot-" -> Chapel Hill lab
  - Machine name starts with "win8" -> Chapel Hill lab (Windows machine
    names can't be very long, so the "skiabot-shuttle-" prefix is dropped.)
  - slave11-c3 is a Chrome infra GCE machine (not to be confused with the Skia
    Buildbots GCE, which we refer to as simply "GCE")

- The [chrome-infra IRC channel](https://comlink.googleplex.com/chrome-infra) is
  useful for questions regarding bots managed by the Chrome Infra team and to
  get visibility into upstream failures that cause problems for us.

- To log in to a Linux buildbot in GCE, use `gcloud compute ssh default@<machine
  name>`. Choose the zone listed for the
  [GCE VM](https://pantheon.corp.google.com/project/31977622648/compute/instances)
  (or specify it using the `--zone` command-line flag).

- To log in to a Windows buildbot in GCE, use
  [Chrome RDP Extension](https://chrome.google.com/webstore/detail/chrome-rdp/cbkkbcmdlboombapidmoeolnmdacpkch?hl=en-US)
  with the
  [IP address of the GCE VM](https://pantheon.corp.google.com/project/31977622648/compute/instances)
  shown on the [host info page](https://status.skia.org/hosts) for that bot. The
  username is chrome-bot and the password can be found on
  [Valentine](https://valentine.corp.google.com/) as "chrome-bot (Win GCE)".

- If there is a problem with a bot in the Chrome Golo or Chrome infra GCE, the
  best course of action is to
  [file a bug](https://code.google.com/p/chromium/issues/entry?template=Build%20Infrastructure)
  with the Chrome infra team. But if you know what you're doing:
  - To access bots in the Chrome Golo,
    [follow these instructions](https://chrome-internal.googlesource.com/infra/infra_internal/+/master/doc/ssh.md).
    - Machine name ends with "a3" or "a4" -> ssh command looks like `ssh
      build3-a3.chrome`
    - Machine name ends with "m3" -> ssh command looks like `ssh build5-m3.golo`
    - For MacOS and Windows bots, you will be prompted for a password, which is
      stored on [Valentine](https://valentine.corp.google.com/) as "Chrome Golo,
      Perf, GPU bots - chrome-bot".
  - To access bots in the Chrome infra GCE -> command looks like `gcutil
    --project=google.com:chromecompute ssh --ssh_user=default slave11-c3` (or
    use the ccompute ssh script from the infra_internal repo).

- Read over the [SkiaLab documentation](../testing/skialab) for more detail on
  dealing with device alerts.

- To stop a buildslave for a device, log in to the host for that device, `cd
  ~/buildbot/<slave name>/build/slave; make stop`. To start it again,
  `TESTING_SLAVENAME=<slave name> make start`.

- Buildslaves can be slow to come up after reboot, but if the buildslave remains
  disconnected, you may need to start it manually. On Mac and Linux, check using
  `ps aux | grep python` that neither buildbot nor gclient are running, then run
  `~/skiabot-slave-start-on-boot.sh`.

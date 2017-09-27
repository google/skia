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

1) Keep an eye on Infra alerts available [here](https://promalerts.skia.org/#/alerts?receiver=skiabot).

2) Resolve the above alerts as they come in.

<a name="view_current_upcoming_troopers"></a>
View current and upcoming troopers
----------------------------------

The list of troopers is specified in the [skia-tree-status web app](http://skia-tree-status.appspot.com/trooper). The current trooper is highlighted in green.
The banner on the top of the [status page](https://status.skia.org) also displays the current trooper.


<a name="how_to_swap"></a>
How to swap trooper shifts
--------------------------

If you need to swap shifts with someone (because you are out sick or on vacation), please get approval from the person you want to swap with. Then make the change in the [cloud console](https://console.cloud.google.com/datastore/entities/query?project=skia-tree-status&organizationId=433637338589&ns=&kind=TrooperSchedules). Add a filter to find the dates you are looking for and then click on the entries you want to edit.

Note: The above link can be used to update the sheriff/wrangler/robocop schedules as well.


<a name="tips"></a>
Tips for troopers
-----------------

- Go over the [trooper handoff doc](https://docs.google.com/document/d/1I1tB0Cv2fme4FY0lAF2gYeEbZ_0kehLIi3vf3vuPkx0/edit) to be aware of ongoing problems and any issues the previous trooper ran into. Document any notes there from your trooper week that might help the next trooper.

- Make sure you are a member of
  [MDB group chrome-skia-ninja](https://ganpati.corp.google.com/#Group_Info?name=chrome-skia-ninja@prod.google.com).
  Valentine passwords and Chrome Golo access are based on membership in this
  group.

- Install the Skia trooper Chrome extension (available [here](https://chrome.google.com/webstore/a/google.com/detail/alerts-for-skia-troopers/fpljhfiomnfioecagooiekldeolcpief)) to be able to see alerts quickly in the browser.

- Where machines are located:
  - Machine name like "skia-gce-NNN", "ct-gce-NNN" -> GCE
  - Machine name ends with "a3", "a4", "m3" -> Chrome Golo
  - Machine name ends with "m5" -> CT bare-metal bots in Chrome Golo
  - Machine name starts with "skiabot-" -> Chapel Hill lab
  - Machine name starts with "win8" -> Chapel Hill lab (Windows machine
    names can't be very long, so the "skiabot-shuttle-" prefix is dropped.)
  - slave11-c3 is a Chrome infra GCE machine (not to be confused with the Skia
    Buildbots GCE, which we refer to as simply "GCE")

- The [chrome-infra hangout](https://goto.google.com/cit-hangout) is useful for
  questions regarding bots managed by the Chrome Infra team and to get
  visibility into upstream failures that cause problems for us.

- To log in to a Linux bot in GCE, use `gcloud compute ssh default@<machine
  name>`. Choose the zone listed for the
  [GCE VM](https://console.cloud.google.com/project/31977622648/compute/instances)
  (or specify it using the `--zone` command-line flag).

- To log in to a Windows bot in GCE, use
  [Chrome RDP Extension](https://chrome.google.com/webstore/detail/chrome-rdp/cbkkbcmdlboombapidmoeolnmdacpkch?hl=en-US)
  with the
  [IP address of the GCE VM](https://console.cloud.google.com/project/31977622648/compute/instances)
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
    - Machine name ends with "m5" -> ssh command looks like `ssh build1-m5.golo`.
      [Example bug](https://bugs.chromium.org/p/chromium/issues/detail?id=638193) to file to Infra Labs.
    - For MacOS and Windows bots, you will be prompted for a password, which is
      stored on [Valentine](https://valentine.corp.google.com/) as "Chrome Golo,
      Perf, GPU bots - chrome-bot".
  - To access bots in the Chrome infra GCE -> command looks like `gcutil
    --project=google.com:chromecompute ssh --ssh_user=default slave11-c3` (or
    use the ccompute ssh script from the infra_internal repo).

- Read over the [Skolo maintenance doc](https://docs.google.com/document/d/1zTR1YtrIFBo-fRWgbUgvJNVJ-s_4_sNjTrHIoX2vulo/edit) for more detail on
  dealing with device alerts.

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
  - Machine name like "skia-gce-NNN", "skia-i-gce-NNN", "ct-gce-NNN", "skia-ct-gce-NNN", "ct-xxx-builder-NNN" -> GCE
  - Machine name ends with "a9", "m3" -> Chrome Golo/Labs
  - Machine name ends with "m5" -> CT bare-metal bots in Chrome Golo
  - Machine name starts with "skia-e-", "skia-i-" (other than "skia-i-gce-NNN"), "skia-rpi-" -> Chapel Hill lab

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

- To log in to other bots, see the [Skolo maintenance doc](https://docs.google.com/document/d/1zTR1YtrIFBo-fRWgbUgvJNVJ-s_4_sNjTrHIoX2vulo/edit#heading=h.2nq3yd1axg0n) remote access section.

- If there is a problem with a bot in the Chrome Golo or Chrome infra GCE, the
  best course of action is to
  [file a bug](https://code.google.com/p/chromium/issues/entry?template=Build%20Infrastructure)
  with the Chrome infra team.

- Read over the [Skolo maintenance doc](https://docs.google.com/document/d/1zTR1YtrIFBo-fRWgbUgvJNVJ-s_4_sNjTrHIoX2vulo/edit) for more detail on
  dealing with device alerts.

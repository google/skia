Downloading SKPs
================

### Contents ###

*   [Overview](#overview)
*   [How to download SKPs](#download_skps)
    +   [Buildbot SKPs (for Googlers)](#buildbot_skps_googler)
    +   [Buildbot SKPs (for trusted partners)](#buildbot_skps_partners)
    +   [Cluster Telemetry SKPs (for Googlers)](#ct_skps_googler)

<a name="overview"></a>
Overview
--------

An SKP is a binary representation of the draw commands Chromium sends to Skia for rasterization.

Skia uses different types of SKPs in our continuous builds and tests:

* [Buildbot](https://skia.org/dev/testing/buildbot) SKPs are the small set (89 as of 2/10/16) of SKPs that are used on almost all our bots.
* [Cluster Telemetry](https://skia.org/dev/testing/ct) SKPs are the much larger set (~900k) that are used by our CT bots (Eg: [CT_BENCH_10k](https://build.chromium.org/p/client.skia/builders/Perf-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-CT_BENCH_10k_SKPs), [CT_DM_10k](https://build.chromium.org/p/client.skia/builders/Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_10k_SKPs), [CT_DM_1m](https://build.chromium.org/p/client.skia.fyi/builders/Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_1m_SKPs)).

Skia developers find it useful to download these SKPs to run local tests to reproduce problems seen on the buildbots. The below sections detail how to download them.

<a name="download_skps"></a>
How to download SKPs
--------------------

<a name="buildbot_skps_googler"></a>
### Buildbot SKPs (for Googlers)

The following will work only if you have a google.com account.

All buildbot SKP files created by the RecreateSKPs bot are available via the asset management scripts:

    $ download_from_google_storage -s infra/bots/tools/luci-go/${PLATFORM}/cipd.sha1 --bucket chromium-luci
    $ infra/bots/tools/luci-go/${PLATFORM}/cipd auth-login
    $ infra/bots/assets/skp/download.py -t ${TARGET_DIR}


<a name="buildbot_skps_partners"></a>
### Buildbot SKPs (for trusted partners)

* Request access to the gs://chrome-partner-telemetry bucket by emailing telemetry@chromium.org ([example](https://groups.google.com/a/chromium.org/d/msg/telemetry/kSwcgH7KiYs/zwBwEZaadOIJ)).
* Once approved you can download Skia's weekly refreshed buildbot SKPs from gs://chrome-partner-telemetry/skps


<a name="ct_skps_googler"></a>
### Cluster Telemetry SKPs (for Googlers)

The following will work only if you have a google.com account.

* Download the gsutil tool https://developers.google.com/cloud/sdk/#Quick_Start.
* Authenticate using your google.com credentials “gcloud auth login”
* Look at the SKP Repository list [here](https://ct.skia.org/lua_script/) for all available SKPs.
* Run in your terminal:

    $ SKP\_DEST=...<br/>
    $ REPO\_TYPE=... (Either All, 100k, Mobile10k, 10k, Dummy1k)<br/>
    $ CHROMIUM\_BUILD=... (Eg: fad657e-276e633)<br/>
    $ gsutil -m cp gs://cluster-telemetry/swarming/skps/${REPO\_TYPE}/${CHROMIUM\_BUILD}/{1..10}/*.skp ${SKP\_DEST}

* Substitute the 1 and 10 above with the start and end ranks of the SKPs you want to copy locally.
* If you are trying to find a particular SKP. Look for it in the CSV [here](https://pantheon.corp.google.com/m/cloudstorage/b/cluster-telemetry/o/csv/top-1m.csv) and then use that rank in the copy command.

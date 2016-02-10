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

All buildbot SKP files created by the RecreateSKPs bot are available in the Google Storage bucket gs://chromium-skia-gm, they can be copied over to your local directory like this:

* Download the gsutil tool https://developers.google.com/cloud/sdk/#Quick_Start.
* Authenticate using your google.com credentials “gcloud auth login”
* Get the current SKP version from https://skia.googlesource.com/skia/+/master/SKP_VERSION.
* Run in your terminal:

    $ SKP\_VERSION=...<br/>
    $ SKP\_DEST=...<br/>
    $ gsutil -m cp gs://chromium-skia-gm/playback\_${SKP\_VERSION}/skps/*.skp ${SKP\_DEST}


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
    $ REPO\_TYPE=... (Either All, Mobile10k, 10k, Dummy1k)<br/>
    $ CHROMIUM\_BUILD=... (Eg: 57259e0-05dcb4c)<br/>
    $ SLAVE\_NUM=... (There are 100 available slaves)<br/>
    $ gsutil -m cp gs://cluster-telemetry/skps/${REPO\_TYPE}/${CHROMIUM\_BUILD}/slave${SLAVE\_NUM}/*.skp ${SKP\_DEST}

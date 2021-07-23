#!/usr/bin/env lucicfg

luci.project(
    name = "Skia, 2D graphics library",
    buildbucket = "cr-buildbucket.appspot.com",
    swarming = "chromium-swarm.appspot.com",
    acls = [
        acl.entry(acl.PROJECT_CONFIGS_READER, groups = ["all"]),
        acl.entry(acl.LOGDOG_READER, groups = ["all"]),
        acl.entry(acl.LOGDOG_WRITER, groups = ["luci-logdog-skia-writers"]),
        acl.entry(acl.CQ_COMMITTER, groups = ["project-skia-committers"]),
        acl.entry(acl.CQ_DRY_RUNNER, groups = ["project-skia-tryjob-access"]),
    ],
    logdog = "luci-logdog",
)

luci.logdog(
    gs_bucket = "skia-logdog",
)

luci.bucket(
    name = "skia.primary",
    acls = [
        acl.entry(acl.BUILDBUCKET_READER, groups = ["all"]),
        acl.entry(acl.BUILDBUCKET_OWNER, groups = [
            "project-skia-external-buildbucket-writers",
        ]),
        acl.entry(acl.BUILDBUCKET_TRIGGERER, groups = [
            "project-skia-tryjob-access",
            "service-account-cq",
        ]),
        acl.entry(acl.BUILDBUCKET_TRIGGERER, projects = [
            "skiabuildbot",
            "skia-skcms",
        ]),
    ],
)

luci.bucket(
    name = "skia.testing",
    acls = [
        acl.entry(acl.BUILDBUCKET_READER, groups = ["all"]),
        acl.entry(acl.BUILDBUCKET_OWNER, groups = [
            "project-skia-external-buildbucket-writers",
            "project-skia-committers",
        ]),
        acl.entry(acl.BUILDBUCKET_TRIGGERER, groups = [
            "project-skia-tryjob-access",
            "service-account-cq",
        ]),
        acl.entry(acl.BUILDBUCKET_TRIGGERER, projects = [
            "skiabot-test",
        ]),
    ],
)


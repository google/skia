#!/usr/bin/env lucicfg

# Enable LUCI Realms support.
lucicfg.enable_experiment("crbug.com/1085650")

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
    bindings = [
        luci.binding(
            roles = "role/swarming.poolOwner",
            groups = "project-skia-admins",
        ),
        luci.binding(
            roles = "role/swarming.poolViewer",
            groups = "chromium-swarm-view-all-bots",
        ),
        luci.binding(
            roles = "role/swarming.taskViewer",
            groups = "chromium-swarm-view-all-tasks",
        ),
        luci.binding(
            roles = [
                "role/swarming.poolUser",
                "role/swarming.taskTriggerer",
            ],
            groups = [
                "project-skia-admins",
                "project-skia-external-task-schedulers",
            ],
        ),
        luci.binding(
            roles = "role/swarming.taskServiceAccount",
            groups = [
                "project-skia-external-task-accounts",
            ],
        ),
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

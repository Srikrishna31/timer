load("@//:config.bzl", "platform_config", "package_copt")

platform_config()

cc_library(
    name = "timer",
    hdrs = [
        "include/timer.h"
    ],
    srcs = [
        "src/timer.cpp",
    ],
    strip_include_prefix = "include",
    copts = package_copt,
    visibility = ["//visibility:public"],
)

cc_test(
    name = "test_timer",
    srcs = ["test/test_timer.cpp"],
    copts = package_copt,
    tags = ["unit"],
    deps = [
        ":timer",
        "@gtest//:gtest",
    ],
    visibility = ["//visibility:private"]
)

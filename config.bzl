
# The option for windows compiler is specified with a /.
package_copt = select({"@bazel_tools//platforms:windows" : ["/std:c++17"],
    "//conditions:default" : ["-std=c++17"],})

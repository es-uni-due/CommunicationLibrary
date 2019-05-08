"""
The location of this file (even when empty) specifies the project root
for more info see https://docs.bazel.build/versions/master/build-ref.html
"""

"""
set the global repository name, this function can only be called from this file
https://docs.bazel.build/versions/master/be/functions.html#workspace
"""

workspace(
    name = "CommunicationModule",
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

"""
Fetch unity and use the file BUILD.unity (residing in this folder) for the build.
We use the prefix new because unity isn't a bazel project, so we need to provide a BUILD file.
More info under https://docs.bazel.build/versions/master/be/workspace.html#new_http_archive
"""

#git_repository(
local_repository(
    name = "EmbeddedSystemsBuildScripts",
    path = "../bazel-avr-toolchain-linux",
    #    commit = "ec8379742aa0859ac157943c0d60b6ad55c3713d",
    #    remote = "https://bitbucket.es.uni-due.de/scm/fks/bazel-avr-toolchain-linux.git",
)

load("@EmbeddedSystemsBuildScripts//:avr.bzl", "avr_toolchain")

avr_toolchain()

http_archive(
    name = "Unity",
    build_file = "@EmbeddedSystemsBuildScripts//:BUILD.Unity",
    strip_prefix = "Unity-master",
    urls = ["https://github.com/ThrowTheSwitch/Unity/archive/master.tar.gz"],
)

http_archive(
    name = "CException",
    build_file = "@EmbeddedSystemsBuildScripts//:BUILD.CException",
    strip_prefix = "CException-master",
    urls = ["https://github.com/ThrowTheSwitch/CException/archive/master.tar.gz"],
)

http_archive(
    name = "CMock",
    build_file = "@EmbeddedSystemsBuildScripts//:BUILD.CMock",
    strip_prefix = "CMock-master",
    urls = ["https://github.com/ThrowTheSwitch/CMock/archive/master.tar.gz"],
)

http_archive(
    name = "LUFA",
    build_file = "@AvrToolchain//:BUILD.LUFA",
    strip_prefix = "lufa-LUFA-170418",
    urls = ["http://fourwalledcubicle.com/files/LUFA/LUFA-170418.zip"],
)

git_repository(
    name = "EmbeddedUtilities",
    commit = "8d02f86d1921fcccd24f8a688dffcc5e3e80169d",
    remote = "https://bitbucket.es.uni-due.de/scm/im/embedded-utilities.git",
)

git_repository(
    #local_repository(
    name = "PeripheralInterface",
    commit = "8e90f180c526be463f84283736535bb3726d3581",
    path = "../peripheralinterface",
    remote = "https://bitbucket.es.uni-due.de/scm/im/peripheralinterface.git",
)

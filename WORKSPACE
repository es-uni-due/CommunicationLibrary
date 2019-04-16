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

"""
Fetch unity and use the file BUILD.unity (residing in this folder) for the build.
We use the prefix new because unity isn't a bazel project, so we need to provide a BUILD file.
More info under https://docs.bazel.build/versions/master/be/workspace.html#new_http_archive
"""

http_archive(
    name = "Unity",
    build_file = "@//:BUILD.Unity",
    strip_prefix = "Unity-master",
    urls = ["https://github.com/ThrowTheSwitch/Unity/archive/master.tar.gz"],
)

http_archive(
    name = "CException",
    build_file = "@//:BUILD.CException",
    strip_prefix = "CException-master",
    urls = ["https://github.com/ThrowTheSwitch/CException/archive/master.tar.gz"],
)

http_archive(
    name = "UnityPlugin",
    strip_prefix = "BazelUnityPlugin-develop",
    urls = ["https://github.com/glencoe/BazelUnityPlugin/archive/develop.tar.gz"],
)

http_archive(
    name = "CMock",
    build_file = "@//:BUILD.CMock",
    strip_prefix = "CMock-master",
    urls = ["https://github.com/ThrowTheSwitch/CMock/archive/master.tar.gz"],
)

http_archive(
    name = "AVR_Toolchain",
    type = "tar.gz",
    urls = ["http://bitbucket.es.uni-due.de:7990/rest/api/latest/projects/FKS/repos/bazel-avr-toolchain-linux/archive?format=tgz"],
)

http_archive(
    name = "LUFA",
    build_file = "@//:BUILD.LUFA",
    strip_prefix = "lufa-LUFA-170418",
    urls = ["http://fourwalledcubicle.com/files/LUFA/LUFA-170418.zip"],
)

local_repository(
    name = "Util",
    path = "../EmbeddedUtil/"
)

local_repository(
    name = "PeripheralInterface",
    path = "../peripheralinterface"
)

#local_repository(
#    name = "LUFA",
#    path = "../lufa-LUFA-170418",
#)

"""
From the Bazel documentation at https://docs.bazel.build/versions/master/build-ref.html#packages_targets :
 The primary unit of code organization in a workspace is the package.
A package is collection of related files and a specification of the dependencies among them.

A package is defined as a directory containing a file named BUILD,
residing beneath the top-level directory in the workspace.
A package includes all files in its directory, plus all subdirectories beneath it,
except those which themselves contain a BUILD file.
"""

"""
This project has the following structure:

/
|- WORKSPACE
|- BUILD.unity
|- .bazelr
|- lib/
    |- BUILD
|- test/
    |- BUILD
    |- unity-helpers.bzl
    |- /Mocks/
        |- BUILD
|- toolchain/
    |- CROSSTOOL
    |- BUILD
"""

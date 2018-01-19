"""
The location of this file (even when empty) specifies the project root
for more info see https://docs.bazel.build/versions/master/build-ref.html
"""

"""
set the global repository name, this function can only be called from this file
https://docs.bazel.build/versions/master/be/functions.html#workspace
"""
workspace (
        name = "CommunicationModule",
        )

"""
Fetch unity and use the file BUILD.unity (residing in this folder) for the build.
We use the prefix new because unity isn't a bazel project, so we need to provide a BUILD file.
More info under https://docs.bazel.build/versions/master/be/workspace.html#new_http_archive
"""
new_http_archive(
  name = "unity",
  urls = ["https://github.com/ThrowTheSwitch/Unity/archive/master.tar.gz"],
  build_file= "BUILD.unity",
  strip_prefix = "Unity-master",
  )

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
|- .bazelrc
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

# About

C++ lib used by https://github.com/blockspacer/flextool

## Before installation: Add conan remotes

To be able to add the list of dependency remotes please type the following command:

```bash
cmake -E time conan config install conan/remotes/
# OR:
# cmake -E time conan config install conan/remotes_disabled_ssl/
```

## Conan profile

Create clang9 profile:

```bash
[settings]
# We are building in Ubuntu Linux

os_build=Linux
os=Linux
arch_build=x86_64
arch=x86_64

compiler=clang
compiler.version=9
compiler.libcxx=libc++

llvm_9:build_type=Release

[options]
llvm_9_installer:compile_with_clang=True

[build_requires]
cmake_installer/3.15.5@conan/stable
```

## Before build (dependencies)

Create profile https://docs.conan.io/en/1.34/reference/profiles.html#examples

Re-build dependencies:

```bash
git clone https://github.com/blockspacer/conan_github_downloader.git ~/conan_github_downloader

cmake \
  -DSCRIPT_PATH="$PWD/get_conan_dependencies.cmake"\
  -DENABLE_CLING=FALSE\
  -DENABLE_LLVM_9=FALSE\
  -DEXTRA_CONAN_OPTS="--profile;clang9\
;-s;build_type=Debug\
;-s;cling_conan:build_type=Release\
;-s;llvm_9:build_type=Release\
;--build;missing" \
  -P ~/conan_github_downloader/conan_github_downloader.cmake
```

## Installation

Create clang_9_cxx11abi_llvm_libs profile:

```bash
[settings]
os=Linux
os_build=Linux
arch=x86_64
arch_build=x86_64
compiler=clang
compiler.version=9
compiler.libcxx=libstdc++11
build_type=Release

[options]
llvm_9_installer:compile_with_clang=True
llvm_9_installer:link_libcxx=False
llvm_9_installer:link_with_llvm_libs=True
llvm_9_installer:include_what_you_use=False
```

```bash
find . -type f -name "*_buildflags.h" -exec rm {} \;
find . -type f -name "*_buildflags.tmp" -exec rm {} \;

(rm -rf local_build || true)

mkdir local_build

cd local_build

export CONAN_REVISIONS_ENABLED=1
export CONAN_VERBOSE_TRACEBACK=1
export CONAN_PRINT_RUN_COMMANDS=1
export CONAN_LOGGING_LEVEL=10
export GIT_SSL_NO_VERIFY=true

# NOTE: use --build=missing if you got error `ERROR: Missing prebuilt package`
cmake -E time \
  conan install .. \
  --install-folder . \
  -s build_type=Debug \
  -s cling_conan:build_type=Release \
  -s llvm_9:build_type=Release \
  -o llvm_9_installer:compile_with_clang=True \
  -o llvm_9_installer:link_libcxx=False \
  -o llvm_9_installer:link_with_llvm_libs=True \
  --profile clang_9_cxx11abi_llvm_libs \
  -e flexlib:enable_tests=True \
  -o flexlib:shared=False \
  -o perfetto:is_hermetic_clang=False

(rm CMakeCache.txt || true)

# You can use `cmake --build . -- -j14` on second run.
cmake -E time \
  conan build .. --build-folder=.

cmake -E time \
  conan package --build-folder=. ..

cmake -E time \
  conan export-pkg .. conan/stable \
  -s build_type=Debug \
  -s cling_conan:build_type=Release \
  -s llvm_9:build_type=Release \
  -o llvm_9_installer:compile_with_clang=True \
  -o llvm_9_installer:link_libcxx=False \
  -o llvm_9_installer:link_with_llvm_libs=True \
  --profile clang_9_cxx11abi_llvm_libs

cmake -E time \
  conan test ../test_package flexlib/master@conan/stable \
  -s build_type=Debug \
  -s cling_conan:build_type=Release \
  -s llvm_9:build_type=Release \
  -o llvm_9_installer:compile_with_clang=True \
  -o llvm_9_installer:link_libcxx=False \
  -o llvm_9_installer:link_with_llvm_libs=True \
  --profile clang_9_cxx11abi_llvm_libs
```

## HOW TO BUILD WITH SANITIZERS ENABLED

See https://github.com/blockspacer/llvm_9_installer#how-to-use-with-sanitizers

## Disclaimer

That open source project based on the Google Chromium project.

This is not official Google product.

Portions Copyright (c) Google Inc.

See LICENSE files.

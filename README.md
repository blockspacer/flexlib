# About

C++ lib used by https://github.com/blockspacer/flextool

## Before installation: Add conan remotes

To be able to add the list of dependency remotes please type the following command:

```bash
cmake -E time conan config install conan/remotes/
# OR:
# cmake -E time conan config install conan/remotes_disabled_ssl/
```

## Before build (dependencies)

Create profile https://docs.conan.io/en/1.34/reference/profiles.html#examples

Re-build dependencies:

```bash
git clone https://github.com/blockspacer/conan_github_downloader.git ~/conan_github_downloader

cmake \
  -DSCRIPT_PATH="$PWD/get_conan_dependencies.cmake"\
  -DENABLE_CLING=TRUE\
  -DENABLE_LLVM=FALSE\
  -DENABLE_LLVM_INSTALLER=FALSE\
  -DEXTRA_CONAN_OPTS="--profile;clang12_compiler\
;-s;build_type=Debug\
;-s;cling_conan:build_type=Release\
;-s;llvm_12:build_type=Release\
;--build;missing" \
  -P ~/conan_github_downloader/conan_github_downloader.cmake
```

## Installation with cling enabled

Create clang12_compiler profile:

```bash
[settings]
# We are building in Ubuntu Linux

os_build=Linux
os=Linux
arch_build=x86_64
arch=x86_64

compiler=clang
compiler.version=12
compiler.libcxx=libstdc++11
compiler.cppstd=17

llvm_9:build_type=Release

[env]
CC=/usr/bin/clang-12
CXX=/usr/bin/clang++-12

[build_requires]
cmake_installer/3.15.5@conan/stable
```

```bash
find . -type f -name "*_buildflags.h" -exec rm {} \;
find . -type f -name "*_buildflags.tmp" -exec rm {} \;

(rm -rf local_build || true)

export CONAN_REVISIONS_ENABLED=1
export CONAN_VERBOSE_TRACEBACK=1
export CONAN_PRINT_RUN_COMMANDS=1
export CONAN_LOGGING_LEVEL=10

export PKG_NAME=flexlib/master@conan/stable

(CONAN_REVISIONS_ENABLED=1 \
    conan remove --force $PKG_NAME || true)

cmake -E time \
  conan install . \
  --install-folder local_build \
  -s build_type=Debug \
  -s cling_conan:build_type=Release \
  -s llvm_12:build_type=Release \
  -o openssl:shared=True \
  -e basis:enable_tests=True \
  -o chromium_base:shared=True \
  -e chromium_base:enable_tests=True \
  -o perfetto:is_hermetic_clang=False \
  --profile clang12_compiler \
  -e flexlib:enable_tests=True \
  -o flexlib:shared=False \
  -o perfetto:is_hermetic_clang=False \
  -o flexlib:enable_cling=True

(rm local_build/CMakeCache.txt || true)

cmake -E time \
  conan source . \
  --source-folder . \
  --install-folder local_build

# You can use `cmake --build . -- -j14` on second run.
cmake -E time \
  conan build . \
  --build-folder local_build \
  --source-folder . \
  --install-folder local_build

conan package . \
  --build-folder local_build \
  --package-folder local_build/package_dir \
  --source-folder . \
  --install-folder local_build

cmake -E time \
  conan export-pkg . conan/stable \
  --force \
  --package-folder local_build/package_dir \
  -s build_type=Debug \
  -s cling_conan:build_type=Release \
  -s llvm_12:build_type=Release \
  -o openssl:shared=True \
  -e basis:enable_tests=True \
  -o chromium_base:shared=True \
  -e chromium_base:enable_tests=True \
  -o perfetto:is_hermetic_clang=False \
  --profile clang12_compiler \
  -e flexlib:enable_tests=True \
  -o flexlib:shared=False \
  -o perfetto:is_hermetic_clang=False \
  -o flexlib:enable_cling=True

cmake -E time \
  conan test test_package \
  flexlib/master@conan/stable \
  -s build_type=Debug \
  -s cling_conan:build_type=Release \
  -s llvm_12:build_type=Release \
  -o openssl:shared=True \
  -e basis:enable_tests=True \
  -o chromium_base:shared=True \
  -e chromium_base:enable_tests=True \
  -o perfetto:is_hermetic_clang=False \
  --profile clang12_compiler \
  -e flexlib:enable_tests=True \
  -o flexlib:shared=False \
  -o perfetto:is_hermetic_clang=False \
  -o flexlib:enable_cling=True
```

## Installation without cling

Create clang12 profile:

```bash
[settings]
os_build=Linux
os=Linux
arch_build=x86_64
arch=x86_64
compiler=clang
compiler.version=12
compiler.libcxx=libc++
compiler.cppstd=17
llvm_12:build_type=Release
[options]
llvm_12_installer:compile_with_clang=True
llvm_12_installer:link_libcxx=False
llvm_12_installer:LLVM_PKG_NAME=llvm_12
llvm_12_installer:LLVM_PKG_VER=master-clang_12
llvm_12_installer:LLVM_PKG_CHANNEL=conan/stable
llvm_12_installer:LLVM_CONAN_CLANG_VER=12.0.1
flexlib:LLVM_PKG_NAME=llvm_12
flexlib:LLVM_PKG_VER=master-clang_12
flexlib:LLVM_PKG_CHANNEL=conan/stable
[build_requires]
cmake_installer/3.15.5@conan/stable
llvm_12/master-clang_12@conan/stable
llvm_12_installer/master-clang_12@conan/stable
```

Create clang12_cxx11abi_llvm_libs profile:

```bash
[settings]
os_build=Linux
os=Linux
arch_build=x86_64
arch=x86_64
compiler=clang
compiler.version=12
compiler.libcxx=libstdc++11
compiler.cppstd=17
llvm_12:build_type=Release
[options]
llvm_12_installer:compile_with_clang=True
llvm_12_installer:link_libcxx=False
llvm_12_installer:link_with_llvm_libs=True
llvm_12_installer:LLVM_PKG_NAME=llvm_12
llvm_12_installer:LLVM_PKG_VER=master-clang_12
llvm_12_installer:LLVM_PKG_CHANNEL=conan/stable
llvm_12_installer:LLVM_CONAN_CLANG_VER=12.0.1
flexlib:LLVM_PKG_NAME=llvm_12
flexlib:LLVM_PKG_VER=master-clang_12
flexlib:LLVM_PKG_CHANNEL=conan/stable
[build_requires]
cmake_installer/3.15.5@conan/stable
llvm_12/master-clang_12@conan/stable
llvm_12_installer/master-clang_12@conan/stable
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

export PKG_NAME=flexlib/master@conan/stable

(CONAN_REVISIONS_ENABLED=1 \
    conan remove --force $PKG_NAME || true)

# NOTE: use --build=missing if you got error `ERROR: Missing prebuilt package`
cmake -E time \
  conan install .. \
  --install-folder . \
  -s build_type=Debug \
  -s cling_conan:build_type=Release \
  -s llvm_12:build_type=Release \
  -o llvm_12_installer:compile_with_clang=True \
  -o llvm_12_installer:link_libcxx=False \
  -o llvm_12_installer:link_with_llvm_libs=True \
  --profile clang12_cxx11abi_llvm_libs \
  -o openssl:shared=True \
  -e basis:enable_tests=True \
  -o chromium_base:shared=True \
  -e chromium_base:enable_tests=True \
  -o perfetto:is_hermetic_clang=False \
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
  -s llvm_12:build_type=Release \
  -o llvm_12_installer:compile_with_clang=True \
  -o llvm_12_installer:link_libcxx=False \
  -o llvm_12_installer:link_with_llvm_libs=True \
  --profile clang12_cxx11abi_llvm_libs

cmake -E time \
  conan test ../test_package flexlib/master@conan/stable \
  -s build_type=Debug \
  -s cling_conan:build_type=Release \
  -s llvm_12:build_type=Release \
  -o llvm_12_installer:compile_with_clang=True \
  -o llvm_12_installer:link_libcxx=False \
  -o llvm_12_installer:link_with_llvm_libs=True \
  --profile clang12_cxx11abi_llvm_libs
```

## HOW TO BUILD WITH SANITIZERS ENABLED

See https://github.com/blockspacer/llvm_12_installer#how-to-use-with-sanitizers

## Disclaimer

That open source project based on the Google Chromium project.

This is not official Google product.

Portions Copyright (c) Google Inc.

See LICENSE files.

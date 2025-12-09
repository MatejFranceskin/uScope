#!/usr/bin/env python3
"""
Build libgphoto2 for Android arm64-v8a
Cross-compiles libgphoto2 using the Android NDK
"""

import os
import sys
import shutil
import subprocess
import urllib.request
import tarfile
from pathlib import Path

# Configuration
GPHOTO2_VERSION = "2.5.33"
ANDROID_ABI = "arm64-v8a"
ANDROID_PLATFORM = "30"

# Paths
SCRIPT_DIR = Path(__file__).parent
PREFIX = SCRIPT_DIR / "thirdparty" / "libgphoto2-android"

def get_ndk_root():
    """Get Android NDK root directory"""
    ndk_root = os.environ.get("ANDROID_NDK_ROOT")
    if not ndk_root:
        print("ERROR: ANDROID_NDK_ROOT not set")
        sys.exit(1)
    
    ndk_path = Path(ndk_root)
    if not ndk_path.exists():
        print(f"ERROR: NDK not found at {ndk_path}")
        sys.exit(1)
    
    return ndk_path

def setup_ndk_environment(ndk_root):
    """Set up NDK toolchain environment"""
    toolchain = ndk_root / "toolchains" / "llvm" / "prebuilt" / "linux-x86_64"
    target = "aarch64-linux-android"
    api = ANDROID_PLATFORM
    
    env = os.environ.copy()
    env.update({
        "TOOLCHAIN": str(toolchain),
        "TARGET": target,
        "API": api,
        "AR": str(toolchain / "bin" / "llvm-ar"),
        "CC": str(toolchain / "bin" / f"{target}{api}-clang"),
        "CXX": str(toolchain / "bin" / f"{target}{api}-clang++"),
        "AS": str(toolchain / "bin" / f"{target}{api}-clang"),
        "LD": str(toolchain / "bin" / "ld"),
        "RANLIB": str(toolchain / "bin" / "llvm-ranlib"),
        "STRIP": str(toolchain / "bin" / "llvm-strip"),
        "NM": str(toolchain / "bin" / "llvm-nm"),
    })
    
    return env

def download_libgphoto2():
    """Download and extract libgphoto2"""
    url = f"https://github.com/gphoto/libgphoto2/releases/download/v{GPHOTO2_VERSION}/libgphoto2-{GPHOTO2_VERSION}.tar.bz2"
    tar_path = Path(f"libgphoto2-{GPHOTO2_VERSION}.tar.bz2")
    extract_dir = Path(f"libgphoto2-{GPHOTO2_VERSION}")
    
    if not tar_path.exists():
        print(f"Downloading libgphoto2 {GPHOTO2_VERSION}...")
        urllib.request.urlretrieve(url, tar_path)
        print(f"Downloaded {tar_path}")
    
    if extract_dir.exists():
        print(f"Cleaning existing directory {extract_dir}...")
        shutil.rmtree(extract_dir)
    
    print("Extracting libgphoto2...")
    with tarfile.open(tar_path, 'r:bz2') as tar:
        tar.extractall()
    
    return extract_dir

def create_stub_ltdl(source_dir):
    """Create stub ltdl library that libgphoto2 can link against"""
    print("Creating stub ltdl library...")
    
    stub_dir = source_dir / "stub_ltdl"
    stub_dir.mkdir(exist_ok=True)
    
    # Create ltdl.h header
    ltdl_h = stub_dir / "ltdl.h"
    ltdl_h.write_text("""/* Stub ltdl.h - libgphoto2 doesn't use dynamic loading on Android */
#ifndef LTDL_H
#define LTDL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void* lt_dlhandle;
typedef void* lt_ptr;

int lt_dlinit(void);
int lt_dlexit(void);
lt_dlhandle lt_dlopen(const char *filename);
lt_dlhandle lt_dlopenext(const char *filename);
int lt_dlclose(lt_dlhandle handle);
void *lt_dlsym(lt_dlhandle handle, const char *symbol);
const char *lt_dlerror(void);
int lt_dlforeachfile(const char *search_path,
                      int (*func)(const char *filename, void *data),
                      void *data);
int lt_dladdsearchdir(const char *search_dir);

#ifdef __cplusplus
}
#endif

#endif /* LTDL_H */
""")
    
    # Create ltdl.c stub implementation
    ltdl_c = stub_dir / "ltdl.c"
    ltdl_c.write_text("""/* Stub ltdl implementation */
#include "ltdl.h"
#include <stddef.h>

int lt_dlinit(void) { return 0; }
int lt_dlexit(void) { return 0; }
lt_dlhandle lt_dlopen(const char *filename) { return NULL; }
lt_dlhandle lt_dlopenext(const char *filename) { return NULL; }
int lt_dlclose(lt_dlhandle handle) { return 0; }
void *lt_dlsym(lt_dlhandle handle, const char *symbol) { return NULL; }
const char *lt_dlerror(void) { return "ltdl disabled on Android"; }
int lt_dlforeachfile(const char *search_path,
                      int (*func)(const char *filename, void *data),
                      void *data) { return 0; }
int lt_dladdsearchdir(const char *search_dir) { return 0; }
""")
    
    return stub_dir

def build_stub_ltdl(stub_dir, env):
    """Compile stub ltdl library"""
    print("Compiling stub ltdl library...")
    
    cc = env["CC"]
    ar = env["AR"]
    api = env["API"]
    
    obj_file = stub_dir / "ltdl.o"
    lib_file = stub_dir / "libltdl.a"
    
    # Compile
    cmd = [
        cc,
        "-c",
        "-fPIC",
        f"-D__ANDROID_API__={api}",
        "-o", str(obj_file),
        str(stub_dir / "ltdl.c")
    ]
    
    result = subprocess.run(cmd, env=env, capture_output=True, text=True)
    if result.returncode != 0:
        print("ERROR: Failed to compile stub ltdl")
        print(result.stderr)
        sys.exit(1)
    
    # Create library
    cmd = [ar, "rcs", str(lib_file), str(obj_file)]
    result = subprocess.run(cmd, env=env, capture_output=True, text=True)
    if result.returncode != 0:
        print("ERROR: Failed to create stub ltdl library")
        print(result.stderr)
        sys.exit(1)
    
    print(f"Created {lib_file}")
    return lib_file

def configure_libgphoto2(source_dir, stub_dir, stub_lib, env):
    """Configure libgphoto2 for Android"""
    print(f"Configuring libgphoto2 for Android {ANDROID_ABI}...")
    
    target = env["TARGET"]
    api = env["API"]
    
    # Set LTDLINCL and LIBLTDL in environment for configure
    env["LTDLINCL"] = f"-I{stub_dir.absolute()}"
    env["LIBLTDL"] = str(stub_lib.absolute())
    
    cmd = [
        "./configure",
        f"--host={target}",
        f"--prefix={PREFIX}",
        "--disable-shared",
        "--enable-static",
        "--without-libusb-1.0",
        "--without-libusb",
        "--disable-nls",
        "--disable-serial",
        f"CFLAGS=-fPIC -D__ANDROID_API__={api}",
        f"CXXFLAGS=-fPIC -D__ANDROID_API__={api}",
    ]
    
    result = subprocess.run(
        cmd,
        cwd=source_dir,
        env=env,
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print("ERROR: Configure failed")
        print(result.stdout)
        print(result.stderr)
        sys.exit(1)
    
    print("Configure completed successfully")

def build_libgphoto2(source_dir, env):
    """Build libgphoto2"""
    print("Building libgphoto2...")
    
    nproc = os.cpu_count() or 1
    cmd = ["make", f"-j{nproc}"]
    
    result = subprocess.run(
        cmd,
        cwd=source_dir,
        env=env,
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print("ERROR: Build failed")
        print(result.stdout)
        print(result.stderr)
        sys.exit(1)
    
    print("Build completed successfully")

def install_libgphoto2(source_dir, env):
    """Install libgphoto2 to prefix"""
    print(f"Installing to {PREFIX}...")
    
    cmd = ["make", "install"]
    
    result = subprocess.run(
        cmd,
        cwd=source_dir,
        env=env,
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print("ERROR: Install failed")
        print(result.stdout)
        print(result.stderr)
        sys.exit(1)
    
    print("Install completed successfully")

def install_stub_ltdl(stub_lib):
    """Install stub ltdl library to PREFIX/lib"""
    print("Installing stub ltdl library...")
    
    lib_dir = PREFIX / "lib"
    lib_dir.mkdir(parents=True, exist_ok=True)
    
    dest = lib_dir / "libltdl.a"
    shutil.copy2(stub_lib, dest)
    
    print(f"Installed {dest}")

def create_pkgconfig():
    """Create pkg-config files"""
    print("Creating pkg-config files...")
    
    pc_dir = PREFIX / "lib" / "pkgconfig"
    pc_dir.mkdir(parents=True, exist_ok=True)
    
    libgphoto2_pc = pc_dir / "libgphoto2.pc"
    libgphoto2_pc.write_text(f"""prefix={PREFIX}
exec_prefix=${{prefix}}
libdir=${{exec_prefix}}/lib
includedir=${{prefix}}/include

Name: libgphoto2
Description: Library for digital camera access
Version: {GPHOTO2_VERSION}
Libs: -L${{libdir}} -lgphoto2 -lgphoto2_port
Cflags: -I${{includedir}}
""")
    
    print(f"Created {libgphoto2_pc}")

def main():
    """Main build function"""
    print("=" * 60)
    print(f"Building libgphoto2 {GPHOTO2_VERSION} for Android {ANDROID_ABI}")
    print("=" * 60)
    
    # Create output directory
    PREFIX.parent.mkdir(parents=True, exist_ok=True)
    
    # Get NDK and set up environment
    ndk_root = get_ndk_root()
    env = setup_ndk_environment(ndk_root)
    
    # Download and extract
    source_dir = download_libgphoto2()
    
    # Create and build stub ltdl
    stub_dir = create_stub_ltdl(source_dir)
    stub_lib = build_stub_ltdl(stub_dir, env)
    
    # Configure
    configure_libgphoto2(source_dir, stub_dir, stub_lib, env)
    
    # Build
    build_libgphoto2(source_dir, env)
    
    # Install
    install_libgphoto2(source_dir, env)
    
    # Install stub ltdl
    install_stub_ltdl(stub_lib)
    
    # Create pkg-config
    create_pkgconfig()
    
    print("\nSUCCESS!")
    print(f"Libraries installed to {PREFIX}")

if __name__ == "__main__":
    main()

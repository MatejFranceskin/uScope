#!/usr/bin/env python3
"""
Build libgphoto2 for Windows x64 using MSYS2/MinGW-w64
Downloads pre-built binaries from MSYS2 repository
"""
import sys
import subprocess
import urllib.request
import shutil
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
OUTPUT_DIR = SCRIPT_DIR / "thirdparty" / "libgphoto2-windows-x64"

# MSYS2 package URLs for libgphoto2 and dependencies
MSYS2_REPO = "https://repo.msys2.org/mingw/mingw64"
PACKAGES = [
    # Main library
    "mingw-w64-x86_64-libgphoto2-2.5.31-2-any.pkg.tar.zst",
    # Direct dependencies - verified available versions
    "mingw-w64-x86_64-libusb-1.0.27-1-any.pkg.tar.zst",
    "mingw-w64-x86_64-libexif-0.6.24-3-any.pkg.tar.zst",
    "mingw-w64-x86_64-libjpeg-turbo-3.0.4-1-any.pkg.tar.zst",
    "mingw-w64-x86_64-libgd-2.3.2-10-any.pkg.tar.zst",  # Note: libgd not gd
    "mingw-w64-x86_64-libxml2-2.15.1-2-any.pkg.tar.zst",
    "mingw-w64-x86_64-libltdl-2.5.4-3-any.pkg.tar.zst",  # Note: libltdl not libtool
]

def download_and_extract_package(package_name):
    """Download and extract MSYS2 package"""
    url = f"{MSYS2_REPO}/{package_name}"
    local_file = Path(package_name)
    
    if not local_file.exists():
        print(f"Downloading {package_name}...")
        try:
            urllib.request.urlretrieve(url, local_file)
        except Exception as e:
            print(f"ERROR: Failed to download {package_name}: {e}")
            sys.exit(1)
    
    print(f"Extracting {package_name}...")
    # Use tar to extract .zst files (available on Windows 10+)
    try:
        subprocess.run(["tar", "-xf", str(local_file)], check=True, 
                      stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except subprocess.CalledProcessError as e:
        print(f"ERROR: Failed to extract {package_name}")
        print(f"stderr: {e.stderr.decode() if e.stderr else 'none'}")
        sys.exit(1)
    except FileNotFoundError:
        print("ERROR: tar command not found. Please ensure you're running on Windows 10+ or install 7-Zip.")
        sys.exit(1)

def install_libraries():
    """Copy libraries and headers to output directory"""
    print(f"Installing libraries to {OUTPUT_DIR}...")
    
    if OUTPUT_DIR.exists():
        shutil.rmtree(OUTPUT_DIR)
    
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    # Copy from extracted mingw64 directory
    mingw64_dir = Path("mingw64")
    if not mingw64_dir.exists():
        print("ERROR: mingw64 directory not found after extraction")
        sys.exit(1)
    
    # Copy libraries
    lib_src = mingw64_dir / "lib"
    lib_dst = OUTPUT_DIR / "lib"
    if lib_src.exists():
        shutil.copytree(lib_src, lib_dst, dirs_exist_ok=True)
        print(f"  Copied libraries from {lib_src}")
    
    # Copy headers
    include_src = mingw64_dir / "include"
    include_dst = OUTPUT_DIR / "include"
    if include_src.exists():
        shutil.copytree(include_src, include_dst, dirs_exist_ok=True)
        print(f"  Copied headers from {include_src}")
    
    # Copy DLLs
    bin_src = mingw64_dir / "bin"
    bin_dst = OUTPUT_DIR / "bin"
    if bin_src.exists():
        bin_dst.mkdir(exist_ok=True)
        for dll in bin_src.glob("*.dll"):
            shutil.copy2(dll, bin_dst)
        print(f"  Copied DLLs from {bin_src}")
    
    print(f"Installation complete")

def cleanup_temp_files():
    """Clean up temporary extraction directory and downloaded packages"""
    print("Cleaning up temporary files...")
    
    mingw64_dir = Path("mingw64")
    if mingw64_dir.exists():
        shutil.rmtree(mingw64_dir)
    
    # Remove downloaded packages
    for package in PACKAGES:
        pkg_file = Path(package)
        if pkg_file.exists():
            pkg_file.unlink()

def main():
    """Main build function"""
    print("=" * 60)
    print("Setting up libgphoto2 for Windows x64 using MSYS2 packages")
    print("=" * 60)
    
    # Download and extract packages
    for package in PACKAGES:
        download_and_extract_package(package)
    
    # Install to output directory
    install_libraries()
    
    # Cleanup
    cleanup_temp_files()
    
    print("\nSUCCESS!")
    print(f"Libraries installed to {OUTPUT_DIR}")

if __name__ == "__main__":
    main()

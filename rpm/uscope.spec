Name:           uscope
Version:        0.1.0
Release:        1%{?dist}
Summary:        Cross-platform microscopy application for USB/UVC cameras

License:        See LICENSE file
URL:            https://github.com/MatejFranceskin/uScope
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.21
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtmultimedia-devel
BuildRequires:  qt6-qtsvg-devel
BuildRequires:  opencv-devel
BuildRequires:  tesseract-devel
BuildRequires:  gstreamer1-devel

Requires:       qt6-qtbase
Requires:       qt6-qtmultimedia
Requires:       qt6-qtsvg
Requires:       opencv
Requires:       tesseract
Requires:       gstreamer1

%description
uScope is an open-source microscopy application for educational institutions,
hobbyists, and small laboratories using USB/UVC-compliant cameras.

Features include live camera preview, manual controls, calibration and
measurement tools, video recording, automated object detection, image
stitching, extended depth of focus, professional export formats, RTSP
streaming, and iNaturalist integration.

%prep
%setup -q

%build
%cmake -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%files
%{_bindir}/uScope
%{_datadir}/applications/uScope.desktop
%{_datadir}/icons/hicolor/scalable/apps/uScope.svg

%changelog
* Fri Dec 06 2024 uScope Team <info@uscope.org> - 0.1.0-1
- Initial release
- Core features: camera preview, image capture, basic controls
- Tile-based UI architecture implementation

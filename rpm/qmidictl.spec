#
# spec file for package qmidictl
#
# Copyright (C) 2010-2022, rncbc aka Rui Nuno Capela. All rights reserved.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

%define name    qmidictl
%define version	0.9.9
%define release	53.1

%define _prefix	/usr

%if %{defined fedora}
%define debug_package %{nil}
%endif

%if 0%{?fedora_version} >= 34 || 0%{?suse_version} > 1500 || ( 0%{?sle_version} == 150200 && 0%{?is_opensuse} )
%define qt_major_version  6
%else
%define qt_major_version  5
%endif

Summary:	A MIDI Remote Controller via UDP/IP Multicast
Name:		%{name}
Version:	%{version}
Release:	%{release}
License:	GPL-2.0+
Group:		Productivity/Multimedia/Sound/Midi
Source0:	%{name}-%{version}.tar.gz
URL:		http://qmidictl.sourceforge.net/
Packager:	rncbc.org

BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-buildroot

BuildRequires:	coreutils
BuildRequires:	pkgconfig
BuildRequires:	glibc-devel
BuildRequires:	cmake >= 3.15
%if 0%{?sle_version} >= 150200 && 0%{?is_opensuse}
BuildRequires:	gcc10 >= 10
BuildRequires:	gcc10-c++ >= 10
%define _GCC	/usr/bin/gcc-10
%define _GXX	/usr/bin/g++-10
%else
BuildRequires:	gcc >= 10
BuildRequires:	gcc-c++ >= 10
%define _GCC	/usr/bin/gcc
%define _GXX	/usr/bin/g++
%endif
%if 0%{qt_major_version} == 6
%if 0%{?sle_version} == 150200 && 0%{?is_opensuse}
BuildRequires:	qtbase6-static >= 6.1
BuildRequires:	qttools6-static
BuildRequires:	qttranslations6-static
BuildRequires:	qtsvg6-static
#BuildRequires:	qtwayland6-static
%else
BuildRequires:	cmake(Qt6LinguistTools)
BuildRequires:	pkgconfig(Qt6Core)
BuildRequires:	pkgconfig(Qt6Gui)
BuildRequires:	pkgconfig(Qt6Widgets)
BuildRequires:	pkgconfig(Qt6Svg)
BuildRequires:	pkgconfig(Qt6Network)
%endif
%else
BuildRequires:	cmake(Qt5LinguistTools)
BuildRequires:	pkgconfig(Qt5Core)
BuildRequires:	pkgconfig(Qt5Gui)
BuildRequires:	pkgconfig(Qt5Widgets)
BuildRequires:	pkgconfig(Qt5Svg)
BuildRequires:	pkgconfig(Qt5Network)
%endif

%description
QmidiCtl is a MIDI remote controller application that sends MIDI data
over the network, using UDP/IP multicast. Inspired by multimidicast
(http://llg.cubic.org/tools) and designed to be compatible with ipMIDI
for Windows (http://nerds.de).

%prep
%setup -q

%build
%if 0%{?sle_version} == 150200 && 0%{?is_opensuse}
source /opt/qt6.5-static/bin/qt6.5-static-env.sh
%endif
CXX=%{_GXX} CC=%{_GCC} \
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -Wno-dev -B build
cmake --build build %{?_smp_mflags}

%install
DESTDIR="%{buildroot}" \
cmake --install build

%clean
[ -d "%{buildroot}" -a "%{buildroot}" != "/" ] && %__rm -rf "%{buildroot}"

%files
%defattr(-,root,root)
%license LICENSE
%doc README ChangeLog
#dir %{_datadir}/applications
%dir %{_datadir}/icons/hicolor
%dir %{_datadir}/icons/hicolor/32x32
%dir %{_datadir}/icons/hicolor/32x32/apps
%dir %{_datadir}/icons/hicolor/scalable
%dir %{_datadir}/icons/hicolor/scalable/apps
%dir %{_datadir}/metainfo
#dir %{_datadir}/man
#dir %{_datadir}/man/man1
#dir %{_datadir}/man/fr
#dir %{_datadir}/man/fr/man1
%{_bindir}/%{name}
%{_datadir}/applications/org.rncbc.%{name}.desktop
%{_datadir}/icons/hicolor/32x32/apps/org.rncbc.%{name}.png
%{_datadir}/icons/hicolor/scalable/apps/org.rncbc.%{name}.svg
%{_datadir}/metainfo/org.rncbc.%{name}.metainfo.xml
%{_datadir}/man/man1/%{name}.1.gz
%{_datadir}/man/fr/man1/%{name}.1.gz

%changelog
* Thu Mar 23 2023 Rui Nuno Capela <rncbc@rncbc.org> 0.9.9
- An Early-Spring'23 Release.
* Wed Dec 28 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.8
- An End-of-Year'22 Release.
* Mon Oct  3 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.7
- An Early-Autumn'22 Release.
* Sat Apr  2 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.6
- A Spring'22 Release.
* Sun Jan  9 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.5
- A Winter'22 Release.
* Sun Jul  4 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.4
- Early-Summer'21 release.
* Tue May 11 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.3
- Spring'21 release.
* Sun Mar 14 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.2
- End-of-Winter'21 release.
* Sun Feb  7 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.1
- Winter'21 release.
* Thu Dec 17 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.9.0
- Winter'20 release.
* Fri Jul 31 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.6.3
- Summer'20 release.
* Tue Mar 24 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.6.2
- Spring'20 release.
* Sun Dec 22 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.6.1
- Winter'19 release.
* Thu Oct 17 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.6.0
- Autumn'19 release.
* Fri Jul 12 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.5.5
- Summer'19 release.
* Thu Apr 11 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.5.4
- Spring-Break'19 release.
* Tue Jan  8 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.5.3
- Early Winter'19 release.
* Sat Jun 30 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.5.2
- Early Summer'18 release.
* Thu Jan 11 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.5.1
- Early Winter'18 release.
* Sat Dec 16 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.5.0
- End of Autumn'17 release.
* Thu Apr 27 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.4.3
- Pre-LAC2017 release frenzy.
* Mon Nov 14 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.4.2
- A Fall'16 release.
* Wed Sep 14 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.4.1
- End of Summer'16 release.
* Tue Apr  5 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.4.0
- Spring'16 release frenzy.
* Mon Sep 21 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.3.0
- Summer'15 release frenzy.
* Mon Mar 23 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.2.0
- Pre-LAC2015 pre-season release.
* Tue Dec 31 2013 Rui Nuno Capela <rncbc@rncbc.org> 0.1.1
- A fifth of a Jubilee release.
* Fri Sep  3 2010 Rui Nuno Capela <rncbc@rncbc.org> 0.1.0
- Second public release.
* Mon May 24 2010 Rui Nuno Capela <rncbc@rncbc.org>
- Standard desktop icon fixing (Fremantle/Hildon).
* Mon Apr 26 2010 Rui Nuno Capela <rncbc@rncbc.org> 0.0.9
- MIDI controller assignment configuration debut.
* Thu Mar 11 2010 Rui Nuno Capela <rncbc@rncbc.org> 0.0.1
- Attempt to comply with maemo.org extras-testing promotion.
* Sun Mar  7 2010 Rui Nuno Capela <rncbc@rncbc.org> 0.0.1
- Created initial qmidictl.spec

Name: libqt5sparql
Version: 0.3.0
Release: 1
Summary: Library for accessing RDF stores
License: LGPLv2 or GPLv3 or LGPLv2 with Nokia Qt LGPL Exception v1.1
URL:     https://github.com/sailfishos/libqtsparql
Source0: %{name}-%{version}.tar.gz
BuildRequires: doxygen
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(tracker-sparql-3.0)

%description
Library for accessing RDF stores.

%package declarative
Summary: QML plugin for QtSparql
Requires: %{name} >= %{version}

%description declarative
%{summary}.

%package devel
Summary:  Qt Sparql development files
Requires: %{name} >= %{version}

%description devel
Library for accessing RDF stores.
documentation

%package tests
Summary:  QtSparql testsuite 
Requires: %{name} >= %{version}
Requires: libqt5sparql-endpoint >= %{version}
Requires: libqt5sparql-tracker-direct >= %{version} 

%description tests
Tests for libqtsparql.

%package endpoint
Summary:  Endpoint driver package for %{name}

%description endpoint 
Endpoint driver for QtSparql.


%package tracker-direct
Summary:  Tracker driver package for %{name}
Requires: %{name} >= %{version}

%description tracker-direct
Tracker direct access driver for QtSparql.

%prep
%setup -q -n %{name}-%{version}

%build
export QT_SELECT=5
./configure -prefix /usr -lib %{_lib}
%qmake5
%make_build
make doc

%install
%qmake_install
sed -i 's,-L/home/abuild/[^ ]*,,' %{buildroot}/%{_libdir}/pkgconfig/*.pc

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%post devel -p /sbin/ldconfig

%postun devel -p /sbin/ldconfig

%files
%license LICENSE.LGPL
%{_libdir}/libQt5Sparql.so.*

%files declarative
%{_libdir}/qt5/qml/QtSparql

%files devel
%{_libdir}/libQt5Sparql.so
%{_libdir}/pkgconfig/*
%{_includedir}/Qt5Sparql/*
%{_libdir}/libQt5Sparql.prl
%{_datadir}/qt5/mkspecs/features/*
%doc %{_datadir}/doc/libqt5sparql-doc/html/*

%files tests
%{_libdir}/libqt5sparql-tests/*
%{_datadir}/libqt5sparql-tests/*

%files endpoint
%{_libdir}/qt5/plugins/sparqldrivers/libqsparqlendpoint.so

%files tracker-direct
%{_libdir}/qt5/plugins/sparqldrivers/libqsparqltrackerdirect.so

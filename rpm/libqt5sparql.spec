Name: libqt5sparql
Version: 0.2.13
Release: 1
Summary: Library for accessing RDF stores
Group:   System/Libraries
License: LGPLv2
URL:     https://github.com/nemomobile/libqtsparql
Source0: %{name}-%{version}.tar.gz
BuildRequires: doxygen
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(tracker-sparql-1.0)

%description
Library for accessing RDF stores.

%package devel
Summary:  Qt Sparql development files
Group:    Development/Libraries
Requires:  %{name} >= %{version}

%description devel
Library for accessing RDF stores.
documentation

%package tests
Summary:  QtSparql testsuite 
Group:    System/X11
Requires:  %{name} >= %{version}
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5Gui)
Requires: libqt5sparql-endpoint >= %{version}
Requires: libqt5sparql-tracker >= %{version}
Requires: libqt5sparql-tracker-direct >= %{version} 

%description tests
Tests for libqtsparql.

%package endpoint
Summary:  Endpoint driver package for %{name}
Group:    Libraries
BuildRequires: pkgconfig(Qt5Xml)

%description endpoint 
Endpoint driver for QtSparql.

%package tracker 
Summary:  Tracker driver package for %{name}
Group:    Libraries
Requires:  %{name} >= %{version}
Requires: tracker >= 0.10.0


%description tracker
Tracker driver for QtSparql.

%package tracker-direct
Summary:  Tracker driver package for %{name}
Group:    Libraries
Requires:  %{name} >= %{version}
Requires: tracker >= 0.10.0

%description tracker-direct
Tracker direct access driver for QtSparql.

%prep
%setup -q -n %{name}-%{version}

%build
export QT_SELECT=5
./configure -prefix /usr
%qmake5
make %{?jobs:-j%jobs}
make doc

%clean
rm -rf $RPM_BUILD_ROOT

%install
%qmake_install
sed -i 's,-L/home/abuild/[^ ]*,,' %{buildroot}/%{_libdir}/pkgconfig/*.pc

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%post devel -p /sbin/ldconfig

%postun devel -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/libQt5Sparql.so.*
%{_libdir}/qt5/qml/QtSparql/qmldir
%{_libdir}/qt5/qml/QtSparql/libsparqlresultslist.so
%{_libdir}/qt5/qml/QtSparql/libsparqlconnection.so
%{_libdir}/qt5/qml/QtSparql/libsparqllistmodel.so

%files devel
%defattr(-,root,root,-)
%{_libdir}/libQt5Sparql.so
%{_libdir}/pkgconfig/*
%{_includedir}/Qt5Sparql/*
%{_libdir}/libQt5Sparql.prl
%{_datadir}/qt5/mkspecs/features/*
%doc %{_datadir}/doc/libqt5sparql-doc/html/*

%files tests
%defattr(-,root,root,-)
%{_libdir}/libqt5sparql-tests/*
%{_datadir}/libqt5sparql-tests/*

%files endpoint
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/sparqldrivers/libqsparqlendpoint.so

%files tracker
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/sparqldrivers/libqsparqltracker.so

%files tracker-direct
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/sparqldrivers/libqsparqltrackerdirect.so

Name: libqtsparql
Version: 0.2.13
Release: 1
Summary: Library for accessing RDF stores
Group:   System/Libraries
License: LGPLv2
URL:     https://github.com/nemomobile/libqtsparql
Source0: %{name}-%{version}.tar.gz
BuildRequires: doxygen
BuildRequires: pkgconfig(QtCore)
BuildRequires: tracker-devel

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
Requires: libqtsparql-tracker >= %{version}
Requires: libqtsparql-tracker-direct >= %{version} 

%description tests
Tests for libqtsparql.

%package endpoint
Summary:  Endpoint driver package for %{name}
Group:    Libraries

%description endpoint 
Endpoint driver for QtSparql.

%package tracker 
Summary:  Tracker driver package for %{name}
Group:    Libraries
Requires: tracker >= 0.10.0

%description tracker
Tracker driver for QtSparql.

%package tracker-direct
Summary:  Tracker driver package for %{name}
Group:    Libraries
Requires: tracker >= 0.10.0

%description tracker-direct
Tracker direct access driver for QtSparql.

%prep
%setup -q -n %{name}-%{version}

%build
./configure -prefix /usr
qmake 
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
%{_libdir}/libQtSparql.so.*
%{_libdir}/qt4/imports/QtSparql/qmldir
%{_libdir}/qt4/imports/QtSparql/libsparqlresultslist.so
%{_libdir}/qt4/imports/QtSparql/libsparqlconnection.so
%{_libdir}/qt4/imports/QtSparql/libsparqllistmodel.so

%files devel
%defattr(-,root,root,-)
%{_libdir}/libQtSparql.so
%{_libdir}/pkgconfig/*
%{_includedir}/QtSparql/*
%{_libdir}/libQtSparql.prl
%{_datadir}/qt4/mkspecs/features/*
%doc %{_datadir}/doc/libqtsparql-doc/html/*

%files tests
%defattr(-,root,root,-)
%{_libdir}/libqtsparql-tests/*
%{_datadir}/libqtsparql-tests/*

%files endpoint
%defattr(-,root,root,-)
%{_libdir}/qt4/plugins/sparqldrivers/libqsparqlendpoint.so

%files tracker
%defattr(-,root,root,-)
%{_libdir}/qt4/plugins/sparqldrivers/libqsparqltracker.so

%files tracker-direct
%defattr(-,root,root,-)
%{_libdir}/qt4/plugins/sparqldrivers/libqsparqltrackerdirect.so

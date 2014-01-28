#sbs-git:slp/apps/c/calculator calculator 0.1.3 3ce35911eff2a8f151a092f346ab7239d7d0658e
%define PREFIX /usr/apps/org.tizen.calculator
Name: org.tizen.calculator
Version:    0.1.31
Release:    1
Summary: SLP Calculator application
URL: http://slp-source.sec.samsung.net
Source: %{name}-%{version}.tar.gz
Source1001: org.tizen.calculator.manifest
License: TIZEN
Group: tizen/Application
BuildRequires: cmake
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(embryo)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(appcore-common)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(feedback)
BuildRequires: gettext-tools
BuildRequires: edje-bin, embryo-bin
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(vconf)
%description
SLP Calculator application

%prep
%setup -q
cp %{SOURCE1001} .

%build

LDFLAGS+="-Wl,--rpath=%{PREFIX}/lib -Wl,--as-needed -Wl,--hash-style=both"; export LDFLAGS

cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%files
%manifest org.tizen.calculator.manifest
%defattr(-,root,root,-)
%attr(-,inhouse,inhouse)
/usr/apps/org.tizen.calculator/bin/calculator
/usr/apps/org.tizen.calculator/res/edje/calculator.edj
/usr/apps/org.tizen.calculator/res/edje/calculator_theme.edj
/usr/share/icons/default/small/org.tizen.calculator.png
/usr/apps/org.tizen.calculator/res/icons/org.tizen.calculator.png
/usr/apps/org.tizen.calculator/res/locale/*
/usr/share/packages/org.tizen.calculator.xml
/usr/share/process-info/calculator.ini


#sbs-git:slp/apps/c/calculator calculator 0.1.3 3ce35911eff2a8f151a092f346ab7239d7d0658e
%define PREFIX /opt/apps/org.tizen.calculator
Name: org.tizen.calculator
Version:    0.1.22
Release:    1
Summary: SLP Calculator application
URL: http://slp-source.sec.samsung.net
Source: %{name}-%{version}.tar.gz
License: TIZEN
Group: tizen/Application
BuildRequires: cmake
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(embryo)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(utilX)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(appcore-common)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(svi)
BuildRequires: pkgconfig(capi-appfw-application)

BuildRequires: gettext-tools
BuildRequires: edje-bin, embryo-bin

%description
SLP Calculator application

%prep
%setup -q

%build

LDFLAGS+="-Wl,--rpath=%{PREFIX}/lib -Wl,--as-needed -Wl,--hash-style=both"; export LDFLAGS

cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%files
%defattr(-,root,root,-)
%attr(-,inhouse,inhouse)
/opt/apps/org.tizen.calculator/bin/calculator
/opt/apps/org.tizen.calculator/res/edje/calculator.edj
/opt/apps/org.tizen.calculator/res/edje/calculator_theme.edj
/opt/share/icons/default/small/org.tizen.calculator.png
/opt/apps/org.tizen.calculator/res/icons/org.tizen.calculator.png
/opt/apps/org.tizen.calculator/res/locale/*
/opt/share/packages/org.tizen.calculator.xml
/opt/share/process-info/calculator.ini


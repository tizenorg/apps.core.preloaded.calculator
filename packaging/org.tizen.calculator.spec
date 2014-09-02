%define PREFIX /usr/apps/org.tizen.calculator
Name: org.tizen.calculator
Version:    0.1.31
Release:    0
Summary: SLP Calculator application
URL: http://slp-source.sec.samsung.net
Source: %{name}-%{version}.tar.gz
Source1001: org.tizen.calculator.manifest
License: FLORA-1.1
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

%__make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

%files
%manifest org.tizen.calculator.manifest
%defattr(-,root,root,-)
%attr(-,inhouse,inhouse)
%license LICENSE
/usr/apps/org.tizen.calculator/bin/calculator
/usr/apps/org.tizen.calculator/res/edje/calculator.edj
/usr/apps/org.tizen.calculator/res/edje/calculator_theme.edj
/usr/share/icons/default/small/org.tizen.calculator.png
/usr/apps/org.tizen.calculator/res/icons/org.tizen.calculator.png
/usr/apps/org.tizen.calculator/res/locale/*
/usr/share/packages/org.tizen.calculator.xml
/usr/share/process-info/calculator.ini

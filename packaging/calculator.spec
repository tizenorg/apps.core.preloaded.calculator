%define _app_prefix /opt/apps/org.tizen.calculator
Name: calculator
Version:	0.1.3
Release: 1
Summary: Calculator application
URL: http://slp-source.sec.samsung.net
Source: %{name}-%{version}.tar.gz
License: Samsung Proprietary License
Group: Applications
BuildRequires: cmake
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(embryo)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(utilX)
#BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(appcore-common)
BuildRequires: pkgconfig(UI-idlecapture)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(svi)
BuildRequires: gettext-tools
BuildRequires: edje-bin, embryo-bin

%description
SLP Calculator application

%prep
%setup -q

%build

LDFLAGS+="-Wl,--rpath=%{_app_prefix}/lib -Wl,--as-needed -Wl,--hash-style=both"; export LDFLAGS

cmake . -DCMAKE_INSTALL_PREFIX=%{_app_prefix}

make %{?jobs:-j%jobs}

%post
chown -R 5000:5000 %{_app_prefix}/data

%install
%make_install


mkdir -p %{buildroot}%{_app_prefix}/data

%find_lang calculator
%files -f %{name}.lang
%{_app_prefix}/data
/opt/apps/org.tizen.calculator/bin/calculator
/opt/apps/org.tizen.calculator/res/edje/calculator.edj
/opt/apps/org.tizen.calculator/res/edje/calculator_theme.edj
/opt/apps/org.tizen.calculator/res/icons/default/small/org.tizen.calculator.png
/opt/share/applications/org.tizen.calculator.desktop
/opt/share/process-info/calculator.ini

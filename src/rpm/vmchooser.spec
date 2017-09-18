Name:		vmchooser
Version:	0.3
Release:	1%{?dist}
Summary:	vmchooser

Group:		application
License:	GPL
URL:		https://github.com/bingch/vmchooser
Source0:	vmchooser-%{version}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:	gtk3-devel,intltool,openldap-devel,glib2-devel
Requires:	intltool,openldap

%description
vmchooser allow you to select a vm name from a drop list, then spew out authenticated user and selected vm name for other program to proceed.

%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
/usr/bin/vmchooser
/usr/share/vmchooser/ui/logo.jpg
/usr/share/vmchooser/ui/vmchooser.conf
/usr/share/vmchooser/ui/vmchooser.ui

%doc
/usr/doc/vmchooser/AUTHORS
/usr/doc/vmchooser/COPYING
/usr/doc/vmchooser/ChangeLog
/usr/doc/vmchooser/INSTALL
/usr/doc/vmchooser/NEWS
/usr/doc/vmchooser/README


%changelog


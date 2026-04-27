Summary: Locale and ISO 2022 support for Unicode terminals
# $XTermId: luit.spec,v 1.78 2025/09/12 23:29:05 tom Exp $
%global real luit
Name: b%{real}
Version: 2.0
Release: 20250912
License: MIT
Group: Applications/System
URL: https://invisible-island.net/%{real}/
Source0: %{real}-%{release}.tgz
Provides: %{real}

%description
Luit is a filter that can be run between an arbitrary application and a
UTF-8 terminal emulator.  It will convert application output  from  the
locale's  encoding  into  UTF-8,  and convert terminal input from UTF-8
into the locale's encoding.

This package installs an alternative binary "bluit", and adds a symbolic link
for "xterm-filter".

%prep

%define debug_package %{nil}

%setup -q -n %{real}-%{release}

%build

INSTALL_PROGRAM='${INSTALL}' \
%configure \
  --program-prefix=b \
  --target %{_target_platform} \
  --prefix=%{_prefix} \
  --bindir=%{_bindir} \
  --libdir=%{_libdir} \
  --mandir=%{_mandir}

make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install                    DESTDIR=$RPM_BUILD_ROOT
( cd $RPM_BUILD_ROOT%{_bindir}      && ln -s %{name} xterm-filter )
( cd $RPM_BUILD_ROOT%{_mandir}/man1 && ln -s %{name}.1 xterm-filter.1 )

strip $RPM_BUILD_ROOT%{_bindir}/%{name}

%files
%defattr(-,root,root)
%doc %{real}.log.html
%{_prefix}/bin/%{name}
%{_prefix}/bin/xterm-filter
%{_mandir}/man1/*

%changelog
# each patch should add its ChangeLog entries here

* Fri Sep 12 2025 Thomas E. Dickey
- testing luit 2.0-20250912

* Tue Jan 11 2022 Thomas Dickey <dickey@his.com>
- update URL, install package as "bluit"

* Sat Jun 05 2010 Thomas Dickey <dickey@his.com>
- Fixes/improvements for FreeBSD and Solaris

* Mon May 31 2010 Thomas Dickey <dickey@his.com>
- initial version

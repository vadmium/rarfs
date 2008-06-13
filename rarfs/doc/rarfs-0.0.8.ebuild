# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils

DESCRIPTION="Fuse-filesystem for not compressed rar archives."
SRC_URI="mirror://sourceforge/rarfs/${P}.tar.gz"
HOMEPAGE="http://rarfs.wiki.sourceforge.net/"
LICENSE="GPL-2"
DEPEND=">=sys-fs/fuse-2.5.0"
KEYWORDS="ppc64 X86"
SLOT="0"
IUSE=""

src_unpack()
{
	unpack ${A}
	cd "${S}"
	epatch "${FILESDIR}"/rarfs.patch
}

src_install ()
{
	make DESTDIR=${D} install || die "make install failed"
	dodoc README NEWS ChangeLog AUTHORS
}


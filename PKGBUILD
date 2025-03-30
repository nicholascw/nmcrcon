# Maintainer: Nicholas Wang <me at nicho1as dot wang>

pkgname=nmcrcon-git
pkgver=r11.e0047a6
pkgrel=1
pkgdesc=" Yet another Minecraft RCON client that isn't against humanity and actually works."
url="https://github.com/nicholascw/nmcrcon"
arch=('any')
license=('GPL-3.0-only')
depends=('libbestline')
makedepends=('git')
source=("nmcrcon::git+https://github.com/nicholascw/nmcrcon.git")
sha256sums=('SKIP')

pkgver () {
  cd "$srcdir/nmcrcon"
  ( set -o pipefail
    git describe --long 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' ||
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
  )
}

build () {
  cd "$srcdir/nmcrcon"
  mkdir obj
  make nmcrcon
}

package () {
  cd "$srcdir/nmcrcon"
  install -Dm755 nmcrcon -t "${pkgdir}/usr/bin/"
  install -Dm644 LICENSE -t "${pkgdir}/usr/share/licenses/nmcrcon/"
}

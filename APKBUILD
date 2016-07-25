# Maintainer: Peter Mount <peter@retep.org>

pkgname="rabbit-logger"
pkgver="0.1"
pkgrel="2"
pkgdesc="Area51 RabbitMQ Logger & Message Archiver"
arch="x86_64"
url="https://area51.onl/"
license="ASL 2.0"
source=""
subpackages="$pkgname-dev"
depends="libarea51 json-c curl"
depends_dev="libarea51-dev json-c-dev curl-dev"
#triggers="$pkgname-bin.trigger=/lib:/usr/lib:/usr/glibc-compat/lib"

builddeps() {
  sudo apk add $depends $depends_dev
}

package() {
  autoconf
  ./configure
  make clean
  make -j1
  mkdir -p "$pkgdir/usr/bin"
  cp -rp build/package/usr/bin/* "$pkgdir/usr/bin"
  mkdir -p "$pkgdir/usr/lib"
  cp -rp build/package/usr/lib/* "$pkgdir/usr/lib"
}

dev() {
  depends="$pkgname $depends_dev"
  mkdir -p "$subpkgdir/usr/include"
  cp -rp build/package/usr/include/* "$subpkgdir/usr/include"
}

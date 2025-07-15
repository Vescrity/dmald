# Maintainer: Vescrity
pkgname=dmald
pkgver="0.1"
pkgrel=1
pkgdesc='Simple daemon to set value of CPU DMA Latency'
groups=('realtime')
arch=( 'x86_64' 'aarch64' )
depends=(
    glibc
)
optdepends=(
)
makedepends=(
    gcc
    tar
)
source=('https://github.com/Vescrity/dmald/archive/refs/tags/v0.1.tar.gz')
sha256sums=('72ab7e56e13ea06680db5878a5afa82491400b89ff4593bec227fb4d9cc4133d')  
build() {
    cd "$srcdir/dmald-$pkgver"
    ./build.sh
}
package() {
    cd "$srcdir/dmald-$pkgver/build"
    install -d "${pkgdir}/usr/bin"
    install -Dm755 "dmald" "${pkgdir}/usr/bin/dmald"
}


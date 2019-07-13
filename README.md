# i1i2i3 phone

eeicのi3実験の成果物

## 準備

make, cmake, boost(1.66以上?)をインストール

```
$ sudo apt update
$ sudo apt install build-essential cmake
```

boostのインストールはhttps://boostjp.github.io/howtobuild.htmlなどを参考

## ビルド

このディレクトリで
```
$ mkdir build && cd build
$ cmake ..
$ make -j4
```

# rv32ui emulator
なんちゃってRISCVエミュレータ．
基本的なユーザー命令だけ実装しています．

  - 開始アドレス：0x8000_0000　と仮定
  - メモリ空間：256KB

## 公式資料
- https://riscv.org/specifications/
  - Unprivileged Specification
  - Privileged ISA Specification

## 参考資料
- https://msyksphinz-self.github.io/riscv-isadoc/html/index.html
- https://rv8.io/isa.html


## How to Build
Boost::formatを使用しています(Boostライブラリ　https://www.boost.org/)
cmake を使っていますが，コンパイル時にboostのincudeディレクトリパスを指定しているだけです．
```
git clone https://github.com/ORYZAPAO/riscv-emu.git
cd risc-emu/cpp
cmake .
make 
```

## Regression Test
```
./REGRESSION_test.sh
```

## テストデータ
 - ../testdat/isa/
  - テストデータを32bit版のRISCVツールチェーンでコンパイルして，テキストに変換したもの
    - https://github.com/riscv/riscv-tests
    - https://github.com/riscv/riscv-gnu-toolchain

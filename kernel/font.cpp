/**
 * @file font.cpp
 * 
 * フォント描画のプログラムを書いたファイル
*/

#include "font.hpp"

// 横8ピクセル、縦16ピクセルのフォント
// 黒く塗る箇所を1, 塗らない箇所を0
const uint8_t kFontA[16] = {
  0b00000000, //
  0b00011000, //    **
  0b00011000, //    **
  0b00011000, //    **
  0b00011000, //    **
  0b00100100, //   *  *
  0b00100100, //   *  *
  0b00100100, //   *  *
  0b00100100, //   *  *
  0b01111110, //  ******
  0b01000010, //  *    *
  0b01000010, //  *    *
  0b01000010, //  *    *
  0b11100111, // ***  ***
  0b00000000, //
  0b00000000, //
};

/*
 * 作成したフォントデータを使用し1文字を描画する
 * 引数にASCIIコードを受け取り、指定した位置に描画
*/
void WriteAscii(PixelWriter& writer, int x, int y, char c, const PixelColor& color) {
  if (c != 'A') {
    return;
  }
  for (int dy = 0; dy < 16; ++dy) { // 縦方向
    for (int dx = 0; dx < 8; ++dx) { // 横方向
      if ((kFontA[dy] << dx) & 0x80u) { // ビットが1かの判定
        writer.Write(x + dx, y + dy, color);
      }
    }
  }
}
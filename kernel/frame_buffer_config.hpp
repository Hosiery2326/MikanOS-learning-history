#pragma once

#include <stdint.h>

enum PixelFormat {
  kPixelRGBResv8BitPerColor,
  kPixelBGRResv8BitPerColor,
};

/*
 * ピクセル描画に必要な情報をまとめたもの
*/
struct FrameBufferConfig {
  uint8_t* frame_buffer; // フレームバッファ領域へのポインタ
  uint32_t pixels_per_scan_line; // フレームバッファの余白を含めた横方向のピクセル数
  uint32_t horizontal_resolution; // 水平と垂直の解像度
  uint32_t vertical_resolution; // ピクセルのデータ形式
  enum PixelFormat pixel_format;  
};
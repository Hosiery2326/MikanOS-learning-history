#pragma once

#include "frame_buffer_config.hpp"

struct PixelColor {
  uint8_t r, g, b;
};

/*
 * ピクセル描画インタフェース
*/
class PixelWriter {
  public:
    // コンストラクタ
    PixelWriter(const FrameBufferConfig& config) : config_{config} {
    }
    // デストラクタ(インスタンスを破棄するのに使用)
    virtual ~PixelWriter() = default;
    // ピクセルを描画する純粋仮想関数(インタフェース)
    virtual void Write(int x, int y, const PixelColor& c) = 0;
  
  protected:
    uint8_t* PixelAt(int x, int y) {
      return config_.frame_buffer + 4 * (config_.pixels_per_scan_line * y + x);
    }

  private:
    const FrameBufferConfig& config_;
};

class RGBResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    // コンストラクタ定義の代わり(using宣言で親クラスのコンストラクタをそのまま使用できる)
    using PixelWriter::PixelWriter;
    // メンバ関数のプロトタイプ宣言
    virtual void Write(int x, int y, const PixelColor& c) override;
};

class BGRResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    using PixelWriter::PixelWriter;
    virtual void Write(int x, int y, const PixelColor& c) override;
};


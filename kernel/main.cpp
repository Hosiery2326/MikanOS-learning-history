/**
 * カーネル本体
*/

#include  <cstdint>
#include  <cstddef>

#include  "frame_buffer_config.hpp"

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

/*
 * PixelWriterクラスを継承したRGBResv8BitPerColorPixelWriterクラスを定義
*/
class RGBResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    // コンストラクタ定義の代わり(using宣言で親クラスのコンストラクタをそのまま使用できる)
    using PixelWriter::PixelWriter;

    virtual void Write(int x, int y, const PixelColor& c) override {
      auto p = PixelAt(x, y);
      p[0] = c.r;
      p[1] = c.g;
      p[2] = c.b;
    }
};

/*
 * PixelWriterクラスを継承したBGRResv8BitPerColorPixelWriterクラスを定義 
*/
class BGRResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    using PixelWriter::PixelWriter;

    virtual void Write(int x, int y, const PixelColor& c) override {
      auto p = PixelAt(x, y);
      p[0] = c.b;
      p[1] = c.g;
      p[2] = c.r;
    }
};

/*
 * 配置new メモリ管理が無くてもクラスのインスタンスを生成可能
*/
void* operator new(size_t size, void* buf) {
  return buf;
}
/*
 * delete演算子が無いとリンク時にエラーになる
*/
void operator delete(void* obj) noexcept {
}

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

/**
 * 構造体のポインタを参照で受け取る
 */
extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
  switch (frame_buffer_config.pixel_format) {
    case kPixelRGBResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{frame_buffer_config};
        break;
    case kPixelBGRResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
  }

  // extern "C"で名前修飾を防ぐ
  for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x) {
    for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y) {
      pixel_writer->Write(x, y, {255, 255, 255});
    }
  }
  for (int x = 0; x < 200; ++x) {
    for (int y = 0; y < 100; ++y) {
      pixel_writer->Write(x, y, {0, 255, 0});
    }
  }

  // CPUを停止
  while (1)  __asm__("hlt");
}
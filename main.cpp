/*
 * フレームバッファの情報を受け取ってピクセルを描く
*/
#include  <cstdint>
extern "C" void KernelMain(uint64_t frame_buffer_base,
                           uint64_t frame_buffer_size) { // extern "C"で名前修飾を防ぐ
  // 整数からポインタへのキャスト
  uint8_t* frame_buffer = reinterpret_cast<uint8_t*>(frame_buffer_base);
  // 画面を塗りつぶす処理
  for (uint64_t i = 0; i < frame_buffer_size; ++i) {
    frame_buffer[i] = i % 256;
  }
  // CPUを停止
  while (1)  __asm__("hlt");
}
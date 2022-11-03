/*
 * 永久ループするカーネル
*/

extern "C" void KernelMain() { // extern "C"で名前修飾を防ぐ
  // CPUを停止
  while (1)  __asm__("hlt");
}
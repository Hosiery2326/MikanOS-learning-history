// ソースコード

#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/PrintLib.h>
#include  <Protocol/LoadedImage.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Protocol/DiskIo2.h>
#include  <Protocol/BlockIo.h>
#include  <Guid/FileInfo.h>

/*
 * メモリマップ構造体
*/
struct MemoryMap {
  UINTN buffer_size;
  VOID* buffer;
  UINTN map_size;
  UINTN map_key;
  UINTN descriptor_size;
  UINT32 descriptor_version;
};

/* 関数呼び出し時点のメモリマップを取得し、引数MemoryMapで指定されたメモリ領域に書き込みを行う
 * 正常に取得された場合はEFI_SUCCESSを返す
*/
EFI_STATUS GetMemoryMap(struct MemoryMap* map) {
  if (map->buffer == NULL) { // メモリ領域がメモリマップを収められない場合
    return EFI_BUFFER_TOO_SMALL;
  }

  map->map_size = map->buffer_size;
  return gBS->GetMemoryMap( // gBSはブートサービス(OSを起動するために必要な機能を提供)を表すグローバル変数
      &map->map_size, // メモリマップ書き込み用のメモリ領域の大きさ(バイト数)
      (EFI_MEMORY_DESCRIPTOR*)map->buffer, // メモリマップ書き込み用のメモリ領域の先頭ポインタ
      &map->map_key, // メモリマップ識別用の値を書き込む変数
      &map->descriptor_size, // メモリディスクリプタのバイト数
      &map->descriptor_version); // メモリディスクリプタの構造体のバージョン
}

/*
 * メモリマップのtype名をUnicodeで返す
*/
const CHAR16* GetMemoryTypeUnicode(EFI_MEMORY_TYPE type) {
  switch (type) {
    case EfiReservedMemoryType: return L"EfiReservedMemoryType";
    case EfiLoaderCode: return L"EfiLoaderCode";
    case EfiLoaderData: return L"EfiLoaderData";
    case EfiBootServicesCode: return L"EfiBootServicesCode";
    case EfiBootServicesData: return L"EfiBootServicesData";
    case EfiRuntimeServicesCode: return L"EfiRuntimeServicesCode";
    case EfiRuntimeServicesData: return L"EfiRuntimeServicesData";
    case EfiConventionalMemory: return L"EfiConventionalMemory";
    case EfiUnusableMemory: return L"EfiUnusableMemory";
    case EfiACPIReclaimMemory: return L"EfiACPIReclaimMemory";
    case EfiACPIMemoryNVS: return L"EfiACPIMemoryNVS";
    case EfiMemoryMappedIO: return L"EfiMemoryMappedIO";
    case EfiMemoryMappedIOPortSpace: return L"EfiMemoryMappedIOPortSpace";
    case EfiPalCode: return L"EfiPalCode";
    case EfiPersistentMemory: return L"EfiPersistentMemory";
    case EfiMaxMemoryType: return L"EfiMaxMemoryType";
    default: return L"InvalidMemoryType";
  }
}

/*
 * メモリマップをCSV形式でファイルに保存する
*/
EFI_STATUS SaveMemoryMap(struct MemoryMap* map, EFI_FILE_PROTOCOL* file) {
  CHAR8 buf[256];
  UINTN len;

  // ヘッダ行を出力
  CHAR8* header =
    "Index, Type, Type(name), PhisicalStart, NumberOfPages, Attribute\n";
  len = AsciiStrLen(header);
  file->Write(file, &len, header);

  Print(L"map->buffer = %08lx, map->map_size = %08lx\n",
      map->buffer, map->map_size);

  // メモリマップの各行をカンマ区切りで出力
  EFI_PHYSICAL_ADDRESS iter; // イテレーター。メモリマップのメモリディスクリプタのアドレスを指す。
  int i;
  for (iter = (EFI_PHYSICAL_ADDRESS)map->buffer, i = 0;
       iter < (EFI_PHYSICAL_ADDRESS)map->buffer + map->map_size;
       iter += map->descriptor_size, i++) {
    EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)iter; // 整数からポインタへのキャスト
    // キャストしたdescを使用して、メモリディスクリプタの値を文字列に変換する。
    // AsciiSPrint()は指定したchar配列に整形した文字列を書き込む(EDKIIのライブラリ関数)
    len = AsciiSPrint(
        buf, sizeof(buf),
        "%u, %x, %-ls, %08lx, %lx, %lx\n",
        i, desc->Type, GetMemoryTypeUnicode(desc->Type),
        desc->PhysicalStart, desc->NumberOfPages,
        desc->Attribute & 0xffffflu);
    // 文字列をファイルに書き出す
    file->Write(file, &len, buf);
       }

  return EFI_SUCCESS;
}

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

  gBS->OpenProtocol(
      image_handle,
      &gEfiLoadedImageProtocolGuid,
      (VOID**)&loaded_image,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  gBS->OpenProtocol(
      loaded_image->DeviceHandle,
      &gEfiSimpleFileSystemProtocolGuid,
      (VOID**)&fs,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  fs->OpenVolume(fs, root);

  return EFI_SUCCESS;
}


EFI_STATUS EFIAPI UefiMain (
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table) {
      Print(L"Hello, Mikan World!\n");
      
      CHAR8 memmap_buf[4096 * 4];
      struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
      GetMemoryMap(&memmap); // メモリマップを取得
      
      // memmapというファイルを書き込みモードで開く(存在しない場合は新規作成)
      EFI_FILE_PROTOCOL* root_dir;
      OpenRootDir(image_handle, &root_dir);

      EFI_FILE_PROTOCOL* memmap_file;
      root_dir->Open(
          root_dir, &memmap_file, L"\\memmap",
          EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
      
      // 開いたファイル(memmap)に取得したメモリマップを保存する
      SaveMemoryMap(&memmap, memmap_file);
      memmap_file->Close(memmap_file);
    
      // カーネルファイル(kernel.elf)を読み込み専用で開く
      EFI_FILE_PROTOCOL* kernel_file;
      root_dir->Open(
          root_dir, &kernel_file, L"\\kernel.elf",
          EFI_FILE_MODE_READ, 0);

      // 開いたファイル全体を読み込むためのメモリを確保する
      // CHAR16*12でカーネルファイル名(\kernel.elf)分の領域(12バイト)を確保
      UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
      UINT8 file_info_buffer[file_info_size];
      kernel_file->GetInfo( // ファイル情報を取得
          kernel_file, &gEfiFileInfoGuid,
          &file_info_size, file_info_buffer);
      
      // FileSizeを読み取る
      EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
      UINTN kernel_file_size = file_info->FileSize;

      // メモリ領域を確保する
      // ld.lld --image-baseで指定した0x100000番地にメモリを確保する
      EFI_PHYSICAL_ADDRESS kernel_base_addr = 0x100000;
      gBS->AllocatePages(
          AllocateAddress, EfiLoaderData,
          (kernel_file_size + 0xfff) / 0x1000, &kernel_base_addr);

      // ファイル全体を読み込む
      kernel_file->Read(kernel_file, &kernel_file_size, (VOID*)kernel_base_addr);
      Print(L"Kernel: 0x%01x (%lu bytes)\n", kernel_base_addr, kernel_file_size);

      // カーネル起動前にブートサービスを停止させる
      // 成功後はブートサービスの機能(Print()、ファイルやメモリ関連の機能)は使えなくなる
      EFI_STATUS status;
      status = gBS->ExitBootServices(image_handle, memmap.map_key);
      if (EFI_ERROR(status)) { 
        status = GetMemoryMap(&memmap); // 失敗したら再度メモリマップを取得する
        if (EFI_ERROR(status)) {
          Print(L"failed to get memory map: %r\n", status);
          while(1);
        }
        status = gBS->ExitBootServices(image_handle, memmap.map_key); // 再取得したマップキーを使って再実行する
        if (EFI_ERROR(status)) {
          Print(L"Could not exit boot service: %r\n", status);
          while(1);
        }
      }

      // カーネルを起動
      // エントリポイント(KernelMain())の場所を計算し呼び出す
      // 64bit ELFのエントリポイントアドレスはオフセット24byteの位置から8byte整数として書かれる
      // 種別がEXECなのでエントリポイントアドレスの値はKernelMain()の実体が置かれたアドレスになる
      UINT64 entry_addr = *(UINT64*)(kernel_base_addr + 24);

      // 引数、戻り値がどちらもvoid型である関数を表すEntryPointType型を作成
      typedef void EntryPointType(void);
      // 新しく作った型を使ってentry_addrを初期値とするポインタ変数を定義
      EntryPointType* entry_point = (EntryPointType*)entry_addr;
      // 関数の先頭アドレスに引数と戻り値の型情報を組み合わせたので、C言語の関数としてentry_point()を呼び出せる
      entry_point(); // この行は((EntryPointType*)entry_addr)();でもOK

      Print(L"All done\n");

      while (1);
      return EFI_SUCCESS;
}


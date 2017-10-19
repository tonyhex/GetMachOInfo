//
// Created by Anton Serov on 18/10/2017.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>

bool is_64bit_arch(uint32_t magic) {
  return magic==MH_MAGIC_64 || magic==MH_CIGAM_64;
}

bool should_swap_bytes(uint32_t magic) {
  return magic==MH_CIGAM || magic==MH_CIGAM_64;
}

void *read_bytes(FILE *file,
                 int offset,
                 size_t headerSize) {
  void *buf = calloc(1, headerSize);
  fseek(file, offset, SEEK_SET);
  fread(buf, headerSize, 1, file);
  return buf;
}

uint32_t read_magic(FILE *file, int offset) {
  uint32_t magicNumber;
  fseek(file, offset, SEEK_SET);
  fread(&magicNumber, sizeof(uint32_t), 1, file);
  return magicNumber;
}

void print_dylib_info(FILE *file,
                      int loadCommandsOffset,
                      int commandsNumber,
                      bool shouldSwapBytes) {
  printf("Dynamically loaded libraries:\n");
  int offset = loadCommandsOffset;

  for (int i = 0; i < commandsNumber; ++i) {
    struct load_command *cmd = read_bytes(file, offset, sizeof(struct load_command));
    if (shouldSwapBytes) {
      swap_load_command(cmd, NX_UnknownByteOrder);
    }

    if (cmd->cmd==LC_LOAD_DYLIB) {
      struct dylib_command *dylibCommand = read_bytes(file, offset, sizeof(struct dylib_command));
      if (shouldSwapBytes) {
        swap_dylib_command(dylibCommand, NX_UnknownByteOrder);
      }
      uint32_t dylibLen = cmd->cmdsize - dylibCommand->dylib.name.offset;
      uint32_t dylibNameOffset = offset + dylibCommand->dylib.name.offset;
      char *dylibName = read_bytes(file, dylibNameOffset, dylibLen);
      printf("\t%s\n", dylibName);
      free(dylibCommand);
    }

    offset += cmd->cmdsize;

    free(cmd);
  }
}

void read_header(FILE *file,
                 int offset,
                 bool is64bitArch,
                 bool isSwapBytes) {
  uint32_t commandsNumber = 0;
  int loadCommandsOffset = offset;

  if (is64bitArch) {
    size_t headerSize = sizeof(struct mach_header_64);
    struct mach_header_64 *header = read_bytes(file, offset, headerSize);
    if (isSwapBytes) {
      swap_mach_header_64(header, NX_UnknownByteOrder);
    }

    commandsNumber = header->ncmds;
    loadCommandsOffset += headerSize;

    free(header);
  } else {
    size_t headerSize = sizeof(struct mach_header);
    struct mach_header *header = read_bytes(file, offset, headerSize);
    if (isSwapBytes) {
      swap_mach_header(header, NX_UnknownByteOrder);
    }

    commandsNumber = header->ncmds;
    loadCommandsOffset += headerSize;

    free(header);
  }

  print_dylib_info(file, loadCommandsOffset, commandsNumber, isSwapBytes);
}

void print_mach_info(FILE *file) {
  uint32_t magic = read_magic(file, 0);
  bool is64bitArch = is_64bit_arch(magic);
  bool shouldSwapBytes = should_swap_bytes(magic);
  read_header(file, 0, is64bitArch, shouldSwapBytes);
}

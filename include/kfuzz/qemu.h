//
// Created by Myriad-Dreamin on 2021/7/27.
//

#ifndef LIBQEMU_QEMU_H
#define LIBQEMU_QEMU_H

int kfuzz_qemu_module_begin(int argc, char **argv, char **envp);
int kfuzz_qemu_module_init_bios(void);
int kfuzz_qemu_module_loop(void);
int kfuzz_qemu_module_cleanup(void);


#endif //LIBQEMU_QEMU_H

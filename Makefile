OS_BIN_NAME = myos.bin

OS_IMAGE_NAME = myos.iso

ISO_DIR = isodir/boot/

SRCS = bootinfos.s kernel.c

OBJS_DIR = .objs

OBJS =  $(OBJS_DIR)/*

#########################################################
#                                                       #
#       Assemble the bootinfos.s containing             #
#       instructions in order to boot the kernel        #
#       correctly like multiboot header in its own      #
#       section, or reserving memory space for a stack  #
#                                                       #
#########################################################
$(OBJS_DIR): $(SRCS)
	@mkdir -p $(OBJS_DIR)
	@echo "    Creating objects file : Compiling "
	nasm -f elf32 bootinfos.s -o $(OBJS_DIR)/bootinfos.o
	gcc -m32 -ffreestanding -O2 -Wall -Wextra -std=gnu99 -c kernel.c -o $(OBJS_DIR)/kernel.o

$(OS_IMAGE_NAME): $(OS_BIN_NAME) Makefile
	@mkdir -p $(ISO_DIR)/grub
	@cp $(OS_BIN_NAME) $(ISO_DIR)/$(OS_BIN_NAME)
	@cp grub.cfg  $(ISO_DIR)/grub/grub.cfg
	grub-mkrescue -o $(OS_IMAGE_NAME) $(dir $(ISO_DIR))

$(OS_BIN_NAME): $(OBJS_DIR) linker.ld 
	@echo "    Creating binary file : Linking"
	gcc -m32 -T linker.ld -o $(OS_BIN_NAME) -ffreestanding -O2 -nolibc -nostdlib -nodefaultlibs $(OBJS)

all: $(OS_IMAGE_NAME)

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(OS_BIN_NAME)
	rm -f $(OS_IMAGE_NAME)
	rm -rf $(dir $(ISO_DIR))

bin: $(OS_BIN_NAME)

multiboot: bin
	@sh test_multiboot.sh

exec: bin
	qemu-system-i386 -kernel $(OS_BIN_NAME)
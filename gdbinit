target remote :3333
set remote hardware-watchpoint-limit 2
mon reset halt
symbol-file ./build/light_bulb.elf
maintenance flush register-cache
thb app_main
c

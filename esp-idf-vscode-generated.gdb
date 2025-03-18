target remote :4444
symbol-file build/light_bulb.elf
flushregs
set remote hardware-watchpoint-limit 2
set logging enabled on
mon reset halt
maintenance flush register-cache
thb app_main
c
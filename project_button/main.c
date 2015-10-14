

#include "vmtype.h"
#include "vmlog.h"
#include "vmsystem.h"
#include "vmtimer.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_kbd.h"
#include "vmkeypad.h"

VM_DCL_HANDLE gpio_handle;
VM_TIMER_ID_PRECISE sys_timer_id = 0;

void gpio_init(void)
{
    gpio_handle = vm_dcl_open(VM_DCL_GPIO, 12);
    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
}

void key_init(void)
{
    VM_DCL_HANDLE kbd_handle;
    vm_dcl_kbd_control_pin_t kbdmap;

    kbd_handle = vm_dcl_open(VM_DCL_KBD, 0);
    kbdmap.col_map = 0x09;
    kbdmap.row_map = 0x05;
    vm_dcl_control(kbd_handle, VM_DCL_KBD_COMMAND_CONFIG_PIN, (void*)(&kbdmap));

    vm_dcl_close(kbd_handle);
}

VMINT handle_keypad_event(VM_KEYPAD_EVENT event, VMINT code)
{
    /* output log to monitor or catcher */
    vm_log_info("key event=%d,key code=%d", event, code); /* event value refer to VM_KEYPAD_EVENT */

    if(code == 30) {
        if(event == 3) { // long pressed

        } else if(event == 2) { // down
            vm_log_debug("key is pressed\n");
        } else if(event == 1) { // up
            static int out = 0;

            out = 1 - out;
            if(out) {
                vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
            } else {
                vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
            }

            vm_log_info("led %s", out ? "on" : "off");
        }
    }
    return 0;
}

/* Entry point */
void vm_main(void)
{
    gpio_init();
    key_init();
    vm_keypad_register_event_callback(handle_keypad_event);
}

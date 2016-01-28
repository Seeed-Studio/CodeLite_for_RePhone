
#include "vmlog.h"
#include "vmsystem.h"
#include "vmtimer.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"

VM_DCL_HANDLE gpio_handle;
VM_TIMER_ID_PRECISE sys_timer_id = 0;

void gpio_init(void)
{
    gpio_handle = vm_dcl_open(VM_DCL_GPIO, 12);     // blue
    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
    vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
}

void sys_timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
    static int out = 0;
    
    out = 1 - out;
    if (out) {
        vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    } else {
        vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
    }
    
    vm_log_info("led %s", out ? "on" : "off");
}

void handle_sysevt(VMINT message, VMINT param)
{
    switch(message) {
    case VM_EVENT_CREATE:
        gpio_init();
        sys_timer_id = vm_timer_create_precise(1000, sys_timer_callback, NULL);
        break;
    case VM_EVENT_QUIT:
        break;
    }
}

/* Entry point */
void vm_main(void)
{
    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}

#include "Driver_USART.h"
#include "RTE_Components.h"

#define ARM_USART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 00)

// Driver Version
static const ARM_DRIVER_VERSION usart_driver_version = { ARM_USART_API_VERSION, ARM_USART_DRV_VERSION };

// end group USART_control

//
//   Functions
//

ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
}

ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
{
}

int32_t ARM_USART_Initialize(ARM_USART_SignalEvent_t cb_event)
{
}

int32_t ARM_USART_Uninitialize(void)
{
}

int32_t ARM_USART_PowerControl(ARM_POWER_STATE state)
{
    switch (state)
    {
    case ARM_POWER_OFF:
        break;

    case ARM_POWER_LOW:
        break;

    case ARM_POWER_FULL:
        break;

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

int32_t ARM_USART_Send(const void *data, uint32_t num)
{
}

int32_t ARM_USART_Receive(void *data, uint32_t num)
{
}

int32_t ARM_USART_Transfer(const void *data_out, void *data_in, uint32_t num)
{
}

uint32_t ARM_USART_GetTxCount(void)
{
}

uint32_t ARM_USART_GetRxCount(void)
{
}

int32_t ARM_USART_Control(uint32_t control, uint32_t arg)
{
}

ARM_USART_STATUS ARM_USART_GetStatus(void)
{
}

int32_t ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
}

ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus(void)
{
}

void ARM_USART_SignalEvent(uint32_t event)
{
    // function body
}

// End USART Interface

ARM_DRIVER_USART Driver_USART = {
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    ARM_USART_Initialize,
    ARM_USART_Uninitialize,
    ARM_USART_PowerControl,
    ARM_USART_Send,
    ARM_USART_Receive,
    ARM_USART_Transfer,
    ARM_USART_GetTxCount,
    ARM_USART_GetRxCount,
    ARM_USART_Control,
    ARM_USART_GetStatus,
    ARM_USART_SetModemControl,
    ARM_USART_GetModemStatus
};

#include "Driver_ETH_MAC.h"

#include "RTE_Components.h"

#define ARM_ETH_MAC_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 00) /* driver version */

#if (defined(RTE_Drivers_ETH_MAC0) && !RTE_ENET)
#error "Ethernet not configured in RTE_Device.h!"
#endif

ARM_DRIVER_VERSION ARM_ETH_MAC_GetVersion(void)
{
}

ARM_ETH_MAC_CAPABILITIES ARM_ETH_MAC_GetCapabilities(void)
{
}

int32_t ARM_ETH_MAC_Initialize(ARM_ETH_MAC_SignalEvent_t cb_event)
{
}

int32_t ARM_ETH_MAC_Uninitialize(void)
{
}

int32_t ARM_ETH_MAC_PowerControl(ARM_POWER_STATE state)
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

int32_t ARM_ETH_MAC_GetMacAddress(ARM_ETH_MAC_ADDR *ptr_addr)
{
}

int32_t ARM_ETH_MAC_SetMacAddress(const ARM_ETH_MAC_ADDR *ptr_addr)
{
}

int32_t ARM_ETH_MAC_SetAddressFilter(const ARM_ETH_MAC_ADDR *ptr_addr, uint32_t num_addr)
{
}

int32_t ARM_ETH_MAC_SendFrame(const uint8_t *frame, uint32_t len, uint32_t flags)
{
}

int32_t ARM_ETH_MAC_ReadFrame(uint8_t *frame, uint32_t len)
{
}

uint32_t ARM_ETH_MAC_GetRxFrameSize(void)
{
}

int32_t ARM_ETH_MAC_GetRxFrameTime(ARM_ETH_MAC_TIME *time)
{
}

int32_t ARM_ETH_MAC_GetTxFrameTime(ARM_ETH_MAC_TIME *time)
{
}

int32_t ARM_ETH_MAC_Control(uint32_t control, uint32_t arg)
{
    switch (control)
    {
    case ARM_ETH_MAC_CONFIGURE:

        switch (arg & ARM_ETH_MAC_SPEED_Msk)
        {
        case ARM_ETH_MAC_SPEED_10M:
            break;
        case ARM_ETH_SPEED_100M:
            break;
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }

        switch (arg & ARM_ETH_MAC_DUPLEX_Msk)
        {
        case ARM_ETH_MAC_DUPLEX_FULL:
            break;
        }

        if (arg & ARM_ETH_MAC_LOOPBACK)
        {
        }

        if ((arg & ARM_ETH_MAC_CHECKSUM_OFFLOAD_RX) ||
            (arg & ARM_ETH_MAC_CHECKSUM_OFFLOAD_TX))
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }

        if (!(arg & ARM_ETH_MAC_ADDRESS_BROADCAST))
        {
        }

        if (arg & ARM_ETH_MAC_ADDRESS_MULTICAST)
        {
        }

        if (arg & ARM_ETH_MAC_ADDRESS_ALL)
        {
        }

        break;

    case ARM_ETH_MAC_CONTROL_TX:
        break;

    case ARM_ETH_MAC_CONTROL_RX:
        break;

    case ARM_ETH_MAC_FLUSH:
        if (arg & ARM_ETH_MAC_FLUSH_RX)
        {
        }
        if (arg & ARM_ETH_MAC_FLUSH_TX)
        {
        }
        break;

    case ARM_ETH_MAC_SLEEP:
        break;

    case ARM_ETH_MAC_VLAN_FILTER:
        break;

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

int32_t ARM_ETH_MAC_ControlTimer(uint32_t control, ARM_ETH_MAC_TIME *time)
{
}

int32_t ARM_ETH_MAC_PHY_Read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
}

int32_t ARM_ETH_MAC_PHY_Write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
}

void ARM_ETH_MAC_SignalEvent(uint32_t event)
{
}

// end group eth_mac_control

// End ETH MAC Interface

ARM_DRIVER_ETH_MAC Driver_ETH_MAC =
{
    ARM_ETH_MAC_GetVersion,
    ARM_ETH_MAC_GetCapabilities,
    ARM_ETH_MAC_Initialize,
    ARM_ETH_MAC_Uninitialize,
    ARM_ETH_MAC_PowerControl,
    ARM_ETH_MAC_GetMacAddress,
    ARM_ETH_MAC_SetMacAddress,
    ARM_ETH_MAC_SetAddressFilter,
    ARM_ETH_MAC_SendFrame,
    ARM_ETH_MAC_ReadFrame,
    ARM_ETH_MAC_GetRxFrameSize,
    ARM_ETH_MAC_GetRxFrameTime,
    ARM_ETH_MAC_GetTxFrameTime,
    ARM_ETH_MAC_ControlTimer,
    ARM_ETH_MAC_Control,
    ARM_ETH_MAC_PHY_Read,
    ARM_ETH_MAC_PHY_Write
};

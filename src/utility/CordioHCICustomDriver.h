#if (defined(ARDUINO_GIGA) && defined(CORE_CM4)) || __has_include("Arduino_PMC.h")

#include "ble/driver/CordioHCIDriver.h"
#include "CyH4TransportDriver.h"

namespace ble {
namespace vendor {
namespace cypress {

class HCIDriver : public CordioHCIDriver {
public:
    HCIDriver(
        ble::vendor::cypress_ble::CyH4TransportDriver& transport_driver,
        bool ps_enabled,
        uint8_t host_wake_irq,
        uint8_t dev_wake_irq
    ) : CordioHCIDriver(transport_driver),
        is_powersave_enabled(ps_enabled),
        host_wake_irq(host_wake_irq),
        dev_wake_irq(dev_wake_irq),
        service_pack_index(0),
        service_pack_ptr(0),
        service_pack_length(0),
        service_pack_next(),
        service_pack_transfered(false),
        cy_transport_driver(transport_driver) {
    }
    virtual buf_pool_desc_t get_buffer_pool_description();
    virtual void do_initialize();
    virtual void do_terminate();
    virtual void start_reset_sequence();
    virtual void handle_reset_sequence(uint8_t *pMsg);
private:
    void prepare_service_pack_transfert(void);
    void prepare_service_pack_transfert2(void);
    void start_service_pack_transfert(void);
    void post_service_pack_transfert(void);
    void terminate_service_pack_transfert(void);
    void inject_bdaddr(uint8_t* pBuf);
    void send_service_pack_command(void);
    void set_sleep_mode();
    void HciUpdateUartBaudRate();
    void HciWriteLeHostSupport();
    void hciCoreReadResolvingListSize(void);
    void hciCoreReadMaxDataLen(void);
    bool is_powersave_on(void);
    bool is_powersave_enabled;
    uint8_t host_wake_irq;
    uint8_t dev_wake_irq;

    size_t service_pack_index;
    const uint8_t* service_pack_ptr;
    int service_pack_length;
    void (HCIDriver::*service_pack_next)();
    bool service_pack_transfered;
    ble::vendor::cypress_ble::CyH4TransportDriver& cy_transport_driver;
};
}
}
}

#if defined(ARDUINO_GIGA) && defined(CORE_CM4)
ble::vendor::cypress_ble::CyH4TransportDriver& ble_cordio_get_custom_h4_transport_driver()
{
    static  ble::vendor::cypress_ble::CyH4TransportDriver s_transport_driver(
       /* TX */ CYBSP_BT_UART_TX, /* RX */ CYBSP_BT_UART_RX,
       /* cts */ CYBSP_BT_UART_CTS, /* rts */ CYBSP_BT_UART_RTS, CYBSP_BT_POWER, DEF_BT_BAUD_RATE
    );
    return s_transport_driver;
}
#endif

#if __has_include("Arduino_PMC.h")
static ble::vendor::cypress_ble::CyH4TransportDriver& ble_cordio_get_custom_h4_transport_driver()
{
    static ble::vendor::cypress_ble::CyH4TransportDriver s_transport_driver(
       /* TX */ CYBSP_BT_UART_TX, /* RX */ CYBSP_BT_UART_RX,
       /* cts */ CYBSP_BT_UART_CTS, /* rts */ CYBSP_BT_UART_RTS, CYBSP_BT_POWER, DEF_BT_BAUD_RATE
    );
    return s_transport_driver;
}
#endif

BLE_NAMESPACE::CordioHCIDriver& ble_cordio_get_custom_hci_driver() {
    static ble::vendor::cypress_ble::CyH4TransportDriver& transport_driver =
          ble_cordio_get_custom_h4_transport_driver();

    static ble::vendor::cypress::HCIDriver hci_driver(
        transport_driver,
        transport_driver.get_enabled_powersave(),
        transport_driver.get_host_wake_irq_event(),
        transport_driver.get_dev_wake_irq_event()
    );
    return hci_driver;
}

#include "ble/BLE.h"
#include "BLEInstanceBaseImpl.h"

ble::BLEInstanceBase* customDeviceInstance()
{
    static ble::impl::BLEInstanceBase instance(
        ble_cordio_get_custom_hci_driver()
    );
    return &instance;
}

ble::BLE& CustomInstance()
{
    static ble::BLEInstanceBase *transport = customDeviceInstance();

    if (!transport) {
        MBED_ERROR(MBED_MAKE_ERROR(MBED_MODULE_BLE, MBED_ERROR_CODE_BLE_BACKEND_NOT_INITIALIZED),
                   "bad handle to underlying transport");
    }

    static ble::BLE ble_instance(*transport);

    return ble_instance;
}

#define CUSTOM_HCI_DRIVER

#endif
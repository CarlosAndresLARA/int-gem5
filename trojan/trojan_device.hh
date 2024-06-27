#ifndef __ATTACK_DVFS_TROJAN_DEVICE_HH__
#define __ATTACK_DVFS_TROJAN_DEVICE_HH__

#include "params/TrojanDevice.hh"
#include "dev/arm/amba_device.hh"

namespace gem5
{

class TrojanDevice: public AmbaDmaDevice
{
    protected:
       
        static const uint64_t AMBA_ID       = 0x00142222ULL;
        
        /** ARM TrojanDevice register map*/
        enum{
        cpuPerfAddrRegister                  = 0x000,
        dataToSendRegister                   = 0x004,
        sizeOfDataRegister                   = 0x008,
        deviceTimeBitSeparatorRegister       = 0x00C,
        
        deviceFreq0Register                  = 0x010,
        deviceFreq1Register                  = 0x014,
        deviceFreqSeparatorRegister          = 0x018,
        cpuTimeBitSeparatorRegister          = 0x01C,
        cpuAttackEnabledRegister             = 0x020,
        
        startAttackRegister                  = 0x024,
        getBitRegister                       = 0x028,
        };

        /** Control register */
        Addr cpuPerfAddrReg;
        uint32_t dataToSendReg;
        uint32_t sizeOfDataReg;
        uint32_t deviceTimeBitSeparatorReg;
        
        uint32_t deviceFreq0Reg;
        uint32_t deviceFreq1Reg;
        uint32_t deviceFreqSeparatorReg;
        uint32_t cpuTimeBitSeparatorReg;
        uint32_t cpuAttackEnabledReg;
        
        uint32_t startAttackReg;
        uint32_t getBitReg;
        
        uint32_t dataToSend;
        uint8_t BitPosition;
        uint8_t bitSeparator;
        
        Tick startTime;
        
        EventFunctionWrapper deviceReciveDataEvent;
        EventFunctionWrapper modifyingCpuFreqEvent;
        
        static const int dmaSize = 4; // 32 bits for 32 bits platform (bus width) 
        static const int buffer_size = 4*sizeof(uint32_t);
        uint8_t *dmaBuffer;
        uint8_t *bitsToSend;
        
        void startDeviceAttack();
        void deviceReciveData();
        
        void startCpuAttack();
        void modifyingCpuFreq();
      
    public:

        using Params = TrojanDeviceParams;
        TrojanDevice(const Params &p);
        ~TrojanDevice();


        Tick read(PacketPtr pkt) override;
        Tick write(PacketPtr pkt) override;
        
        /**
         * Determine the address ranges that this device responds to.
         *
         * @return a list of non-overlapping address ranges
         */
        AddrRangeList getAddrRanges() const override;
};

} // namespace gem5

#endif // __ATTACK_DVFS_TROJAN_DEVICE_HH__

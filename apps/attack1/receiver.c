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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>

int nanosleep(const struct timespec *req, struct timespec *rem);


#define MAX_LENGTH 10
#define NUM_SAMPLES 6000
#define cpuinfo_cur_freq "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq"


/*
* The receiver process.
* This function receives the data outside the secure zone
* and creates a file for convenient evaluation.
*/
int main(int argc, char *argv[])
{
    printf("R: begin \n");

    int i=0, j=0;
    char *fname = "dvfs_attack_1_data.txt";
    char received_data[NUM_SAMPLES][MAX_LENGTH];

    /*struct timespec tempo_1;
    tempo_1.tv_sec = 0;
    tempo_1.tv_nsec = 500L;*/

    //Determine if the program can read the system frequency
    FILE* fr = fopen(cpuinfo_cur_freq,"r");
    if(fr == NULL)
    {
        printf("R: cpuinfo_cur_freq not open\n");
        return 1;
    }
    fclose(fr);

    //Create the file to store data
    FILE* fq = fopen(fname, "w+");
    if(fq == NULL)
    {
         printf("R: %s not open\n", fname);
         return 2;
    }
    fclose(fq);

    //Sample
    while(access("goodbye.txt", F_OK) != 0)
    {
        fr = fopen(cpuinfo_cur_freq,"r");
        fgets(received_data[i], MAX_LENGTH, fr );
        fclose(fr);
        i++;
        //nanosleep(&tempo_1 , NULL);
    }

    //Store the data
    fq = fopen(fname, "w+");
    for(j=0;j<i;j++)
        fprintf(fq, received_data[j]);
    fclose(fq);
    
    printf("R: finish \n");

    return 0;
}

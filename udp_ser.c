#include "headsock.h"
#include <assert.h>

// transmitting and receiving function
void str_ser(int sockfd, struct sockaddr *addr, int addrlen);             
int compareFiles(char arg1[], char arg2[]);
int randint(int max);

int main(void)
{
	int sockfd, ret;
	struct sockaddr_in my_addr;

	//create socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);          
	if (sockfd<0)
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);
	//bind socket
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                
	if (ret<0) {
		printf("error in binding");
		exit(1);
	}
	
	printf("waiting for data\n");
	//receive packet and response
	str_ser(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));                
  
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd, struct sockaddr *addr, int addrlen)
{
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	int end = 0, n = 0;
	long lseek=0;
	double randomnum = 0.0;

	while(!end) {
		//printf("------------------\n");

		// error simulation
        //randomnum = (double) rand() / (RAND_MAX);
        randomnum = randint(100);
        //printf("random number is %f \n", randomnum);
        if(randomnum < ERROR_PROB) {
            // damaged packet
            printf("[server] damaged packet\n");
			// send NACK 
			ack.num = -1;
			ack.len = 0;
			if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen))==-1) {
				printf("send error!");								
				exit(1);
			}
			printf("[server] sent an NACK\n");
        }
        else {
			// complete packet
			if ((n= recvfrom(sockfd, &recvs, DATALEN, 0, addr, (socklen_t *)&addrlen))==-1) {
				printf("error when receiving\n");
				exit(1);
			}
			//printf("[server] received a packet\n");
			
			//end of the file
			if (recvs[n-1] == '\0')	{
				end = 1;
				n --;
			}

			// process the received packet
			memcpy((buf+lseek), recvs, n);
			//printf("%d bytes of data received\n", n);
			lseek += n;

			// send ack 
			ack.num = 1;
			ack.len = 0;
			if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen))==-1) {
				printf("send error!");								
				exit(1);
			}
			//printf("[server] sent an ack\n");
		}

		if (end == 1){
			printf("Reached the end of document\n");
		}
		
	}
  
	if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exist\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
	
	if (compareFiles("myUDPreceive.txt","myfile.txt") == 0){
		printf("THe 2 files are identical\n");
	}
	else{
		printf("THe 2 files are not identical\n");
	}
	
}




// Compare files	
int compareFiles(char arg1[], char arg2[]) {
    char line1[BUFSIZE];
    char line2[BUFSIZE];

    FILE *file1 = fopen(arg1, "r");
    FILE *file2 = fopen(arg2, "r");

    if (file1 == NULL || file2 == NULL) {
        printf("Error opening one or both files.\n");
        return -1;
    }

    while (fgets(line1, BUFSIZE, file1) != NULL && fgets(line2, BUFSIZE, file2) != NULL) {
        // Remove newline characters from the lines
        size_t len1 = strlen(line1);
        size_t len2 = strlen(line2);

        if (len1 > 0 && line1[len1 - 1] == '\n') {
            line1[len1 - 1] = '\0';
        }

        if (len2 > 0 && line2[len2 - 1] == '\n') {
            line2[len2 - 1] = '\0';
        }

        if (strcmp(line1, line2) != 0) {
            printf("%s\n%s\n", line1, line2);
            fclose(file1);
            fclose(file2);
            return 0;
        }
    }

    if (fgets(line1, BUFSIZE, file1) != NULL || fgets(line2, BUFSIZE, file2) != NULL) {
        printf("One of the files ended prematurely.\n");
    }

    fclose(file1);
    fclose(file2);

    return 0;
}

int randint(int max){
	if ((max-1) == RAND_MAX){
		return rand();
	}
	else{
		assert (max <= RAND_MAX);
		int end = RAND_MAX /max;
		assert (end > 0);
		end *= max;
		
		int r;
		while ((r = rand()) >= end);
			return r% max;
	}
}


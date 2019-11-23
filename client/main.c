#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <stdint.h>
#include <json.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE        4096
#define WIDTH_IMAGE     1920
#define CAM_ALL		0
#define CAM_0		1
#define CAM_1		2
#define BUF_CAM_SIZE	1920

int main( int argc, char* argv[] )  
{  
	printf("(i) Test Client Program Started.\n");

	if(argc!=2) {
		printf("Usage: %s <send JSON file name>\n", argv[0]);
		exit(1);
	}

	int sock;
	char address[] = "127.0.0.1";
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;

	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(address);
	serv_addr.sin_port = htons(7076);
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	{
		fprintf(stderr,"(!) connect error!\n");
		return 0;
	}

	FILE *fp = fopen(argv[1], "rb");
	int read_cnt = 0;
	char buf[BUF_SIZE];
	memset(buf,0,BUF_SIZE);

	while(1) // send JSON file
	{
		read_cnt = fread((void*)buf, 1, BUF_SIZE, fp);
		if(read_cnt<BUF_SIZE)
		{
			write(sock, buf, read_cnt);
			break;
		}
		write(sock, buf, BUF_SIZE);
	}

	fclose(fp);


	// receive JSON packet
	while(1)
	{
		read_cnt=read(sock, buf, BUF_SIZE);
		if(read_cnt<=0)
		{
				printf("read count %d \n", read_cnt);
				break; 
		}
		printf("(i) client: read(%d)%s\n", read_cnt, buf);

		buf[read_cnt] = 0;
		printf("%s", buf);
		if(strchr(buf,'}')!=NULL) 
		{
			printf("terminate \n");
			break; // end of the packet
		}
	}

	close(sock);

	return 0;
}  




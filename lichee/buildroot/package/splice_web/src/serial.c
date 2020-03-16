/////////////////////////////////////////////////////////////////////////////
//  gadgetctrl daemon program...nqs_upgrade.c					   //
//  Created by s.j xeon2k@onsystech.com					   //
//		2007.6.15.. :> 	
//
//		version 0.1						   //
/////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h> 
#include <fcntl.h>

#include <stdio.h>
#include <termios.h>
#include <sys/signal.h>
#include <semaphore.h>
#include <sys/time.h>
#include <dirent.h> // /dev/ttyUSBx È®ÀÎ¿ë..


#define  PageByte       256

struct termios oldtio;

struct termios old_term, new_term;

//extern int baud;
#if 0
void cfmakeraw(struct termios *t)
{
    t->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    t->c_oflag &= ~OPOST;
    t->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    t->c_cflag &= ~(CSIZE | PARENB);
    t->c_cflag |= CS8;
}
#endif
int uart_open( char *ser_dev , int baud)
{
    int fd;
	//fd = open(ser_dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
	fd = open(ser_dev, O_RDWR | O_NOCTTY);
			
	if (fd < 0)
	{

      		printf("[E] Serial %s open error ",ser_dev);
      		return -1;
	}

    tcgetattr(fd,&old_term);
    memcpy((char *)&new_term,(char *)&old_term, sizeof(new_term));

    cfmakeraw( &new_term);
    if(baud == 115200)
    {
    cfsetispeed(&new_term,B115200);
    cfsetospeed(&new_term,B115200);
    }
    else if(baud == 57600)
    {
    cfsetispeed(&new_term,B57600);
    cfsetospeed(&new_term,B57600);
    }
    else if(baud == 38400)
    {
    cfsetispeed(&new_term,B38400);
    cfsetospeed(&new_term,B38400);
    }
    else if(baud == 19200)
    {
    cfsetispeed(&new_term,B19200);
    cfsetospeed(&new_term,B19200);
    }
    else if(baud == 9600)
    {
    cfsetispeed(&new_term,B9600);
    cfsetospeed(&new_term,B9600);
    }
    else
    {
        printf("Invalid baud = %d\n",baud);
        return -1;
    }
    new_term.c_cflag &= ~CSIZE;
    new_term.c_cflag |= CS8;
    new_term.c_cflag &= ~(PARENB|PARODD);
//    new_term.c_cflag |= PARENB;
    new_term.c_cflag &= ~CSTOPB;
    new_term.c_cflag &= ~CRTSCTS;

    tcsetattr(fd, TCSANOW, &new_term);
    tcdrain(fd);

	return fd ;
	
} 

/*-------------------- wmsd_service ----------------------*
 *
 *  FUNCTION: ionsv_uart_close()
 *
 *	Created By s.j ..
 *
 */
 
void uart_close(int fd)
{	
	tcsetattr(fd,TCSANOW,&old_term); 
	//close the com port
	close(fd); 
}

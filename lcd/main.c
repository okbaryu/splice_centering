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

#define BUF_SIZE        4096
#define WIDTH_SCREEN    800
#define HEIGHT_SCREEN   480
#define WIDTH_IMAGE     1920


extern int uart_open(char *devname, int baud);
extern void  uart_close( int fd );

void render(char * framebuffer, char * buf, int size)
{
    int x=0, y=0;
    for(x=0; x<WIDTH_SCREEN; x++)
    {
        for(y=0; y<HEIGHT_SCREEN; y++)
        {
            // resizing
            int index = (int)(x*2.4);
            char value1 = buf[index];
            char value2 = buf[index+1];
            char average = (value1+value2)/2;

            // rendering
            int offset = (WIDTH_SCREEN * y + x)*4;
            framebuffer[offset]  =average;//B  
            framebuffer[offset+1]=average;//G  
            framebuffer[offset+2]=average;//R  
            framebuffer[offset+3]=255;//A  
        }
    }
}

char g_frame[4096];
int g_index = 0;

void drawFrame(char * framebuffer, const char * buf, int size)
{
    char startOfFrame = buf[0];
    char colorData = buf[1];
    int count = ( (buf[2] & 0x7F)<<8) | buf[3]; // MSB not used. 15bit values max

    if(g_index>0 && startOfFrame==0x02)
    {
        //printf("(i) frame will be rendered\n");
        render(framebuffer, g_frame, 1920);
        g_index=0;
        return;
    }

    // put pixel on frame
    int i=0;
    for(i=0; i<count; i++)
    {
        if(g_index>1920) break;
//            printf("(i) g_index=%d, count=%d, buf[2]=%02X, buf[3]=%02X\n" , 
//                    g_index, count, buf[2], buf[3]);

        else g_frame[g_index++] = colorData;
    }
/*
//    printf("(i) drawFrame function called:size=%d\n, bf=%p, buf=%p\n", size, framebuffer, buf);
    int i=0, j=0;
    for(i=0; i<size; i++)
    {
        for(j=0; j<HEIGHT_SCREEN; j++)
        {
            int x = i%WIDTH_IMAGE;
            int y = j;
            if(x>=WIDTH_SCREEN) 
            {
//                printf("(i) drawFrame continue next pixel: x=%d, y=%d, w_i=%d, w_s=%d, h_s=%d\n", 
//                        x, y, WIDTH_IMAGE, WIDTH_SCREEN, HEIGHT_SCREEN);
                continue;
            }
            if(y>=HEIGHT_SCREEN) 
            {
//                printf("(i) drawFrame break loop: x=%d, y=%d, w_i=%d, w_s=%d, h_s=%d\n", 
//                        x, y, WIDTH_IMAGE, WIDTH_SCREEN, HEIGHT_SCREEN);
                break;
            }
            int index = (int)(i*2.4);
            char value1 = buf[index];
            char value2 = buf[index+1];
            char average = (value1+value2)/2;

            int offset = (WIDTH_SCREEN * y + x)*4;
            framebuffer[offset]  =average;//B  
            framebuffer[offset+1]=average;//G  
            framebuffer[offset+2]=average;//R  
            framebuffer[offset+3]=255;//A  
        }
    }
    printf("(i) drawFrame function will be returned\n");
    */
}

void receiveFrame(char* fb, char * strCom)
{
    int i = 0;
    int index = 0;
    int fd = uart_open(strCom, 115200);
    if(fd<0) {
        fprintf(stderr, "Fail to open serial port\n");
        exit(1);
    }
    printf("Serial Port open success!:%s\n", strCom);

    char buf[BUF_SIZE] = "";
    char temp[BUF_SIZE] = "";
    memset(buf,0,BUF_SIZE);
    memset(temp,0,BUF_SIZE);

    while(1)
    {
        ssize_t len = read(fd, temp, BUF_SIZE);
        if(len==-1) {
            fprintf(stderr,"Fail to read serial port\n");
            break;
        }
//        printf("(i) serial read success(%d):\n", len);

        for(i=0; i<len; i++)
        {
//            printf("%02X", temp[i]);
            if(temp[i]==0) // new packet 
            {
//                printf("\n(i) new packet(%d):", index);
                drawFrame(fb, buf, index);
                index = 0;
                continue;
            }

            buf[index++] = temp[i];
        }


    }

    uart_close(fd);
    printf("(i) UART Port closed\n");
}

    
int main( int argc, char* argv[] )  
{  
    if(argc!=2) {
        perror("Useage: lcd <Serial Port path>\n");
        perror("Example: lcd /dev/ttyS2\n");
        exit(1);
    }

    char strFb[255]="/dev/fb0";
    char strCom[255] = "";
    strcpy(strCom, argv[1]);
    printf("Serial Port path: %s\n", strCom);

    int framebuffer_fd = 0;  
    struct fb_var_screeninfo framebuffer_variable_screeninfo;  
    struct fb_fix_screeninfo framebuffer_fixed_screeninfo;  
                    
    framebuffer_fd = open( strFb, O_RDWR );  
    if ( framebuffer_fd <  0 ){  
        perror( "Error: cannot open framebuffer device\n" );  
        exit(1);  
    }  
    printf("frame buffer open success!:%s\n", strFb);


    if ( ioctl(framebuffer_fd, FBIOGET_VSCREENINFO,   
        &framebuffer_variable_screeninfo) )  
    {  
        perror( "Error: reading variable screen infomation\n" );  
        exit(1);  
    }  
    printf("(i) ioctl get v screen info success");


    printf("(i) xres = %d\n", framebuffer_variable_screeninfo.xres);
    printf("(i) yres = %d\n", framebuffer_variable_screeninfo.yres);
    printf("(i) xres_virtual = %d\n", framebuffer_variable_screeninfo.xres_virtual);
    printf("(i) yres_virtual = %d\n", framebuffer_variable_screeninfo.yres_virtual);
    printf("(i) bits_per_pixel = %d\n", framebuffer_variable_screeninfo.bits_per_pixel );
    printf("(i) height = %d\n", framebuffer_variable_screeninfo.height);
    printf("(i) width = %d\n", framebuffer_variable_screeninfo.width);

    printf("(i) ioctl put v screen info success");

    if ( ioctl(framebuffer_fd, FBIOGET_FSCREENINFO,   
        &framebuffer_fixed_screeninfo) )  
    {  
        perror( "Error: reading fixed screen infomation\n" );  
        exit(1);  
    }  

    printf("(i) smem_len = %d\n", framebuffer_fixed_screeninfo.smem_len);
    printf("(i) type = %d\n", framebuffer_fixed_screeninfo.type);
    printf("(i) mmio_len = %d\n", framebuffer_fixed_screeninfo.mmio_len);
               
    printf( "framebuffer Display information\n" );  
    printf( " %d x %d  %d bpp\n", 
        framebuffer_variable_screeninfo.xres,  
        framebuffer_variable_screeninfo.yres,   
        framebuffer_variable_screeninfo.bits_per_pixel );  
                                                   
    int width  = framebuffer_variable_screeninfo.xres;  
    int height = framebuffer_variable_screeninfo.yres;  
    int bpp     = framebuffer_variable_screeninfo.bits_per_pixel/8;  
    int xoffset = framebuffer_variable_screeninfo.xoffset;  
    int yoffset = framebuffer_variable_screeninfo.yoffset;  

    printf( " width:%d , height: %d , bpp: %d, xoffset: %d, yoffset:%d \n",
            width, height, bpp, xoffset, yoffset);

//    write(framebuffer_fd, "000", 3);
                                                                       
    long int screensize = width*height*bpp;  
                                                                            
    char *framebuffer_pointer = (char*)mmap( 0, screensize,  
        PROT_READ|PROT_WRITE,  MAP_SHARED,  framebuffer_fd, 0 );  
                                                                
    if ( framebuffer_pointer == MAP_FAILED )  
    {  
        perror( "Error : mmap\n" );  
        exit(1);  
    }  
    
    else  
    {  
        receiveFrame(framebuffer_pointer, strCom);
        // draw a frame on FB
        /*
        int x,y;  
        for ( y=0; y<height; y++)  
        for ( x=0; x<width; x++)  
        {  
            unsigned int pixel_offset = 
                (y+yoffset)*framebuffer_fixed_screeninfo.line_length +(x+xoffset)*bpp;  
            if (bpp==4){  
                if ( x<=width*1/3){    
                    framebuffer_pointer[pixel_offset]=255;//B  
                    framebuffer_pointer[pixel_offset+1]=0;//G  
                    framebuffer_pointer[pixel_offset+2]=0;//R  
                    framebuffer_pointer[pixel_offset+3]=255;//A  
                }  
                if ( x>width*1/3 && x<=width*2/3){      
                    framebuffer_pointer[pixel_offset]=0;//B  
                    framebuffer_pointer[pixel_offset+1]=255;//G  
                    framebuffer_pointer[pixel_offset+2]=0;//R  
                    framebuffer_pointer[pixel_offset+3]=255;//A  
                }  
                if ( x>width*2/3){     
                    framebuffer_pointer[pixel_offset]=0;//B  
                    framebuffer_pointer[pixel_offset+1]=0;//G  
                    framebuffer_pointer[pixel_offset+2]=255;//R  
                    framebuffer_pointer[pixel_offset+3]=255;//A  
                }  
            }  
        }  
        */
    }   
    munmap( framebuffer_pointer, screensize );   

    close( framebuffer_fd );  

    return 0;
}  




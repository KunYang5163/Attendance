/*
 * uart.c
 *
 *  Created on: 2015年6月1日
 *      Author: sed
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> //文件控制定义
#ifndef Q_OS_WIN
#include <termios.h>//终端控制定义
#endif
#include <errno.h>
#include <string.h>

#include "uart.h"


static int speed_value[] =
{
    230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 600, 300
};

static int speed_param[] =
{
    B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300
};

static void set_serial_param(int fd, int speed, int databits, int stopbits, int parity, int opostflag)
{
    int i, speednum = (int)(sizeof(speed_param)/sizeof(int));
    struct termios options;
    tcgetattr(fd, &options);
    for (i = 0; i < speednum; i++)
    {
        if (speed == speed_value[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&options, speed_param[i]);
            cfsetospeed(&options, speed_param[i]);
            tcsetattr(fd, TCSANOW, &options);
        }
    }

    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
        case 5:
            options.c_cflag |= CS5;
            break;
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            break;
    }

    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            break;
    }

    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB; /*   Clear   parity   enable   */
            options.c_iflag &= ~INPCK; /*   CLEAR   parity   checking   */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK; /*   ENABLE   parity   checking   */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB; /*   Enable   parity   */
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK; /*   ENABLE   parity   checking   */
            break;
        case 'S':
        case 's':
            /*as   no   parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            options.c_iflag &= ~INPCK; /*   Disnable   parity   checking   */
            break;
        default:
            options.c_iflag &= ~INPCK;
            break;
    }
    options.c_iflag &= ~IXON; /* disable XON/XOFF control */

    options.c_iflag |= IGNBRK | IGNPAR;
    //options.c_iflag |= BRKINT | IGNBRK | IGNPAR;
    options.c_iflag &= ~(INLCR | ICRNL | IGNCR);

    options.c_lflag = 0;
    options.c_oflag = 0;

    if(opostflag) options.c_oflag |= OPOST;
    else options.c_oflag &= ~OPOST;
    options.c_oflag &= ~(ONLCR | OCRNL | ONOCR | ONLRET);

    options.c_cflag |= CLOCAL | CREAD;
    options.c_cc[VTIME] = 1; /*  0: 15   seconds   */
    options.c_cc[VMIN] = 1;  /* 1 */

    tcflush(fd, TCIFLUSH); /*   Update   the   options   and   do   it   NOW   */
    tcsetattr(fd, TCSANOW, &options);
}


/*
 * 打开串口设备，成功返回文件描述符， 失败返回-1
 */
int UART_Init_COM(char *pDev, int brate)
{
	int serial_fd;

    serial_fd = open(pDev, O_RDWR);
    if (serial_fd < 0) {
        perror("open");
        return -1;
    }

    set_serial_param(serial_fd, 115200, 8, 1, 'n', 0);

    return serial_fd;
}


/*
 * 关闭串口设备
 */
void UART_Close(int serial_fd)
{
	printf("UART_Close\n");
	tcflush(serial_fd, TCIOFLUSH);
	close(serial_fd);

	return ;
}

/**
*串口发送数据
*@fd:串口描述符
*@data:待发送数据
*@datalen:数据长度
*
* 发送成功返回0, 失败返回-1。
*/
int UART_Send(int serial_fd, char *data, int datalen)
{
    int len = 0;
    int i;

    UART_FLUSH(serial_fd);
    len = write(serial_fd, data, datalen);//实际写入的长度
    if(len == datalen) {
//        printf("\n==============================\n");
//        printf("%s: len %d\n", __func__, datalen);
//        for(i = 0; i < datalen; i++)
//        {
//        	printf("0x%02x, ", data[i]);
//        }
//        printf("\n");
//
//        printf("\n==============================\n");
        return 0;
    } else {
    	perror("write");
    	printf("only %d bytes wrote, expected %d\n", len, datalen);
    	usleep(80);
    	len = write(serial_fd, data, datalen);
    	if(len == datalen) {
    		printf("\n==============================\n");
    		printf("%s: len %d\n", __func__, datalen);
    		for(i = 0; i < datalen; i++)
    		{
    			printf("0x%02x, ", data[i]);
    		}
    		printf("\n");

    		printf("\n==============================\n");
    	}
//        tcflush(serial_fd, TCOFLUSH);//TCOFLUSH刷新写入的数据但不传送
        return -1;
    }

    return -1;
}

/**
*串口接收数据
*/
int UART_Recv(int serial_fd, char *data, int datalen)
{
    int len = 0;
    fd_set readfds;
    struct timeval tv;
    int iTotalLen = 0, iExpectLen = datalen;
    char *pOffset = data;
    int ret = 0;

    FD_ZERO(&readfds);
    FD_SET(serial_fd, &readfds);

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while(1)
    {
        ret = select(serial_fd + 1, &readfds, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(serial_fd, &readfds))
        {
            if((len = read(serial_fd, pOffset, iExpectLen - iTotalLen)) > 0)
            {
                iTotalLen += len;
                pOffset += len;
            }

            if(iTotalLen >= iExpectLen)
            {
                break;
            }
            else
            {
                continue;
            }

        }
        break;
    }
    return iTotalLen;
}


void UART_FLUSH(int serial_fd)
{
	tcflush(serial_fd, TCIOFLUSH);
}


#ifdef __cplusplus
}
#endif

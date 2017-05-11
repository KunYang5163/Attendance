/*
 * uart.h
 *
 *  Created on: 2015Äê6ÔÂ1ÈÕ
 *      Author: sed
 */

#ifndef COMMON_UART_H_
#define COMMON_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef Q_OS_ANDROID
#include <termios.h>
#include <unistd.h>
#endif

#define SERIAL_NUM_LEN	14


#define DEVICE_UART3 "/dev/ttyS3"
#define DEVICE_UART0 "/dev/ttyS0"

int UART_Init_COM(char *pDev, int brate);
int UART_Send(int serial_fd, char *data, int datalen);
int UART_Recv(int serial_fd, char *data, int datalen);
void UART_Close(int serial_fd);
void UART_FLUSH(int serial_fd);


#ifdef __cplusplus
}
#endif

#endif /* COMMON_UART_H_ */

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/keyboard.h>
#include <sys/types.h>
#include <sys/ipc.h>
//#include <sys/msg.h>
//#include <linux/watchdog.h>
#include <termios.h>
#include <errno.h>
#include <time.h>

#define _COMM_DEBUG	 0

static int handle = -1;

/* 最大学员人脸特征文件大小为(11 * 1024)字节 */
#define MAX_FACE_BUFFER_LEN	(11 * 1024)

/* 最大配置文件大小为 32k */
#define MAX_FILE_BUFFER_LEN	(32 * 1024)

/* 升级文件大小为 */
#define MAX_UPDATE_FILE_LEN (3 * 1024 * 1024) 

/* 和嘉485命令字 */
/* 协议请求数据头 */
#define HEJIA_REQ_START_DATA0  		0x23
#define HEJIA_REQ_START_DATA1  		0x23
/* 协议应答数据头 */
#define HEJIA_RES_START_DATA0  		0x26
#define HEJIA_RES_START_DATA1  		0x26
/* 协议结束符 */
#define HEJIA_END_DATA0  			0x0d
#define HEJIA_END_DATA1  			0x0a

/* 外设类型，固定为0x0b */
#define HEJIA_DEVICE_TYPE  			0x0b

/* 主命令字，固定为0x40 */
#define HEJIA_MAIN_CMD  			0x40

/* 从命令字，握手信息 */
#define HEJIA_SUB_CMD_AUTH  		0x01

/* 从命令字，人员注册 */
#define NENGSHI_SUB_CMD_REGISTER  		0x02

/* 从命令字，上报人员注册结果到主机 */
#define NENGSHI_RESULT_REPORT_REGISTER  0x03

/* 从命令字，比对操作 */
#define NENGSHI_SUB_CMD_COMPARE  		0x04

/* 从命令字，从设备上报比对结果到主机 */
#define NENGSHI_RESULT_REPORT_COMPARE  	0x05

/* 从命令字，获取用户信息和照片 */
#define NENGSHI_GET_USER_INFO_PIC  		0x06

/* 从命令字，获取用户特征 */
#define NENGSHI_GET_USER_INFO_FEATURE  	0x07

/* 从命令字，删除用户信息 */
#define NENGSHI_SUB_DEL_USER_INFO   	0x08

/* 从命令字，下发用户信息照片 */
#define NENGSHI_SUB_CMD_SEND_PIC 		0x09

/* 从命令字，下发特征文件 */
#define NENGSHI_SUB_CMD_SEND_FEATURE 	0x0A

/* 从命令字，232串口1:n 识别结果上报 */
#define  ONE_TO_MORE_VERIFY_RESULT_REPORT  0x0E	

/*-----------------------------------------------------------------------------------------*/

/* 定义从485接口接收最大数据长度 */
#define MAX_RECV_BUF_SIZE 			(2 * 1024)

/* 定义文件名全名的长度 */
#define MAX_FILE_PATH_LEN			512

/* 定义一个包的最大数据长度*/
//#define MAX_PACKET_SIZE				1024
#define MAX_PACKET_SIZE				512

/* 定义一张图片最大长度 */			
#define MAX_PIC_SIZE				(20 * 1024)

/* 定义用户id长度 */
#define MAX_ID_LEN						8

#define MAX_NAME_LEN					16

/* 定义用户卡号长度 */
#define MAX_CARD_LEN					8

/* 和嘉485请求包 */
typedef struct HeJia_Request
{
	unsigned char head[2];
	unsigned char len[2];
	unsigned char deviceType;
	unsigned char mainCmd;
	unsigned char subCmd;
}HeJia_Request, *LPHeJia_Request;

/* 和嘉485应答包 */
typedef struct HeJia_Response
{
	unsigned char head[2];
	unsigned char len[2];
	unsigned char deviceType;
	unsigned char mainCmd;
	unsigned char subCmd;
}HeJia_Response, *LPHeJia_Response;

/* 能士集中注册模板学员信息 */
typedef struct NengShi_CenterTmpl
{
	unsigned char id[MAX_ID_LEN + 1];
	unsigned char card[MAX_CARD_LEN + 1];
	unsigned char flag;
	unsigned char packCnt;
	unsigned char packIndex;
	unsigned char packData[MAX_PACKET_SIZE];
}NengShi_CenterTmpl, *LPNengShi_CenterTmpl;

typedef struct _time_m
{
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char week;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
}time_m;

static int speed_value[] =
{
    230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 600, 300
};

static int speed_param[] =
{
    B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300
};

char g_updateFileBuf[MAX_UPDATE_FILE_LEN] = {0};

/* 学员人脸特征文件内容 */
char g_faceBuf[MAX_FACE_BUFFER_LEN] = {0};

/* 配置文件最大缓冲区 */
char g_fileBuf[MAX_FILE_BUFFER_LEN] = {0};

/* 特征文件 */
unsigned int g_faceBufOffset = 0;

/* 配置文件 */
unsigned int g_fileBufOffset = 0;

/* 配置文件缓存偏移量 */
unsigned int g_updateBufOffset = 0;

char g_facePicName[MAX_PACKET_SIZE] = {0};
char g_facePicBuf[MAX_PIC_SIZE] = {0};

/* 升级文件内容 */

/* 握手是否通过  1 为通过 0为失败*/
char g_AuthFlag = 0;

#define USER_DATA_LEN 59



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

unsigned short getXorResult(unsigned char * buf, unsigned short len)
{
	unsigned char tmpXor = 0;
	unsigned short ret;

	//printf("len %d %s %d\r\n", len, __FUNCTION__, __LINE__);
	
	if (buf == NULL || len <= 0)
	{
		return 0;
	}
	
	while (len != 0)
	{		
		tmpXor = tmpXor ^ (*buf);
		buf++;
		len--;
	}

	//printf("tmpXor 0x%x %s %d\r\n", tmpXor, __FUNCTION__, __LINE__);

	ret = (tmpXor & 0xFF);
	
	return ret;
}
#if 0
{
	unsigned short tmpXor = 0;

	printf("len %d %s %d\r\n", len, __FUNCTION__, __LINE__);
	
	if (buf == NULL || len <= 0)
	{
		return 0;
	}
	
	while (len != 0)
	{		
		tmpXor = tmpXor ^ ((unsigned short)*buf);
		buf++;
		len--;
	}

	printf("tmpXor 0x%x %s %d\r\n", tmpXor, __FUNCTION__, __LINE__);
	
	return tmpXor;
}
#endif

//返回实际读取的数据长度，-1出错
int ReadFeature(char* pBuf, int nBufLen, char* pFileName)
{
	int handle = -1;
	int nFileLen = 0;
	int nReadLen = 0;
	struct stat st;
	
	if (pBuf == NULL || pFileName == NULL)
	{
		return -1;
	}

	handle = open(pFileName, O_RDONLY);
	if (handle < 0)
	{
		printf("open %s error\n", pFileName);
		
		return -1;
	}

	if ( stat( pFileName, &st) == 0 )
	{
		nFileLen = st.st_size;
	}

	if(nFileLen > nBufLen)
	{
		nFileLen = nBufLen;
		
		printf("nFileLen > nBufLen\n");
	}

	//一次全部读取完
	nReadLen = read(handle, pBuf, nFileLen);

	close(handle);

	return nReadLen;
}

int ReadUpdateFile(char* pBuf, int nBufLen, char* pFileName)
{
	int handle = -1;
	int nFileLen = 0;
	int nReadLen = 0;
	struct stat st;
	
	if (pBuf == NULL || pFileName == NULL)
	{
		return -1;
	}

	handle = open(pFileName, O_RDONLY);
	if (handle < 0)
	{
		printf("open %s error (%s)\n", pFileName, strerror(errno));
		
		return -1;
	}

	if ( stat( pFileName, &st) == 0 )
	{
		nFileLen = st.st_size;
	}

	if(nFileLen > nBufLen)
	{
		nFileLen = nBufLen;
		
		printf("nFileLen > nBufLen\n");
	}

	//一次全部读取完
	nReadLen = read(handle, pBuf, nFileLen);

	close(handle);

	return nReadLen;
}

int CheckPackContent(unsigned char * pPack, int len, int * validLen)
{
	int i = 0;
	int OffSet = -1;
	unsigned char lenLow;
	unsigned char lenHigh;
	unsigned short dataLen;
		
	for (i = 0; i < len; i++)
	{
		if ((pPack[i] == HEJIA_RES_START_DATA0)
			&& (pPack[i + 1] == HEJIA_RES_START_DATA0))
		{
			OffSet = i;
			
			lenLow = pPack[OffSet + 2];
			lenHigh = pPack[OffSet + 3];

			dataLen = ((unsigned short)lenHigh << 8) + lenLow;

			if (dataLen < (/*MAX_PACKETHEJIA_DEVICE_TYPE_SIZE*/1024 + 32)
				&& (pPack[OffSet + 4] == HEJIA_DEVICE_TYPE)
				&& (pPack[OffSet + dataLen - 2] == HEJIA_END_DATA0)
				&& (pPack[OffSet + dataLen - 1] == HEJIA_END_DATA1))
			{
				* validLen = dataLen;
				
				return OffSet;
			}			
		}
	}

	return -1;	
}

/************************************
函数名称：ReadPacket
函数功能: 读一个包的数据
入口参数：
		Handle			: RS485设备句柄
		pValidBuf		: 存放读取的数据
		pValidLen		: 有效长度
返回值  ：
		数据在pValidBuf中的偏移
************************************/
int ReadPacket(int Handle, unsigned char * pValidBuf, int * pValidLen)
{
	int ret= 0;
	int readBufLen = 0;
	int OffSet = -1;
	int validLen = 0;
	
	for (;;)
	{
		ret = read(Handle, (pValidBuf + readBufLen), MAX_RECV_BUF_SIZE);
		if(ret >= 0)
		{
#if 0
			int i;
			for (i = 0; i < readBufLen + ret; i++)
			{
				if (i % 16 == 0)
				{
					printf("\r\n");
				}
				
				printf("0x%02x \t", pValidBuf[i]);
			}
			printf("ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
#endif
			readBufLen += ret;
     
			OffSet = CheckPackContent(pValidBuf, readBufLen, &validLen);
			if (OffSet != -1)
			{
				*pValidLen = validLen;
				
				break;
			}
		}
		else
		{
			printf("errno %d %s %d\r\n", errno, __FUNCTION__, __LINE__);

			break;
		}
	}


	return OffSet;
}

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned long UINT32;

//取字的高字节
#define HIBYTE(w)   ((UINT8) (((UINT16) (w) >> 8) & 0xFF))
//取字的低字节
#define LOBYTE(w)   ((UINT8) (w))

unsigned short crcxor(unsigned char *bufPtr, unsigned short lenth)
{
	unsigned short flag=0;
	while(lenth != 0)
	{
		flag ^= (unsigned short)*bufPtr;
		bufPtr++;
		lenth--;
	}
	return flag;
}

int Rs232RecvMsg(int fd, unsigned char * pValidBuf, int * pValidLen)
{
	static unsigned char recvbuf[1024];
	short readLen = 0;
	short recvLen = 0;
	unsigned short crc = 0;
    
	if (fd < 0)
	{
		printf("%s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	readLen = read(fd, recvbuf, 2);

    if (readLen == 0)
    {
        return -2;
    }
    
	if (readLen != 2)
	{
		printf("readLen %d %s %d\r\n", readLen, __FUNCTION__, __LINE__);
		return -1;
	}
		
	if (!((recvbuf[0] == 0x26) && (recvbuf[1] == 0x26)))
	{
		printf("%s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	readLen = read(fd, recvbuf+2, 2);

	if (readLen != 2)
	{
		printf("%s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	recvLen = recvbuf[2]&0xff;
	recvLen += (((unsigned short)recvbuf[3]) << 8)&0xff00;

	if (recvLen < 12 || recvLen > 1024)
	{
		printf("recvLen %d %s %d\r\n", recvLen, __FUNCTION__, __LINE__);
		return -1;
	}

	printf("%s %d, recvLen:%d\r\n", __FUNCTION__, __LINE__, recvLen);

	unsigned short offset = 4;
	unsigned short readagain = recvLen - 4;
	unsigned short timeOut = 0;
	while(readagain != 0)
	{
		readLen = read(fd, recvbuf+offset, readagain);
		if (readLen == readagain)
		{
			break;
		}

		if (readLen < 0)
		{
			printf("%s %d\r\n", __FUNCTION__, __LINE__);
			return -1;
		}
		else if (readLen == 0)
		{
			timeOut++;
			if (timeOut > 10)
			{
				printf("timeout %s %d\r\n", __FUNCTION__, __LINE__);
				return -1;
			}
            usleep(50*1000);
		}
		readagain -= readLen;
		offset += readLen;
	}
	
	if (!((recvbuf[recvLen-2]==0x0d) && (recvbuf[recvLen-1]==0x0a))) //结束符
	{
		printf("%s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	crc = crcxor(recvbuf, recvLen-4);
	if (!((LOBYTE(crc) == recvbuf[recvLen-4]) && (HIBYTE(crc) == recvbuf[recvLen-3])))
	{
		printf("%s %d\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

    if (recvLen >= 12)
	{
        memcpy(pValidBuf, recvbuf, recvLen); //复制数据
        *pValidLen = recvLen; //数据长度
	}
	else
	{
		*pValidLen = 0;
	}

#if _COMM_DEBUG
	printf("ok ---------------------------------- %s %d\r\n", __FUNCTION__, __LINE__);
#endif

	return 0;
}

time_t DatetimeToTime(char *pDatetime)
{
	struct tm tmTime;
	time_t ti = 0;

	if(pDatetime) {
		sscanf(pDatetime, "%d-%02d-%02d %02d:%02d:%02d", 
			&tmTime.tm_year, &tmTime.tm_mon, &tmTime.tm_mday, &tmTime.tm_hour, &tmTime.tm_min, &tmTime.tm_sec);
		tmTime.tm_year-=1900;
		tmTime.tm_mon-=1;
		ti = mktime(&tmTime);
	}

	return ti;
}


int dealAuthCmd(char *pGetData, int *len)
{
	int ret = 0;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];	
	unsigned short sendXor;
	fd_set readfds;
	struct timeval tv;
	int readBufLen;
	int iOffset = 0;
	int iValidLen = 0;

	/* 握手 */
	/* 23 23 12 00 0b 40 01 14 0c 0b 0f 10 2a 1a 64 00 0d 0a */
	printf(" %s %d\r\n", __FUNCTION__, __LINE__);

    tcflush(handle, TCIOFLUSH);

	time_t ulCurTime;
	struct tm *tm = NULL;

	ulCurTime = time(NULL);
	tm = gmtime(&ulCurTime);
	
	memset(sendBuf, 0, sizeof(sendBuf));

	/* 数据头 2字节 */
	sendBuf[0] = HEJIA_REQ_START_DATA0;
	sendBuf[1] = HEJIA_REQ_START_DATA1;

	/* 数据长度 2字节 */
	sendBuf[2] = 0x12;
	sendBuf[3] = 0x00;

	/* 外设类型 */
	sendBuf[4] = HEJIA_DEVICE_TYPE;

	/* 主命令字 */
	sendBuf[5] = HEJIA_MAIN_CMD;
	sendBuf[6] = HEJIA_SUB_CMD_AUTH;

	/* 数据7个字节的当前时间 */
	sendBuf[7] = ((tm->tm_year + 1900) / 100) & 0xFF;
	sendBuf[8] = ((tm->tm_year + 1900) % 100) & 0xFF;
	sendBuf[9] = (tm->tm_mon + 1) & 0xFF;
	sendBuf[10] = (tm->tm_mday) & 0xFF;
	sendBuf[11] = (tm->tm_hour + 8) & 0xFF;
	sendBuf[12] = (tm->tm_min) & 0xFF;
	sendBuf[13] = (tm->tm_sec) & 0xFF;

	printf("sendBuf[7] %d \r\n", sendBuf[7]);
	printf("sendBuf[8] %d \r\n", sendBuf[8]);
	printf("sendBuf[9] %d \r\n", sendBuf[9]);
	printf("sendBuf[10] %d \r\n", sendBuf[10]);
	printf("sendBuf[11] %d \r\n", sendBuf[11]);
	printf("sendBuf[12] %d \r\n", sendBuf[12]);
	printf("sendBuf[13] %d \r\n", sendBuf[13]);
	
	int i;
	for(i = 7; i < 14; i++)
	{
		printf("buf[%d] %d\t", i, sendBuf[i]);
	}
	
	printf("\r\n");
	
	sendXor = getXorResult(sendBuf, 14);
	printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
	
	sendBuf[14] = sendXor & 0xFF;
	sendBuf[15] = (sendXor >> 8) & 0xFF;

	sendBuf[16] = HEJIA_END_DATA0;
	sendBuf[17] = HEJIA_END_DATA1;

#if _COMM_DEBUG
	printf("---------------------------Auth SendBuf----------------------\r\n");
	for(i = 0; i < sendBuf[2]; i++)
	{
		if(i % 6 == 0)
		{
			printf("\r\n");
		}
		printf("buf[%d] 0x%02x\t", i, sendBuf[i]);
	}
	printf("\r\n");			
#endif

    ret = write(handle, sendBuf, 18);

	//usleep(200);

	FD_ZERO(&readfds);
	FD_SET(handle, &readfds);
	
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	memset(recvBuf, 0, sizeof(recvBuf));
	readBufLen = 0;

	ret = select(handle + 1, &readfds, NULL, NULL, &tv);
	printf("###ret=%d \r\n", ret);
	if (ret > 0 && FD_ISSET(handle, &readfds)) 
	{
		iOffset = ReadPacket(handle, validBuf, &iValidLen);

		memcpy(recvBuf, validBuf + iOffset, iValidLen);
		readBufLen = iValidLen;

        *len = iValidLen;
        memcpy(pGetData, validBuf + iOffset, iValidLen);

		printf("---------------------------Auth RecvBuf----------------------\r\n");
		printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
		for (i = 0; i < readBufLen; i++)
		{
			if (i % 6 == 0)
			{
				printf("\r\n");
			}
			
			printf("buf[%d] 0x%02x\t", i, recvBuf[i]);
		}
		printf("\r\n");
        return (int)recvBuf[7];
	}

    if(ret == 0)
        ret = -2;
    return ret;
}

int dealRegisterCmd(const char *pUserId, const char *pUserName, char *pData, int *pLen)
{
    int ret = 0;
    unsigned char sendBuf[1024];
    unsigned char recvBuf[1024];
    unsigned char validBuf[1024];
    unsigned short sendXor;
    fd_set readfds;
    struct timeval tv;
    int readBufLen;
    int iOffset = 0;
    int iValidLen = 0;
    int i;
    printf(" %s %d\r\n", __FUNCTION__, __LINE__);


    tcflush(handle, TCIOFLUSH);

    if(NULL == pUserId || NULL == pUserName)
        return -10;

    memset(sendBuf, 0, sizeof(sendBuf));

    /* 数据头 2字节 */
    sendBuf[0] = HEJIA_REQ_START_DATA0;
    sendBuf[1] = HEJIA_REQ_START_DATA1;

    /* 数据长度 2字节 */
    sendBuf[2] = 0x23;
    sendBuf[3] = 0x00;

    /* 外设类型 */
    sendBuf[4] = HEJIA_DEVICE_TYPE;

    /* 主命令字 */
    sendBuf[5] = HEJIA_MAIN_CMD;
    sendBuf[6] = NENGSHI_SUB_CMD_REGISTER;

    int idLen = strlen(pUserId);
    int nameLen = strlen(pUserName);
    if(idLen > MAX_ID_LEN || nameLen > MAX_NAME_LEN)
    {
        return -11;
    }
    memcpy(sendBuf + 7,  pUserId, idLen);
    memcpy(sendBuf + 15, pUserName, nameLen);

    sendXor = getXorResult(sendBuf, 31);
    printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);

    sendBuf[31] = sendXor & 0xFF;
    sendBuf[32] = (sendXor >> 8) & 0xFF;

    sendBuf[33] = HEJIA_END_DATA0;
    sendBuf[34] = HEJIA_END_DATA1;

#if _COMM_DEBUG
    printf("---------------------------Register SendBuf----------------------\r\n");
    for(i = 0; i < sendBuf[2]; i++)
    {
        if(i % 6 == 0)
        {
            printf("\r\n");
        }
        printf("buf[%d] 0x%02x\t", i, sendBuf[i]);
    }
    printf("\r\n");
#endif

    write(handle, sendBuf, 35);

    //usleep(200);

    FD_ZERO(&readfds);
    FD_SET(handle, &readfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    memset(recvBuf, 0, sizeof(recvBuf));
    readBufLen = 0;

    ret = select(handle + 1, &readfds, NULL, NULL, &tv);
    printf("###ret=%d \r\n", ret);
    if (ret > 0 && FD_ISSET(handle, &readfds))
    {
        iOffset = ReadPacket(handle, validBuf, &iValidLen);

        memcpy(recvBuf, validBuf + iOffset, iValidLen);
        readBufLen = iValidLen;

        printf("---------------------------Register RecvBuf----------------------\r\n");
        printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
        for (i = 0; i < readBufLen; i++)
        {
            if (i % 6 == 0)
            {
                printf("\r\n");
            }

            printf("buf[%d] 0x%02x\t", i, recvBuf[i]);
        }
        printf("\r\n");

//        if(0x0 != recvBuf[7])
//        {
//            return -2;
//        }

        //        return 0;

        while(1)
        {
            FD_ZERO(&readfds);
            FD_SET(handle, &readfds);

            tv.tv_sec = 5;
            tv.tv_usec = 0;

            memset(recvBuf, 0, sizeof(recvBuf));
            readBufLen = 0;

            ret = select(handle + 1, &readfds, NULL, NULL, &tv);
            if (ret > 0 && FD_ISSET(handle, &readfds))
            {
                iOffset = ReadPacket(handle, validBuf, &iValidLen);

                memcpy(recvBuf, validBuf + iOffset, iValidLen);
                readBufLen = iValidLen;

                printf("-----------------RecvBuf Register result begin----------------------\r\n");
                printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
                for (i = 0; i < readBufLen; i++)
                {
                    if (i % 6 == 0)
                    {
                        printf("\r\n");
                    }

                    printf("buf[%d] 0x%02x\t", i, recvBuf[i]);
                }
                printf("\r\n");


                if(0x3 == recvBuf[6])
                {
                    if(0x0 == recvBuf[7])
                    {
                        return 0;
                    }
                    else if(0x1 == recvBuf[7])
                    {
                        // already registered
                        return -3;
                    }
                    else if(0x2 == recvBuf[7])
                    {
                        // failed to register due to time out
                        return -4;
                    }

                }
                printf("-----------------RecvBuf Register result end----------------------\r\n");
                return 0;
            }
            sleep(1);
        }
    }
    else
    {
        ret = -5;
    }

    return -12;
}


/*
 * 1:1 recognition by the userNumber specified
 *
 *
 * return 1 if verified successfully, 2 on user not found, -1 for other error,
 *  -2 for failing to verify
 */
int dealFaceVerifyCmd(char *userNum, char *pGetName, char *pScore)
{
	int ret = 0;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];	
	unsigned short sendXor;
	fd_set readfds;
	struct timeval tv;
//	int readBufLen;
	int iOffset = 0;
	int iValidLen = 0;

    tcflush(handle, TCIOFLUSH);
	
	printf("%s %d\r\n", __FUNCTION__, __LINE__);

	memset(sendBuf, 0, sizeof(sendBuf));

	/* 数据头 2字节 */
	sendBuf[0] = HEJIA_REQ_START_DATA0;
	sendBuf[1] = HEJIA_REQ_START_DATA1;

    /* 数据长度 2字节 */
    sendBuf[2] = 0x13;
    sendBuf[3] = 0x00;

	/* 外设类型 */
	sendBuf[4] = HEJIA_DEVICE_TYPE;

	/* 主命令字 */
	sendBuf[5] = HEJIA_MAIN_CMD;
	sendBuf[6] = NENGSHI_SUB_CMD_COMPARE;

    int iLen = strlen(userNum);
    if(iLen > MAX_ID_LEN)
    {
        return -3;
    }
    memcpy(sendBuf + 7, userNum, iLen);
	
    sendXor = getXorResult(sendBuf, 15);
	printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
	
	sendBuf[15] = sendXor & 0xFF;
	sendBuf[16] = (sendXor >> 8) & 0xFF;

	sendBuf[17] = HEJIA_END_DATA0;
	sendBuf[18] = HEJIA_END_DATA1;

#if _COMM_DEBUG
	printf("---------------------------FaceVerify SendBuf----------------------\r\n");
	for(i = 0; i < sendBuf[2]; i++)
	{
		if(i % 6 == 0)
		{
			printf("\r\n");
		}
		printf("buf[%d] 0x%02x\t", i, sendBuf[i]);
	}
	printf("\r\n");			
#endif

    write(handle, sendBuf, 19);

	//usleep(200);

	FD_ZERO(&readfds);
	FD_SET(handle, &readfds);
	
    tv.tv_sec = 5;
	tv.tv_usec = 0;

	memset(recvBuf, 0, sizeof(recvBuf));
//	readBufLen = 0;

	ret = select(handle + 1, &readfds, NULL, NULL, &tv);
	printf("###ret=%d \r\n", ret);
	if (ret > 0 && FD_ISSET(handle, &readfds)) 
	{
        iOffset = ReadPacket(handle, validBuf, &iValidLen);

        memcpy(recvBuf, validBuf + iOffset, iValidLen);
//        readBufLen = iValidLen;

//        printf("---------------------------FaceVerify RecvBuf----------------------\r\n");
//        printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
//        for (i = 0; i < readBufLen; i++)
//        {
//            if (i % 6 == 0)
//            {
//                printf("\r\n");
//            }
			
//            printf("buf[%d] 0x%02x\t", i, recvBuf[i]);
//        }
//        printf("\r\n");

        if(recvBuf[7] == 0x01)
        {
            return 2;
        }


        int retry = 5;
        for(;retry > 0; retry--)
		{
			FD_ZERO(&readfds);
			FD_SET(handle, &readfds);
			
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			memset(recvBuf, 0, sizeof(recvBuf));
//			readBufLen = 0;
			
			ret = select(handle + 1, &readfds, NULL, NULL, &tv);
			if(ret > 0 && FD_ISSET(handle, &readfds)) 
			{
                bzero(validBuf, sizeof(validBuf));
                iOffset = ReadPacket(handle, validBuf, &iValidLen);
//	            if(iOffset != 0 || iValidLen == 0)
//	            {
//                    sleep(1);
//	                continue;
//	            }
				memcpy(recvBuf, validBuf + iOffset, iValidLen);
//				readBufLen = iValidLen;
				
//				printf("-----------------RecvBuf FaceVerify result begin----------------------\r\n");
//				printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
//				for (i = 0; i < readBufLen; i++)
//				{
//					if (i % 6 == 0)
//					{
//						printf("\r\n");
//					}
					
//					printf("buf[%d] 0x%02x \t", i, recvBuf[i]);
//				}
//				printf("\r\n");
//				printf("-----------------RecvBuf FaceVerify result begin----------------------\r\n");
//                if(recvBuf[6] != 0x05)
//                    continue;
            	if(recvBuf[7] == 0x00)
				{
                    memcpy(pGetName, &recvBuf[0], iValidLen);
					printf("verify result: failed \n");
                    return -2;
				}
				else
				{
					char acUserName[32 + 1] = {0}; 	/* 用户名 */
					char acUserId[8 + 1] = {0};		/* 用户id */
					char cScore = 1;            /* 识别分值 */
					
                    memcpy(&acUserName, &recvBuf[8], 32);
                    memcpy(pGetName, &recvBuf[8], 32);
					memcpy(&acUserId, &recvBuf[40], 8);
					memcpy(&cScore, &recvBuf[48], 1);
                    *pScore = recvBuf[48];
					printf("verify result: success \n");
					printf("UserName: %s \n", acUserName);
					printf("UserId: %s \n", acUserId);
					printf("Score: %d \n", cScore);

                    return 1;
				}
//				break;
			}
            sleep(1);
		}
	}

    return -1;
}


/*
 * Get user info and picture for the userNum specified
 *
 * return:
 *  0 on success, 1 for user not found, -1 on error
 */
int dealGetUserPicCmd(const char *userNum, char *pGetPic, int *pLen)
{
	printf(" %s %d\r\n", __FUNCTION__, __LINE__);

	unsigned short sendXor;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];	
	fd_set readfds;
	struct timeval tv;
	int readBufLen;
	int iValidLen = 0;
	
	char photoinfo[256];
	int iTotalPackets = 0; /* 执行成功时的总包数 */
	int iIndex = 0;		   /* 执行成功时的包索引 */
	int iOffset = 0;
	int iPicOffset = 0;
	int iCurPacket = 0;
	unsigned int iPacketLen = 0;
//	int hWrite = -1;
//	char PicPath[MAX_FILE_PATH_LEN];
	char studentID[32] = {0};
	char studentName[32] = {0};
//	int i = 0;
	int ret = 0;

    tcflush(handle, TCIOFLUSH);

	memset(g_facePicBuf, 0, sizeof(g_facePicBuf));
	memset(sendBuf, 0, sizeof(sendBuf));
	
    sendBuf[0] = HEJIA_REQ_START_DATA0;
    sendBuf[1] = HEJIA_REQ_START_DATA1;
    sendBuf[2] = 0x14;
    sendBuf[3] = 0x00;
    sendBuf[4] = HEJIA_DEVICE_TYPE;
    sendBuf[5] = HEJIA_MAIN_CMD;
    sendBuf[6] = NENGSHI_GET_USER_INFO_PIC;  /* 发送获取照片命令时无数据部分 */

    int iLen = strlen(userNum);
    if(iLen > MAX_ID_LEN)
    {
        return -1;
    }
    memcpy(sendBuf + 7, userNum, iLen);
//	memcpy(sendBuf + 7, "1001", sizeof("1001"));
    sendBuf[15] = 0;

    sendXor = getXorResult(sendBuf, 16);

    sendBuf[16] = sendXor & 0xFF;
    sendBuf[17] = (sendXor >> 8) & 0xFF;

    sendBuf[18] = HEJIA_END_DATA0;
    sendBuf[19] = HEJIA_END_DATA1;

    write(handle, sendBuf, 20);

	sleep(1);

	FD_ZERO(&readfds);
	FD_SET(handle, &readfds);
	
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	memset(recvBuf, 0, sizeof(recvBuf));
	readBufLen = 0;
	
	ret = select(handle + 1, &readfds, NULL, NULL, &tv);
	if(ret > 0 && FD_ISSET(handle, &readfds)) 
	{
		iOffset = ReadPacket(handle, validBuf, &iValidLen);

		memcpy(recvBuf, validBuf + iOffset, iValidLen);
		readBufLen = iValidLen;
			
		printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);

        if(recvBuf[6] != 0x06)
        {
            return recvBuf[6] ;
        }
		if(recvBuf[7] == 0x00) /* 用户不存在 */
		{
			printf("no user info .... \r\n");
            return 1;
		}
		else
		{
			if(readBufLen < 512)
			{
				printf("Have info, but don't have picture .... \r\n");
			}
			else
			{
                printf("Have picture user .... \r\n");
			}
			
			iTotalPackets = recvBuf[31];
			iIndex = recvBuf[32];

			memcpy(studentID, recvBuf + 7, 8);
			memcpy(studentName, recvBuf + 15, 16);
			
			printf("iTotalPackets: %d, %s %d\r\n", iTotalPackets, __FUNCTION__, __LINE__);
			printf("iIndex: %d, %s %d\r\n", iIndex, __FUNCTION__, __LINE__);
			printf("studentID: %s, %s %d\r\n", studentID, __FUNCTION__, __LINE__);
			printf("studentName: %s, %s %d\r\n", studentName, __FUNCTION__, __LINE__);

			snprintf(photoinfo, sizeof(photoinfo), "UserPic.jpg");
				
			iPacketLen = readBufLen - 37;
			
			memcpy(g_facePicBuf + iPicOffset, recvBuf + 33, iPacketLen);  //包索引为0的包接收完成

			iPicOffset += iPacketLen;
			

			/* 获得总包数后，包索引从1开始获取数据包 */
			for (iCurPacket = 1; iCurPacket < iTotalPackets; iCurPacket++)
			{
				memset(sendBuf, 0, sizeof(sendBuf));
	
				sendBuf[0] = HEJIA_REQ_START_DATA0;
				sendBuf[1] = HEJIA_REQ_START_DATA1;
				sendBuf[2] = 0x0C;
				sendBuf[3] = 0x00;
				sendBuf[4] = HEJIA_DEVICE_TYPE;
				sendBuf[5] = HEJIA_MAIN_CMD;
				sendBuf[6] = NENGSHI_GET_USER_INFO_PIC;

				sendBuf[7] = iCurPacket;
				
				sendXor = getXorResult(sendBuf, 8);
				printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
				
				sendBuf[8] = sendXor & 0xFF;
				sendBuf[9] = (sendXor >> 8) & 0xFF;

				sendBuf[10] = HEJIA_END_DATA0;
				sendBuf[11] = HEJIA_END_DATA1;
				
				write(handle, sendBuf, 12);

				sleep(1);

				FD_ZERO(&readfds);
				FD_SET(handle, &readfds);
				
				tv.tv_sec = 2;
				tv.tv_usec = 0;

				memset(recvBuf, 0, sizeof(recvBuf));
				readBufLen = 0;
				
				ret = select(handle + 1, &readfds, NULL, NULL, &tv);
				if(ret > 0 && FD_ISSET(handle, &readfds))
				{
					iOffset = ReadPacket(handle, validBuf, &iValidLen);

					memcpy(recvBuf, validBuf + iOffset, iValidLen);
					readBufLen = iValidLen;
						
					printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);

					if(recvBuf[7] == 0x00) /* 接收数据包失败 */
					{
						printf("Recv Packet %d fail, Retry.... \r\n", iCurPacket);
						iCurPacket = iCurPacket - 1;  //重新获取包序号为iCurPacket的包
						continue;
					}
					else
					{
						iTotalPackets = recvBuf[31];
						iIndex = recvBuf[32];

						printf("Total Packet %d, iIndex %d, Recv Packet %d is OK!\r\n", iTotalPackets, iIndex, iCurPacket);
						

						iPacketLen = readBufLen - 37;
			
						memcpy(g_facePicBuf + iPicOffset, recvBuf + 33, iPacketLen);  //包索引为0的包接收完成

						iPicOffset += iPacketLen;
					}
				}
				
				usleep(10 * 1000);
			}

			usleep(10 * 1000);

			if(iPicOffset > 0)
			{
                memcpy(pGetPic, g_facePicBuf, iPicOffset);
                *pLen = iPicOffset;
                return 0;
//				sprintf(PicPath, "%s", photoinfo);
//				hWrite = open(PicPath, O_RDWR | O_CREAT);
//				if(hWrite < 0)
//				{
//					printf("open file %s error\n", PicPath);
//				}
//				else
//				{
//					printf("iPicOffset %d\n", iPicOffset);
					
//					write(hWrite, g_facePicBuf, iPicOffset);

//					printf("Write %s ok %s %d\n", PicPath, __FUNCTION__, __LINE__);
//					close(hWrite);
//				}
			}
		}
	}

    return -1;
}

void dealGetUserFeatureCmd(const char *pUserID, char *pGetData, int *pLen)
{
	printf(" %s %d\r\n", __FUNCTION__, __LINE__);

	unsigned short sendXor;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];	
	fd_set readfds;
	struct timeval tv;
	int readBufLen;
	int iValidLen = 0;
	
	char photoinfo[256];
	int iTotalPackets = 0; /* 执行成功时的总包数 */
	int iIndex = 0;		   /* 执行成功时的包索引 */
	int iOffset = 0;
	int iPicOffset = 0;
	int iCurPacket = 0;
	unsigned int iPacketLen = 0;
	int hWrite = -1;
	char PicPath[MAX_FILE_PATH_LEN];
	char studentID[32] = {0};
	char studentName[32] = {0};
	int i = 0;
	int ret = 0;
	memset(g_faceBuf, 0, sizeof(g_faceBuf));
	memset(sendBuf, 0, sizeof(sendBuf));
	
	sendBuf[0] = HEJIA_REQ_START_DATA0;
	sendBuf[1] = HEJIA_REQ_START_DATA1;
	sendBuf[2] = 0x14;
	sendBuf[3] = 0x00;
	sendBuf[4] = HEJIA_DEVICE_TYPE;
	sendBuf[5] = HEJIA_MAIN_CMD;
	sendBuf[6] = NENGSHI_GET_USER_INFO_FEATURE;  /* 发送获取照片命令时无数据部分 */

    memcpy(sendBuf + 7, pUserID, strlen(pUserID));
	sendBuf[15] = 0;
	
	sendXor = getXorResult(sendBuf, 16);
	printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
	
	sendBuf[16] = sendXor & 0xFF;
	sendBuf[17] = (sendXor >> 8) & 0xFF;

	sendBuf[18] = HEJIA_END_DATA0;
	sendBuf[19] = HEJIA_END_DATA1;
	
	write(handle, sendBuf, 20);

	sleep(1);

	FD_ZERO(&readfds);
	FD_SET(handle, &readfds);
	
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	memset(recvBuf, 0, sizeof(recvBuf));
	readBufLen = 0;
	
	ret = select(handle + 1, &readfds, NULL, NULL, &tv);
	if(ret > 0 && FD_ISSET(handle, &readfds)) 
	{
		iOffset = ReadPacket(handle, validBuf, &iValidLen);

		memcpy(recvBuf, validBuf + iOffset, iValidLen);
		readBufLen = iValidLen;
			
		printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
#if 0
		for (i = 0; i < 12; i++)
		{
			if (i % 8 == 0)
			{
				printf("\r\n");
			}
			
			printf("buf[%d] 0x%02x\t", i, recvBuf[i]);
		}
		printf("\r\n");
#endif
		if(recvBuf[7] == 0x00) /* 用户不存在 */
		{
			printf("no user info .... \r\n");
		}
		else
		{
			if(readBufLen < 512)
			{
				printf("Have info, but don't have picture .... \r\n");
			}
			else
			{
				printf("Have picture user .... \r\n");
			}
			
			iTotalPackets = recvBuf[31];
			iIndex = recvBuf[32];

			memcpy(studentID, recvBuf + 7, 8);
			memcpy(studentName, recvBuf + 15, 16);
			
			printf("iTotalPackets: %d, %s %d\r\n", iTotalPackets, __FUNCTION__, __LINE__);
			printf("iIndex: %d, %s %d\r\n", iIndex, __FUNCTION__, __LINE__);
			printf("studentID: %s, %s %d\r\n", studentID, __FUNCTION__, __LINE__);
			printf("studentName: %s, %s %d\r\n", studentName, __FUNCTION__, __LINE__);

			//snprintf(photoinfo, sizeof(photoinfo), "user_pic_%s", studentID);
				
			iPacketLen = readBufLen - 37;
			
			memcpy(g_faceBuf + iPicOffset, recvBuf + 33, iPacketLen);  //包索引为0的包接收完成

			iPicOffset += iPacketLen;
			
			/* 获得总包数后，包索引从1开始获取数据包 */
			for (iCurPacket = 1; iCurPacket < iTotalPackets; iCurPacket++)
			{
				memset(sendBuf, 0, sizeof(sendBuf));
	
				sendBuf[0] = HEJIA_REQ_START_DATA0;
				sendBuf[1] = HEJIA_REQ_START_DATA1;
				sendBuf[2] = 0x0C;
				sendBuf[3] = 0x00;
				sendBuf[4] = HEJIA_DEVICE_TYPE;
				sendBuf[5] = HEJIA_MAIN_CMD;
				sendBuf[6] = NENGSHI_GET_USER_INFO_FEATURE;
				
				sendBuf[7] = iCurPacket;
				sendXor = getXorResult(sendBuf, 8);
				printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
				
				sendBuf[8] = sendXor & 0xFF;
				sendBuf[9] = (sendXor >> 8) & 0xFF;

				sendBuf[10] = HEJIA_END_DATA0;
				sendBuf[11] = HEJIA_END_DATA1;
				
				write(handle, sendBuf, 12);

				sleep(1);

				FD_ZERO(&readfds);
				FD_SET(handle, &readfds);
				
				tv.tv_sec = 2;
				tv.tv_usec = 0;

				memset(recvBuf, 0, sizeof(recvBuf));
				readBufLen = 0;
				
				ret = select(handle + 1, &readfds, NULL, NULL, &tv);
				if(ret > 0 && FD_ISSET(handle, &readfds))
				{
					iOffset = ReadPacket(handle, validBuf, &iValidLen);

					memcpy(recvBuf, validBuf + iOffset, iValidLen);
					readBufLen = iValidLen;
						
					printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
#if 0
					for (i = 0; i < 12; i++)
					{
						if (i % 6 == 0)
						{
							printf("\r\n");
						}
						
						printf("buf[%d] 0x%02x\t", i, recvBuf[i]);
					}
					printf("\r\n");
#endif
					if(recvBuf[7] == 0x00) /* 接收数据包失败 */
					{
						printf("Recv Packet %d fail, Retry.... \r\n", iCurPacket);
						iCurPacket = iCurPacket - 1;  //重新获取包序号为iCurPacket的包
						continue;
					}
					else
					{
						iTotalPackets = recvBuf[31];
						iIndex = recvBuf[32];

						printf("Total Packet %d, iIndex %d, Recv Packet %d is OK!\r\n", iTotalPackets, iIndex, iCurPacket);
						

						iPacketLen = readBufLen - 37;
			
						memcpy(g_faceBuf + iPicOffset, recvBuf + 33, iPacketLen);  //包索引为0的包接收完成

						iPicOffset += iPacketLen;
					}
				}
				
                usleep(5 * 1000);
			}

            usleep(5 * 1000);

			if(iPicOffset > 0)
			{
                sprintf(PicPath, "FeatureTest.dat");
                system("rm FeatureTest.dat");
                hWrite = open(PicPath, O_RDWR | O_CREAT);
                if(hWrite < 0)
                {
                    printf("open file %s error\n", PicPath);
                    *pLen = -1;
                }
                else
                {
                    printf("iPicOffset %d\n", iPicOffset);
					
                    write(hWrite, g_faceBuf, iPicOffset);
//                    memcpy(pGetData, g_faceBuf, iPicOffset);
                    *pLen = iPicOffset;
//                    printf("Write %s ok %s %d\n", PicPath, __FUNCTION__, __LINE__);
                    close(hWrite);
                }
			}
		}
	}
}

void dealDelUserInfoCmd(int handle)
{
	int ret = 0;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];	
	unsigned short sendXor;
	fd_set readfds;
	struct timeval tv;
	int readBufLen;
	int iOffset = 0;
	int iValidLen = 0;
	int i;

	printf(" %s %d\r\n", __FUNCTION__, __LINE__);
	char stuID[32] = {0};
	strcpy(stuID, "1001");
	
	memset(sendBuf, 0, sizeof(sendBuf));
	
	sendBuf[0] = HEJIA_REQ_START_DATA0;
	sendBuf[1] = HEJIA_REQ_START_DATA1;
	sendBuf[2] = 0x13;
	sendBuf[3] = 0x00;
	sendBuf[4] = HEJIA_DEVICE_TYPE;
	sendBuf[5] = HEJIA_MAIN_CMD;
	sendBuf[6] = NENGSHI_SUB_DEL_USER_INFO;

	memcpy(sendBuf + 7, stuID, 8);
		
	sendXor = getXorResult(sendBuf, 15);
	printf("sendXor 0x%04x %s %d\r\n", sendXor, __FUNCTION__, __LINE__);
	
	sendBuf[15] = sendXor & 0xFF;
	sendBuf[16] = (sendXor >> 8) & 0xFF;

	sendBuf[17] = HEJIA_END_DATA0;
	sendBuf[18] = HEJIA_END_DATA1;

#if _COMM_DEBUG
	printf("---------------------------dealUser SendBuf----------------------\r\n");
	for(i = 0; i < sendBuf[2]; i++)
	{
		if(i % 6 == 0)
		{
			printf("\r\n");
		}
		printf("buf[%d] 0x%02x\t", i, sendBuf[i]);
	}
	printf("\r\n");
#endif

	write(handle, sendBuf, 19);

	sleep(1);

	FD_ZERO(&readfds);
	FD_SET(handle, &readfds);
	
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	memset(recvBuf, 0, sizeof(recvBuf));
	readBufLen = 0;
	
	ret = select(handle + 1, &readfds, NULL, NULL, &tv);
	if(ret > 0 && FD_ISSET(handle, &readfds)) 
	{
		iOffset = ReadPacket(handle, validBuf, &iValidLen);

		memcpy(recvBuf, validBuf + iOffset, iValidLen);
		readBufLen = iValidLen;
			
#if _COMM_DEBUG	
		printf("---------------------------dealUser RecvBuf----------------------\r\n");
		printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
		
		for (i = 0; i < readBufLen; i++)
		{
			if (i % 6 == 0)
			{
				printf("\r\n");
			}
			
			printf("buf[%d] 0x%02x \t", i, recvBuf[i]);
		}
		printf("\r\n");
		printf("recvBuf[7] 0x%02X Del User %s..... %s %d\r\n", recvBuf[7], 
		(recvBuf[7] == 0x01) ? "Succ" : "Fail", __FUNCTION__, __LINE__);
#endif
	}
}

void dealPutUserPicCmd(int handle)
{
	int ret = 0;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];	
	unsigned short sendXor;
	fd_set readfds;
	struct timeval tv;
	int readBufLen;
	int iOffset = 0;
	int iValidLen = 0;
	
	char FeaturePath[MAX_FILE_PATH_LEN];
	int nPicLen = 0;
	int nPacketTotal = 0;
	unsigned char nPacketCur = 0;
	int nPacketDataLen = 0;
	int i = 0;
	
	char ID[32] = {0};
	char Name[32] = {0};

	sprintf(ID, "1001");
	sprintf(Name, "张三");
	
	sprintf(FeaturePath, "UserPic.jpg");
	
	nPicLen = ReadFeature(g_faceBuf, MAX_FACE_BUFFER_LEN, FeaturePath);
	g_faceBufOffset = 0;

	printf("nPicLen %d \r\n", nPicLen);

    if(nPicLen == -1)
    {
        printf("Open Pic fail, please check UserPic.jpg \r\n");
        return ;
    }
		
	nPacketTotal = nPicLen / MAX_PACKET_SIZE;
	
	if((nPicLen % MAX_PACKET_SIZE) != 0)
	{
		nPacketTotal++; 
	}
	
	printf("Packet Total is %d\r\n", nPacketTotal);
	
	for(nPacketCur = 0; nPacketCur < nPacketTotal; nPacketCur++)
	{
		printf("Cur packet is %d, Total packet is %d\r\n", nPacketCur, nPacketTotal);
		
		memset(sendBuf, 0, sizeof(sendBuf));
		
		sendBuf[0] = HEJIA_REQ_START_DATA0;
		sendBuf[1] = HEJIA_REQ_START_DATA1;
		sendBuf[4] = HEJIA_DEVICE_TYPE;
		sendBuf[5] = HEJIA_MAIN_CMD;
		sendBuf[6] = NENGSHI_SUB_CMD_SEND_PIC;
		
		memcpy(sendBuf + 7, ID, 8);
		memcpy(sendBuf + 15, Name, 16);

		sendBuf[31] = nPacketTotal;
		sendBuf[32] = nPacketCur;
		
		/* 拷备Feature 数据 */
		if((nPicLen - g_faceBufOffset) >= MAX_PACKET_SIZE)
		{
			nPacketDataLen = MAX_PACKET_SIZE;
		}
		else
		{
			nPacketDataLen = nPicLen - g_faceBufOffset;
		}

		printf("g_faceBufOffset is %d, nPacketDataLen is %d\r\n", g_faceBufOffset, nPacketDataLen);

		memcpy(sendBuf + 33, g_faceBuf + g_faceBufOffset, nPacketDataLen);
		
		g_faceBufOffset += nPacketDataLen;

		/* 数据长度 从数据头到结束符的整个包的长度 */
		sendBuf[2] = (33 + nPacketDataLen + 4) & 0xff;
		sendBuf[3] = ((33 + nPacketDataLen + 4) >> 8) & 0xff;

		/* 从数据头到到校验值之前的全部数据 */
		sendXor = getXorResult(sendBuf, 33 + nPacketDataLen);
		
		sendBuf[33 + nPacketDataLen] = sendXor & 0xFF;
		sendBuf[33 + nPacketDataLen + 1] = (sendXor >> 8) & 0xFF;

		sendBuf[33 + nPacketDataLen + 2] = HEJIA_END_DATA0;
		sendBuf[33 + nPacketDataLen + 3] = HEJIA_END_DATA1;


		
		ret = write(handle, sendBuf, 37 + nPacketDataLen);

		printf("write ret is %d %s %d\r\n", ret, __FUNCTION__, __LINE__);

#if 0
		printf("---------------------------SendBuf----------------------\r\n");
		for (i = 0; i < 37 + nPacketDataLen; i++)
		{
			if (i % 8 == 0)
			{
				printf("\r\n");
			}
			
			printf("0x%02x\t", sendBuf[i]);
		}
		printf("\r\n%s %d\r\n", __FUNCTION__, __LINE__);
#endif

		FD_ZERO(&readfds);
		FD_SET(handle, &readfds);
		
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		memset(validBuf, 0, sizeof(validBuf));
		memset(recvBuf, 0, sizeof(recvBuf));
		readBufLen = 0;
		iValidLen = 0;
		
		ret = select(handle + 1, &readfds, NULL, NULL, &tv);

		printf("ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		
		if(ret > 0 && FD_ISSET(handle, &readfds)) 
		{
			iOffset = ReadPacket(handle, validBuf, &iValidLen);
		
			memcpy(recvBuf, validBuf + iOffset, iValidLen);
			readBufLen = iValidLen;
			
			printf("value %d %s \r\n", recvBuf[33], ((recvBuf[33] == 0x01) ? "ok" : "fail"));
#if 0	
			printf("---------------------------RecvBuf----------------------\r\n");
			printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
			for (i = 0; i < readBufLen; i++)
			{
				if (i % 6 == 0)
				{
					printf("\r\n");
				}
				
				printf("buf[%d] 0x%02x\t", i, recvBuf[i]);
			}
			printf("\r\n");
#endif
			if (recvBuf[33] != 0x01)
			{
				g_faceBufOffset -= nPacketDataLen;
				nPacketCur--;
			}
		}
		else
		{
			g_faceBufOffset -= nPacketDataLen;
			nPacketCur--;
		}
		
		sleep(1);
	}

	printf("All pic data have send\n");
}


void dealPutUserFeatureCmd(int handle)
{
	int ret = 0;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];	
	unsigned short sendXor;
	fd_set readfds;
	struct timeval tv;
	int readBufLen;
	int iOffset = 0;
	int iValidLen = 0;
	
	char FeaturePath[MAX_FILE_PATH_LEN];
	int nFeatureLen = 0;
	int nPacketTotal = 0;
	unsigned char nPacketCur = 0;
	int nPacketDataLen = 0;
	
	char ID[32] = {0};
	char Name[32] = {0};

	sprintf(ID, "1001");
	sprintf(Name, "张三");
	
	sprintf(FeaturePath, "FeatureTest.dat");
	
	nFeatureLen = ReadFeature(g_faceBuf, MAX_FACE_BUFFER_LEN, FeaturePath);
	g_faceBufOffset = 0;

	printf("nFeatureLen %d \r\n", nFeatureLen);

    if(nFeatureLen == -1)
    {
        printf("Open feature fail, please check feature FeatureTest.dat \r\n");
        return ;
    }
		
	nPacketTotal = nFeatureLen / MAX_PACKET_SIZE;
	
	if((nFeatureLen % MAX_PACKET_SIZE) != 0)
	{
		nPacketTotal++; 
	}
	
	printf("Packet Total is %d\r\n", nPacketTotal);
	
	for(nPacketCur = 0; nPacketCur < nPacketTotal; nPacketCur++)
	{
		printf("Cur packet is %d, Total packet is %d\r\n", nPacketCur, nPacketTotal);
		
		memset(sendBuf, 0, sizeof(sendBuf));
		
		sendBuf[0] = HEJIA_REQ_START_DATA0;
		sendBuf[1] = HEJIA_REQ_START_DATA1;
		sendBuf[4] = HEJIA_DEVICE_TYPE;
		sendBuf[5] = HEJIA_MAIN_CMD;
		sendBuf[6] = NENGSHI_SUB_CMD_SEND_FEATURE;
		
		memcpy(sendBuf + 7, ID, 8);
		memcpy(sendBuf + 15, Name, 16);

		sendBuf[31] = nPacketTotal;
		sendBuf[32] = nPacketCur;
		
		/* 拷备Feature 数据 */
		if((nFeatureLen - g_faceBufOffset) >= MAX_PACKET_SIZE)
		{
			nPacketDataLen = MAX_PACKET_SIZE;
		}
		else
		{
			nPacketDataLen = nFeatureLen - g_faceBufOffset;
		}

		memcpy(sendBuf + 33, g_faceBuf + g_faceBufOffset, nPacketDataLen);
		
		g_faceBufOffset += nPacketDataLen;

		/* 数据长度 从数据头到结束符的整个包的长度 */
		sendBuf[2] = (33 + nPacketDataLen + 4) & 0xff;
		sendBuf[3] = ((33 + nPacketDataLen + 4) >> 8) & 0xff;

		/* 从数据头到到校验值之前的全部数据 */
		sendXor = getXorResult(sendBuf, 33 + nPacketDataLen);
		
		sendBuf[33 + nPacketDataLen] = sendXor & 0xFF;
		sendBuf[33 + nPacketDataLen + 1] = (sendXor >> 8) & 0xFF;

		sendBuf[33 + nPacketDataLen + 2] = HEJIA_END_DATA0;
		sendBuf[33 + nPacketDataLen + 3] = HEJIA_END_DATA1;
		
		ret = write(handle, sendBuf, 37 + nPacketDataLen);
		
		printf("write ret is %d %s %d\r\n", ret, __FUNCTION__, __LINE__);

#if 0
		printf("---------------------------SendBuf----------------------\r\n");
		for (i = 0; i < 37 + nPacketDataLen; i++)
		{
			if (i % 8 == 0)
			{
				printf("\r\n");
			}
			
			printf("0x%02x\t", sendBuf[i]);
		}
		printf("\r\n%s %d\r\n", __FUNCTION__, __LINE__);
#endif
		
		FD_ZERO(&readfds);
		FD_SET(handle, &readfds);
		
		tv.tv_sec = 3;
		tv.tv_usec = 0;

		memset(validBuf, 0, sizeof(validBuf));
		memset(recvBuf, 0, sizeof(recvBuf));
		readBufLen = 0;
		iValidLen = 0;
		
		ret = select(handle + 1, &readfds, NULL, NULL, &tv);

		printf("ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		
		if(ret > 0 && FD_ISSET(handle, &readfds)) 
		{
			iOffset = ReadPacket(handle, validBuf, &iValidLen);
		
			memcpy(recvBuf, validBuf + iOffset, iValidLen);
			readBufLen = iValidLen;
			
			printf("value %d %s \r\n", recvBuf[33], ((recvBuf[33] == 0x01) ? "ok" : "fail"));
#if 0	
			printf("---------------------------RecvBuf----------------------\r\n");
			printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
			for (i = 0; i < readBufLen; i++)
			{
				if (i % 6 == 0)
				{
					printf("\r\n");
				}
				
				printf("buf[%d] 0x%02x\t", i, recvBuf[i]);
			}
			printf("\r\n");
#endif
			if (recvBuf[33] != 0x01)
			{
				g_faceBufOffset -= nPacketDataLen;
				nPacketCur--;
			}
		}
		else
		{
			g_faceBufOffset -= nPacketDataLen;
			nPacketCur--;
		}
		
		sleep(1);
	}

	printf("All feature data have send\n");
}

void recvReportVerifyResult(char *pUser, char *pId, char *pScore)
{
	int ret = 0;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];	
	unsigned short sendXor;
	fd_set readfds;
	struct timeval tv;
	int readBufLen;
	int iOffset = 0;
	int iValidLen = 0;
	int i = 0;
    printf("---------------------------Wait----------------------\r\n");

	while(1)
	{
		FD_ZERO(&readfds);
		FD_SET(handle, &readfds);
		
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		memset(recvBuf, 0, sizeof(recvBuf));
		readBufLen = 0;
		
		ret = select(handle + 1, &readfds, NULL, NULL, &tv);
		if(ret > 0 && FD_ISSET(handle, &readfds)) 
		{
            iOffset = Rs232RecvMsg(handle, validBuf, &iValidLen);

            if(iOffset != 0 || iValidLen == 0)
            {
                continue;
            }
			memcpy(recvBuf, validBuf + iOffset, iValidLen);
			readBufLen = iValidLen;
				
#if _COMM_DEBUG	
			printf("---------------------------RecvBuf----------------------\r\n");
			printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
			for (i = 0; i < readBufLen; i++)
			{
				if (i % 6 == 0)
				{
					printf("\r\n");
				}
				
				printf("buf[%d] 0x%02x \t", i, recvBuf[i]);
			}
			printf("\r\n");
#endif

            if(recvBuf[6] == ONE_TO_MORE_VERIFY_RESULT_REPORT)
            {
            	if(recvBuf[7] == 0x00)
				{

					printf("verify result: failed \n");
				}
				else
				{
					char acUserName[32 + 1] = {0}; 	/* 用户名 */
					char acUserId[8 + 1] = {0};		/* 用户id */
					char cScore = 1;            /* 识别分值 */
					

					memcpy(&acUserName, &recvBuf[8], 32);
                    memcpy(pUser, &recvBuf[8], 32);
					memcpy(&acUserId, &recvBuf[40], 8);
                    memcpy(pId, &recvBuf[40], 8);
					memcpy(&cScore, &recvBuf[48], 1);
                    *pScore = recvBuf[48];
					printf("verify result: success \n");
					printf("UserName: %s \n", acUserName);
					printf("UserId: %s \n", acUserId);
					printf("Score: %d \n", cScore);
				}
            }
		}

		sleep(1);
	}	
}

int dealCmd(int handle, char cmd)
{
	int ret = 0;
	unsigned char iSendIndex = 0;
	unsigned char sendBuf[1024];
	unsigned char recvBuf[1024];
	unsigned char validBuf[1024];	
	unsigned short sendXor;
	fd_set readfds;
	struct timeval tv;
	int readBufLen;
	unsigned int i;
	int iOffset = 0;
	int iValidLen = 0;

	{
		FD_ZERO(&readfds);
		FD_SET(handle, &readfds);
		
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		memset(recvBuf, 0, sizeof(recvBuf));
		readBufLen = 0;

		ret = select(handle + 1, &readfds, NULL, NULL, &tv);
		if (ret > 0 && FD_ISSET(handle, &readfds)) 
		{
			iOffset = ReadPacket(handle, validBuf, &iValidLen);

			memcpy(recvBuf, validBuf + iOffset, iValidLen);
			readBufLen = iValidLen;

			printf("begin-----------------RecvBuf: UPDATA----------------------\r\n");
			printf("readBufLen %d %s %d\r\n", readBufLen, __FUNCTION__, __LINE__);
			for (i = 0; i < readBufLen; i++)
			{
				if (i % 6 == 0)
				{
					printf("\r\n");
				}
				
				printf("buf[%d] 0x%02x\t", i, recvBuf[i]);
			}
			printf("\r\n");
			printf("end-------------------RecvBuf: UPDATA----------------------\r\n");
		}
	}
	
	switch(cmd)
	{
		/* 从命令字，握手信息 */
		case '1':
		{
//            dealAuthCmd();
			break;
		}
		/* 从命令字，人员注册 */
		case '2':
		{
//            dealRegisterCmd();
			break;
		}
		/* 从命令字，比对操作 */
		case '3':
		{
//			dealFaceVerifyCmd(handle);
			break;
		}
		/* 从命令字，获取用户信息和照片 */
		case '4':
		{
//			dealGetUserPicCmd(handle);
			break;
		}			
		/* 从命令字，获取用户特征 */
		case '5':
		{
//			dealGetUserFeatureCmd(handle);
			break;
		}	
		/* 从命令字，删除用户信息 */
		case '6':
		{
			dealDelUserInfoCmd(handle);
			break;
		}
		/* 从命令字，下发用户信息照片 */
		case '7':
		{
			dealPutUserPicCmd(handle);
			break;
		}
		/* 从命令字，下发特征文件 */
		case '8':
		{
			dealPutUserFeatureCmd(handle);
			break;
		}
		/* 从命令字，232串口1:n 识别结果上报 */
        case 'f':
        {
//            recvReportVerifyResult();
			break;
		}
		default:
		{
			printf("\r\n");
			printf("1: (0x01) auth\r\n");
			printf("2: (0x02) Register user\r\n");
			printf("3: (0x04) user face verify\r\n");
			printf("4: (0x06) get user pic\r\n");
			printf("5: (0x07) get user feature\r\n");
			printf("6: (0x08) delete user info\r\n");
			printf("7: (0x09) put user pic\r\n");
			printf("8: (0x0a) put user feature\r\n");
            printf("f: (0x0e) get verify result.\r\n");
			printf("\r\n");
			break;
		}
	}

	return ret;
}


int openFacialDev(void)
{
    if(handle >= 0)
    {
        close(handle);
    }

    // UART3:ttyS3, UART0:ttyS0
    handle = open("/dev/ttyS0", O_RDWR);

    if(handle >= 0)
    {
        set_serial_param(handle, 115200, 8, 1, 'n', 0);
    }
    else
    {
        perror("open");
        return -1;
    }

    return 0;
}


int closeFacialDev(void)
{
    close(handle);
}


#if 0
int main(int argc, char* argv[])
{
	int handle = -1;
	char devPath[64];
	char cmd = 0;
	int ret = 0;
	
	sprintf(devPath, "/dev/ttyS%s", argv[1]);

	printf("devPath %s %s %d\r\n", devPath, __FUNCTION__, __LINE__);
	
	handle = open(devPath, O_RDWR);
	printf("handle %d %s %d\r\n", handle, __FUNCTION__, __LINE__);
	if(handle >= 0)
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);
		
		set_serial_param(handle, 115200, 8, 1, 'n', 0);
	}
	else
	{
		printf(" %s %d\r\n", __FUNCTION__, __LINE__);

		return -1;
	}
	
	while(1)
	{
		printf("\r\nPlease input cmd:");
		cmd = getchar();
		
		if(((cmd >= '0') && (cmd <= '9')) || ((cmd >= 'a') && (cmd <= 'z')))
		{
			while ((getchar()) != '\n')
			{
			}
		}

		if ((cmd == 'q') || (cmd == 'Q'))
		{
			break;
		}

		/*if(g_AuthFlag == 0 && (cmd != '1' && cmd != 0x0a))
		{
			printf("cmd is invalid\n");
			continue;
		}*/
		
		ret = dealCmd(handle, cmd);
		
		if(ret < 0)
		{
			break;
		}
	}
	
	close(handle);
	
	return 0;
}
#endif

#ifdef __cplusplus
}
#endif

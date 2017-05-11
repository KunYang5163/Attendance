#ifndef FACIAL_H
#define FACIAL_H


#ifdef __cplusplus
extern "C" {
#endif
int openFacialDev(void);
int closeFacialDev(void);

// 握手
int dealAuthCmd(char *pGetData, int *len);
int dealRegisterCmd(const char *pUserId, const char *pUserName, char *pData, int *pLen);
int dealFaceVerifyCmd(char *userNum, char *pGetName, char *pScore);
void recvReportVerifyResult(char *pUser, char *pId, char *pScore);
int dealGetUserPicCmd(char *userNum, char *pGetPic, int *pLen);
void dealGetUserFeatureCmd(const char *pUserID, char *pGetData, int *pLen);

#ifdef __cplusplus
}
#endif

#endif // FACIAL_H

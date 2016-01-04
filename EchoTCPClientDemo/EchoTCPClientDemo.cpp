// EchoTCPClientDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>       
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;

// 连接到winsock2对应的lib文件： Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

//定义默认的缓冲区长度和端口号
#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "27015"

int getFileSizeSystemCall(char * strFileName)
{
	struct stat temp;
	stat(strFileName, &temp);
	return temp.st_size;
}

int main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char sendbuf[DEFAULT_BUFLEN];// = "this is a test!";;//发送内容
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	// 验证参数的合法性
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// 初始化套接字
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSA启动失败！错误编号: %d\n", iResult);
		return 1;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// 解析服务器地址和端口号
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("获取地址失败！错误编号: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// 尝试连接服务器地址，直到成功
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// 创建套接字
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("Socket失败！错误编号: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// 向服务器请求连接
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		struct sockaddr_in sa;
		int len = sizeof(sa);
		if (!getsockname(ConnectSocket, (struct sockaddr *)&sa, &len))
		{
			printf("客户端IP地址：%s \n", inet_ntoa(sa.sin_addr));
			printf("客户端端口地址：%d \n\n", ntohs(sa.sin_port));
		}
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("不能连接服务器！\n");
		WSACleanup();
		return 1;
	}

	do {
		FILE *file=NULL;
		char filename[100];
		int filelen=0;
		char buf[1];
		while (file == NULL)
		{
			memset(filename, 0, strlen(filename));
			memset(buf, 0, strlen(buf));
			printf("输入文件名：  ");
			cin >> filename;
			iResult = send(ConnectSocket, filename, strlen(filename), 0);//发送文件名
			filename[strlen(filename) - 1] = '\0';
			cout << "文件名长度为" << strlen(filename) << endl;
			file = fopen(filename, "rb");
		}
		
		
		filelen = getFileSizeSystemCall(filename);
		cout << "数据长度为  " << filelen << endl;

		int invfilelen = htonl(filelen);
		cout << "数据长度转换为网络字节序为  " << invfilelen << endl;
		/*char *filelenchar;
		filelenchar = (char *)(&invfilelen);*/
		
		iResult = send(ConnectSocket, (char *)(&invfilelen), 4, 0);//发送文件长度

		int tosendlen = filelen;

		// 发送缓冲区中的测试数据
		while (tosendlen > 0)
		{
			cout << "还有" << tosendlen << "个字节需要发送" << endl;
			fread(sendbuf, 1, DEFAULT_BUFLEN, file);
			int iSend=DEFAULT_BUFLEN;
			if (tosendlen < DEFAULT_BUFLEN) iSend = tosendlen;
			iResult = send(ConnectSocket, sendbuf, iSend, 0);
			printf("装填成功！\n");

			if (iResult == SOCKET_ERROR) {
				printf("发送失败！错误编号: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			//printf("发射成功: %s(%ld)\n\n", sendbuf, iResult);
			tosendlen -= iResult;
		}
		
		fclose(file);

	} while (iResult > 0);


	// 数据发送结束，调用shutdown()函数声明不再发送数据，此时客户端仍可以接收数据
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("关闭失败！错误编号: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	


	// 关闭套接字
	closesocket(ConnectSocket);
	// 释放资源
	WSACleanup();

	printf("press any key to continue");
	getchar();
	return 0;
}


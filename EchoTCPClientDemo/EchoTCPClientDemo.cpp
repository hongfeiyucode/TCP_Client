// EchoTCPClientDemo.cpp : �������̨Ӧ�ó������ڵ㡣
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

// ���ӵ�winsock2��Ӧ��lib�ļ��� Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

//����Ĭ�ϵĻ��������ȺͶ˿ں�
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
	char sendbuf[DEFAULT_BUFLEN];// = "this is a test!";;//��������
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	// ��֤�����ĺϷ���
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// ��ʼ���׽���
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSA����ʧ�ܣ�������: %d\n", iResult);
		return 1;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// ������������ַ�Ͷ˿ں�
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("��ȡ��ַʧ�ܣ�������: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// �������ӷ�������ַ��ֱ���ɹ�
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// �����׽���
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("Socketʧ�ܣ�������: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// ���������������
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		struct sockaddr_in sa;
		int len = sizeof(sa);
		if (!getsockname(ConnectSocket, (struct sockaddr *)&sa, &len))
		{
			printf("�ͻ���IP��ַ��%s \n", inet_ntoa(sa.sin_addr));
			printf("�ͻ��˶˿ڵ�ַ��%d \n\n", ntohs(sa.sin_port));
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
		printf("�������ӷ�������\n");
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
			printf("�����ļ�����  ");
			cin >> filename;
			iResult = send(ConnectSocket, filename, strlen(filename), 0);//�����ļ���
			filename[strlen(filename) - 1] = '\0';
			cout << "�ļ�������Ϊ" << strlen(filename) << endl;
			file = fopen(filename, "rb");
		}
		
		
		filelen = getFileSizeSystemCall(filename);
		cout << "���ݳ���Ϊ  " << filelen << endl;

		int invfilelen = htonl(filelen);
		cout << "���ݳ���ת��Ϊ�����ֽ���Ϊ  " << invfilelen << endl;
		/*char *filelenchar;
		filelenchar = (char *)(&invfilelen);*/
		
		iResult = send(ConnectSocket, (char *)(&invfilelen), 4, 0);//�����ļ�����

		int tosendlen = filelen;

		// ���ͻ������еĲ�������
		while (tosendlen > 0)
		{
			cout << "����" << tosendlen << "���ֽ���Ҫ����" << endl;
			fread(sendbuf, 1, DEFAULT_BUFLEN, file);
			int iSend=DEFAULT_BUFLEN;
			if (tosendlen < DEFAULT_BUFLEN) iSend = tosendlen;
			iResult = send(ConnectSocket, sendbuf, iSend, 0);
			printf("װ��ɹ���\n");

			if (iResult == SOCKET_ERROR) {
				printf("����ʧ�ܣ�������: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			//printf("����ɹ�: %s(%ld)\n\n", sendbuf, iResult);
			tosendlen -= iResult;
		}
		
		fclose(file);

	} while (iResult > 0);


	// ���ݷ��ͽ���������shutdown()�����������ٷ������ݣ���ʱ�ͻ����Կ��Խ�������
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("�ر�ʧ�ܣ�������: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	


	// �ر��׽���
	closesocket(ConnectSocket);
	// �ͷ���Դ
	WSACleanup();

	printf("press any key to continue");
	getchar();
	return 0;
}


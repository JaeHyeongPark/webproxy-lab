#include "csapp.h"

/* 텍스트 줄을 읽고 반복적으로 echo해주는 echo 함수 */


void echo(int connfd) // 연결 식별자 
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio; // robust I/O (Rio); 뭔가 입출력 관련 함수?
    Rio_readinitb(&rio, connfd);

    //  Rio_readlineb 함수가 EOF를 만날 때까지 반복적으로 읽고 print
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        printf("server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
    }
}



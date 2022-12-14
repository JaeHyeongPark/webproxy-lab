#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    /* 사용자 입력은 프로그램이름, host명, port번호 */
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd); /* init Robust I/O structure w/ clientfd */

    while (Fgets(buf, MAXLINE, stdin) != NULL)
    {
        Rio_writen(clientfd, buf, strlen(buf)); /* buffer에서 clientfd로 n바이트 전송함 */
        Rio_readlineb(&rio, buf, MAXLINE); /* 텍스트 줄을 파일 rio에서 읽고, 이것을 메모리위치 buf로 복사하고 텍스트라인을 null(0) 문자로 종료시킴 */
        Fputs(buf, stdout); /* buf가 가지고 있는 문자열을 stdout에 쓴다. 표준 출력으로 표시. */
    }
    Close(clientfd);
    exit(0);    
}
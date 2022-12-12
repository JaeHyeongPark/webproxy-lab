#include "csapp.h"

/* Echo 클라이언트의 메인 루틴 */

int main(int argc, char **argv) // 인자: 호스트 네임(도메인명), 포트

{
    int clientfd; // 클라이언트 식별자
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    // 인자를 세개 받지 않으면 종료 ("이 프로그램은 다음과 같은 인자가 필요해!")
    // argv[0]는 프로그램의 이름
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port); // 클라이언트의 소켓; 서버와 연결을 설정
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) // `Fgets`가 EOF 표준 입력을 만나면 종료
    {
        Rio_writen(clientfd, buf, strlen(buf));
        // 서버는 `rio_readlineb` 함수에서 리턴 코드 0을 받으면 루프 종료
        Rio_readlineb(&rio, buf, MAXLINE); 
        Fputs(buf, stdout);
    }
    Close(clientfd); // 클라이언트 식별자 닫음 
    exit(0);
}
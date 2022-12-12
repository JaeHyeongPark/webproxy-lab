#include "csapp.h"

/* 반복적 Echo 서버 메인 루틴 */

void echo(int connfd);

int main(int argc, char **argv) // argv[1] → port
{
    int listenfd, connfd; // 듣기 식별자, 연결 식별자
    socklen_t clientlen;
    /* accept로 보내지는 소켓 주소 구조체
    accept가 리턴하기 전에 clientaddr에는 연결의 다른 쪽 끝의 클라이언트의 소켓 주소로 채워짐
    sockaddr_storage형으로 선언함으로써, 모든 형태의 소켓 주소를 저장하기에 충분히 큼
     */
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE]; // 클라이언트 호스트이름, 클라이언트 포트

    // 인자가 두개가 아니면 에러("이 프로그램은 다음과 같은 인자가 필요해!")
    // argv[0]는 프로그램의 이름
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]); 
        exit(0);
    }

    // 인자로 주어진 포트 번호로 듣기 식별자 소환 
    listenfd = Open_listenfd(argv[1]);

    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);

        // 클라이언트로부터 연결 요청 기다리기 
        // Accept 함수; 듣기 식별자, 클라이언트 주소, 주소 길이 → 연결 식별자 생성
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        
        // Getnameinfo: 소켓 주소를 location, service name으로 번역해줌
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        
        // "클라이언트 도메인 네임, 클라이언트 포트에 연결되었음"
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);  // echo.c의 echo 함수 호출
        Close(connfd); // 연결 식별자 닫음 
    }
    exit(0);
}


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


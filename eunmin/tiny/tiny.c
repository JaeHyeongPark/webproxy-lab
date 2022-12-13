/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv) // 포트 번호 인자로 받음
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  // 듣기 소켓 오픈
  listenfd = Open_listenfd(argv[1]);

  // 무한 서버 루프
  while (1)
  {
    clientlen = sizeof(clientaddr);

    // 연결 요청 접수
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    // 트랜잭션 수행
    doit(connfd); // line:netp:tiny:doit

    // 연결 종료
    Close(connfd); // line:netp:tiny:close
  }
}

// HTTP transaction을 다루는 함수: 연결 식별자
void doit(int fd)
{
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  // GET 메소드가 아닌 경우 에러 -> 11.11번 문제랑 관련 있을 듯
  if (strcasecmp(method, "GET"))
  {
    clienterror(fd, method, "501", "Not implemented",
                "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio);

  /* GET 요청으로 들어온 URI에서 파싱 */
  is_static = parse_uri(uri, filename, cgiargs); // uri에 CGI인자가 없으면 1 반환 → 정적 컨텐츠
  if (stat(filename, &sbuf) < 0)
  {
    clienterror(fd, filename, "404", "Not found",
                "Tiny couldn’t find this file");
    return;
  }

  /* Serve static content */
  if (is_static)
  {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn’t read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size); // 연결식별자, 파일명, 파일사이즈(?)
  }

  /* Serve dynamic content */
  else
  {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn’t run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs); // 연결식별자, 파일명, CGI 인자
  }
}

// 클라이언트에게 에러 메시지를 보내는 함수
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor="
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.%s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

// request 헤더를 읽고 무시하는 함수
void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);

  // 요청 헤더를 종료하는 빈 텍스트 줄: `carriage return`과 `line feed`의 쌍
  while (strcmp(buf, "\r\n")) // 두 문자열이 같으면 결과값이 0

  {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

// HTTP URI를 parsing 하는 함수: URI, 파일명, CGI인자
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  // URI에 "cgi-bin" 문자열이 존재하지 않으면 정적 컨텐츠 → 1반환
  if (!strstr(uri, "cgi-bin"))
  {
    // 문자열 복사 함수: 복사한 문자열을 붙여넣기 할 주소, 복사할 문자열의 시작 주소
    strcpy(cgiargs, "");   // CGI 인자 스트링 지움
    strcpy(filename, "."); // 파일명을 .으로 바꿈
    strcat(filename, uri); // 파일명과 uri를 합침 -> "./파일명.html" 같은 상대 경로가 됨?

    // 만일 uri의 끝 문자가 '/'로 끝나면
    if (uri[strlen(uri) - 1] == '/')
      strcat(filename, "home.html"); // 파일명 + home.html(기본파일이름)
    return 1;
  }

  // URI에 "cgi-bin" 문자열이 존재하면 동적 컨텐츠 → 0반환
  else
  {
    /* 모든 CGI 인자 추출하기 */
    ptr = index(uri, '?'); // URI에서 ?의 위치

    // URI에 ?가 있으면
    if (ptr)
    {
      strcpy(cgiargs, ptr + 1); // ? 다음에 오는 문자열을 cgiargs에 복사
      *ptr = '\0';
    }

    // URI에 ?가 없으면
    else
      strcpy(cgiargs, ""); // cgiargs 공백으로
    /* 모든 CGI 인자 추출하기 끝 */

    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}

// 정적 콘텐츠를 클라이언트에게 serve하는 함수: 연결식별자, 파일명, 파일크기
void serve_static(int connfd, char *filename, int filesize)
{
  int srcfd; // 소스 파일 식별자
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  // 클라이언트에게 응답 line과 응답 헤더를 보냄
  get_filetype(filename, filetype); // 파일 이름의 접미어 검사 → 파일 타입 결정
  sprintf(buf, "HTTP/1.2OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(connfd, buf, strlen(buf));
  // 클라이언트에 응답 line과 응답 헤더를 보냄 - 끝

  printf("Response headers:\n");
  printf("%s", buf); // 빈 줄로 헤더 종료

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0); // read를 위한 소스 file 오픈 + 식별자 얻어옴

  // 요청한 파일을 `mmap` 함수로 가상메모리 영역으로 매핑:
  // private read-only 가상 메모리 영역으로 매핑
  // 소스 파일 srcfd의 filesize 바이트의 가상메모리에서의 시작 주소 srcp
  // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  
  // 문제 11.9번을 위함 
  char *temp = Malloc(filesize); // malloc으로 filesize 만큼 메모리 할당
  Rio_readn(srcfd, temp, filesize); // 소스파일에서 temp로 파일사이즈만큼 바이트 전송 

  // 파일을 메모리로 매핑한 후, 더이상 식별자 필요 없으니 파일 close (메모리 누수 방지)
  Close(srcfd);

  // `srcp`에서 시작하는 `filesize`를 클라이언트의 연결 식별자로 복사
  // Rio_writen(connfd, srcp, filesize); // 원본
  // 문제 11.9번을 위함 - 연결 식별자에 temp로부터 filesize만큼 바이트 전송 
  Rio_writen(connfd, temp, filesize);  

  // 매핑된 가상메모리 주소를 반환 (메모리 누수 방지)
  Free(temp);
  // Munmap(srcp, filesize);
}

/* get_filetype - 파일명으로부터 파일 타입 추출 */
void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else if (strstr(filename, ".mp4")) // 동영상 mp4 추가
    strcpy(filetype, "video/mp4");
  else
    strcpy(filetype, "text/plain");
}

// 동적 콘텐츠를 클라이언트에게 serve하는 함수: 파일 식별자, 파일명, CGI인자
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n"); // 클라이언트에게 성공 메시지 전달
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  // 새로운 자식 프로세스 포크
  if (Fork() == 0)
  {
    /* Real server would set all CGI vars here */
    // 자식 프로세스는 query_string 환경 변수를 요청온 URI의 CGI 인자로 초기화
    setenv("QUERY_STRING", cgiargs, 1);
    Dup2(fd, STDOUT_FILENO);              // 자식 프로세스는 자식의 표준 출력(stdout)을 연결 파일 식별자로 재지정
    Execve(filename, emptylist, environ); // CGI 프로그램 로드 후 실행
  }
  Wait(NULL); // 부모는 자식이 종료되어 정리되는 것을 기다리기 위해 `wait` 함수에서 블록
}

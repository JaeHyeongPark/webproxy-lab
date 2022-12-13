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

void doit(int fd)
{
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  // request line과 headers 읽기
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  // GET 메소드 외에 다른 메소드 요청시 에러 메시지 전송
  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio); // 정상적으로 요청 라인을 읽었을 경우 다른 헤더들을 무시한다

  // Get request 로 요청된 URI 구문 분석
  is_static = parse_uri(uri, filename, cgiargs); // 정적 컨텐츠 요청인지, 동적 컨텐츠 요청인지 설정하는 플래그
  if (stat(filename, &sbuf) < 0) { // 파일이 디스크에 존재하지 않을 시 즉시 에러메시지 출력
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if(is_static) { //정적 컨텐츠 제공
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { // 보통 파일이라는 것과 읽기 권한을 가지고 있는지 검증
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);
  }
  else { // 동적 컨텐츠 제공
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { // 이 파일이 실행 가능한지 검증 
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs);
  }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  // HTTP 응답 body HTML 빌드하기
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  // HTTP 응답 print 하기
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp) // 요청 헤더를 읽고 무시하는 함수
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  while(strcmp(buf, "\r\n")) { // 요청 헤더를 종료하는 빈 텍스트 줄(carriage return과 line feed 쌍으로 구성)을 체크한다
    rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  if (!strstr(uri, "cgi-bin")) { // 정적 컨텐츠를 위한 것이면
    strcpy(cgiargs, ""); // 정적 컨텐츠를 위한 것이라면 cgi 인자 스트링을 지운다
    strcpy(filename, "."); //URI를 '.index.html'같은 대응하는 리눅스 경로이름으로 변환
    strcat(filename, uri);
    if (uri[strlen(uri)-1] == '/') //URI가 '/'문자로 끝난다면 기본 파일 이름을 추가한다
      strcat(filename, "home.html");
    return 1;
  }
  else { //동적 컨텐츠를 위한 것이면
    ptr = index(uri, '?'); // 모든 CGI 인자들을 추출하기
    if (ptr) {
      strcpy(cgiargs, ptr+1);
      *ptr = '\0';
    }
    else
      strcpy(cgiargs, "");
    strcpy(filename, "."); //나머지 URI 부분을 대응하는 리눅스 파일 이름으로 변환
    strcat(filename, uri);
    return 0;
  }
}

void serve_static(int fd, char *filename, int filesize)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  // 응답 헤더를 클라이언트에게 보내기
  get_filetype(filename, filetype); //파일 이름의 접미어 부분을 검사해서 파일 타입 결정
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype); // 빈 줄 하나 더가 헤더를 종료함을 알림
  Rio_writen(fd, buf, strlen(buf)); 
  printf("Response headers:\n");
  printf("%s", buf);

  // 응답 바디를 클라이언트에게 보내기
  srcfd = Open(filename, O_RDONLY, 0); // 파일을 읽기 위해 filename을 오픈하고 식별자 얻어오기
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // mmap함수를 호출하여 요청한 파일을 가상메모리 영역으로 매핑
  Close(srcfd); // 식별자 사용 완료되어 닫기(하지 않으면 메모리 누수 위험)
  Rio_writen(fd, srcp, filesize); // 실제로 파일을 클라이언트에게 전송 (주소 srcp에서 시작하는 filesize 바이트를 클라이언트의 연결 식별자로 복사하여 전송)
  Munmap(srcp,filesize); // 매핑된 가상메모리 주소 반환(메모리 누수 방지)
}

// filename 으로 filetype을 끌어내기
void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if(strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if(strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if(strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  char buf[MAXLINE], *emptylist[] = { NULL };

  // HTTP 응답 첫 부분 리턴(status line+ 헤더)
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0) { // 자식 프로세스 fork
    // 실제 서버는 모든 CGI 환경변수를 여기에 설정한다(우리는 이 부분 생략하고 QUERY_STRING만 확인)
    setenv("QUERY_STRING", cgiargs, 1); // 자식은 QUERY_STRING 환경 변수를 요청하여, URI의 CGI인자들로 초기화한다
    Dup2(fd, STDOUT_FILENO); // 자식 프로세스의 표준 출력을 연결 식별자로 재지정
    Execve(filename, emptylist, environ); // CGI 프로그램 실행
  }
  Wait(NULL); // 부모는 대기 후, 자식 프로세스를 거둬들임
}

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}
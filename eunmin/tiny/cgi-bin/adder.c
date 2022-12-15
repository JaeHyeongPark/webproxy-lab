/*
 * adder.c - a minimal CGI program that adds two numbers together
 */

#include "csapp.h"

int main(void)
{

  char *buf, *p, *a, *b;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  /* Extract the two arguments */
  if ((buf = getenv("QUERY_STRING")) != NULL)
  {
    p = strchr(buf, '&'); // buf -> number-1=15000 \0 number-2=213
    *p = '\0';

    // 문제 11.10을 위해 수정한 부분
    strcpy(arg1, buf + 9); // arg1 -> number-1=15000 -> 15000
    strcpy(arg2, p + 10);  // arg2 -> number-2=213 -> 213

    // 민섭님이 리뷰해준 코드: =의 위치를 찾아서 인자 추출하기!
    // strcpy(arg1, buf);
    // strcpy(arg2, p + 1);
    // n1 = strtol(strchr(arg1, '=') + 1, NULL, 10);
    // n2 = strtol(strchr(arg2, '=') + 1, NULL, 10);

    // 문제 11.10을 위해 수정한 부분 - 끝

    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  /* Make the response body */
  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>",
          content, n1, n2, n1 + n2);
  sprintf(content, "%sThanks for visiting!\r\n", content);

  /* Generate the HTTP response */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);

  exit(0);
}

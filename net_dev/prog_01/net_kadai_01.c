/*
 ============================================================================
 Name        : net_kadai_01.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

typedef enum
{
    Stop,
    Continue
} isContinue_t;

typedef enum
{
    ForkError,
    Child,
    Master
} Case_t;

typedef enum
{
    false,
    true
} bool;

/* コマンド入力の処理 */
int get_cmd(char *cmd)
{
    printf(">");
    if (fgets(cmd, 128, stdin) == NULL)
    {
        return 0;
    }
    int len = strlen(cmd);
    len = strlen(cmd);

    /* 改行文字を削除 */
    if (len > 0 && cmd[len - 1] == '\n')
    {
        cmd[--len] = '\0';
    }
    /* exitが入力されるとループを抜けるようにする */
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0)
    {
        return Stop;
    }
    else//入力が例外処理を伴わないとき
    {
        return Continue;
    }

    return 1;
}

/* 子プロセスの処理 */
void child_prcess(char *cmd, char **pargs)
{
    printf("\n[子]\n");
    printf("%s実行\n", cmd);
    /*コマンドを実行*/
    int isError = execv(cmd, pargs);
    // int isError = 0;
    if (isError == -1)
    {
        printf("no such file or directory\n");
        return;
    }

    printf("子終了\n");
}

/* 親プロセスの処理 */
void master_process(int status)
{
    pid_t wait_pid = wait(&status);
    printf("\n[親]\n");
    printf("wait_pid: %d\n", wait_pid);
    if (wait_pid == -1)
    {
        puts("wait error");
    }
    printf("status: %d\n", status);
}

int main(void)
{
    puts("!!!Hello World!!!"); 
    pid_t pid = 0;
    pid_t wait_pid = 0;
    char cmd[128];
    //	char baffer[128];
    char *pargs[2] = {cmd, NULL};
    int status;
    isContinue_t isContinue = Continue;

    while (isContinue != Stop)
    {
        isContinue = get_cmd(cmd);
        pid = fork();
        printf("fork: %d\n", pid);
        if (pid < 0)
        {
            puts("fork error");
            isContinue = Stop;
            break;
        }
        else if (pid == 0)//子プロセス
        {
            child_prcess(cmd, pargs);
            _exit(0);
        }
        else//親プロセス
        {
            master_process(status);
        }

    }
    puts("終了");
    return 1;
}

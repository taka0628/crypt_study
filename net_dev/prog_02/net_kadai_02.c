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

#define CMD_SIZE 128
#define PARGS_SIZE 3
#define DEBAG 1


typedef enum
{
    Stop,
    Continue
} isContinue_t;

typedef enum
{
    false,
    true
} bool;

// 1次元配列
typedef struct{
    char *array;//配列本体
    int size;//要素数
}
array_1_t;

typedef struct{
    char **array;//配列本体
    int col;//列の要素数, n
    int row;//行の要素数, m
}
array_2_t;

/* コマンド入力の処理, 空白の個数を算出 */
int get_cmd(char *cmd, int* space_cnt)
{
    printf(">");
    if (fgets(cmd, CMD_SIZE, stdin) == NULL)
    {
        return 0;
    }
    int len = strlen(cmd);

    /* 改行文字を削除 */
    if (len > 0 && cmd[len - 1] == '\n')
    {
        cmd[--len] = '\0';
    }

    // 空白の個数を計算
    for (int i = 0; i < strlen(cmd); i++)
    {
        if(cmd[i] == ' '){
            *space_cnt = *space_cnt + 1;
        }
    }
    if(DEBAG){
        printf("space_cnt: %d\n", *space_cnt);
    }


    /* exitが入力されるとループを抜けるようにする */
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0)
    {
        return Stop;
    }

    return Continue;
}

// 引数を作成
void make_pargs(array_2_t pargs, char* const cmd){
    int space_cnt = 0;
    int space_idx[PARGS_SIZE];
    int temp = 0;//空白があった際のcmdの配列番号を記録
    int pargs_idx = 0;//pargsに書き込む配列番号
    //cmd内の空白の数と場所を把握
    // #define STRTOK
    #ifndef STRTOK
    for (int i = 0; i < pargs.row - 1; i++){
        for (int cmd_idx = temp; cmd_idx < strlen(cmd); cmd_idx++){
            if(cmd[cmd_idx] == ' '){
                temp = cmd_idx + 1;
                pargs_idx = 0;
                break;
            }
            pargs.array[i][pargs_idx++] = cmd[cmd_idx];
        }
    }
    #else
    char *tp = strtok(cmd, " ");
    pargs.array[0] = tp;
    while(tp = strtok(NULL, " ")){
        if(tp == NULL){
            break;
        }
        pargs_idx ++;
        pargs.array[pargs_idx] = tp;
        printf("pargs[%d]: %s\n", pargs_idx, pargs.array[pargs_idx]);
    }
    #endif
}

/* 子プロセスの処理 */
void child_prcess(array_2_t const pargs)
{
    printf("\n[子]\n");
    printf("%s実行\n", pargs.array[0]);
    /*コマンドを実行*/
    int isError = execv(pargs.array[0], pargs.array);
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

// 二次元配列のメモリ解放
void delete_mem(array_2_t array){
    puts("mem_free");
    for (int i = 0; i < array.row; i++)
    {
        free(array.array[i]);
    }
    free(array.array);
    array.col = array.row = 0;
}

int main(void)
{
    puts("!!!Hello World!!!");
    pid_t pid = 0;
    pid_t wait_pid = 0;
    char cmd[CMD_SIZE];
    //	char baffer[128];
    // char *pargs[PARGS_SIZE] = {cmd_first, cmd_second, NULL};
    array_2_t pargs;
    int space_cnt = 0;
    int status;

    isContinue_t isContinue = Continue;

    while (isContinue != Stop)
    {
        space_cnt = 0;
        isContinue = get_cmd(cmd, &space_cnt);
        // pargsのメモリを確保
        pargs.array = (char**)malloc(sizeof(char*) * (space_cnt + 2)); //+2はcmdをコピーする分とNULL
        pargs.row = space_cnt + 2;
        pargs.col = CMD_SIZE;
        pargs.array[space_cnt + 2 - 1] = NULL;
        for (int i = 0; i < space_cnt + 1; i++)
        {
            pargs.array[i] = (char*)malloc(sizeof(char) * CMD_SIZE);
            // 初期化
            for (int j = 0; j < pargs.row; j++){
                pargs.array[i][j] = '\0';
            }
        }
        // for (int i = 0; i < pargs.col; i++){
        //     printf("array[0][%d]: %d\n", i, (int)pargs.array[0][i]);
        // }

        make_pargs(pargs, cmd);

        if(DEBAG){
            for (int i = 0; i < pargs.row - 1; i++){
                printf("pargs[%d]: %s\n", i, pargs.array[i]);
            }
            printf("pargs_size; %d\n", pargs.row);
        }

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
            child_prcess(pargs);
            _exit(0);
        }
        else//親プロセス
        {
            master_process(status);
        }
        // メモリ解放
        delete_mem(pargs);
    }
    puts("終了");
    return 1;
}

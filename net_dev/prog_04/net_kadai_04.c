#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define CMD_SIZE 128
#define PARGS_SIZE 3
#define BG_JOBS_SIZE 10
#define DEBAG 0
#define FG_DEBAG 1

typedef enum
{
    Stop,
    Continue,
    BG, //バックグラウンド実行を検出
    JOBS,
    CHANGE_BG_TO_FG
} operation_state_t;

typedef enum
{
    false,
    true
} bool;

// 1次元配列
typedef struct
{
    char *array; //配列本体
    int size;    //要素数
} array_1_t;

typedef struct
{
    char **array; //配列本体
    int col;      //列の要素数, n
    int row;      //行の要素数, m
} array_2_t;

typedef struct
{
    char comand_name[CMD_SIZE];
    pid_t pid;
    int serial_num;
} dis_jobs_t;

void master_process(pid_t pid, int isBG);
void child_prcess(array_2_t const pargs, pid_t pid, int isBG);
void delete_mem(array_2_t array);
void jobs_operation(dis_jobs_t *this);

/* コマンド入力の処理, 空白の個数を算出 */
int get_cmd(char *cmd, int *space_cnt)
{
    printf(">");
    int isBG = 0;
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
        if (cmd[i] == ' ' && cmd[i + 1] != '&')
        {
            *space_cnt = *space_cnt + 1;
        }
        else if (cmd[i] == ' ' && cmd[i + 1] == '&')
        {
            isBG = BG;
        }
    }
    if (DEBAG)
    {
        printf("space_cnt: %d\n", *space_cnt);
    }

    /* 実行方法を設定 */
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0)
    {
        return Stop;
    }
    else if (strcmp(cmd, "jobs") == 0)
    {
        return JOBS;
    }
    else if (strcmp(cmd, "fg") == 0)
    {
        return CHANGE_BG_TO_FG;
    }
    else if (isBG == BG)
    {
        return BG;
    }

    return Continue;
}

// 引数を作成
void make_pargs(array_2_t pargs, char *const cmd)
{
    int space_cnt = 0;
    int space_idx[PARGS_SIZE];
    int temp = 0;      //空白があった際のcmdの配列番号を記録
    int pargs_idx = 0; //pargsに書き込む配列番号
//cmd内の空白の数と場所を把握
// #define STRTOK
#ifndef STRTOK
    for (int i = 0; i < pargs.row - 1; i++)
    {
        for (int cmd_idx = temp; cmd_idx < strlen(cmd); cmd_idx++)
        {
            if (cmd[cmd_idx] == ' ')
            {
                temp = cmd_idx + 1;
                pargs_idx = 0;
                break;
            }
            if (cmd[cmd_idx] == '&')
            {
            }
            pargs.array[i][pargs_idx++] = cmd[cmd_idx];
        }
    }
#else
    char *tp = strtok(cmd, " ");
    pargs.array[0] = tp;
    while (tp = strtok(NULL, " "))
    {
        if (tp == NULL)
        {
            break;
        }
        pargs_idx++;
        pargs.array[pargs_idx] = tp;
        printf("pargs[%d]: %s\n", pargs_idx, pargs.array[pargs_idx]);
    }
#endif
}

/* 子プロセスの処理 */
void child_prcess(array_2_t const pargs, pid_t pid, int isBG)
{
    /*コマンドを実行*/
    // 子プロセスが終了するまで待機するプロセス
    if (isBG != BG)
    {
        printf("\n[子]\n");
        printf("%s実行\n", pargs.array[0]);
        int isError = execv(pargs.array[0], pargs.array);
        // int isError = 0;
        if (isError == -1)
        {
            printf("no such file or directory\n");
            return;
        }
    }
    else
    { //バックグラウンド処理
        pid_t pid_child = pid;
        if (pid_child < 0)
        {
            puts("BG fork error");
            return;
        }
        else if (pid_child == 0) //子プロセス
        {
            if (DEBAG)
            {
                puts("[BG child]");
            }
            int isError = execv(pargs.array[0], pargs.array);
            // int isError = 0;
            if (isError == -1)
            {
                printf("no such file or directory\n");
                return;
            }
        }
    }

    // printf("子終了\n");
}

/* 親プロセスの処理 */
void master_process(pid_t pid, int isBG)
{
    int status;
    // 通常待機
    if (isBG == false)
    {
        pid_t wait_pid = waitpid(pid, &status, 0);
        printf("\n[親]\n");
        printf("wait_pid: %d\n", wait_pid);
        if (wait_pid == -1)
        {
            puts("wait error");
        }
        printf("status: %d\n", status);
    }
    else
    { //バックグラウンド処理
        pid_t wait_pid = waitpid(pid, &status, 0);
        if (DEBAG)
        {
            printf("\n[親BG]\n");
            printf("wait_pid: %d\n", wait_pid);
        }
        if (wait_pid == -1)
        {
            puts("wait error");
        }
    }
}

// 二次元配列のメモリ解放
void delete_mem(array_2_t array)
{
    puts("mem_free");
    for (int i = 0; i < array.row; i++)
    {
        free(array.array[i]);
    }
    free(array.array);
    array.col = array.row = 0;
}

void jobs_list_add(dis_jobs_t *jobs_list, pid_t const pid, array_2_t const pargs)
{
    for (int i = 0; i < BG_JOBS_SIZE; i++)
    {
        if (jobs_list[i].serial_num < 0)
        {
            jobs_list[i].pid = pid;
            jobs_list[i].serial_num = i + 1;
            strcpy(jobs_list[i].comand_name, pargs.array[0]);
            return;
        }
    }
    puts("[jobs_list_add error]");
}

int jobs_list_delete(dis_jobs_t *this, pid_t const pid, int *jobs_count)
{
    int isJobExists = false;
    int target_idx = 0;
    // リストに選択されたプロセスが存在することを確認
    for (int i = 0; i < BG_JOBS_SIZE; i++)
    {
        if (this[i].pid == pid)
        {
            isJobExists = true;
            target_idx = i;
            break;
        }
    }
    if (isJobExists == false)
    {
        puts("[error]\njobs_list_delete");
        printf("pid: %d はjobs_listに存在しません\n", pid);
        jobs_operation(this);
        exit(1);
    }
    dis_jobs_t save_process = this[target_idx];
    dis_jobs_t temp;
    // BGリストの更新
    for (int i = target_idx; i < BG_JOBS_SIZE; i++)
    {
        if (this[i].serial_num > 0)
        {
            this[i] = this[i + 1];
        }
        else
        {
            break;
        }
    }
    *jobs_count--;
    jobs_operation(this);
    int status;
    printf("%d\t%d\t%sのプロセスをjobs_listから削除しました\n",
           save_process.serial_num, save_process.pid, save_process.comand_name);
}

void jobs_operation(dis_jobs_t *this)
{
    char buff[CMD_SIZE] = {"[jobs list]"};
    printf("\n[jobs list]");
    for (int i = 0; i < 10; i++)
    {
        printf("-");
    }
    printf("\nnum\tpid\tcommand\n");
    for (int i = 0; i < BG_JOBS_SIZE; i++)
    {
        if (this[i].serial_num > 0)
        {
            printf("%d\t%d\t%s\n", this[i].serial_num, this[i].pid, this[i].comand_name);
        }
    }
    for (int i = 0; i < strlen(buff) + 10; i++)
    {
        printf("-");
    }
    puts("\n");
}

int fg_operation(dis_jobs_t *this, int *jobs_count)
{
    jobs_operation(this);
    puts("FGに変更するプロセスをnumから選び入力してください");
    int select_num;
    char buff[CMD_SIZE] = {'\0'};

    // 入力受付
    while (1)
    {
        printf(">");
        if (fgets(buff, CMD_SIZE, stdin) == NULL)
        {
            return 0;
        }
        if (buff[0] >= '1' && buff[0] <= '9') //入力チェック
        {
            if(atoi(&buff[0]) <= *jobs_count){
                break;
            }else{
                puts("指定された番号がリストにありません");
            }
        }
        else
        {
            // puts("1~で入力してください");
            printf("1~%dで入力してください\n", *jobs_count);
            printf("buff: %s\n", buff);
            memset(buff, 0, CMD_SIZE);
        }
    }

    select_num = atoi(buff);
    if (FG_DEBAG)
    {
        for (int i = 0; i < strlen(buff); i++)
        {
            printf("buff[%d]: %c\n", i, buff[i]);
        }
        printf("select_num: %d\n", select_num);
    }
    dis_jobs_t temp = this[select_num - 1];
    jobs_list_delete(this, temp.pid, jobs_count);
    printf("%s　の終了を待っています。\n", temp.comand_name);
    waitpid(temp.pid, NULL, 0);
}

int main(void)
{
    pid_t pid = 0;
    pid_t wait_pid = 0;
    char cmd[CMD_SIZE];
    //	char baffer[128];
    // char *pargs[PARGS_SIZE] = {cmd_first, cmd_second, NULL};
    array_2_t pargs;
    int space_cnt = 0;
    int status;
    dis_jobs_t jobs_list[BG_JOBS_SIZE];
    int jobs_count = 0;
    for (int i = 0; i < BG_JOBS_SIZE; i++)
    {
        jobs_list[i].pid = jobs_list[i].serial_num = -1;
    }

    operation_state_t operation_state = Continue;

    while (operation_state != Stop)
    {
        // fgを通さずにBGプロセスが終了した際にBG実行中のプロセスのリストを更新
        for (int i = 0; i < jobs_count; i++)
        {
            if (waitpid(jobs_list[i].pid, &status, WNOHANG) > 0)
            {
                printf("\n[BG process(%s) end]\n", jobs_list[i].comand_name);
                puts("jobs_list update");
                jobs_list_delete(jobs_list, jobs_list[i].pid, &jobs_count);
                jobs_count--;
            }
        }
        // 念の為ゾンビプロセスを回収
        while (pid = waitpid(-1, &status, WNOHANG) > 0)
        {
            printf("\n[BG process end]\n");
            // jobs_list_delete(jobs_list, pid);
            puts("jobs_list update");
        }
        operation_state = Continue;
        // if(WIFEXITED(status)
        space_cnt = 0;
        operation_state = get_cmd(cmd, &space_cnt);
        // pargsのメモリを確保
        pargs.array = (char **)malloc(sizeof(char *) * (space_cnt + 2)); //+2はcmdをコピーする分とNULL
        pargs.row = space_cnt + 2;
        pargs.col = CMD_SIZE;
        pargs.array[space_cnt + 2 - 1] = NULL;
        for (int i = 0; i < space_cnt + 1; i++)
        {
            pargs.array[i] = (char *)malloc(sizeof(char) * CMD_SIZE);
            // 初期化
            for (int j = 0; j < pargs.row; j++)
            {
                pargs.array[i][j] = '\0';
            }
        }
        // for (int i = 0; i < pargs.col; i++){
        //     printf("array[0][%d]: %d\n", i, (int)pargs.array[0][i]);
        // }

        make_pargs(pargs, cmd);
        if (operation_state == BG)
        {
            printf("[BG process start]\n");
        }
        else if (operation_state == Continue)
        {
            printf("[FG process start]\n");
        }

        if (DEBAG && operation_state != BG)
        {
            for (int i = 0; i < pargs.row - 1; i++)
            {
                printf("pargs[%d]: %s\n", i, pargs.array[i]);
            }
            printf("pargs_size; %d\n", pargs.row);
        }

        // プロセス生成
        if (operation_state == Continue)
        {
            pid = fork();
            printf("\nfork: %d\n", pid);
            if (pid < 0)
            {
                puts("fork error");
                operation_state = Stop;
                break;
            }
            else if (pid == 0) //子プロセス
            {
                child_prcess(pargs, pid, false);
                _exit(0);
            }
            else //親プロセス
            {
                master_process(pid, false);
                // メモリ解放
                delete_mem(pargs);
            }
        }
        else if (operation_state == BG)
        { //BG処理
            pid = fork();
            jobs_count++;
            // printf("fork: %d\n", pid);
            if (pid < 0)
            {
                puts("fork error");
                operation_state = Stop;
                break;
            }
            // BGのプロセスを登録
            jobs_list_add(jobs_list, pid, pargs);
            if (pid == 0) //子プロセス
            {
                child_prcess(pargs, pid, operation_state);
                delete_mem(pargs);
                exit(1);
            }
        }
        else if (operation_state == JOBS)
        {
            jobs_operation(jobs_list);
        }
        else if (operation_state == CHANGE_BG_TO_FG)
        {
            fg_operation(jobs_list, &jobs_count);
        }
    }
    // puts("終了");
    return 1;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>

#define CMD_SIZE 128
#define PARGS_SIZE 3
#define BG_JOBS_SIZE 10
#define DEBAG 0
#define FG_DEBAG 1

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

sig_atomic_t isExit = 0;

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
    __sighandler_t sig = signal(SIGINT, SIG_IGN);
    if (sig == SIG_ERR)
    {
        ERROR;
        exit(1);
    }
    if (fgets(cmd, CMD_SIZE, stdin) == NULL)
    {
        ERROR;
        return 0;
    }
    /* 改行文字を削除 */
    int len = strlen(cmd);
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
    printf("space_cnt: %d\n", *space_cnt);
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

// envからcmdまでのpathを生成
#define PATH_DEBAG 0
bool get_true_path(char *path, char const *cmd, char const *env)
{
    char *dis_path;
    char temp_env[1024];
    strncpy(temp_env, env, 1024);
    if (PATH_DEBAG)
        printf("path_sorce: %s\n", temp_env);
    dis_path = strtok(temp_env, ":");
    DIR *dir = opendir(dis_path);
    struct dirent *dis_file;
    if (PATH_DEBAG)
    {
        printf("search_dir: %s\n", dis_path);
        printf("cmd: %s\n", cmd);
    }

    while (dis_path != NULL)
    {
        if (PATH_DEBAG)
            printf("search_dir: %s\n", dis_path);
        if ((dir = opendir(dis_path)) != NULL)
        {
            while ((dis_file = readdir(dir)) != NULL)
            {
                // if (PATH_DEBAG)
                //     printf("\t%s\n", dis_file->d_name);
                if (strlen(cmd) == strlen(dis_file->d_name) && (strstr(dis_file->d_name, cmd) != NULL))
                {
                    if (PATH_DEBAG)
                        printf("[get!]: %s\n", dis_path);
                    sprintf(path, "%s/%s", dis_path, cmd);
                    if (PATH_DEBAG)
                        printf("path: %s\n", path);
                    closedir(dir);
                    return true;
                }
            }
        }
        else
        {
            ERROR;
            perror("opendir");
            return false;
        }
        closedir(dir);
        dis_path = strtok(NULL, ":");
        if (PATH_DEBAG)
            puts("");
    }
}

// 引数を作成
void make_pargs(array_2_t pargs, char *const cmd, char *const env)
{
    int space_cnt = 0;
    int space_idx[PARGS_SIZE];
    int temp = 0;      //空白があった際のcmdの配列番号を記録
    int pargs_idx = 0; //pargsに書き込む配列番号

    //cmd内の空白からコマンドを切り分け
    char *tp = strtok(cmd, " ");
    strncpy(pargs.array[0], tp, pargs.col);
    while ((tp = strtok(NULL, " ")) != NULL)
    {
        if (strstr(tp, "&") == NULL)
        {
            pargs_idx++;
            strncpy(pargs.array[pargs_idx], tp, pargs.col);
            printf("pargs[%d]: %s\n", pargs_idx, pargs.array[pargs_idx]);
        }
    }
    // 環境変数を所得
    // 絶対パスが指定されている場合はコマンドを探索しない
    if (strstr(pargs.array[0], "/") == NULL)
    {
        char cmd_path[1024] = {'\0'};
        if (PATH_DEBAG)
            printf("pargs.array[0]: %s\n", pargs.array[0]);
        if (get_true_path(cmd_path, pargs.array[0], env))
        {
            printf("get psth-> %s\n", cmd_path);
            strncpy(pargs.array[0], cmd_path, pargs.col);
        }
    }
}

void exit_process(int sig)
{
    printf("sig: %d\n", sig);
    isExit = 1;
}

/* 子プロセスの処理 */
void child_prcess(array_2_t const pargs, pid_t pid, int isBG)
{
    /*コマンドを実行*/
    // 子プロセスが終了するまで待機するプロセス
    setpgid(0, 0);
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
    pid_t wait_pid;
    // 通常待機
    if (isBG == false)
    {
        // Ctrl+Cの処理
        if ((signal(SIGINT, exit_process)) == SIG_ERR)
        {
            ERROR;
            exit(1);
        }
        while (1)
        {
            if (isExit == 1)
            {
                puts("kill");
                kill(pid, SIGABRT);
                isExit = 0;
            }
            if (wait_pid = waitpid(pid, NULL, WNOHANG) > 0)
            {
                break;
            }
        }

        printf("\n[親]\n");
        printf("wait_pid: %d\n", wait_pid);
        if (wait_pid == -1)
        {
            ERROR;
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
    puts("mem_free\n");
    for (int i = 0; i < array.row; i++)
    {
        free(array.array[i]);
    }
    free(array.array);
    array.col = array.row = 0;
}

// バックグラウンド処理のリストに追加
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

// バックグラウンド処理のリストから特定のプロセスを除外
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

// jobsが入力されたときの動作
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

// fgの動作
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
            if (atoi(&buff[0]) <= *jobs_count)
            {
                break;
            }
            else
            {
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

void BG_child_end(int sig)
{
    printf("BG process end");
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
    char *env = getenv("PATH");
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
        space_cnt = 0;

        // 入力受付
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
        if (operation_state != Stop)
        {
            // コマンド作成
            make_pargs(pargs, cmd, env);
        }
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
            printf("fork: %d\n", pid);
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
            else
            {
                if (signal(SIGCHLD, BG_child_end) == SIG_ERR)
                {
                    ERROR;
                    exit(1);
                }
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

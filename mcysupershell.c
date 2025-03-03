#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#define MAXCH 64
#define DELIM "\t \r\n"

char *readline();
char **spiltline(char *line);
void doline(char **args);
int checkcommand(char **args);
void sig_hangler(int signum){
    printf("ctrl+c is blocked\n");
}

void mysh_loop() {
    char *line;
    char **args;
    do {
        char path[100];
        getcwd(path, 100);
        char now[200] = "[myshell ";
        strcat(now, path);
        strcat(now, "]$ ");
        printf("%s", now);

        line = readline();
        if (strcmp(line, "exit\n") == 0) {
            break;
        }
        args = spiltline(line);
        if (checkcommand(args) == 0) {
            printf("wrong command\n");
            continue;
        }
        doline(args);
        

        free(line);
        free(args);

    } while (1);
}

char *readline() {
    char *line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

char **spiltline(char *line) {
    int argssize = MAXCH;
    int i = 0;
    char **args;
    args = (char **)malloc(sizeof(char *) * argssize);
    char *token;
    token = strtok(line, DELIM);
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, DELIM);
    }

    args[i] = NULL;
    return args;
}

int checkcommand(char **args) {
    int incount = 0;
    int outcount = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
            incount++;
            if (args[i + 1] == NULL || i == 0) {
                return 0;
            }
        }
        if (strcmp(args[i], "<") == 0) {
            outcount++;
            if (i < 1 || args[i + 1] == NULL) {
                return 0;
            }
        }
        if (strcmp(args[i], "|") == 0) {
            if (args[i + 1] == NULL || i == 0) {
                return 0;
            }
        }
    }
    if (outcount > 1 || incount > 1) {
        return 0;
    }
    return 1;
}

void doline(char **args) {
    int background = 0;
    int pipeflag = 0;
    int normalflag = 0;
    int dupflag = 0;
    int cdflag = 0;

    int inputflag = 0;
    int outputflag = 0;
    int addflag = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strncmp(args[i], "&", 1) == 0) {
            if (args[i + 1] == NULL) {
                background = 1;
                args[i] = NULL;
                break;
            } else {
                printf("wrong command\n");
                return;
            }
        }
        if (strcmp(args[i], "cd") == 0) {
            if (i == 0)
                cdflag = 1;
            else {
                printf("wrong command\n");
                return;
            }
        }
        if (strcmp(args[i], "|") == 0) {
            pipeflag = 1;
        }
        if (strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0 || strcmp(args[i], ">>") == 0) {
            if (strcmp(args[i], ">") == 0)
                outputflag = 1;
            if (strcmp(args[i], "<") == 0)
                inputflag = 1;
            if (strcmp(args[i], ">>") == 0)
                addflag = 1;
            dupflag = 1;
        }
    }
    if (cdflag == 0 && pipeflag == 0 && dupflag == 0) {
        normalflag = 1;
    }

    if (cdflag) {
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                printf("error: lack of args");
            }
            if (chdir(args[1]) != 0) {
                printf("args error\n");
            }
            return;
        }
    }

    pid_t childpid;
    int status = 0;
    switch (childpid = fork()) {
        case 0:
            if (normalflag) {
                if (execvp(args[0], args) == -1) {
                    printf("can not find command\n");
                }
                exit(0);
            }
            if (dupflag && pipeflag == 0) {
                for (int i = 0; args[i] != NULL; i++) {
                    if (strcmp(args[i], "<") == 0) {
                        int fd = open(args[i + 1], O_RDONLY);
                        if (fd == -1) {
                            perror("open");
                            return;
                        }
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                        for (int j = i; args[j] != NULL; j++) {
                            args[j] = args[j + 2];
                        }
                        break;
                    }
                }
                for (int i = 0; args[i] != NULL; i++) {
                    if (strcmp(args[i], ">") == 0) {
                        int fd = open(args[i + 1], O_CREAT | O_RDWR | O_TRUNC, 0754);
                        dup2(fd, 1);
                        args[i] = NULL;
                        break;
                    }
                    if (strcmp(args[i], ">>") == 0) {
                        int fd = open(args[i + 1], O_CREAT | O_RDWR | O_APPEND, 0754);
                        dup2(fd, 1);
                        args[i] = NULL;
                        break;
                    }
                }
                if (execvp(args[0], args) == -1) {
                    printf("can not find command\n");
                }
                exit(0);
            }
            if (pipeflag) {
                int pipe_count = 0; //这里的pipecount计数的是命令数而不是管道数
                char ***pipe_args = (char***)malloc(MAXCH * sizeof(char **));
                int arg_start = 0;

                // 分割各个管道阶段的参数
                for (int i = 0; args[i] != NULL; i++) {
                    if (strcmp(args[i], "|") == 0) {
                        pipe_args[pipe_count] = &args[arg_start];
                        args[i] = NULL;  // 截断当前命令
                        pipe_count++;
                        arg_start = i + 1;
                    }
                }
                pipe_args[pipe_count] = &args[arg_start];  // 最后一个命令
                pipe_count++;

                char **temp_files = (char**)malloc(pipe_count * sizeof(char *));
                pid_t *pids = (pid_t*)malloc(pipe_count * sizeof(pid_t));

                // 创建临时文件链
                for (int i = 0; i < pipe_count - 1; i++) {
                    temp_files[i] = (char*)malloc(32);
                    snprintf(temp_files[i], 32, ".pipe_temp_%d_%d", getpid(), i);
                }

                // 顺序执行每个管道阶段
                for (int i = 0; i < pipe_count; i++) {
                    pids[i] = fork();
                    if (pids[i] == 0) {

                        //处理输入重定向与管道的结合
                        if(i==0 && inputflag){
                            char inputfile[MAXCH];
                            int inputfd = 0;
                            for(int j = 0;pipe_args[0][j]!=NULL;j++){
                                if(strcmp(pipe_args[0][j],"<") == 0){
                                    strcpy(inputfile,pipe_args[0][j+1]);
                                    inputfd = open(inputfile,O_RDONLY);
                                    dup2(inputfd,STDIN_FILENO);
                                    pipe_args[0][j] = NULL;
                                    break;
                                }
                            }
                        }

                        // 输入重定向（前一个临时文件）
                        if (i > 0) {
                            int in_fd = open(temp_files[i - 1], O_RDONLY);
                            dup2(in_fd, STDIN_FILENO);
                            close(in_fd);
                        }

                        // 输出重定向（下一个临时文件）
                        if (i < pipe_count - 1) {
                            int out_fd = open(temp_files[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            dup2(out_fd, STDOUT_FILENO);
                            close(out_fd);
                        }

                        //处理管道与输出重定向的结合
                        if(i == pipe_count -1 && (outputflag || addflag)){
                            char outputfile[MAXCH];
                            int outfd;
                            for(int j = 0;pipe_args[i][j] != NULL;j++){
                                if(strcmp(pipe_args[i][j],">") == 0 || strcmp(pipe_args[i][j],">>") == 0){
                                    strcpy(outputfile,pipe_args[i][j+1]);
                                    if(outputflag){
                                        outfd = open(outputfile, O_RDWR|O_CREAT|O_FSYNC, 0754);
                                    }
                                    if(addflag){
                                        outfd = open(outputfile, O_CREAT|O_RDWR|O_APPEND, 0754);
                                    }
                                    dup2(outfd,STDOUT_FILENO);
                                    close(outfd);
                                    pipe_args[i][j]=NULL;
                                }
                            }
                        }

                        // 执行当前阶段命令
                        execvp(pipe_args[i][0], pipe_args[i]);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    } 
                    else {
                        // 等待当前阶段完成
                        waitpid(pids[i], NULL, 0);

                        // 清理前一个临时文件
                        if (i > 0) {
                            remove(temp_files[i - 1]);
                        }
                    }
                }

                // 清理最后一个临时文件
                if (pipe_count > 1) {
                    unlink(temp_files[pipe_count - 2]);
                }

                // 释放资源
                for (int i = 0; i < pipe_count - 1; i++) {
                    free(temp_files[i]);
                }
                free(temp_files);
                free(pids);
                free(pipe_args);
                exit(0);

            }
        default:
            break;
    }
    if (background == 1) {
        printf("[process id %d]\n", childpid);
        return;
    }
    if (waitpid(childpid, &status, 0) == -1)
        printf("wait for child process error\n");
}

int main(int argc, char *argv[]) {
    signal(SIGINT,sig_hangler);
    mysh_loop();
}

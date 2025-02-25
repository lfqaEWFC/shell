#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

char *readline();
char **spiltline();
void doline();

void mysh_loop(){
    char *line;
    char **args;

    do
    {
        char path[100];
        getcwd(path,100);
        char now[200] = "[myshell ";
        strcat(now,path);
        strcat(now, "]$ ");
        printf("%s", now);

        line = readline();
        if(strcmp(line,"exit\n") == 0){
            break;
        }
        args = spiltline();
        doline();

        free(line);
        free(args);

    }while(1);

}

char *readline(){

    char *line = NULL;
    size_t bufsize = 0;
    getline( &line, &bufsize, stdin);
    return line;

}

char **spiltline(){


}

int main(int argc, char *argv[]){
    mysh_loop();
}

//问题1.属于的命令该如何分割，是以管道为基准，还是以空格为基准；
//思路：在处理管道与重定向时，应该分为两类，只有重定向与两者都有或只有管道，先需要fork一次，之后分类，在遇到管道时接着fork，
//     如果两者都有在重定向时先定向到临时文件，然后再定向到最终的地方；
//     同时在最开始应该对重定向符号后面的文件特殊处理，将其文件句柄进行录入；
//问题2.怎么处理&符号；
//难点：实现管道；
//     重定向与管道都存在的复杂问题；

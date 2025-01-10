#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// 信号处理器函数，用于处理接收到的信号
void sigint_handler(int signum) {
    printf("Received SIGINT signal (Ctrl + C). Exiting gracefully...\n");
    // 这里可以添加清理资源等操作，然后退出程序
    exit(0);
}

int main() {
    struct sigaction sa;
    // 将sa结构体的所有成员初始化为0，确保没有未定义的行为
    sigemptyset(&sa.sa_mask);
    // 设置信号处理函数为sigint_handler
    sa.sa_handler = sigint_handler;
    // 设置信号处理的相关标志，这里使用SA_RESTART，表示系统调用被信号中断后可以自动重启
    sa.sa_flags = SA_RESTART;

    // 使用sigaction函数注册SIGINT信号的处理动作
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error registering SIGINT signal with sigaction");
        return 1;
    }

    printf("Program is running... Press Ctrl + C to send SIGINT signal.\n");
    // 模拟程序的持续运行，这里简单地让程序进入一个无限循环
    while (1) {
        // 可以在这里添加其他正常的程序操作，比如处理业务逻辑等
    }

    return 0;
}

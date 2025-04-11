#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(){
    // char ip[] = system("curl 2ip.ua");
    // printf("%s", ip);
    
    

    FILE *fp;
    char path[1035];
    
    fp = popen("curl 2ip.ua|grep ip", "r");  // Open process and read its output
    if (fp == NULL) {
        printf("Failed to run command\n");
        return 1;
    }
    
    // Read the output line by line
    while (fgets(path, sizeof(path), fp) != NULL) {
        printf("%s", path);
    }
    
    pclose(fp);
    return 0;
}
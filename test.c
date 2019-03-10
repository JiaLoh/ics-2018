#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(){
    char a[5];
    for(int i=0;i<5;i++){
        a[i]='a';
    }
    char *str= a + 'a';
    printf("%s\n",str);
    return 0;
}
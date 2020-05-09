/*
    MOVE: MV (biasa, *)
    memindahkan file-file ke folder tujuan

    $ ls
    $ mv file1.txt ../tujuan/
    $ ls
    $ ls ../tujuan/
    $ mv * ../tujuan/
    $ ls ../tujuan/
*/
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"


char* fmtname (char *path)
{
    static char buf[512];
    char *p;
    for (p=path+strlen(path); p>=path && *p!='/'; p--);
    p++;
    memmove (buf, p, strlen(p));
    return buf;
}

char* strcat (char *d, char *s)
{
    char *temp = d;
    while(*d) 
        ++d;
    while(*s) 
        *d++=*s++;
    *d=0;
    return temp;
}

void move (char *from, char *to)
{
    struct stat st;
    char *buf = (char*)malloc(512*sizeof(char));
    int fd0;
    // OPEN FILE FROM
    if ((fd0=open(from,O_RDONLY)) < 0) {
        printf(2,"mv: cannot open '%s' No such file or directory\n",from);
        exit();
    }
    // JIKA ADALAH DIREKTORI
    if (fstat(fd0,&st)>=0) {
        if(st.type==T_DIR) {
            printf(2,"mv: cannot copy directory '%s'\n",from);
            exit();
        }
    }

    char *temp = (char*)malloc(512*sizeof(char));
    if (to[strlen(to)-1] == '/') to[strlen(to)-1]=0;
    // OPEN FILE TO
    int fd1 = open(to,0);
    if (1) {
        // JIKA ADALAH DIREKTORI
        if (fstat(fd1,&st)>=0 && st.type == T_DIR) {
            strcat(temp,to);
            strcat(temp,"/");
            strcat(temp,from);
            close(fd1);
            if ((fd1=open(temp,O_CREAT | O_TRUNC | O_WRONLY)) < 0) {
                printf(2,"mv: error while create '%s'\n",temp);
                exit();
            }
        }
        // JIKA ADALAH FILE
        else {
            close(fd1);
            if((fd1=open(to,O_CREAT | O_TRUNC | O_WRONLY)) < 0) {
                printf(2,"mv: error while create '%s'\n",to);
                exit();
            }
        }
    }
    int n;
    while ((n=read(fd0,buf,sizeof(buf)))>0) {
        printf(fd1,"%s",buf);
    }
    close(fd1);
    free(temp);
    free(buf);
    unlink(from);
}

void mv_ls (char *path, int panjang, char *ekstensi)
{
    char *buff = (char*)malloc(512*sizeof(char*));
    int fd0, fd1;
    struct dirent de;
    struct stat st;
    if (path[strlen(path)-1]=='/') 
        path[strlen(path)-1]=0;
    if ((fd0=open(".",0)) < 0) {
        printf (2,"mv: cannot open '\".\"' No such file or directory\n");
        exit();
    }

    if ((fd1=open(path,O_RDONLY)) < 0) {
        printf(2,"~mv: cannot open '%s' No such file or directory\n",path);
        exit();
    }
    if (fstat(fd1,&st) < 0) {
        printf(2,"mv: cannot stat '%s' No such file or directory\n",path);
        exit();
    }
    else {
        if(st.type!=T_DIR) {
            printf(2,"mv: '%s' is not directory\n",path);
            exit();
        }
    }
    // tidak perlu switch karena sudah pasti masuk ke direktori
    strcat(buff,path);
    strcat(buff,"/");
    int len = strlen(buff);
    while (read(fd0,&de,sizeof(de))==sizeof(de)) {
        if (de.inum == 0) 
            continue;
        if (de.name[0] == '.')
            continue;
        if (stat(de.name, &st) >= 0 && st.type == T_DIR) 
            continue;
        memmove(buff+len,de.name,strlen(de.name));
        //if(strcmp(de.name+(strlen(de.name)-panjang-1),ekstensi)==0) 
        move(de.name,buff);
        memset(buff+len,'\0',sizeof(buff)+len);
    }
    free(buff);
    close(fd0);
}

void mv_rek (char *from, char *to)
{
    char *buff = (char*)malloc(512*sizeof(char*));
    int fd0;
    struct dirent de;
    struct stat st;
    if (from[strlen(from)-1] == '/') 
        from[strlen(from)-1] = 0;
    if (to[strlen(to)-1] == '/') 
        to[strlen(to)-1] = 0;
    printf(1,"%s\n",to);
    if ((fd0=open(from,0)) < 0) {
        printf(2,"mv: cannot open '%s' No such file or directory\n",from);
        exit();
    }
    if (fstat(fd0,&st) < 0) {
        printf(2,"mv: cannot stat '%s' No such file or directory\n",from);
        exit();
    }
    char *temp,*temp2;
    temp = (char*)malloc(512*sizeof(char*));
    temp2 = (char*)malloc(512*sizeof(char*));
    switch(st.type) {
        case T_FILE: {
            move(from,to);
            break;
        }
        case T_DIR: {
            strcpy(buff,to);
            strcat(buff,"/");
            strcat(buff,from);
            if (mkdir(to)>=0) {
                while(read(fd0,&de,sizeof(de))==sizeof(de)) {
                    if(de.inum==0 || de.name[0]=='.') 
                        continue;
                    strcpy(temp,from);
                    strcat(temp,"/");
                    strcat(temp,de.name);
                    strcpy(temp2,to);
                    strcat(temp2,"/");
                    strcat(temp2,de.name);
                    mv_rek(temp,temp2);
                }
            }
            else {
                while(read(fd0,&de,sizeof(de))==sizeof(de)) {
                    if(de.inum==0 || de.name[0]=='.') 
                        continue;
                    strcpy(temp,from);
                    strcat(temp,"/");
                    strcat(temp,de.name);
                    strcpy(temp2,buff);
                    strcat(temp2,"/");
                    strcat(temp2,de.name);
                    mv_rek(temp,temp2);
                }
                unlink(temp);
            }
            unlink(to);
            break;
        }
    }
    close (fd0);
    free (temp);
    free (temp2);
    free (buff);
}

int main(int argc,char *argv[])
{
    if(argc<2)
        printf(2,"Usage : mv [source] [dest]\n");
    else if(strcmp(argv[1],"*") == 0) {
        mv_rek (".",argv[2]);
        exit ();
    }
    else {
        move(argv[1],argv[2]);
        exit();
    }
    exit();
}




//MV Biasa
// #define BUFFER_SIZE 256

// int main(int argc, char *argv[]){
//     if (argc != 3) {
//         printf(1, "Command Error\n");
//         exit();
//     }

//     int flag = mv(argv[1],argv[2]);
//     if (flag <0) { 
//         if(flag == -3){
//         printf(1, "mv: permission deny.\n");
//         }
//         else if(flag == -2){
//         printf(1, "mv: error to open the file.\n");
//         }
//         else if(flag == -1){
//         printf(1, "mv: error to open the file.\n");
//         }
//         exit();
//     }
//     else {
//         printf(1,"good\n");
//     }

//     int source = open(argv[1], 0);
//     if (source == -1) {
//         printf(1, "Source file can not open.\n");
//         exit();
//     }
    
//     int distination = open(argv[2], O_WRONLY | O_CREATE);
//     if (distination == -1) {
//         printf(1, "Distination file can not write, Please Contact the root.\n");
//         exit();
//     }
//     else if (distination == -2) {
//         printf(1, "mv: permission deny.\n");
//         exit();
//     }

//     char buffer[BUFFER_SIZE];
//     int length = 0;
//     while((length = read(source, buffer, BUFFER_SIZE)) > 0)
//         write(distination, buffer, length);

//     close(source);
//     close(distination);

//     if (unlink(argv[1]) < 0)
//         printf(1, "Can not Delete Source file.\n");
//     exit();
// }
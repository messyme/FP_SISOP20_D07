/*
    cp (biasa, -R, *)
    Mengcopy file
    Contoh: 
    cp file1.c file2.c -> mengcopy file1.c menjadi file2.c
    cp -r dir1 dir2 -> mengcopy direktori dir1 menjadi dir2
    cp * /var/www/html -> mengcopy semua file di direktori saat ini ke /var/www/html

    $ rm cp.o
    $ code Makefile
    $ cp wc.c(file1) cp.c(file2)  -->file 1 dan file 2 yang akan di copy
    $ code cp.c
    $ code Makefile
    $ make
    $ qemu-nox
*/

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"


int pertama = 0;

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
    char *temp=d;
    while (*d) 
        ++d;
    while (*s) 
        *d++=*s++;
    *d=0;
    return temp;
}


void copy (char *from, char *to) 
{
    struct stat st;
    char *buf;
    buf = (char*)malloc(512*sizeof(char));
    // OPEN FILE FROM
    if((int fd0=open(from,O_RDONLY)) < 0) {
        printf(2,"cp: cannot open '%s' No such file or directory\n",from);
        exit();
    }
    // JIKA ADALAH DIREKTORI
    if (fstat(fd0,&st) >= 0) {
        if (st.type == T_DIR) {
            printf(2, "cp: cannot copy directory '%s'\n", from);
            exit ();
        }
    }

    char *temp;
    temp = (char*)malloc(512*sizeof(char));
    if(to[strlen(to)-1]=='/') to[strlen(to)-1]=0;
    // OPEN FILE TO
    int fd1 = open(to,0);
    if (1) {
        // JIKA ADALAH DIREKTORI
        if (fstat(fd1,&st) >= 0 && st.type == T_DIR) {
            mkdir (to);
            strcat (temp,to);
            strcat (temp,"/");
            strcat (temp,from);
            close (fd1);
            if ((fd1=open(temp,O_CREATE | O_RDWR))<0) {
                printf(2,"cp: error while create '%s'\n",temp);
                exit();
            }
        }
        // JIKA ADALAH FILE
        else {
            close (fd1);
            if ((fd1=open(to, O_CREATE | O_RDWR))<0) {
                printf (2, "cp: error while create '%s'\n", to);
                exit ();
            }
        }
    }
    while ((int n=read(fd0, buf, sizeof(buf)))>0) {
        printf(fd1, "%s", buf);
    }
    close (fd1);
    free (temp);
    free (buf);
}


void cp_ls (char *path, int panjang, char *ekstensi)
{
    char* buff = (char*)malloc(512*sizeof(char*));
    int fd0, fd1;
    struct dirent de;
    struct stat st;
    if (path[strlen(path)-1]=='/') path[strlen(path)-1]=0;
    if ((fd0=open(".",0)) < 0) {
        printf(2,"cp: cannot open '\".\"' No such file or directory\n");
        exit();
    }
    if ((fd1=open(path,O_RDONLY)) < 0) {
        printf(2,"cp: cannot open '%s' No such file or directory\n",path);
        exit();
    }
    if (fstat(fd1,&st) < 0) {
        printf(2,"cp: cannot stat '%s' No such file or directory\n",path);
        exit();
    }
    else {
        if (st.type != T_DIR) {
            printf(2,"cp: '%s' is not directory\n",path);
            exit();
        }
    }
    // tidak perlu switch karena sudah pasti masuk ke direktori
    strcat (buff,path);
    strcat (buff,"/");
    int len=strlen(buff);
    while(read(fd0,&de,sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) 
            continue;
        if (de.name[0] == '.')
            continue;
        if (stat(de.name, &st) >= 0 && st.type == T_DIR) 
            continue;
        memmove(buff+len,de.name,strlen(de.name));
        if (strcmp(de.name+(strlen(de.name)-panjang+1),ekstensi) == 0) 
            copy (de.name,buff);
        memset (buff+len, '\0', sizeof(buff)+len);
    }
    free (buff);
    close (fd0);
}

void cp_rek (char *from, char *to)
{
    char *buff = (char*)malloc(512*sizeof(char*));
    int fd0;
    struct dirent de;
    struct stat st;
    if (from[strlen(from)-1] == '/') 
        from[strlen(from)-1] = 0;
    if (to[strlen(to)-1] == '/') 
        to[strlen(to)-1] = 0;
    if ((fd0=open(from, 0))<0) {
        printf(2,"cp: cannot open '%s' No such file or directory\n",from);
        exit();
    }
    if (fstat(fd0, &st) < 0) {
        printf(2,"cp: cannot stat '%s' No such file or directory\n",from);
        exit();
    }
    char *temp,*temp2;
    temp=(char*)malloc(512*sizeof(char*));
    temp2=(char*)malloc(512*sizeof(char*));
    switch(st.type) {
        case T_FILE: {
            copy(from,to);
            break;
        }
        case T_DIR: {
            strcpy(buff,to);
            strcat(buff,"/");
            strcat(buff,from);
            if(mkdir(to) >= 0) {
                while(read(fd0,&de,sizeof(de))==sizeof(de)) {
                    if(de.inum==0 || de.name[0]=='.') 
                        continue;
                    strcpy(temp,from);
                    strcat(temp,"/");
                    strcat(temp,de.name);
                    strcpy(temp2,to);
                    strcat(temp2,"/");
                    strcat(temp2,de.name);
                    cp_rek(temp,temp2);
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
                    cp_rek(temp,temp2);
                }
            }
            break;
        }
        close(fd0);
    }
    free(temp);
    free(temp2);
    free(buff);
}

int main(int argc,char *argv[])
{
    if(argc<2) {
        printf(2,"Usage : cp [OPTIONS] [source] [dest]\n");
        printf(2,"Options:\n");
        printf(2,"  -R : copy file secara recursive\n");
    }
    if(argv[1][0]=='*') {
        int panjang=strlen(argv[1]);
        char eks[512];
        strcpy(eks,argv[1]+1);
        cp_ls(argv[2],panjang,eks);
        exit();
    }
    else if(strcmp(argv[1],"-R")==0) {
        char *temp;
        temp=(char*)malloc(512*sizeof(char*));
        strcat(temp,argv[3]);
        strcat(temp,"/");
        strcat(temp,argv[2]);
        mkdir(temp);
        cp_rek(argv[2],argv[3]);
        free(temp);
        exit();
    }
    else {
        copy(argv[1],argv[2]);
        exit();
    }
    exit();
}	



//D07
//dr utube
// char buf [512];

// int main (int argc, char *argv[])
// {
//     int fd1, fd2, n;

//     if (argc <= 2) {
//         printf (1, "Butuh 2 arguments!\n");
//         exit ();
//     }
//     if ((fd1 = 0 open(argv[1], O_RDONLY)) < 0) {
//         printf (1, "cp: cannot open %s\n", argv[1]);
//         exit();
//     }
//     if ((fd2 = 0 open(argv[2], O_CREATE|O_RDWR)) < 0) {
//         printf (1, "cp: cannot open %s\n", argv[2]);
//         exit();
//     }
//     while ((n = read(fd1,buf,sizeof(buf))) > 0) {
//         write (fd2 buf, n);
//     }
//     close (fd1);
//     close (fd2);
// }


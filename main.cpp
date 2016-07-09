#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <fcntl.h>

#include <sys/types.h>  
#include <sys/stat.h>  
#include <time.h>
#include <signal.h>
/*

DEPSFLAGS := -lpthread 

*/

#define MAXLEN 2048
#define BUF_LEN 200

#define LOG_OUT_DIR (char *)"/home/android/build-log"

#define GET_TIME_SET(x,h,m) \
do { \
	char buf[10] = {0}; \
	char *p = argv[x]; \
	char *q = buf; \
	while(*p) { \
		if(*p != ':') { \
			*q++ = *p; \
		} \
		p++; \
	} \
	*h = atoi(buf)/100; \
	*m = atoi(buf)%100; \
} while(0)


#define SET_DAEMON(x,d) \
do { \
	*d = x; \
	if(x) { \
		printf("\nDaemon mode,background runing\n"); \
	} \
} while(0)


#define SHOW_HELP() \
do { \
	printf("\n\n--help :\n");\
	printf("\n ./work-timer -c  - kill proc\n");\
	printf("\n ./work_timerd -d -p v3991 -b userdebug -r /home/android/work/prj/3991/debug/ -t 09:58 & \n");\
	printf("\n--end \n");\
} while(0)

static int exec_system_call(char *cmd,char *ret, int len);

static void log(char *info);
static void daemon_mode(void);
static int kill_proc(char *proc);
static void signal_handler(int sig);

pthread_t work_thd;

typedef struct Project_info {
	int h;
	int m;
	int is_daemon;
	int time_out;
	char prj_name[20];
	char build_type[20];
	char script[BUF_LEN];
	char pwd[BUF_LEN];
	char prj_path[BUF_LEN];
	char console[BUF_LEN];
};

static Project_info mProject_info;
static Project_info *mPrj = &mProject_info;

void getLocalTime(int *y,int *mon, int *d, int *h, int *m, int *s) {
	time_t timep;
	struct tm *p;

	time(&timep);
    p = localtime(&timep);

    *y = p->tm_year + 1900;
    *mon = p->tm_mon + 1;
    *d = p->tm_mday;
    *h = p->tm_hour;
    *m = p->tm_min;
    *s = p->tm_sec;
}

void* (*timeout_fun_ptr) (void *) = NULL;  


static void go_go_go(int do_what) {
    char *exec = mPrj->script;

	if(mPrj->is_daemon) {
		memset(mPrj->console,0,BUF_LEN);
		sprintf(mPrj->console,"echo exec : %s start! >> %s/%s-%s.log",exec,LOG_OUT_DIR,mPrj->prj_name,mPrj->build_type);
		system(mPrj->console);
	}
	else {
		printf("exec : %s start!\n", exec);
	}

	if(system(exec) > 0) {
		if(mPrj->is_daemon) {
			memset(mPrj->console,0,BUF_LEN);
			sprintf(mPrj->console,"echo exec : %s fail! >> %s/%s-%s.log",exec,LOG_OUT_DIR,mPrj->prj_name,mPrj->build_type);
			system(mPrj->console);
		}
		else {
			printf("exec : %s fail!\n", exec);
		}
		return;
    }

	if(mPrj->is_daemon) {
		memset(mPrj->console,0,BUF_LEN);
		sprintf(mPrj->console,"echo exec : %s success! >> %s/%s-%s.log",exec,LOG_OUT_DIR,mPrj->prj_name,mPrj->build_type);
		system(mPrj->console);
	}
	else {
		printf("exec : %s success!\n", exec);
	}

}


static void *work_main_thread(void *args) 
{
    int y, mon, d;
	int h, m, s;
	int i = 0, j = 0;

	memset(mPrj->console,0,BUF_LEN);
	sprintf(mPrj->console,"rm -rf %s/%s-%s.log",LOG_OUT_DIR,mPrj->prj_name,mPrj->build_type);
	system(mPrj->console);

    for(;;) 
	{
		getLocalTime(&y,&mon,&d,&h,&m,&s);

		if(++i >= 60) { //10 min
			i = 0;
			if(++j >= 6*2) { //120 min
				j = 0;
				if(mPrj->time_out) {
					mPrj->time_out = 0;
				}
			}		
		}
		else {
			memset(mPrj->console,0,BUF_LEN);
		    sprintf(mPrj->console,"echo %04d/%02d/%02d-%02d:%02d:%02d { %02d:%02d } - %s >>%s/%s-%s.log", 
				y, mon, d, h, m, s,mPrj->h,mPrj->m,mPrj->script,LOG_OUT_DIR,mPrj->prj_name,mPrj->build_type);

			if(mPrj->is_daemon) {
				system(mPrj->console);
			}
			else {
				printf("%04d/%02d/%02d-%02d:%02d:%02d { %02d:%02d } - %s\n", y, mon, d, h, m, s,mPrj->h,mPrj->m,mPrj->script);
			}
		}

		if(mPrj->time_out == 0 && h == mPrj->h && m >= mPrj->m) {
			mPrj->time_out = 1;
			go_go_go(0);
		}

		// 10 s
		sleep(10);
	}

	return NULL;
}


int exec_system_call(char *cmd,char *ret, int len) {
    char tmp[1024] = {0};
    FILE *pp;

    if((pp = popen(cmd, "r")) == NULL) {
        return -1;
    }
    while(fgets(tmp, sizeof(tmp), pp) != NULL) {
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0'; 
        }
    }
    
    pclose(pp); 

	if(len > 1024) {
        len = 1024;
	}
		
    memcpy(ret,tmp,len);
	return 0;
}

int parse_from_file(char *filename,int *hh, int *mm) {
	FILE *file;
	int pos = 0,temp,i;
	char dest[MAXLEN] = {0};

	file = fopen(filename,"r");
	if(file == NULL) {
		printf("cannot open file : %s\n",filename);
		return -1;
	}

	for(i = 0; i < MAXLEN-1; i++) {
		temp = fgetc(file);
		if(EOF == temp) {
			break;
		}

		dest[pos++] = temp;
	}
	dest[pos] = 0;
	fclose(file);


	char buf[10] = {0};
	char *q = (char *)"TIMER_SET=";
	char *p = strstr(dest,q);
	if(p != NULL) {
		int ii = 5;
		int j = 0;
		p = p+strlen(q);
		while(ii-- > 0) {
			if(*p != ':') {
				buf[j++] = *p;
			}
		    p++;
		}
	}

	*hh = atoi(buf)/100;
	*mm = atoi(buf)%100;

	//printf("buf : %s\n",buf);
	return 0;
}

int parse_args(int argc, char **argv) {
	int i;

	memset(mPrj,0,sizeof(Project_info));
	exec_system_call((char *)"pwd",mPrj->pwd,BUF_LEN);

	for(i = 0; i < argc; i++) {
		if(strcmp(argv[i],"-c") == 0) {
			kill_proc((char*)"work_timerd");
			return 1;
		}
		else
		if(strcmp(argv[i],"-h") == 0) {
			SHOW_HELP();
			return 1;
		}
		else
		if(strcmp(argv[i],"-d") == 0) {
			SET_DAEMON(1,&mPrj->is_daemon);
		}
		else
		if(strcmp(argv[i],"-p") == 0) {
			strcpy(mPrj->prj_name,argv[i+1]); 
		}
		else
		if(strcmp(argv[i],"-b") == 0) {
			strcpy(mPrj->build_type,argv[i+1]); 
		}
		else
		if(strcmp(argv[i],"-r") == 0) {
			strcpy(mPrj->prj_path,argv[i+1]); 
		}
		else
		if(strcmp(argv[i],"-t") == 0) {
			GET_TIME_SET(i+1,&mPrj->h,&mPrj->m);
		}
	}

	sprintf(mPrj->script, "%s/my-task.sh %s %s %s ",
		mPrj->pwd,mPrj->prj_name,mPrj->build_type,mPrj->prj_path);

	//printf("script: %s\n",mPrj->script);
	return 0;
}


int main(int argc,char *argv[])
{
	if(parse_args(argc,argv) > 0) {
		return 1;
	}

	if(!(mPrj->h >= 0 && mPrj->h <= 23) || !(mPrj->m >= 0 && mPrj->m <= 59)) {
        printf("time input format err - %02d:%02d\n", mPrj->h, mPrj->m);
		exit(0);
		return -1;
	}

	printf("init : \n\n%d,%s,%02d:%02d,\n\n",
		mPrj->is_daemon,
		mPrj->script,
		mPrj->h,
		mPrj->m);

	if(mPrj->is_daemon) {
		daemon_mode();
	}
	else {
		signal(SIGINT, signal_handler); //Ctrl+C
		signal(SIGTSTP, signal_handler); //Ctrl+Z
	}

	pthread_create(&work_thd, NULL, work_main_thread, NULL);
    pthread_join(work_thd, NULL); 
	return EXIT_SUCCESS;
}

void daemon_mode(void)
{
    int fr = 0;

    fr = fork();
    if(fr < 0) {
        fprintf(stderr, "fork() failed\n");
        exit(1);
    }
    if(fr > 0) {
        exit(0);
    }

    if(setsid() < 0) {
        fprintf(stderr, "setsid() failed\n");
        exit(1);
    }

    fr = fork();
    if(fr < 0) {
        fprintf(stderr, "fork() failed\n");
        exit(1);
    }
    if(fr > 0) {
        fprintf(stderr, "forked to background (%d)\n", fr);
        exit(0);
    }

    umask(0);

    fr = chdir("/");
    if(fr != 0) {
        fprintf(stderr, "chdir(/) failed\n");
        exit(0);
    }

    close(0);
    close(1);
    close(2);

    open("/dev/null", O_RDWR);

    fr = dup(0);
    fr = dup(0);
}

int kill_proc(char *proc) {
    char p[BUF_LEN];

	printf("\n\n%s: %s\n\n", __func__, proc);
    memset(p,0,BUF_LEN);
	sprintf(p,"%s/stop.sh %s", mPrj->pwd, proc);
	if(system(p) < 0) {
		printf("kill : %s fail!\n",p);
		return -1;
	}
    return 0;
}


static void signal_handler(int sig) {
	printf("\n\n%s: %d\n\n", __func__, sig);
    kill_proc((char*)"work_timerd");
	exit(0);
}


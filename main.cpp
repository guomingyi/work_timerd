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

#define TIMER_RUN_LOG "/home/android/timer-log.txt"
#define EXEC_PATH_LEN sizeof(exec_script_path)
#define MAXLEN 2048
#define is_daemon 1

int exec_system_call(char *cmd,char *ret, int len);
void parse_config(void);
void log(char *info);


pthread_t work_thd;
int time_out_flag = 0;
int mHour = 0;
int mMin = 0;
char exec_script_path[1024];
char console[1024] = {0};


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

static void *timeout_work_thread(void *args) {
    char exec[100] = {0};
	int ret = 0;
	
	sprintf(exec, "%s &",exec_script_path);
	if(system(exec) > 0) {
        printf("exec : %s fail!\n", exec);
		ret = -1;
    }

	memset(console,0,sizeof(console));
	sprintf(console,"echo %s:%d >> %s",exec,ret,(char *)TIMER_RUN_LOG);
	system(console);

    printf("###exec : %s\n",exec);
	return NULL;
}


static void go_go_go(int do_what) {
	printf("time out ,doing you want to do: %d\n",do_what);

	if(do_what == 0) {
		pthread_t timeout_thd = 0;
		pthread_create(&timeout_thd, NULL, timeout_work_thread, NULL);
		pthread_join(timeout_thd, NULL); 
    }
}


static void *work_main_thread(void *args) 
{
    int y, mon, d;
	int h, m, s;
	int i = 0, j = 0;

    for(;;) {
		
		getLocalTime(&y,&mon,&d,&h,&m,&s);
		memset(console,0,sizeof(console));
        sprintf(console,"echo run - %04d/%02d/%02d-%02d:%02d:%02d >> %s", 
			y, mon, d, h, m, s,(char *)TIMER_RUN_LOG);
		
		if(!is_daemon) {
			printf("set:%02d:%02d,curr:%s\n", mHour, mMin,console);
		}

		if(time_out_flag == 0 && h == mHour && m >= mMin) {
			time_out_flag = 1;
			go_go_go(0);
		}

        sleep(10);// 10 s
		if(++i >= 60) { //10 min
			i = 0;
			if(++j >= 6*2) { //120 min
				j = 0;
				if(is_daemon) {
					memset(console,0,sizeof(console));
					sprintf(console,"echo update-parse_config >> %s",(char *)TIMER_RUN_LOG);
					system(console);
				}
				parse_config(); //update config.
				time_out_flag = 0;
			}		
		}
		else {
			if(is_daemon) {
				system(console);
			}
		}
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

void parse_config(void) {
    char *p = exec_script_path;

    memset(p,0,EXEC_PATH_LEN);
	exec_system_call((char *)"pwd",p,EXEC_PATH_LEN);
	sprintf(p,"%s/my-task.sh",p);
    printf("wait exec: %s\n",p);
    parse_from_file(p, &mHour,&mMin);
}

void daemon_mode(void)
{
    int fr = 0;

	//printf("\n\n%s\n\n", __func__);

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
    char p[EXEC_PATH_LEN];

    memset(p,0,EXEC_PATH_LEN);
	exec_system_call((char *)"pwd",p,EXEC_PATH_LEN);
	sprintf(p,"%s/stop.sh %s", p, proc);
	if(system(p) < 0) {
		printf("kill : %s fail!\n",p);
		return -1;
	}
    return 0;
}


static void signal_handler(int sig) {
	printf("\n\n%s: %d\n\n", __func__, sig);
	exit(0);
}

/*
*
* start :
* ./work-timer 
* ./work-timer hh:mm
* ./work-timer hh mm
*
* stop :
* ./work-timer -c
*
*/

int main(int argc,char *argv[])
{
	if(argc == 2 && strcmp(argv[1],"-c") == 0) {
		kill_proc((char*)"work_timerd");
		return 1;
	}
	
	parse_config();

	if(argc == 2) {
		char buf[10] = {0};
		char *p = argv[1];
		char *q = buf;
		while(*p) {
			if(*p != ':') {
				*q++ = *p;
			}
			p++;
		}
		mHour = atoi(buf)/100;
		mMin = atoi(buf)%100;
	}
	else
	if(argc == 3) {
		mHour = atoi(argv[1]);
		mMin = atoi(argv[2]);
	}

	if(!(mHour >= 0 && mHour <= 23) || !(mMin >= 0 && mMin <= 59)) {
        printf("time input format err - %02d:%02d\n", mHour, mMin);
		return -1;
	}

	printf("Timer set - %02d:%02d\n", mHour, mMin);

	//Ctrl+C/Z
	signal(SIGINT, signal_handler);
	signal(SIGTSTP, signal_handler);

	if(is_daemon) {
		daemon_mode();
	}

	pthread_create(&work_thd, NULL, work_main_thread, NULL);
    pthread_join(work_thd, NULL); 

	return EXIT_SUCCESS;
}



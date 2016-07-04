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

/*

DEPSFLAGS := -lpthread 

*/

//#define EXEC_PATH "/home/android/work/script/work/my-task.sh &"

int exec_system_call(char *cmd,char *ret, int len);


static pthread_t work_thd;
static int time_out_flag = 0;
static int mHour = 0;
static int mMin = 0;

static char exec_script_path[100];
#define EXEC_PATH_LEN sizeof(exec_script_path)

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
	
	sprintf(exec, "%s &",exec_script_path);
	if(system(exec) > 0) {
        printf("exec : %s fail!\n", exec);
		return NULL;
    }

    printf("exec : %s\n",exec);
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

    for(;;) {
		
		getLocalTime(&y,&mon,&d,&h,&m,&s);
        printf("%04d/%02d/%02d-%02d:%02d:%02d\n", y, mon, d, h, m, s);
		printf("set:%02d:%02d\n", mHour, mMin);
		if(time_out_flag == 0 && h == mHour && m >= mMin) {
			time_out_flag = 1;
			go_go_go(0);
		}

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
    //printf("ret: %s\n",tmp);

	if(len > 1024) {
        len = 1024;
	}
		
    memcpy(ret,tmp,len);
	return 0;
}

int parse_from_file(char *filename,int *hh, int *mm) {
	FILE *file;
	int pos = 0,temp,i;
	#define MAXLEN 200
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


/*
* ./work-timer 
* ./work-timer hh:mm
* ./work-timer hh mm
*/

int main(int argc,char *argv[])
{
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
	pthread_create(&work_thd, NULL, work_main_thread, NULL);
    pthread_join(work_thd, NULL); 

	return EXIT_SUCCESS;
}



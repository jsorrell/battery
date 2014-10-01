/* Author: Jack Sorrell */
/* Creation Date: September 6, 2014 */

#include <stdio.h>
#include <stdlib.h>

//logging
#include <string.h>
#include <time.h>
#include <stdarg.h>

//signals and branches
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

//file logging not thread safe. Oh well. I really don't care
#define LOG_FILE_NAME "/home/jsorrell/Dropbox/Projects/battery/battery.log"
#define SLEEP_TIMER_TIME 90


//stringify number
#define _STR(s) #s
#define STR(s) _STR(s)

FILE* logfile;
int zenityPid;

char *getTime() {
	time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  char *timeStr = asctime(timeinfo);
  timeStr[strlen(timeStr)-1] = '\0';
  return timeStr;
}

#define log(...) Log(__VA_ARGS__)
void Log(const char* format, ...) {
		char newFormat[300];
		snprintf(newFormat, 300, "%s: %s\n", getTime(), format);
    va_list args;
    va_start(args, format);
		vfprintf(logfile, newFormat, args);
    va_end(args);
    fflush(logfile);
}

inline void makeCompSleep()
{
	log("Computer Going to Sleep");
	system("dbus-send --system --print-reply \
		--dest='org.freedesktop.UPower' \
		/org/freedesktop/UPower \
		org.freedesktop.UPower.Suspend");
}

inline int createNotification()
{
	int pid;
	if ( !(pid = fork()) ) {
		execl("/usr/bin/zenity","zenity","--warning","--text",
				"The power cord has been unplugged.\nComputer will sleep in " STR(SLEEP_TIMER_TIME) " seconds without action.",
				"--ok-label=Cancel Sleep", NULL);
	}
	return pid;
}

inline void sigHandler(int sig)
{
	if (zenityPid) {
		kill(zenityPid,SIGTERM);
		if (sig == SIGUSR1)
			log("Power Restored");
		else if (sig == SIGTERM)
			log("Terminating Notification Controller");
	} else {
		if (sig == SIGTERM)
			log("Terminating Sleep Timer");
	}
	exit(0);
}

int main()
{
	logfile = fopen(LOG_FILE_NAME,"a");
	if (logfile == NULL){
		fprintf(stderr, "Could not write to logfile\n");
		exit(1);
	}
	log("Power Unplugged");

	int notificationPid;
	int timerPid = getpid();
	signal(SIGUSR1,sigHandler);
	signal(SIGTERM,sigHandler);

	if ( (notificationPid = fork()) ) {
		//parent
		zenityPid = 0;
		//create sleep_timer
		sleep(SLEEP_TIMER_TIME);
		kill(notificationPid, SIGTERM);
		makeCompSleep();
	} else {
		setsid();
		zenityPid = createNotification();
		if (zenityPid == -1) {
			log("Fork Error");
			log("errno: %d",errno);
			exit(2);
		}
		int status;
		waitpid(zenityPid,&status,0);

		if (WIFSIGNALED(status)) {
			//notification terminated by timer. or summit
		}
		else if (!WEXITSTATUS(status)) {
			log("Sleep Timer Cancelled by User");
			kill(timerPid,SIGTERM);
		} else {
			//continue timer and die.
		}
	}
	return 0;
}
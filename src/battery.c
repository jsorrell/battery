/* Author: Jack Sorrell */
/* Creation Date: September 6, 2014 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <sys/wait.h>

//file logging not thread safe. Oh well. I really don't care
#define LOG_FILE_NAME "battery.log"
#define CHECK_TIME 5
#define SLEEP_TIMER_TIME 10


//stringify number
#define _STR(s) #s
#define STR(s) _STR(s)

FILE* logfile;
int notificationPid;

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

inline int powerUnplugged()
{
	char buffer[30];
	FILE* powerInfoFile = fopen("/proc/acpi/ac_adapter/ADP0/state","r");
	fscanf(powerInfoFile, "state: %s\n",buffer);
	fclose(powerInfoFile);
	return !strncmp(buffer,"off-line",30);
}

inline void makeCompSleep()
{
	kill(notificationPid,SIGINT);
	log("Computer Going to Sleep");
	system("dbus-send --system --print-reply \
		--dest='org.freedesktop.UPower' \
		/org/freedesktop/UPower \
		org.freedesktop.UPower.Suspend");
}

inline void createSleepTimer(unsigned time)
{
	alarm(time);
}

inline void stopSleepTimer()
{
	alarm(0);
}


inline int createNotification()
{
	int pid;
	if (!(pid = fork())) {
		//child
		execl("/usr/bin/zenity","zenity","--warning","--text",
			"The power cord has been unplugged.\nComputer will sleep in " STR(SLEEP_TIMER_TIME) " seconds without action.",
			"--ok-label=Cancel Sleep", NULL);
	}
	return pid;
}

inline void notificationAnswerHandler(int sig)
{
	int status;
	waitpid(-1, &status, 0);
	if (!WIFSIGNALED(status) && WEXITSTATUS(status) == 0) {
		stopSleepTimer();
		log("Sleep Timer Cancelled by User");
	}
}

void sigIntHandler(int sig)
{
	kill(notificationPid,SIGINT);
	log("Stopping");
	exit(0);
}


int main()
{
	logfile = fopen(LOG_FILE_NAME,"a");
	if (logfile == NULL)
		fprintf(stderr, "Could not write to logfile\n");
	log("Starting");
	int online = 1;
	signal(SIGALRM, makeCompSleep);
	signal(SIGCHLD, notificationAnswerHandler);
	signal(SIGINT, sigIntHandler);

	while(1) {
		if (powerUnplugged()) {
			if (online) {
				online = 0;
				log("Power Unplugged");
				createSleepTimer(SLEEP_TIMER_TIME);
				notificationPid = createNotification();
			}
		} else {
			if (!online) {
				online = 1;
				stopSleepTimer();
				kill(notificationPid,SIGINT);
				log("Power Restored");
			}
		}
		sleep(CHECK_TIME);
	}
}
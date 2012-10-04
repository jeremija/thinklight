#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <errno.h>

#include <signal.h> /* for ctrl+c catching */

#include "logger/logger.h"
#include "logger/string_allocation.h"

const char* FILENAME = "/proc/acpi/ibm/light";
const char* PID = "/var/run/thinklight.pid";
const char* APP_NAME = "thinklight";

int terminated = 0;

void turn_light_on();
void turn_light_off();
void write_pid(pid_t pid);
void terminate_if_running();

void ex_program(int sig) {
  LOG(WARN, "Caught signal: %d! Turning light off...\n", sig);
  terminated = 1;
  turn_light_off();
  write_pid((pid_t) 0);
  exit(0);
  
  (void) signal(SIGINT, SIG_DFL);
//  (void) signal(SIGINT, ex_program);
}

void write_pid(pid_t pid) {
  FILE* writepid = fopen(PID, "w");
  if (writepid == NULL) die("cannot open the pid file");
  fprintf(writepid, "%d\n", pid);
  fclose(writepid);
  LOG(INFO, "written pid=%d to %s", pid, PID);
}

/*
 * returns 1 if another instance is running, 0 if not
 */
int check_if_another_instance_running() {
  int pid_file = open(PID, O_CREAT | O_RDWR, 0666);
  int rc = flock(pid_file, LOCK_EX | LOCK_NB);
  if (rc) {
    if (EWOULDBLOCK == errno) {
      //???
    }
    
    /* another instance is running */
    return 1;
  }
  
  /* another instance isn't running*/
  
  /* register a signal callback (ctrl+c for example) */
  (void) signal(SIGINT, ex_program);
  
  return 0;
}


void check_thinklight_access() {
  FILE* light = fopen(FILENAME, "w");
  if (light == NULL) {
    die("Cannot access the acpi thinklight");
  }
  fclose(light);
}

void turn_light_on() {
  if (terminated) return;
  FILE* light;
  light = fopen(FILENAME, "w");
  fprintf(light, "on");
  fclose(light);
}

void turn_light_off() {
  FILE* light;
  light = fopen(FILENAME, "w");
  fprintf(light, "off");
  fclose(light);
}

/*
 * call this only after you've checked if another instance is running
 * with check_if_another_instance_running() because the pid file can
 * contain old pid (if the process has been killed from task manager)
 * and we don't want to kill another potential process with the same pid.
 */ 
void read_pid_and_terminate() {
  int pid = 0;

  FILE* get_pid = fopen(PID, "r");
  if (get_pid == NULL) die("failed to open the pid file for reading!");
  fscanf(get_pid, "%d", &pid);
  fclose(get_pid);
  
  LOG(INFO, "read pid=%d from %s", pid, PID);
  if (pid == 0) {
    LOG(INFO, "Process not running, exiting");
    return;
  }

  char* kill_command = print_to_string("kill %d", pid);
  FILE* kill = popen(kill_command, "r");
  free(kill_command);
  if (kill == NULL) die("failed to call 'kill PID'");
  LOG(INFO, "instance with pid=%d terminated", pid);
  pclose(kill);
  
  turn_light_off();
  
  write_pid(0);
}

void help() {
  printf(
      "usage: %s [OPTIONS]\n\n"
      "Options:\n"
      "    -h        help\n"
      "    -i        add a custom interval in milliseconds.\n"
      "              ok value is 100000. default is 200000.\n"
      "    -q        attempt to kill an existing thinklight process\n"
//      "    -v        verbose (should be first in line (after -u), or something may not be outputted)\n"
      "\n"
      "NOTE: you should run this app with sudo!\n"
      , APP_NAME);
  exit(0);
} 

int main(int argc, char *argv[]) {
  
  int interval = 200000;
  int kill = 0;

  /* variables used with getopt() */
  extern char *optarg;
  extern int optind, opterr;
  
  int argument;
  /*
   * read arguments in while loop and set the neccessary variables
   */
  while( (argument = getopt(argc, argv, "hi:q")) != -1  ) {
    switch(argument) {
      case 'h':
        /* print help text */
        help();
        break;
      case 'i':
        /* custom interval */
        interval = atoi(optarg);
        break;
      case 'q':
        kill = 1;
        break;
      default: /* '?' */
        die("Wrong arguments or argument requires an option which was not supplied");
    }
  }
  
  check_thinklight_access();
  
  LOG(INFO, "checking if another instance running...");  
  
  /* check if another instance running */
  if (check_if_another_instance_running()) {
    /* if -q flag used, kill another instance and finish program */
    if (kill) {
      read_pid_and_terminate();
      turn_light_off();
      exit(0);
    }
    else {
      die("another instance of thinklight is running!");
    }
  } 
  else if (kill) {
    LOG(INFO, "no other instance is running");
    exit(0);
  }
  
  /* this is the only instance */
  write_pid(getpid());
  LOG(INFO, "about to start flashing...");
  
  while(1) {
    turn_light_on();
    usleep(interval);
    
    turn_light_off();    
    usleep(interval);
  }

  return 0;
}


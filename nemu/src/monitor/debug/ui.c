#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
	char *arg=strtok(NULL," ");
	int N;
	if(arg==NULL) cpu_exec(1);
	else{
		sscanf(arg,"%d",&N);
		cpu_exec(N);
	}
	return 0;
}

static int cmd_x(char *args){
	char *arg = strtok(NULL," ");
	uint32_t N1;
	sscanf(arg,"%d",&N1);

	char *argp=strtok(NULL," ");
	uint32_t N2;
	sscanf(argp,"0x%x",&N2);
	uint32_t i;
	i=0;
	while(i<N1){
		printf("%x\n",paddr_read(N2,4));
		N2=N2+4;
		i=i+1;
	}
	return 0;
}

static int cmd_info(char *args){
	char *arg = strtok(NULL," ");
	if (*arg =='r'){
		printf("eax 0x%08x\n", cpu.eax);
		printf("ebx 0x%08x\n", cpu.ebx);
		printf("ecx 0x%08x\n", cpu.ecx);
		printf("edx 0x%08x\n", cpu.edx);
		printf("eip 0x%08x\n", cpu.eip);
		printf("esp 0x%08x\n", cpu.esp);
		printf("ebp 0x%08x\n", cpu.ebp);
		printf("edi 0x%08x\n", cpu.edi);
		printf("esi 0x%08x\n", cpu.esi);
	}
	return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si","Single step",cmd_si},
  { "info","Print the info of register",cmd_info},
  { "x","scan the memory", cmd_x},

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

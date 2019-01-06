#include "command.h"

#include <string.h>
Command::Command(char * cmd) : commandFromStdin(cmd) {
  isExitOrNothing = false;
}

Command::~Command() {}

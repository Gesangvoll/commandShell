#ifndef _COMMAND_H
#define _COMMAND_H
#include <cstdlib>
#include <string>
#include <vector>
#define MAX_ARGC

class Command
{
 public:
  bool withSlash;
  bool isExitOrNothing;
  std::string commandFromStdin;
  std::vector<std::string> parameters;
  std::vector<std::string> possibleAbsoluteFilename;
  std::string basename;
  bool withParameter;
  Command();
  Command(char * cmd);
  ~Command();
};
#endif

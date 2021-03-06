#ifndef _PARSER_H
#define _PARSER_H
#include <stdio.h>
#include <string.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "command.h"
class Parser
{
 private:
  Command * cmd;
  std::string envv;
  int findBorderBetweenFilenameAndParameters(std::string & readyForProcessing);
  void trimSpaces(std::string & rawFilename);
  void formatParameters(std::string rawParameters);
  void setPossibleAbsoluteFilename(std::string rawFilename);
  std::string replaceVars(std::string commandFromStdin);

 public:
  Parser();
  Parser(char * commandFromStdin, std::string _envv);
  ~Parser();
  Command * parse();
  Command * getCommand();
  static bool isValidVarCharacter(char & ch);
  static bool isBase10Number(std::string str);
  static std::string addByOne(std::string str);
};
#endif

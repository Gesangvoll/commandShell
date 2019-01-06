#ifndef _MYSHELL_H
#define _MYSHELL_H
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <map>
#include <set>

#include "command.h"
class MyShell
{
 private:
  bool replStatus;
  char * envvariable;
  void displayPrompt();
  void executeBuiltinCommand(Command * cmd);
  void executeExternalCommand(Command * cmd);
  void executeCommand(Command * cmd);
  std::set<std::string> builtinCommands;

 public:
  MyShell();
  ~MyShell();
  static std::map<std::string, std::string> environVarMap;
  static std::map<std::string, std::string> varMap;
  static std::string getVar(std::string key);
  static void initializeEnvironVarMap(char ** environ);
  void runRepl();
  void executeCd(Command * cmd);
  void executeSet(Command * cmd);
  void executeExport(Command * cmd);
  void executeInc(Command * cmd);
};
#endif

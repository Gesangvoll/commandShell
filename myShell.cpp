#include "myShell.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

#include "command.h"
#include "parser.h"
MyShell::MyShell() : replStatus(true), builtinCommands() {
  builtinCommands.insert("cd");
  builtinCommands.insert("set");
  builtinCommands.insert("export");
  builtinCommands.insert("inc");
}

MyShell::~MyShell() {}

void MyShell::displayPrompt() {
  char * buf = NULL;
  buf = getcwd(buf, 0);
  std::cout << "myShell:" << buf << " $ ";
  free(buf);
}

std::string MyShell::getVar(std::string key) {
  std::map<std::string, std::string>::iterator it = varMap.find(key);
  if (it != varMap.end()) {
    return it->second;
  }
  return "";
}

void MyShell::executeCd(Command * cmd) {
  if (cmd->parameters[0] == "") {
    std::cout << "Please enter a directory name! " << std::endl;
  }
  else if (chdir(cmd->parameters[0].c_str()) != 0) {
    std::cout << "Can not open directory: " << cmd->parameters[0] << std::endl;
  }
}

void MyShell::executeSet(Command * cmd) {
  if (cmd->parameters.size() != 2) {
    std::cout << "Wrong Format for Set! " << std::endl;
    return;
  }
  std::cout << "-" << cmd->parameters[0] << "-" << std::endl;
  std::cout << "Length is " << cmd->parameters[0].length() << std::endl;
  for (size_t i = 0; i < cmd->parameters[0].length(); i++) {
    std::cout << cmd->parameters[0][i] << " ";
    if (!Parser::isValidVarCharacter(cmd->parameters[0][i])) {
      std::cout << "Set: Invalid Variable Name:  " << cmd->parameters[0] << std::endl;
      return;
    }
  }
  std::map<std::string, std::string>::iterator it = varMap.find(cmd->parameters[0]);
  if (it != varMap.end()) {
    it->second = cmd->parameters[1];
  }
  else {
    varMap.insert(std::pair<std::string, std::string>(cmd->parameters[0], cmd->parameters[1]));
  }
}

void MyShell::executeExport(Command * cmd) {
  if (cmd->parameters.size() != 1) {
    std::cout << "Wrong Format for Export! " << std::endl;
    return;
  }
  std::map<std::string, std::string>::iterator it = varMap.find(cmd->parameters[0]);
  if (it == varMap.end()) {
    // variable does not exist
    std::cout << "Can not find variable: " << cmd->parameters[0] << std::endl;
    return;
  }
  if (environVarMap.find(it->first) == environVarMap.end()) {
    // new exported var
    environVarMap.insert(std::pair<std::string, std::string>(it->first, it->second));
  }
  else {
    //update exported var
    environVarMap.find(it->first)->second = it->second;
  }
  setenv(it->first.c_str(), it->second.c_str(), 1);
}

void MyShell::executeInc(Command * cmd) {
  if (cmd->parameters.size() != 1) {
    std::cout << "Wrong Format for Inc! " << std::endl;
    return;
  }

  for (size_t i = 0; i < cmd->parameters[0].length(); i++) {
    if (!Parser::isValidVarCharacter(cmd->parameters[0][i])) {
      std::cout << "Inc: Invalid Variable Name:  " << cmd->parameters[0] << std::endl;
      return;
    }
  }
  // Now it is a valiad var name
  if (varMap.find(cmd->parameters[0]) == varMap.end()) {
    // Not found, new var
    varMap.insert(std::pair<std::string, std::string>(cmd->parameters[0], "1"));
  }
  else {
    // Found, update it
    std::map<std::string, std::string>::iterator it = varMap.find(cmd->parameters[0]);
    if (Parser::isBase10Number(it->second)) {
      it->second = Parser::addByOne(it->second);
    }
    else {
      it->second = "1";
    }
  }
}

void MyShell::executeBuiltinCommand(Command * cmd) {
  if (cmd->basename == "cd") {
    executeCd(cmd);
  }
  else if (cmd->basename == "set") {
    executeSet(cmd);
  }
  else if (cmd->basename == "export") {
    executeExport(cmd);
  }
  else {
    executeInc(cmd);
  }
}

void MyShell::executeExternalCommand(Command * cmd) {
  int waitpidStatus;
  pid_t pid = fork();
  if (pid > 0) {
    //Parent Process
    waitpid(pid, &waitpidStatus, 0);
    if (WIFSIGNALED(waitpidStatus)) {
      std::cout << "Program was killed by signal " << WTERMSIG(waitpidStatus) << std::endl;
    }
    else {
      std::cout << "Program exited with status " << WEXITSTATUS(waitpidStatus) << std::endl;
    }
  }
  else if (pid == 0) {
    // Child Process
    std::vector<char *> argv;
    for (std::vector<std::string>::iterator loop = cmd->parameters.begin();
         loop != cmd->parameters.end();
         ++loop) {
      argv.push_back(&(*loop)[0]);
    }
    argv.push_back(NULL);
    argv.insert(argv.begin(), const_cast<char *>(cmd->basename.c_str()));

    for (size_t i = 0; i < cmd->possibleAbsoluteFilename.size(); ++i) {
      if (execve(cmd->possibleAbsoluteFilename[i].c_str(), &argv[0], environ) == -1) {
        std::cout << "Can not execute " << cmd->possibleAbsoluteFilename[i] << std::endl;
      }
    }
    std::cout << "Execute Fail!" << std::endl;
    exit(EXIT_FAILURE);
  }
  else {
    // Fork Fail
    perror("Fork Fail!");
  }
}

void MyShell::executeCommand(Command * cmd) {
  std::cout << "Here is input: " << cmd << std::endl;
  if (cmd->isExitOrNothing) {
    replStatus = false;
    std::cout << "Meets exit or nothing!!";
    return;
  }

  if (!cmd->withSlash && builtinCommands.find(cmd->basename) != builtinCommands.end()) {
    executeBuiltinCommand(cmd);
  }
  else {
    executeExternalCommand(cmd);
  }
}

void MyShell::runRepl() {
  size_t sz = 0;
  const char * ECE551PATH = getenv("PATH");
  setenv("ECE551PATH", ECE551PATH, 1);
  while (replStatus) {
    ECE551PATH = getenv("PATH");
    char * commandFromStdin = NULL;
    displayPrompt();
    if (getline(&commandFromStdin, &sz, stdin) >= 0) {
      std::cout << "This is process " << getpid() << std::endl;
      char * nextlineIndex = strchr(commandFromStdin, '\n');
      if (nextlineIndex != NULL) {
        *nextlineIndex = '\0';
      }
      Parser parser = Parser(commandFromStdin, ECE551PATH);
      Command * cmd = parser.parse();
      executeCommand(cmd);
    }
    else {
      replStatus = false;
    }
    free(commandFromStdin);
  }
}

void initializeEnvironVarMap(char ** environ) {
  size_t i = 0;
  std::string name;
  std::string value;
  while (environ[i] != NULL) {
    int equalIndex = 0;
    while (environ[i][equalIndex] != '=') {
      name.push_back(environ[i][equalIndex]);
      ++equalIndex;
    }
    value = std::string(strstr(environ[i], "=")).substr(1);
    MyShell::environVarMap.insert(std::pair<std::string, std::string>(name, value));
    name.clear();
    i++;
  }
}

std::map<std::string, std::string> MyShell::varMap;
std::map<std::string, std::string> MyShell::environVarMap;
int main() {
  initializeEnvironVarMap(environ);
  MyShell::varMap.insert(std::pair<std::string, std::string>("AAA", "mp_miniproject"));
  MyShell::varMap.insert(std::pair<std::string, std::string>("AAAA", "ece551"));
  MyShell().runRepl();
}

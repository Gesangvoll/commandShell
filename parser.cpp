#include "parser.h"

#include <stdlib.h>

#include <algorithm>
#include <sstream>

#include "myShell.h"

Parser::Parser(char * commandFromStdin, std::string _envv) :
    cmd(new Command(commandFromStdin)),
    envv(_envv) {}

Parser::~Parser() {
  delete cmd;
}

Command * Parser::parse() {
  // Remove whitespaces at the front of commandFromStdin
  trimSpaces(cmd->commandFromStdin);
  std::string readyForProcessing = replaceVars(cmd->commandFromStdin);
  std::cout << "Ready for Processing is: " << readyForProcessing << std::endl;
  // Find the river between filename part and parameters part
  size_t border = findBorderBetweenFilenameAndParameters(readyForProcessing);
  // "raw" means not put into Command object yet
  std::string rawFilename;
  std::string rawParameters;
  if (border == std::string::npos) {
    rawFilename = readyForProcessing;
    rawParameters = "";
  }
  else {
    rawFilename = readyForProcessing.substr(0, border);
    rawParameters = readyForProcessing.substr(border);
  }

  // Shell supports '\ ' when input, however c/c++ does not, so
  // we need to remove the '\ ' in filename
  if (rawFilename.length() == 0 || rawFilename == "exit") {
    cmd->isExitOrNothing = true;
  }
  else {
    rawFilename.erase(remove(rawFilename.begin(), rawFilename.end(), '\\'), rawFilename.end());

    // Decide whether filename is relative or absolute path
    if (rawFilename.find('/') == std::string::npos) {
      cmd->withSlash = false;
      cmd->basename = rawFilename;
    }
    else {
      cmd->withSlash = true;
      cmd->basename = rawFilename.substr(rawFilename.find_last_of('/') + 1);
    }
    std::cout << "rawFilename is " << rawFilename << std::endl;
    setPossibleAbsoluteFilename(rawFilename);

    if (border != std::string::npos) {
      // Remove whitespaces at the front of parameters
      trimSpaces(rawParameters);
      // Format the parameters
      if (rawParameters.empty()) {
      }
      else {
        std::cout << " rawParameters is " << rawParameters << std::endl;
        formatParameters(rawParameters);
      }
    }
  }

  return cmd;
}

Command * Parser::getCommand() {
  return cmd;
}
bool Parser::isBase10Number(std::string str) {
  for (size_t i = 0; i < str.length(); i++) {
    if (i == 0 && str[i] == '-') {
      continue;
    }
    if (str[i] >= 48 && str[i] <= 57) {
    }
    else {
      return false;
    }
  }
  return true;
}

std::string Parser::addByOne(std::string str) {
  bool isNegative = false;
  int res = 0;
  std::ostringstream os;
  if (str[0] == '-') {
    isNegative = true;
  }
  if (isNegative) {
    for (size_t i = 1; i < str.length(); i++) {
      res = 10 * res + str[i] - '0';
    }
    res = -res;
  }
  else {
    for (size_t i = 0; i < str.length(); i++) {
      res = 10 * res + str[i] - '0';
    }
  }
  ++res;
  os << res;
  std::cout << "XSDDDDDDDDDDDDDDDDDd" << res << os.str() << std::endl;
  return os.str();
}

int Parser::findBorderBetweenFilenameAndParameters(std::string & readyForProcessing) {
  size_t index = 0;
  while (true) {
    index = readyForProcessing.find(" ", index);
    if (index == std::string::npos) {
      return index;
    }
    if (readyForProcessing[index - 1] == '\\') {
      ++index;
    }
    else {
      return index;
    }
  }
}

void Parser::trimSpaces(std::string & str) {
  size_t firstNotSpace = str.find_first_not_of(" ");
  if (firstNotSpace == std::string::npos) {
    str.clear();
  }
  else {
    str.erase(0, firstNotSpace);
    str.erase(str.find_last_not_of(" ") + 1);
    if (str[str.length() - 1] == '\\') {
      str.append(" ");
    }
  }
}

bool Parser::isValidVarCharacter(char & ch) {
  std::cout << "Now is " << int(ch) << std::endl;
  if (((ch >= 97) && (ch <= 122)) || ((ch >= 65) && (ch <= 90)) || ((ch >= 48) && (ch <= 57)) ||
      (ch == 95)) {
    return true;
  }
  return false;
}

std::string Parser::replaceVars(std::string commandFromStdin) {
  size_t len = commandFromStdin.length();
  size_t indexOfDollar = 0;
  std::string possibleVar = "";
  size_t varStartIndex;
  size_t varEndIndex;
  indexOfDollar = commandFromStdin.find('$', indexOfDollar);
  while (indexOfDollar != std::string::npos) {
    varStartIndex = indexOfDollar + 1;
    varEndIndex = varStartIndex;
    while (varEndIndex < len) {
      possibleVar =
          MyShell::getVar(commandFromStdin.substr(varStartIndex, varEndIndex - varStartIndex + 1));
      if (possibleVar != "") {
        // A variable is found, THE DUTY IS NOT FINISHED,
        // We shoud try to find the var with the longest name starting at varStartindex
        int maxValidVarEndIndex = varEndIndex;
        std::string valueOfVarWithMaxLen = possibleVar;
        while (varEndIndex < len) {
          ++varEndIndex;
          if (!isValidVarCharacter(commandFromStdin[varEndIndex])) {
            break;
          }
          possibleVar = MyShell::getVar(
              commandFromStdin.substr(varStartIndex, varEndIndex - varStartIndex + 1));
          if (possibleVar != "") {
            maxValidVarEndIndex = varEndIndex;
            valueOfVarWithMaxLen = possibleVar;
          }
        }
        commandFromStdin = commandFromStdin.replace(
            indexOfDollar, maxValidVarEndIndex - indexOfDollar + 1, valueOfVarWithMaxLen);
        len = commandFromStdin.length();
        indexOfDollar = commandFromStdin.find('$', indexOfDollar + 1);
        break;
      }
      else {
        // Expand the search range to try to find
        ++varEndIndex;
        if (!isValidVarCharacter(commandFromStdin[varEndIndex])) {
          indexOfDollar = commandFromStdin.find('$', indexOfDollar + 1);
          break;
        }
      }
    }
  }
  return commandFromStdin;
}

void Parser::formatParameters(std::string rawParameters) {
  size_t sliceLeftIndex = 0;
  size_t i;
  std::string curr;
  for (i = 0; i < rawParameters.length(); i++) {
    if (sliceLeftIndex == std::string::npos) {
      return;
    }
    if (i < sliceLeftIndex) {
      i = sliceLeftIndex;
    }
    if (rawParameters[i] == ' ') {
      if (rawParameters[i - 1] == '\\') {
        if (i == rawParameters.length() - 1) {
          curr = rawParameters.substr(sliceLeftIndex);
          curr.erase(remove(curr.begin(), curr.end(), '\\'), curr.end());
          cmd->parameters.push_back(curr);
          curr.clear();
        }
      }
      else {
        curr = rawParameters.substr(sliceLeftIndex, i - sliceLeftIndex);
        curr.erase(remove(curr.begin(), curr.end(), '\\'), curr.end());
        cmd->parameters.push_back(curr);
        curr.clear();
        sliceLeftIndex = rawParameters.find_first_not_of(' ', i);
      }
    }
    else if (i == rawParameters.length() - 1) {
      curr = rawParameters.substr(sliceLeftIndex);
      curr.erase(remove(curr.begin(), curr.end(), '\\'), curr.end());
      cmd->parameters.push_back(curr);
      curr.clear();
    }
  }
}

void Parser::setPossibleAbsoluteFilename(std::string rawFilename) {
  if (cmd->withSlash) {
    cmd->possibleAbsoluteFilename.push_back(rawFilename);
  }
  else {
    std::cout << "envv is " << envv << std::endl;
    char * curr;
    char * tempEnvv = const_cast<char *>(envv.c_str());
    for (curr = strsep(&tempEnvv, ":"); curr != NULL; curr = strsep(&tempEnvv, ":")) {
      cmd->possibleAbsoluteFilename.push_back(std::string(curr).append("/" + rawFilename));
    }
  }
}

// std::map<std::string, std::string> MyShell::varMap;
// int main() {
//   size_t sz = 0;
//   char * commandFromStdin;
//   char * envv = getenv("PATH");

//   MyShell::varMap.insert(std::pair<std::string, std::string>("AAA", "XYZ"));
//   MyShell::varMap.insert(std::pair<std::string, std::string>("BBB", "123"));
//   MyShell::varMap.insert(std::pair<std::string, std::string>("AAAA", "MNKJ"));
//   while (getline(&commandFromStdin, &sz, stdin) >= 0) {
//     char * nextlineIndex = strchr(commandFromStdin, '\n');
//     *nextlineIndex = '\0';
//     Parser parser(commandFromStdin, std::string(envv));
//     Command * cmd = parser.parse();
//     std::cout << "Filenames: " << std::endl;
//     for (size_t i = 0; i < cmd->possibleAbsoluteFilename.size(); i++) {
//       std::cout << cmd->possibleAbsoluteFilename[i] << std::endl;
//     }
//     std::cout << "Basename: " << std::endl;
//     std::cout << cmd->basename << std::endl;

//     std::cout << "Parameters: " << std::endl;
//     for (size_t i = 0; i < cmd->parameters.size(); i++) {
//       std::cout << cmd->parameters[i] << "-";
//     }
//   }
// }

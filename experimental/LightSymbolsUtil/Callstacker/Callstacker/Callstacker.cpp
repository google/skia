// Callstacker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
#include <map>
#include <vector>

using namespace std;

// can't delete, only add files repository!
class SkSourceDb {
public:
  SkSourceDb(const char* szBaseSrcPath, const char* szLightSymbolsDbFile) {
    this->baseSrcPath = szBaseSrcPath;
    this->lightSymbolsDbFile = szLightSymbolsDbFile;
    nextId = 1;
  }

  const string& getBaseSrcPath() const {
    return baseSrcPath;
  }

  string GetStoredFilename(const string& filename) {
    string base = filename.substr(0, baseSrcPath.length());
    if (base != baseSrcPath) {
      return "";
    }

    string relative = filename.substr(baseSrcPath.length());
    char tmp[10000];
    strcpy(tmp, relative.c_str()); // insecure
    char* sz = tmp;
    while (*sz) {
      if (*sz == '\\') *sz = '/';
      sz++;
    }
    sz = tmp;
    if (*sz == '/') sz++;

    return string(sz);
  }

  int obtainFileId(const string& filename) {
    string stored = GetStoredFilename(filename);
    if (stored.empty()) {
      return -1;
    }

    if (filenames.find(stored) == filenames.end()) {
      int id = nextId;
      nextId++;
      filenames[stored] = id;
      return id;
    } else {
      return filenames[stored];
    }
  }

  static void Load(char* szFileName, SkSourceDb** ret, const char* whereToSave, const char* szBaseSrcPath) {
    char szLine[10000];
    FILE* file = fopen(szFileName, "rt");
    if (file == NULL) {
      *ret = NULL;
      return;
    }

    const char* trimed;
    SkSourceDb* db = new SkSourceDb(szBaseSrcPath, whereToSave == NULL ? szFileName : whereToSave);

    map<int, string> ids;
    int id;
    while (true) {
      id = -1;
      if (fscanf(file, "%i", &id) == 0) break;
      if (id == -1) break;
      *szLine = '\0';
      fgets(szLine, 10000, file);
      trimed = trim(szLine);

      if (id < 0 || ids[id] != "") {
        printf("fatal error: duplicate value for id = %i, existing = \"%s\", new = \"%s\"\n", id, ids[id].c_str(), trimed);
        exit(-1);
      }

      if (trimed == NULL || *trimed == '\0') {
        printf("fatal error: no valuefor id = %i\n", id);
        exit(-1);
      }

      if (db->filenames.find(trimed) != db->filenames.end()) {
        printf("fatal error: duplicate id for same file: file = %s, existing id = %i, new id = %i\n", trimed, db->filenames[trimed], id);
//        exit(-1);
      }

      string value = trimed;
      ids[id] = value;
      db->filenames[value] = id;
      if (db->nextId <= id) {
        db->nextId = id + 1;
      }
    }

    *ret = db;
  }

  // dumb comit, smarter: use append
  void commit() {
    save(lightSymbolsDbFile.c_str());
  }

private:

  static const char* trim(char* sz) {
    if (sz == NULL) return NULL;

    while (*sz == ' ' || *sz == '\t' || *sz == '\r' || *sz == '\n' || *sz == ',')
      sz++;

    if (*sz == '\0') return sz;

    int len = strlen(sz);
    char* start = sz;
    sz = sz + (len - 1);

    while (sz >= start && (*sz == ' ' || *sz == '\t' || *sz == '\r' || *sz == '\n' || *sz == ',')) {
      *sz = '\0';
      sz--;
    }

    return start;
  }

  void save(const char* szFilename) {
    char szLine[10000];
    FILE* file = fopen(szFilename, "wt");

    map<string, int>::const_iterator itr;

    for(itr = filenames.begin(); itr != filenames.end(); ++itr){
      fprintf(file, "%i, %s\n", (*itr).second, (*itr).first.c_str());
    }
    fclose(file);
  }

  string baseSrcPath;
  string lightSymbolsDbFile;
  map<string, int> filenames;
  int nextId;
};

SkSourceDb* source_db = NULL;

bool endsWith(const char* who, const char* what) {
    int a = strlen(who);
    int b = strlen(what);
    return stricmp(who + a - b, what) == 0; // insecure
}

bool sourceFile(const char* szFileName) {
    return endsWith(szFileName, ".h") || endsWith(szFileName, ".c") || endsWith(szFileName, ".cpp") || endsWith(szFileName, ".cc");
}

// "
// //
// /*
class CppState {
public:

    CppState() : line(1), inComment(false), inLineComment(false), inDoubleQuote(false), inSingleQuote(false), isEscaping(false), commentEnding(false), commentMightBeStarting(false) {
    }

    void apply(int ch) {
        if (ch == '\n') {
            line++;
            if (inLineComment) inLineComment = false;
        }

        if (inLineComment) {
            return;
        }

        if (commentMightBeStarting) {
            commentMightBeStarting = false;
            if (ch == '*') {
                inComment = true;
            } else if (ch == '/') {
                inLineComment = true;
            } else {
                add('/');//previously we has / but was not pushed on tokens
                newToken();//
            }
        }

        if (inSingleQuote) {
            if (isEscaping)
                isEscaping = false;
            else if (ch == '\\')
                isEscaping = true;
            else if (ch == '\'') {
                inSingleQuote = false;
                newToken();
                pushToken("__SINGLE_QUOTE__");
                newToken();
            }

            return;
        } else if (inDoubleQuote) {
            if (isEscaping)
                isEscaping = false;
            else if (ch == '\\')
                isEscaping = true;
            else if (ch == '\"') {
                inDoubleQuote = false;
                newToken();
                pushToken("__DOUBLE_QUOTE__");
                newToken();
            }

            return;
        } else if (inComment) {
            if (ch == '*') {
                commentEnding = true;
            } else if (ch == '/') {
                inComment = false;
                commentEnding = false;
            } else {
                commentEnding = false;
            }

            return;
        }

        switch (ch) {
        case '\'':
            newToken();
            inSingleQuote = true;
            return;

        case '\"':
            newToken();
            inDoubleQuote = true;
            return;

        case '/':
            newToken();
            commentMightBeStarting = true;
            return;
        }

        if (isspace(ch)) {
            newToken();
        } else if (tokenDelimiter(ch)) {
            newToken();
            if (isSingleCharToken(ch)) {
                add(ch);
                newToken();
            }
        } else if (isTokenable(ch)) {
              add(ch);
        } else {
            printf("undexpected ... %c", (char)ch);
        }
    }

    bool enteredFunction() {
        if (inComment || inLineComment || inDoubleQuote || inSingleQuote || commentMightBeStarting) {
            return false;
        }

        if (tokens.size() == 0) {
            return false;
        }

        if (tokens[tokens.size() - 1] != "{") {
            return false;
        }

        int i = tokens.size() - 2;

        bool foundCloseBraket = false;
        int innerBrakets = 0;
        bool foundOpenBraket = false;
    int iName = -1;

        while (i >= 0) {
            string t_i = tokens[i]; // debugging sucks!

      if (!foundCloseBraket && (tokens[i] == "enum"
                             || tokens[i] == "struct"
                             || tokens[i] == "class"
                             || tokens[i] == "namespace"
                             || tokens[i] == "public"
                             || tokens[i] == "private"
                             || tokens[i] == "protected"
                             || tokens[i] == "__asm"
                             || tokens[i] == "catch"
                             || tokens[i] == "__except"
                             )) {
        return false;
      }

            if (tokens[i] == ")") {
                if (foundCloseBraket)
                    innerBrakets++;
                else if (i >= 3 && tokens[i - 1] == "." && tokens[i - 2] == "." && tokens[i - 3] == ".") {
                    i -= 3;
                }
                foundCloseBraket = true;
            } else if (tokens[i] == "(" && innerBrakets > 0) {
                innerBrakets--;
            } else if (tokens[i] == "(" && innerBrakets == 0) {
                foundOpenBraket = true;
                i--; if ( i < 0) return false;
                string name = tokens[i];
        iName = i;

                if (name == "if" || name == "while" || name == "switch"|| name == "for") {
                    return false;
                }

                if (!CouldBeFunctionName(name)) return false;
                if (i >= 6 && tokens[i - 1] == ":" && tokens[i - 2] == ":" && CouldBeClassnName(tokens[i - 3]) && tokens[i - 4] == ":" && tokens[i - 5] == ":" && CouldBeClassnName(tokens[i - 6])) {
                    name =  tokens[i - 6] + "::" + tokens[i - 3] + "::" + name;
          iName = i - 6;
                    if (i >= 7 && (tokens[i - 7] == ":" || tokens[i-7] == ",")) {
                        i -= 7 + 1;
                        name = "";
                        foundCloseBraket = false;
                        foundOpenBraket = false;
                        innerBrakets = 0;
                        continue;
                    }
                }
                else if (i >= 3 && tokens[i - 1] == ":" && tokens[i - 2] == ":" && CouldBeClassnName(tokens[i - 3])) {
                    name = tokens[i - 3] + "::" + name;
          iName = i - 3;
                    if (i >= 4 && (tokens[i - 4] == ":" || tokens[i-4] == ",")) {
                        i -= 4 + 1;
                        name = "";
                        foundCloseBraket = false;
                        foundOpenBraket = false;
                        innerBrakets = 0;
                        continue;
                    }
                }
                else if (i >= 1 && (tokens[i - 1] == ":" || tokens[i-1] == ",")) {
                    i -= 1 + 1;
                    name = "";
                    foundCloseBraket = false;
                    foundOpenBraket = false;
                    innerBrakets = 0;
                    continue;
                }

                if (name == "") {
                    return false;
                }

        if (iName >= 2 && tokens[iName - 2] == "#" && tokens[iName - 1] == "define") {
          return false;
        }

        if (iName >= 1 && (tokens[i - 1] == "enum"
                        || tokens[i - 1] == "struct"
                        || tokens[i - 1] == "class"
                        || tokens[i - 1] == "namespace"
                        || tokens[i - 1] == "public"
                        || tokens[i - 1] == "private"
                        || tokens[i - 1] == "protected"
                        || tokens[i - 1] == "__asm"
                        || tokens[i - 1] == "if"
                        || tokens[i - 1] == "while"
                        || tokens[i - 1] == "for"
                        || tokens[i - 1] == "switch"
                        || tokens[i - 1] == "!"
                        )) {
          return false;
        }

        int k = 10;
        i = iName - 2;
        bool isInline = false;// heuristic for inline functions
        while (k > 0 && i >= 0) {
          if (tokens[i] == "inline") {
            isInline = true;
            break;
          }
          if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}") {
            break;
          }
          i--;
          k--;
        }

        if (isInline) return false; //do not trace inline functions

                lastFunctionName = name;
                return true;
            } else {
                if (!foundCloseBraket) {
                    if (!IgnorableFunctionModifier(tokens[i])) {
                        return false;
                    }
                } else {
                    if (!IgnorableFunctionParameter(tokens[i])) {
                        return false;
                    }
                }
            }

            i--;
        }

        return false;
    }

    const char* functionName() {
        return lastFunctionName.c_str();
    }

    int lineNumber() {
        return line;
    }

private:

    bool CouldBeFunctionName(const string& str) {
        if (str.empty()) return false;
        if (!isalpha(str[0]) && str[0] != '_' && str[0] != '~' && str[0] != ':') return false;
        for (int i = 1; i < str.length(); i++) {
            if (!isalpha(str[i]) && !isdigit(str[i]) && str[i] != '_' && str[i] != ':') return false;
        }

        return true;
    }

    bool isNumber(const string& str) {
        if (str.empty()) return false;
        for (int i = 0; i < str.length(); i++) {
            if (!isdigit(str[i]) && str[i] != '.' && str[i] != 'x' && str[i] != 'X' && str[i] != 'e' && str[i] != 'E') return false;
        }

        return true;
    }


    bool isOperator(const string& str) {
        if (str.empty()) return false;
        for (int i = 1; i < str.length(); i++) {
            switch (str[i]) {
            case '<':
            case '>':
            case '=':
            case '+':
            case '-':
            case '*':
            case '/':
            case '(':
            case ')':
            case '[':
            case ']':
            case '!':
            case '|':
            case '&':
            case '^':
            case '%':
                break;
            default:
                return false;
            }
        }

        return true;
    }

    bool CouldBeClassnName(const string& str) {
        return CouldBeFunctionName(str);
    }

    bool IgnorableFunctionModifier(const string& str) {
        return str.empty() || CouldBeFunctionName(str);
    }

    bool IgnorableFunctionParameter(const string& str) {
        if (str.empty()) return true;
        if (CouldBeFunctionName(str)) return true;
        if (str == ",") return true;
        if (str == "*") return true;
        if (str == "=") return true;
        if (str == "&") return true;
        if (str == "<") return true;
        if (str == ">") return true;
        if (str == ":") return true;
        if (str == "=") return true;
        if (isNumber(str)) return true;
        if (str == "]") return true;
        if (str == "[") return true;

    if (str == ";") return false;

        return false;
    }


    bool tokenDelimiter(int ch) {
        if (isspace(ch))    return true;
        if (isdigit(ch))    return false;
        if (isalpha(ch))    return false;
        if (ch == '_')        return false;
                            return true;
    }

    bool isTokenable(int ch) {
        if (isdigit(ch))    return true;
        if (isalpha(ch))    return true;
        if (ch == '_')        return true;
                            return false;
    }

    bool isSingleCharToken(int ch) {
        if (isspace(ch))    return false;
        if (isdigit(ch))    return false;
        if (isalpha(ch))    return false;
        if (ch == '_')        return false;
                            return true;
    }

    void add(char ch) {
      token += ch;
    }

    void pushToken(const char* sz) {
        newToken();
        token = sz;
        newToken();
    }

    void newToken() {
        if (token.empty()) return;

        if (tokens.size() > 0) {
            string last = tokens[tokens.size() -1];
            if (last == "operator") {
                if (isOperator(op + token)) {
                    op += token;
                    token = "";
                    return;
                } else if (op != "" && isOperator(op)) {
                    tokens[tokens.size() -1] = last + op;
                    op = "";
                    return;
                } else if (isOperator(token)) {
                    op = token;
                    token = "";
                    return;
                } else {
                    // compile error?
                    op = "";
                }
            } else if (last == "~") {
                tokens[tokens.size() -1] = last + token;
                token = "";
                return;
            }
        }

        tokens.push_back(token);
        token = "";
    }

    int line;
    vector<string> tokens;
    string token;
    string lastFunctionName;

    bool inComment;
    bool inLineComment;
    bool inDoubleQuote;
    bool inSingleQuote;
    bool isEscaping;
    bool commentEnding;
    bool commentMightBeStarting;

    string op;
};

char output[100000000];
char* now;


void emit(char ch) {
  *now = ch;
  now++;
}

void emit(const char* szCode, const char* szFunctionName, int fileId, int line) {
    sprintf(now, szCode, szFunctionName, fileId, line);
    while (*now) {
        now++;
    }
}

void runFile(const char* szFileNameInput, const char* szFileNameOutput, const char* szInclude) {
  printf("%s\n", szFileNameInput);


  if (!sourceFile(szFileNameInput))
    return;

  now = output;
  int fileId = source_db->obtainFileId(szFileNameInput);
  FILE* file = fopen(szFileNameInput, "rt");
  int ch;
  CppState state;
  while (true) {
    int ch = getc(file);
    if (ch == -1)
        break;
    state.apply(ch);
    emit(ch);
    if (ch == '{' && state.enteredFunction()) { // {
      emit("LS_TRACE(\"%s\", %i, %i);", state.functionName(), fileId, state.lineNumber()); // light symbol traces, create a macro to define it
    }
  }
  fclose(file);

  file = fopen(szFileNameOutput, "wt");
  // TODO: input parameter
  fprintf(file, "#include \"%s\"\n", szInclude);
  fwrite(output, 1, now - output, file);
  fclose(file);
  //source_db->commit();
}

// to create the list file:
// dir *.cpp;*.h;*.cc /s /b
void runAll(char* szFileHolder, const char* szInclude) {
  FILE* file = fopen(szFileHolder, "rt");
  if (file == NULL) {
    return;
  }

  while (true) {
    char szFileName[10000] = "";
    fgets(szFileName, 10000, file);
      char* end = szFileName + strlen(szFileName) - 1;
      while (end > szFileName && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t')) {
        *end = 0;
        end--;
      }
    if (strlen(szFileName) == 0)
        break;

  runFile(szFileName, szFileName, szInclude);
  }
  fclose(file);
  source_db->commit();
}

int _tmain(int argc, char* argv[])
{
  // base path, include, list.txt, lightSymbolFile, [lightSymbolsOut]
  SkSourceDb::Load(argv[4], &source_db, argc == 5 ? argv[4] : argv[5], argv[1]);
  if (source_db == NULL) {
    source_db = new SkSourceDb(argv[1], argv[4]);
  }

  runAll(argv[3], argv[2]); // e.g. foo\\src\\lightsymbols\\lightsymbols.h");

  return 0;
}

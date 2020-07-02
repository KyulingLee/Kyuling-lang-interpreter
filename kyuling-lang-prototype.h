/*
 * 생성자: Kyuling Lee
 * 여러 소스코드 파일들이 공존하는데
 * 그 안에서 있는 전체 함수들의 프로토타입 헤더를 맡는다.
 */

//토큰 처리용 소스코드
//kyuling-lang-token.cpp
void initCharType();
Token nextToken();
void nextLine();
Token nextLineToken();
void fileOpen(char *fname);
TknKind getKind(const string& s);
bool isOpe2(char c1, char c2);
Token checkNextToken(const Token& tk, int kind2);
void setTokenPointer(char *p);
string kindToString(int kd);
string kindToString(const CodeSet& cd);
int getLineNo();

//심볼 테이블 처리용 소스코드
//kyuling-lang-table.cpp
int enter(SymTbl& tb, SymKind kind);
void setStartLocalTable();
bool isLocalName(const string& name, SymKind kind);
int searchName(const string& s, int mode);
vector<SymTbl>::iterator tablePointer(const CodeSet& cd);

//구문 분석용 소스코드
//kyuling-lang-parser.cpp
void init();
void convertToInternalCode(char *fname);
void convert();
void convertBlockSet();
void convertBlock();
void convertRest();
void optionSet();
void varDeclare();
void varNameCheck(const Token& tk);
void setName();
void setaryLength();
void functionDeclare(); 
void backPatch(int line, int n);
void setCode(int cd);
int setCode(int cd, int nbr);
void setCodeRest();
void setCodeEnd();
void setCodeEofLine();
void pushInterCode();
bool isLocalScope();

//프로그램을 위해 추가로 이용하는 소스코드
//kyuling-lang-misc.cpp
string doubleToString(double d);
string errorMessage(const string& a, const string& b);
void errorExit(Tobj a, Tobj b, Tobj c, Tobj d);

//실제 소스코드 분석하고 처리하기 위해 이용하는 소스코드
//kyuling-lang-code.cpp
void syntaxCheck();
void setStartPc(int n);
void execute();
void statement();
void block();
double getExpression(int kind1, int kind2);
void expression(int kind1, int kind2);
void expression();
void term(int n);
void factor();
int opOrder(TknKind kd);
void binaryExpr(TknKind op);
void postIfSet(bool& flg);
void functionCallSyntax(int fncNbr);
void functionCall(int fncNbr);
void functionExecute(int fncNbr);
void sysFunctionExecuteSyntax(TknKind kd);
void sysfunctionExecute(TknKind kd);
int getMemoryAddress(const CodeSet& cd);
int getTopAddress(const CodeSet& cd);
int endlineOfIf(int line);
void checkEofLine();
TknKind lookCode(int line);
CodeSet checkNextCode(const CodeSet& cd, int kind2);
CodeSet firstCode(int line);
CodeSet nextCode();
void checkdtType(const CodeSet& cd);
void setdtType(const CodeSet& cd, char typ);
int setLITERAL(double d);
int setLITERAL(const string& s);
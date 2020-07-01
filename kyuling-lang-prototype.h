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
Token nextLine_token();
void fileOpen(char *fname);
TokenKind get_kind(const string& s);
bool is_ope2(char c1, char c2);
Token check_nextToken(const Token& tk, int kind2);
void set_token_p(char *p);
string kind_to_s(int kd);
string kind_to_s(const CodeSet& cd);
int get_lineNo();

//심볼 테이블 처리용 소스코드
//kyuling-lang-table.cpp
int enter(SymTbl& tb, SymKind kind);
void set_startLtable();
bool is_localName(const string& name, SymKind kind);
int searchName(const string& s, int mode);
vector<SymTbl>::iterator tableP(const CodeSet& cd);

//구문 분석용 소스코드
//kyuling-lang-parser.cpp
void init();
void convert_to_internalCode(char *fname);
void convert();
void convert_block_set();
void convert_block();
void convert_rest();
void convert_rest();
void varDecl();
void var_namechk(const Token& tk);
void set_name();
void set_aryLen();
void fncDecl(); 
void backPatch(int line, int n);
void setCode(int cd);
int setCode(int cd, int nbr);
void setCode_rest();
void setCode_End();
void setCode_EofLine();
void push_intercode();
bool is_localScope();

//프로그램을 위해 추가로 이용하는 소스코드
//kyuling-lang-misc.cpp
string dbl_to_s(double d);
string error_message(const string& a, const string& b);
void error_exit(Tobj a, Tobj b, Tobj c, Tobj d);

//실제 소스코드 분석하고 처리하기 위해 이용하는 소스코드
//kyuling-lang-code.cpp
void syntaxCheck();
void set_startPc(int n);
void execute();
void statement();
void block();
double get_expression(int kind1, int kind2);
void expression(int kind1, int kind2);
void expression();
void term(int n);
void factor();
int opOrder(TknKind kd);
void binaryExpr(TknKind op);
void post_if_set(bool& flg);
void fncCall_syntax(int fncNbr);
void fncCall(int fncNbr);
void fncExec(int fncNbr);
void sysFncExec_syntax(TknKind kd);
void sysFncExec(TknKind kd);
int get_memAdrs(const CodeSet& cd);
int get_topAdrs(const CodeSet& cd);
int endline_of_If(int line);
void chk_EofLine();
TknKind lookCode(int line);
CodeSet chk_nextCode(const CodeSet& cd, int kind2);
CodeSet firstCode(int line);
CodeSet nextCode();
void chk_dtTyp(const CodeSet& cd);
void set_dtTyp(const CodeSet& cd, char typ);
int set_LITERAL(double d);
int set_LITERAL(const string& s);
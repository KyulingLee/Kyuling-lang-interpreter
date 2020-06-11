/*
 * 생성자: Kyuling Lee
 * 여러 소스코드 파일들이 공존하는데
 * 그 안에서 있는 전체 함수들의 프로토타입 헤더를 맡는다.
 */

//kyuling-lang-token
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

//kyuling-lang-parser




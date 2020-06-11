/*
 * 생성자: Kyuling Lee
 * 메인 헤더
 */

#include <iostream>
#include <string>
#include <vector>
#include <stack>

#include <cstdio>
#include <cstring>
#include <cstdlib>


using namespace std;

//토큰으로 만들 요소들에 대해서 enum으로 정리하였다.
enum TokenKind {                                                
    Lparen='(',
    Rparen=')',
    Lbracket='[',
    Rbracket=']',
    Plus='+',
    Minus='-',
    Multi='*',
    Divi='/',
    Mod='%', 
    Not='!',
    Ifsub='?', 
    Assign='=',
    IntDivi='\\',
    Comma=',',  
    DblQ='"',
    Func=150, 
    Var,   
    If, 
    Elif, 
    Else,  
    For, 
    To, 
    Step,  
    While,
    End,   
    Break, 
    Return, 
    Option, 
    Print, 
    Println, 
    Input, 
    Toint,
    Exit,   
    Equal, 
    NotEq,  
    Less,   
    LessEq, 
    Great,   
    GreatEq, 
    And, 
    Or,
    END_KeyList,
    Ident,  
    IntNum, 
    DblNum, 
    String,  
    Letter, 
    Doll, 
    Digit,
    Gvar,
    Lvar, 
    Fcall,  
    EofProg, 
    EofLine, 
    Others
};

//토큰을 관리하는 구조체
struct Token {                
  TokenKind kind;                                                               //토큰 종류
  string  text;                                                                 //토큰 문자열
  double  dblVal;                                                               //수치 상수로 변환할 때의 값
  Token() {  kind=Others; text=""; dblVal=0.0; }
  Token (TknKind k)           { kind=k; text=""; dblVal=0.0; }
  Token (TknKind k, double d) { kind=k; text=""; dblVal=d; }
  Token (TknKind k, const string& s) { kind=k; text=s; dblVal=0.0; }
  Token (TknKind k, const string& s, double d) { kind=k; text=s; dblVal=d; }
};


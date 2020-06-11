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

//define은 여기다가 일단 다 모았다.
//short int형 크기
#define SHORT_SIZE  sizeof(short int)                 
//short int형 포인터 변환
#define SHORT_POINT(p) (short int *)(p)            
//unsigned char형 포인터 변환
#define UCHAR_POINT(p) (unsigned char *)(p)   
//소스 한 줄 최대 크기
#define LINE_SIZE 255                          

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
  //토큰 종류           
  TokenKind kind;                                                               
  //토큰 문자열
  string  text;                                                              
  //수치 상수로 변환할 때의 값
  double  doubleValue;
    
  Token() 
  {  
    kind=Others; 
    text=""; 
    doubleValue=0.0; 
  }

  Token (TknKind k)           
  { 
    kind=k; 
    text="";
    doubleValue=0.0; 
  }

  Token (TknKind k, double d)
  { 
    kind=k;
    text=""; 
    doubleValue=d; 
  }

  Token (TknKind k, const string& s) 
  { 
    kind=k; 
    text=s; 
    doubleValue=0.0;
  }

  Token (TknKind k, const string& s, double d)
  { 
    kind=k; 
    text=s;
    doubleValue=d;
  }
};


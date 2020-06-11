/*
 * 생성자: Kyuling Lee
 * 토큰 처리를 진행하는 소스코드 파일
 */

#include "kyuling-lang.h"



//토큰 타입 -> 캐릭터 문자 갯수에 맞춰서 토큰 타입들을 미리 만들어준다.
//이것이 나중에 비교 구분으로 이용될려나...?
TokenKind chartype[256]; 

//캐릭터 타입을 지정하고 초기화 하는 함수
//일단 아스키 기반으로 만들었다.
//사실 그냥 시스템 기반 언어를 써도 되겠지만 이걸 이용해보고 싶었다.
void initCharType()
{                  
    int i;
    for (i=0; i<256; i++)
    {
        chartype[i] = Others; 
    }
    for (i='0'; i<='9'; i++) 
    {
        chartype[i] = Digit;
    }
    for (i='A'; i<='Z'; i++) 
    {
        chartype[i] = Letter; 
    }
    for (i='a'; i<='z'; i++) 
    { 
        chartype[i] = Letter; 
    }

    chartype['_']  = Letter;  
    chartype['$']  = Doll;
    chartype['(']  = Lparen;  
    chartype[')']  = Rparen;
    chartype['[']  = Lbracket; 
    chartype[']']  = Rbracket;
    chartype['<']  = Less;    
    chartype['>']  = Great;
    chartype['+']  = Plus;   
    chartype['-']  = Minus;
    chartype['*']  = Multi;  
    chartype['/']  = Divi;
    chartype['!']  = Not;    
    chartype['%']  = Mod;
    chartype['?']  = Ifsub;  
    chartype['=']  = Assign;
    chartype['\\'] = IntDivi; 
    chartype[',']  = Comma;
    chartype['\"'] = DblQ;
}

#define CH (*token_p)
#define C2 (*(token_p+1))
#define NEXT_CH()  ++token_p

//실제로 토큰을 추출하는 함수.
//kyuling: 일단 기조 함수 구조만 만들어봤다.
//실제로는 아직 연결되지 않음.
Token nextToken() 
{
    TokenKind kind;
    string txt = "";

    //파일의 마지막일 때  
    if (endOfFile_F) 
        return Token(EofProg);
    
    //공백 문자를 스킵한다.                    
    while (isspace(CH)) 
        NEXT_CH();                             
    
    //라인의 마지막일 때
    if (CH == '\0')  
        return Token(EofLine);                          

    //캐릭터 타입마다 비교 처리를 할 수 있다.
    //위에서 데이터를 만들어서 이용하는 것을 해놨기 때문에 가능한 것이다.
    switch (chartype[CH]) {
        //식별자 추출
        case Doll: 
        case Letter:
            txt += CH; 
            NEXT_CH();
            while (chartype[CH]==Letter || chartype[CH]==Digit) 
            { 
                txt += CH; 
                NEXT_CH(); 
            }
            break;
        //숫자 상수 추출
        case Digit:                                                     
            kind = IntNum;
            while (chartype[CH] == Digit)   
            { 
                txt += CH; NEXT_CH(); 
            }
            if (CH == '.') 
            { 
                kind = DblNum; 
                txt += CH; 
                NEXT_CH(); 
            }
            while (chartype[CH] == Digit)   
            { 
                txt += CH; 
                NEXT_CH(); 
            }
            return Token(kind, txt, atof(txt.c_str()));       
        //문자열 리터럴 추출
        case DblQ:                                                   
        NEXT_CH();
        while (CH!='\0' && CH!='"') { txt += CH; NEXT_CH(); }
        if (CH == '"') NEXT_CH(); else err_exit("");
        return Token(String, txt);
        //연산자 등의 기본 기호 추출
        default:
        if (CH=='/' && C2=='/') return Token(EofLine);               
        if (is_ope2(CH, C2)) { txt += CH; txt += C2; NEXT_CH(); NEXT_CH(); }
        else                 { txt += CH; NEXT_CH(); }
    }
    kind = get_kind(txt);                                            

    if (kind == Others) err_exit(": ", txt);
    return Token(kind, txt);
}

//문장의 다음 라인의 토큰을 받도록 처리한다.
Token nextLine_tkn() 
{
  nextLine();
  return nextTkn();
}
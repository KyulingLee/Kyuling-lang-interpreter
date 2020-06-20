/*
 * 생성자: Kyuling Lee
 * 토큰 처리를 진행하는 소스코드 파일
 */

#include "kyuling-lang.h"
#include "kyuling-lang-prototype.h"

//최대 프로그램 행 수
#define MAX_LINE 2000                                                 

//소스 행 번호
int sourceLineno;                                       
//문자 하나 획득하기 위해 이용하는 문자 포인터                               
char *token_p;                              
//파일 종료인지 아닌지 확인하는 플래그
bool endOfFile_Flag;
//소스코드 읽어들일 곳                               
char buf[LIN_SIZE+5]; 
//입력 스트림                      
ifstream fin;                           

//문자 종류표 배열 -> 선언
TokenKind chartype[256]; 

//언어에서 쓰이는 어휘 종류에 대해서 대응하는 구조체
struct KeyWord {     
    //해당하는 키워드 포인터                            
    const char *keyName;
    //대응하는 값                                     
    TokenKind keyKind;                
};

//키워드 테이블
//프로그램 언어에서 이용하는 예약어, 심볼, 종별에 대해서 대응하는 테이블
KeyWord KeyWordTable[] = {                         
    {"func"   , Func  }, 
    {"var"    , Var    },
    {"if"     , If    },
    {"elif"   , Elif   },
    {"else"   , Else  }, 
    {"for"    , For    },
    {"to"     , To    },
    {"step"   , Step   },
    {"while"  , While }, 
    {"end"    , End    },
    {"break"  , Break }, 
    {"return" , Return },
    {"print"  , Print },
    {"println", Println},
    {"option" , Option},
    {"input"  , Input  },
    {"toint"  , Toint },
    {"exit"   , Exit   },
    {"("  , Lparen    }, 
    {")"  , Rparen   },
    {"["  , Lbracket  },
    {"]"  , Rbracket },
    {"+"  , Plus      },
    {"-"  , Minus    },
    {"*"  , Multi     }, 
    {"/"  , Divi     },
    {"==" , Equal     },
    {"!=" , NotEq    },
    {"<"  , Less      }, 
    {"<=" , LessEq   },
    {">"  , Great     },
    {">=" , GreatEq  },
    {"&&" , And       }, 
    {"||" , Or       },
    {"!"  , Not       },
    {"%"  , Mod      },
    {"?"  , Ifsub     },
    {"="  , Assign   },
    {"\\" , IntDivi   },
    {","  , Comma    },
    {"\"" , DblQ      },
    {"@dummy", END_KeyList},
};

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

//어휘 분석 오퍼레이션을 위해 정의한 define들...
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

            while (CH!='\0' && CH!='"') 
            { 
                txt += CH; 
                NEXT_CH(); 
            }
            
            if (CH == '"') 
                NEXT_CH(); 
            else 
                error_exit("");
            
            return Token(String, txt);
        //연산자 등의 기본 기호 추출
        default:
        //주석 처리
            if (CH=='/' && C2=='/') 
                return Token(EofLine);               
            
            if (is_ope2(CH, C2)) 
            { 
                txt += CH; 
                txt += C2; 
                NEXT_CH(); 
                NEXT_CH(); 
            }
            
            else 
            { 
                txt += CH;
                NEXT_CH(); 
            }
    }
    kind = get_kind(txt);                                            

    //전혀 다른 토큰이 처리되었을 때
    if (kind == Others) 
        error_exit("잘못된 토큰입니다. 토큰: ", txt);
    
    return Token(kind, txt);
}

//다음 한 라인을 가져오는 함수
void nextLine() 
{
    string s;

    if (endOfFile_F) 
        return;
    
    //한 행을 읽어낸다
    fin.getline(buf, LIN_SIZE+5);                                
    
    //파일의 끝일 경우
    if (fin.eof()) 
    {         
        //재 오픈에 대비해서 클리어를 해둔다.                                  
        fin.clear(); 
        fin.close();  
        endOfFile_Flag = true; 
        return;
    }

    if (strlen(buf) > LIN_SIZE)
    {
        cout << "프로그램은 1줄에 " << LIN_SIZE << "문자 이내로 작성해 주세요." <<endl;
        error_exit("프로그램은 1줄에 ", LIN_SIZE, " 문자 이내로 작성해 주세요.");
    }

    if (++srcLineno > MAX_LINE)
    {
        cout << "프로그램이 " << MAX_LINE << "을 넘었습니다." <<endl;
        error_exit("프로그램이 ", MAX_LINE, " 을 넘었습니다.");
    }

    //토큰 분석용 포인터를 buf의 맨 앞에 위치시킨다.
    token_p = buf;               
}

//문장의 다음 라인의 토큰을 받도록 처리한다.
Token nextLine_token() 
{
  nextLine();
  return nextToken();
}

//소스코드 파일을 여는 함수
void fileOpen(char *fname) 
{
    sourceLineno = 0;
    endOfFile_Flag = false;
    fin.open(fname);

    if (!fin) 
    { 
        cout << fname << " 을(를) 열 수 없습니다.\n"; 
        exit(1); 
    }
}

//토큰 종류별로 설정할 것이 있으면 설정한다.
TokenKind get_kind(const string& s) 
{
    for (int i=0; KeyWordTable[i].keyKind != END_KeyList; i++) 
    {
        if (s == KeyWordTable[i].keyName)
        {
            return KeyWordTable[i].keyKind;
        }
    }

    if (ctyp[s[0]]==Letter || ctyp[s[0]]==Doll) 
        return Ident;
    
    if (ctyp[s[0]] == Digit)  
        return DblNum;
    
    //마지막에 안걸린 것은 아예 없는 것. -> 다른 곳에서 오류 처리를 하도록 설정되어있음.
    return Others; 
}

//2 문자에 대해서 연산자일 경우 true 처리를 진행
bool is_ope2(char c1, char c2)
{
    char s[] = "    ";
    if (c1=='\0' || c2=='\0') 
        return false;

    s[1] = c1; s[2] = c2;
    return strstr(" ++ -- <= >= == != && || ", s) != NULL;
}

//획인하는 쪽의 토큰을 획들해서 넘긴다
Token check_nextToken(const Token& tk, int kind2)
{
    if (tk.kind != kind2) 
    {
        cout << "에러: " << tk.text << ", " << kind_to_s(kind2) << endl;
        error_exit(error_message(tk.text, kind_to_s(kind2)));
    }
    return nextToken();
}

//토큰 포인터를 지정하는 함수
void set_token_p(char *p) 
{
    token_p = p;
}

//키워드 종류별 -> 문자별로 바꾸는 함수
//키워드 테이블을 통해서 처리하는 방법
string kind_to_s(int kd)
{
    for (int i=0; ; i++) {
        if (KeyWordTable[i].keyKind == END_KeyList) 
            break;
        if (KeyWordTable[i].keyKind == kd) 
            return KeyWordTable[i].keyName;
        }
    return "";
}

//키워드 종류별 -> 문자별로 바꾸는 함수
//코드셋을 통해서 처리하는 함수
string kind_to_s(const CodeSet& cd) 
{
    switch (cd.kind) {
        case Lvar: 
        case Gvar: 
        case Fcall: 
            return tableP(cd)->name;
        
        case IntNum: 
        case DblNum: 
            return dbl_to_s(cd.doubleValue);
        
        case String: 
            return string("\"") + cd.text + "\"";
        
        case EofLine: 
            return "";
    }

    return kind_to_s(cd.kind);
}

//읽기 또는 실행중에 라인 번호 확인
int get_lineNo() 
{
    extern int Pc;

    //분석중일 경우 리턴 -> 실행중인 것
    return (Pc == -1) ? sourceLineno : Pc;        
}

/*
 * 생성자: Kyuling Lee
 * 구문 분석(파서) 기능만을 모아놓은 소스코드
 * 분야별 소스코드를 만들면서 이 작업을 진행하는 제일 첫 파일.
 */

#include "kyuling-lang.h"
#include "kyuling-lang-prototype.h"

//아직 주소가 미정일 경우에 이용하는 define
#define NO_FIX_ADDRESS 0    
//현재 처리중인 토큰
Token token;           
//임시 저장으로 이용하는 심볼 테이블
SymbolTable tmpTb;         

//블록의 깊이
int blockNest;    

//로컬 변수 주소 - 관리용
int localAddress;                       

//메인 함수가 있을 경우, 해당되는 심볼 테이블의 위치
int mainTableNumber;                 

//루프 중첩을 확인
int loopNest;               

//함수 정의 처리중인지 아닌지 확인하는 플래그
bool fncDecl_Flag;                              

//변수 선언의 경계를 확인하는 플래그
bool explicit_Flag;                               

//코내부 코드 생성작업을 위해 이용하는 버퍼
char codebuf[LIN_SIZ+1], *codebuf_p;             

//변환을 마친 내부 코드를 벡터로 저장
extern vector<char*> intercode;    

//초기화 작업
void init() 
{
    //문자 종류표를 초기화한다
    initChTyp();                                                   
    mainTableNumber = -1;
    blockNest = loopNest = 0;
    fncDecl_Flag = explicit_Flag = false;
    codebuf_p = codebuf;
}

//내부 코드로 변환한다
void convertToInternalCode(char *fname)
{
    //문자 종류표 등에 대해서 초기화를 진행한다.
    init();                                     

    //함수 정의 이름만을 일단 등록을 한다.
    fileOpen(fname);    
    while (token=nextLine_tkn(), token.kind != EofProg) 
    {
        if (token.kind == Func) 
        {
    
            token = nextTkn(); 
            setName(); 
            enter(tmpTb, fncId);
        }
    }

    //내부 코드로 변환을 한다.
    //변환할 때, 0행째는 그냥 매운다
    pushInterCode();      
    fileOpen(fname);
    token = nextLine_tkn();
    
    //문자열을 계속 내부코드로 변환한다
    while (token.kind != EofProg) 
    {
        convert();         
    }

    //메인 함수가 있으면 메인 함수 호출하는 코드를 설정한다.
    //1행부터 시작하도록 설정
    set_startPc(1);                                     
    if (mainTableNumber != -1) 
    {
        //메인에서부터 실행 시작
        set_startPc(intercode.size());  
        setCode(Fcall, mainTableNumber); 
        setCode('('); 
        setCode(')');
        pushInterCode();
    }
}

//앞단에 출현하는 코드를 변환해서 처리한다.
//그 위에에 나머지 변환이 필요한 것은 convertRest() 함수라고 따로 만들었다.
//앞단 처리는 다른 곳과 문 처리가 달라서 따로 함수를 뺐다.
void convert()
{
    //토큰 종류에 따라 처리할 수 있도록 하는 스위치
    switch (token.kind) 
    {
        //옵션 설정
        case Option:
            optionSet(); 
            break;                               
        //변수 설정
        case Var:   
            varDeclare();   
            break;                              
        //함수 정의 설정
        case Func:   
            functionDeclare();  
            break;                              
        //반복문
        case While: 
        case For:
            ++loopNest;
            convertBlockSet(); 
            setCodeEnd();
            --loopNest;
            break;
        //제어문 -> 이건 좀 제어문을 처리하는 과정을 제대로 짜야 한다.
        case If:
            //if 처리
            convertBlockSet();                                
            
            //elif 처리
            while (token.kind == Elif) 
            { 
                convertBlockSet(); 
            } 
            
            //else 처리
            if (token.kind == Else)    
            { 
                convertBlockSet(); 
            }

            //end 처리
            setCodeEnd();                                      
            break;
        case Break:
            if (loopNest <= 0) 
            {
                cout<< "잘못된 break입니다." << endl;
                errorExit("잘못된 break입니다.");
            }
            setCode(token.kind); 
            token = nextTkn(); 
            convertRest();
            break;
        case Return:
            if (!fncDecl_Flag) 
            {
                cout << "잘못된 return입니다." << endl;
                errorExit("잘못된 return입니다.");
            }
            setCode(token.kind); 
            token = nextTkn(); 
            convertRest();
            break;
        case Exit:
            setCode(token.kind); 
            token = nextTkn(); 
            convertRest();
            break;
        case Print: 
        case Println:
            setCode(token.kind); 
            token = nextTkn(); 
            convertRest();
            break;
        case End:
            cout << "잘못된 end입니다." << endl;
            errorExit("잘못된 end입니다.");    
            break;
        default: 
            convertRest(); 
            break;
    }
}

//불록 처리를 설정하고 관리하는 함수
void convertBlockSet()
{
    int patch_line;
    patch_line = setCode(token.kind, S);
    token = nextTkn();
    convertRest();

    //직접 블록을 처리하는 부분
    convertBlock();           
    //NO_FIX_ADDRESS를 수정한다.
    //이건 end행 번호로 바꿈
    backPatch(patch_line, get_lineNo());      
}

//블록을 직접 처리하는 함수
void convertBlock() 
{
    TokenKind k;

    //블록 끝까지 문을 분석한다.
    ++blockNest;                                    
    while(k=token.kind, k!=Elif && k!=Else && k!=End && k!=EofProg) {
        convert();
    }
    --blockNest;
}

//코드 문의 나머지를 처리하는 함수
void convertRest() 
{
    int tblNbr;

    for (;;) {
      if (token.kind == EofLine)
      break;

      switch (token.kind) 
      {      
        //이 아래에 있는 녀석들은 처리문에서 나타날 리 없다.
        //그러므로 이것이 도중에 발견되면 잘못된 코드 처리라고 해서 오류 처리한다.
        case If: 
        case Elif: 
        case Else: 
        case For: 
        case While: 
        case Break:
        case Func:  
        case Return:  
        case Exit:  
        case Print:  
        case Println:
        case Option: 
        case Var: 
        case End:
          errorExit("잘못된 코드입니다. : ", token.text);
          cout << "잘못된 코드입니다. : " << token.text << endl;
          break;

        //함수 호출과 그 변수를 확인
        case Ident:     
          setName();
          
          //함수 등록이 있을 경우
          if ((tblNbr=searchName(tmpTb.name, 'F')) != -1) 
          { 
            //main 함수 호출할 경우
            if (tmpTb.name == "main") 
            {
              errorExit("main 함수는 호출할 수 없습니다.");
              cout << "main 함수는 호출할 수 없습니다." << endl;
            }
            setCode(Fcall, tblNbr); 
            continue;
          }
          
          //변수 등록 없을 경우
          if ((tblNbr=searchName(tmpTb.name, 'V')) == -1) 
          {    
            //변수 선언이 필요한 경우
            if (explicit_Flag) 
            {
              errorExit("변수 선언이 필요합니다. : ", tmpTb.name);
              cout << "변수 선언이 필요합니다. " << tmpTb.name << endl;
            }

            //변수는 그냥 자동으로 등록한다.
            tblNbr = enter(tmpTb, varId);                      
          }
          
          if (is_localName(tmpTb.name, varId)) 
          {
            setCode(Lvar, tblNbr);
          }
          else                                 
          {
            setCode(Gvar, tblNbr);
          }

          continue;

        //int여도 저장할 때의 기본은 double로 한다.
        case IntNum: 
        case DblNum:                        
          setCode(token.kind, set_LITERAL(token.dblVal));
          break;

        case String:
          setCode(token.kind, set_LITERAL(token.text));
          break;

        //연산기호 같은 녀석들은 여기서 처리... +, - 같은 것들.
        default:               
          setCode(token.kind);
          break;
      }
      token = nextTkn();
    }
    pushInterCode();
    token = nextLine_tkn();
}

//옵션 설정
void optionSet() 
{
  //코드 변환에서 옵션으로 처리
  setCode(Option);             
  //이 이후에는 그냥 원래대로 저장을 한다.
  setCodeRest();                         
  //변수 선얼을 강제로 처리한다.
  //나중에 처리 단계중에 옵션인지 아닌지를 확인하여 한번에 할 수 있다.
  token = nextTkn();            
  if (token.kind==String && token.text=="var") 
  {
    explicit_Flag = true;
  }
  else 
  {
    errorExit("option 지정이 올바르지 않습니다.");
    cout << "option 지정이 올바르지 않습니다." << endl;
  }

  token = nextTkn();
  setCodeEofLine();
}

//var를 사용하는 변수가 선언됨
void varDeclare()
{
  //코드 반환을 var 형태로 설정
  setCode(Var);                
  //이후에 나머지는 걍 원래대로 저장한다.
  setCodeRest();              
  
  //무한반복으로 선언 처리를 검사
  for (;;) 
  {
    token = nextTkn();
    //이름 검사 및 설정
    varNameCheck(token); 
    setName();
    
    //배열이면 길이 지정
    setArrayLength();       
    //변수랑 주소 등록
    enter(tmpTb, varId);

    //선언 끝나면 종료
    if (token.kind != ',') 
      break; 
  }
  setCodeEofLine();
}

//변수 이름 검사
void varNameCheck(const Token& tk)
{
  if (tk.kind != Ident) 
  {
    cout << tk.text << " 식별자" << endl;
    errorExit(errorMessage(tk.text, " 식별자"));
  }

  if (isLocalScope() && tk.text[0] == '$')
  {
    cout << "함수내 var 선언에서는 $가 붙은 이름을 지정할 수 없습니다. : " << tk.text << endl;
    errorExit("함수내 var 선언에서는 $가 붙은 이름을 지정할 수 없습니다. : ", tk.text);
  }  
  if (searchName(tk.text, 'V') != -1)
  {
    cout << "식별자가 중복되었습니다. : " << tk.text << endl;
    errorExit("식별자가 중복되었습니다. : ", tk.text);
  }
}

//변수 이름 설정
void setName()
{
  if (token.kind != Ident)
  { 
    cout << "식별자가 필요합니다. : " << token.text << endl;
    errorExit("식별자가 필요합니다. : ", token.text);
  }

  tmpTb.clear(); 
  //실제로 이름 설정
  tmpTb.name = token.text;                       
  token = nextTkn();
}

//배열 길이를 확인하고 지정
void setArrayLength()
{
  tmpTb.aryLen = 0;
  
  //배열인지 아닌지 확인
  if (token.kind != '[') 
    return;                           

  token = nextTkn();
  
  if (token.kind != IntNum)
  { 
    cout << "배열의 길이는 양의 정수(+)로 지정해 주세요. : " << token.text << endl;
     errorExit("배열의 길이는 양의 정수(+)로 지정해 주세요. : ", token.text);
  }

  //첨자 0부터 시작이므로 마지막 인덱스 +1 처리를 해줘야 함.
  //예를 들어 a[5]이면 0~4가 실값, 나머지 +1 해서 배열의 마지막을 정의해야 함.
  tmpTb.aryLen = (int)token.dblVal + 1;  
  token = chk_nextTkn(nextTkn(), ']');
  if (token.kind == '[') 
  {
    cout << "다차원 배열은 선언할 수 없습니다. 언어 형식을 확인하세요." << token.text << endl;
    errorExit("다차원 배열은 선언할 수 없습니다. 언어 형식을 확인하세요.");
  }
}

//함수를 정의한다
void functionDeclare() 
{
  //글로벌 심볼 테이블
  extern vector<SymbolTable> Gtable;   
  int tblNbr, patch_line, fncTblNbr;

  //함수 정의 위치 오류
  if(blockNest > 0)
  {
    cout << "함수 정의 위치가 바르지 않습니다." << endl;
     errorExit("함수 정의 위치가 바르지 않습니다.");
  }

  //함수 처리를 시작하는 플래그 설정
  fncDecl_Flag = true;                 
  //로컬 영역의 할당 주소 카운터 초기화
  localAddress = 0;                    
  //로컬 시작 테이블의 시작 위치를 설정함.
  set_startLtable();                      
  //이후에 end행 번호를 넣고 처리하는 녀석
  patch_line = setCode(Func, S);     
  token = nextTkn();

  //함수명을 맨 처음에 등록한다.
  fncTblNbr = searchName(token.text, 'F');
  //함수명은 double로 고정한다.
  Gtable[fncTblNbr].dtTyp = DBL_T; 

  //가인수 분석을 시작함.
  token = nextTkn();
  //'('일 것
  token = chk_nextTkn(token, '(');         
  setCode('(');
  
  //안에 인수 있을 경우
  if (token.kind != ')') 
  {                    
    for (;; token=nextTkn()) 
    {
      setName();
      //인수 등록
      tblNbr = enter(tmpTb, paraId); 
      //Lvar로 인수 처리
      setCode(Lvar, tblNbr);
      //인수 개수 증가
      ++Gtable[fncTblNbr].args;   
      
      //인수 선언이 종료될 경우
      if (token.kind != ',') 
        break;                 
      
      setCode(',');
    }
  }

  //')'일 것
  token = chk_nextTkn(token, ')');  
  setCode(')'); 
  setCodeEofLine();
  //함수 본체를 처리해서 블록으로 만든다.
  convertBlock();                  

  //NO_FIX_ADDRESS 처리
  backPatch(patch_line, get_lineNo());   
  setCodeEnd();
  //프레임 크기 설정
  Gtable[fncTblNbr].frame = localAddress;      

  //main 함수에 대한 처리를 진행
  if (Gtable[fncTblNbr].name == "main") 
  { 
    mainTableNumber = fncTblNbr;
    
    if (Gtable[mainTableNumber].args != 0)
    {
      cout << "main 함수에서는 가인수를 지정할 수 없습니다." << endl;
      errorExit("main 함수에서는 가인수를 지정할 수 없습니다.");
    }
  }

  //함수 처리를 종료함
  fncDecl_Flag = false;                                          
}

//라인 행에 라인 번호 n을 지정함
void backPatch(int line, int n) 
{
  *SHORT_P(intercode[line] + 1) = (short)n;
}

//코드와 코드의 short값을 저장함
//여기선 코드만 저장
void setCode(int cd) 
{
  *codebuf_p++ = (char)cd;
}

//여기선 코드와 short값 둘 다 저장
int setCode(int cd, int nbr) 
{
  *codebuf_p++ = (char)cd;
  *SHORT_P(codebuf_p) = (short)nbr; 
  codebuf_p += SHORT_SIZ;

  //backPatch용으로 저장행을 반환함
  return get_lineNo();             
}

//나머지 텍스트에 대해서 그대로 저장한다
void setCodeRest() 
{
  extern char *token_p;
  strcpy(codebuf_p, token_p);
  codebuf_p += strlen(token_p) + 1;
}

//end의 저장을 처리함
void setCodeEnd() 
{
  if (token.kind != End)
  {
    cout << token.text << " end" << endl;
     errorExit(errorMessage(token.text, "end"));
  }
  setCode(End); 
  token = nextTkn();
  setCodeEofLine();
}

//파일의 최종 라인을 처리
void setCodeEofLine() 
{
  if (token.kind != EofLine) 
  {
    cout << "잘못된 기술입니다. : " << token.text << endl;
    errorExit("잘못된 기술입니다. : ", token.text);
  }

  //변환한 내부 코드를 저장한다
  pushInterCode();
  //다음 행으로 진행한다.
  token = nextLine_tkn();  
}

//변환한 내부 코드를 저장한다.
void pushInterCode() 
{
  int len;
  char *p;

  *codebuf_p++ = '\0';
  if ((len = codebuf_p-codebuf) >= LIN_SIZ)
  {
    cout << "변환 후의 내부 코드가 너무 깁니다. 식을 줄여주세요." << endl;
    errorExit("변환 후의 내부 코드가 너무 깁니다. 식을 줄여주세요.");
  } 

  //메모리를 직접 인터프리터가 건드리는 곳이기 때문에 에러 처리를 해주는 형태로 코드를 작성
  try 
  {
    //메모리를 확보한다
    p = new char[len];    
    memcpy(p, codebuf, len);
    intercode.push_back(p);
  }
  catch (bad_alloc) 
  {
    cout << "메모리를 확보할 수 없습니다." << endl;
    errorExit("메모리를 확보할 수 없습니다."); 
  }

  //다음 처리를 대비해서 저장할 곳의 맨 앞으로 위치를 지정한다.
  codebuf_p = codebuf;        
}

//함수내에서 처리중일 경우에 true 처리를 한다.
bool isLocalScope() 
{
  return fncDecl_Flag;
}


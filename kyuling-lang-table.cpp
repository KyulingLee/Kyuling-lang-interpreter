/*
 * 생성자: Kyuling Lee
 * 구문 분석 이전에 심볼 테이블을 별도로 만들어서 처리해야 하는
 * 녀석이 필요하다는 걸 알아서 도중에 추가함.
 * 식 표현, 역홀란트 표기법으로 분석, 변환, 평가까지 하고
 * 구문 분석을 위한 재귀 루틴, 트리를 위해서는 필요한 녀석이다.
 */


#include "kyuling-lang.h"
#include "kyuling-lang-prototype.h"

//심볼 테이블
//글로벌용 테이블
vector<SymTbl> GlobalTable;           
//로컬용 테이블
vector<SymTbl> LocalTable;           
//로컬 테이블 시작 위치
int startLocalTable;           		

//심볼 테이블에 등록하는 함수
int enter(SymTbl& tb, SymKind kind) 
{
    int n, mem_size;
    bool isLocal = isLocalName(tb.name, kind);
    //로컬 변수의 주소 관리하는 녀석
    extern int localAdrs;                              
    //가상머신의 메모리
    extern Mymemory Dmem;                                          

    ////확인하는 부분
    mem_size = tb.aryLen;
    //단순 변수
    if (mem_size == 0) 
        mem_size = 1;                         
    
    //$사용 확인해서 변수인지 확인
    if (kind!=varId && tb.name[0]=='$')  
    {   
        cout << "변수명 이외에서 $를 사용할 수 없습니다. : " << tb.name << endl;                     
        errorExit("변수명 이외에서 $를 사용할 수 없습니다. : ", tb.name);
    }
    
    tb.nmKind = kind;
    n = -1;       
    
    //중복을 확인한다
    if (kind == fncId)  
        n = searchName(tb.name, 'G');
    if (kind == paraId) 
        n = searchName(tb.name, 'L');
    if (n != -1) 
    {
        cout << "변수 이름이 중복되었습니다. : " << tb.name << endl;
        errorExit("or변수 이름이 중복되었습니다. : ", tb.name);
    }

    //주소를 설정한다.
    if (kind == fncId)
    { 
        //함수의 시작 행
        tb.adrs = get_lineNo();                   
    }
    else 
    {
        //로컬일 경우
        if (isLocal) 
        { 
            //로컬 영역을 확보한다.
            tb.adrs = localAdrs; localAdrs += mem_size; 
        }     
        //글로벌일 경우
        else 
        {        
            //글로벌 영역을 확보한다
            tb.adrs = Dmem.size();      
            Dmem.resize(Dmem.size() + mem_size);           
        }
    }

    //이제 메모리에 등록한다.
    //로컬일 경우
    if (isLocal) 
    { 
        n = LocalTable.size(); 
        LocalTable.push_back(tb); 
    }           
    else
    { 
        n = GlobalTable.size(); 
        GlobalTable.push_back(tb);
    }         

    //등록한 위치를 리턴한다.
    return n;                                                      
}

//로컬 심볼 테이블의 시작 위치를 리턴한다
//그냥 로컬 테이블의 사이즈 다음부터 처리하자.
void setStartLocalTable() 
{
    startLtable = LocalTable.size();
}

//로컬쪽의 이름이면 true를 넘긴다
bool isLocalName(const string& name, SymKind kind) 
{
    if (kind == paraId) 
        return true;
    
    if (kind == varId) {
        if (is_localScope() && name[0]!='$') 
            return true; 
        else 
            return false;
    }
    return false;                                                      
}

//글로벌 심볼 테이블에서 해당되는 이름을 검색한다
int searchName(const string& s, int mode) 
{
    int n;
    switch (mode) 
    {
        //글로벌 심벌 테이블 검색
        case 'G': 										
            for (n=0; n<(int)GlobalTable.size(); n++) 
            {
                if (GlobalTable[n].name == s) 
                    return n;
            }
            break;
        
        //로컬 심볼 테이블 검색
        case 'L':  											
            for (n=startLtable; n<(int)LocalTable.size(); n++) 
            {
                if (LocalTable[n].name == s) 
                    return n;
            }
            break;
        
        //함수명을 검색
        case 'F':  													 
            n = searchName(s, 'G');
            if (n != -1 && GlobalTable[n].nmKind==fncId) 
                return n;
            break;
        
        //변수명 검색
        case 'V':  													 
            if (searchName(s, 'F') != -1)
            {
                cout << "함수명과 중복되었습니다. : " << s << endl;
                errorExit("or함수명과 중복되었습니다. : ", s);
            }
            if (s[0] == '$')     
                return searchName(s, 'G');
            if (is_localScope()) 
                return searchName(s, 'L');     
            else   
                return searchName(s, 'G');     
    }
    return -1; 
}

//반복자를 획득한다
vector<SymTbl>::iterator tablePointer(const CodeSet& cd)
{
    if (cd.kind == Lvar) 
        return LocalTable.begin() + cd.symNbr;           
    return GlobalTable.begin() + cd.symNbr;               
}


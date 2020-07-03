/*
 * 생성자: Kyuling Lee
 * 메인 헤더 (공통)
 */
 
#include <iostream>
#include <fstream>    // 파일 처리용
#include <sstream>    // 문자열 스트림용 
#include <string>
#include <vector>
#include <stack>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cctype>

using namespace std;

//define은 여기다가 일단 다 모았다.
//short int형 크기
#define SHORT_SIZ  sizeof(short int)         
//short int형 포인터 변환
#define SHORT_P(p) (short int *)(p)
//unsigned char형 포인터 변환
#define UCHAR_P(p) (unsigned char *)(p)
//소스 한 줄 최대 크기
#define LIN_SIZ 255   


//토큰으로 만들 요소들에 대해서 enum으로 정리하였다.
enum TknKind
{
	Lparen = '(',
	Rparen = ')',
	Lbracket = '[',
	Rbracket = ']',
	Plus = '+',
	Minus = '-',
	Multi = '*',
	Divi = '/',
	Mod = '%',
	Not = '!',
	Ifsub = '?',
	Assign = '=',
	IntDivi = '\\',
	Comma = ',',
	DblQ = '"',
	Func = 150,
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
struct Token
{
	//토큰 종류
	TknKind kind;
	//토큰 문자열
	string  text;
	//수치 상수로 변환할 때의 값
	double  dblVal;

	Token()
	{
		kind = Others;
		text = "";
		dblVal = 0.0;
	}

	Token(TknKind k)
	{
		kind = k;
		text = "";
		dblVal = 0.0;
	}

	Token(TknKind k, double d)
	{
		kind = k;
		text = "";
		dblVal = d;
	}

	Token(TknKind k, const string& s)
	{
		kind = k;
		text = s;
		dblVal = 0.0;
	}

	Token(TknKind k, const string& s, double d)
	{
		kind = k;
		text = s;
		dblVal = d;
	}
};

//심볼 테이블에서의 등록 이름의 종류
enum SymKind
{
	noId,
	varId,
	fncId,
	paraId
};

//심볼 테이블에서의 타입 이름
enum DtType
{
	NON_T,
	DBL_T
};

//심볼 테이블을 구성하는 구조체
struct SymTbl
{
	//변수나 함수의 이름
	string  name;
	//종류
	SymKind nmKind;
	//변수명
	char    dtTyp;
	//배열의 길이
	int     aryLen;
	//함수의 인수 갯수
	short   args;
	//변수, 함수의 주소
	int     adrs;
	//함수의 프레임 크기
	int     frame;

	SymTbl()
	{
		clear();
	}

	void clear()
	{
		name = "";
		nmKind = noId;
		dtTyp = NON_T;
		aryLen = 0;
		args = 0;
		adrs = 0;
		frame = 0;
	}
};

//소스코드의 코드를 관리하는 구조체
struct CodeSet
{
	//종류
	TknKind kind;
	//문자열 중 리터널일 때의 위치 포인터
	const char* text;
	//수치 상수일 때의 값
	double dblVal;
	//심볼 테이블의 포인트 위치
	int    symNbr;
	//점프할 주소
	int    jmpAdrs;

	CodeSet()
	{
		clear();
	}

	CodeSet(TknKind k)
	{
		clear();
		kind = k;
	}

	CodeSet(TknKind k, double d)
	{
		clear();
		kind = k;
		dblVal = d;
	}

	CodeSet(TknKind k, const char* s)
	{
		clear();
		kind = k;
		text = s;
	}

	CodeSet(TknKind k, int sym, int jmp)
	{
		clear();
		kind = k;
		symNbr = sym;
		jmpAdrs = jmp;
	}

	void clear()
	{
		kind = Others;
		text = "";
		dblVal = 0.0;
		jmpAdrs = 0;
		symNbr = -1;
	}
};

//오브젝트 형 정보를 가진 토크
struct Tobj
{
	//저장형 타입
	//d: double
	//s: string
	//'-': 미정
	char type;
	double d;
	string s;

	Tobj()
	{
		type = '-';
		d = 0.0;
		s = "";
	}

	Tobj(double dt)
	{
		type = 'd';
		d = dt;
		s = "";
	}

	Tobj(const string& st)
	{
		type = 's';
		d = 0.0;
		s = st;
	}

	Tobj(const char* st)
	{
		type = 's';
		d = 0.0;
		s = st;
	}
};

//언어 실행을 위해 이용하는 가상 메모리 클래스
class Mymemory
{
private:
	vector<double> mem;

public:
	//재확보를 어느정도 억제하기 위해 자동으로 확보함.
	void auto_resize(int n)
	{
		if (n >= (int)mem.size())
		{
			n = (n / 256 + 1) * 256;
			mem.resize(n);
		}
	}

	//메모리 작성
	void set(int adrs, double dt)
	{
		mem[adrs] = dt;
	}

	//메모리 더하기
	void add(int adrs, double dt)
	{
		mem[adrs] += dt;
	}

	//메모리 읽기
	double get(int adrs)
	{
		return mem[adrs];
	}

	//메모리 저장 크기 확인
	int size()
	{
		return (int)mem.size();
	}

	//메모리 크기 리사이즈해서 추가 확보
	void resize(unsigned int n)
	{
		mem.resize(n);
	}
};


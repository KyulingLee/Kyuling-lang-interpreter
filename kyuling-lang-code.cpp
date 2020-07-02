/*
 * 생성자: Kyuling Lee
 * 코드 처리를 진행하는 소스코드 파일
 */

#include "kyuling-lang.h"
#include "kyuling-lang-prototype.h"

 //코드 셋
CodeSet code;

//시작하는 위치의 프로그램 카운터
int startPc;

//프로그램 카운터.
//-1로 두고 실행중이 아닌것으로 먼저 설정함.
int Pc = -1;

//베이스 레지스터
int baseReg;

//스텍 포인터 레지스터
int spReg;

//프로그램 코드의 끝 라인
int maxLine;

//변환 후에 내부 코드를 저장하고 있는 벡터
vector<char*> intercode;

//내부 코드를 분석하기 위해 이용하는 포인터
char* code_ptr;

//함수 변환값
//너무 여러곳에서 써서 밖에 뺐다.
double returnValue;

//제어를 위한 플래그들
//브레이크문 제어, 리턴문 제어, 종료문 제어
bool break_Flg, return_Flg, exit_Flg;

//가상메모리
Mymemory Dmem;

//문자열 리터럴을 저장하는 벡터
vector<string> strLITERAL;

//수치 리터럴을 저장하는 백터
vector<double> nbrLITERAL;

//구문검사용 -> 검사 되면 참으로 바꿔서 처리함.
bool syntaxChk_mode = false;

//글로벌 심볼 테이블
extern vector<SymTbl> Gtable;

//stl에 있는 stack을 감싸서 처리하는 클래스
//내가 좀 원하는 형태를 추가하고 싶었다.
class Mystack {
private:
	stack<double> st;

public:
	//넣기
	void push(double n)
	{
		st.push(n);
	}

	//크기 반환
	int size()
	{
		return (int)st.size();
	}

	//비우기
	bool empty()
	{
		return st.empty();
	}

	//읽은 다음에 빼기
		//보통은 그냥 빼기만 하지만 일단 읽는 작업도 한다.
		//이거땜에 다시 짬.
	double pop()
	{
		if (st.empty())
			err_exit("stack underflow");

		double d = st.top();
		st.pop();
		return d;
	}
};

//실제로 내 인터프리터가 이용할 스택
//오퍼랜드를 가지고 꺼내는 녀석이다.
Mystack stk;

//구문 검사를 진행
void syntaxChk()
{
	syntaxChk_mode = true;

	for (Pc = 1; Pc < (int)intercode.size(); Pc++)
	{
		code = firstCode(Pc);
		switch (code.kind)
		{
			//검사 완료
		case Func:
		case Option:
		case Var:
			break;
		case Else:
		case End:
		case Exit:
			code = nextCode();
			chk_EofLine();
			break;
		case If:
		case Elif:
		case While:
			code = nextCode();
			//식값에 대해서 처리
			(void)get_expression(0, EofLine);
			break;
		case For:
			code = nextCode();
			//제어 변수의 주소
			(void)get_memAdrs(code);
			//초기값
			(void)get_expression('=', 0);
			//최종값
			(void)get_expression(To, 0);

			//스탭이 증가될 경우
			if (code.kind == Step)
			{

				//증분값 처리
				(void)get_expression(Step, 0);
			}
			chk_EofLine();
			break;
			//이 함수는 대입이 없는 함수임.
		case Fcall:
			fncCall_syntax(code.symNbr);
			chk_EofLine();
			//따라서 별도의 반환을 안해도 됨
			(void)stk.pop();
			break;
		case Print:
		case Println:
			sysFncExec_syntax(code.kind);
			break;
			//대입문들
		case Gvar:
		case Lvar:
			//좌변의 주소
			(void)get_memAdrs(code);
			//우변의 식의 값
			(void)get_expression('=', EofLine);
			break;
			//반환값
		case Return:
			code = nextCode();
			if (code.kind != '?' && code.kind != EofLine)
				(void)get_expression();

			if (code.kind == '?')
				(void)get_expression('?', 0);

			chk_EofLine();
			break;

		case Break:
			code = nextCode();

			if (code.kind == '?')
				(void)get_expression('?', 0);

			chk_EofLine();
			break;
		case EofLine:
			break;
		default:
			cout << "잘못된 서술입니다.: " << kind_to_s(code.kind) << endl;
			err_exit("잘못된 서술입니다.: ", kind_to_s(code.kind));
		}
	}

	syntaxChk_mode = false;
}

//시작행의 프로그램 카운터 설정
void set_startPc(int n)
{
	startPc = n;
}

//실행
void execute()
{
	//베이스 레지스터 값을 초기화
	baseReg = 0;
	//스텍 포인터를 초기화
	spReg = Dmem.size();
	//메모리 영역을 초기에 어느정도 확보
	Dmem.resize(spReg + 1000);
	//각종 플래그 설정
	break_Flg = return_Flg = exit_Flg = false;

	//프로그램 카운터 설정
	Pc = startPc;
	//최대 라인 설정
	maxLine = intercode.size() - 1;

	//구문 처리
	while (Pc <= maxLine && !exit_Flg)
	{
		statement();
	}

	//프로그램 카운터를 비실행 상태로 돌림
	Pc = -1;
}

//구문 처리
void statement()
{
	CodeSet save;
	int top_line, end_line, varAdrs;
	double wkVal, endDt, stepDt;

	//마지막 라인이나 프로그램 종료 플래그일 경우
	//프로그램을 종료한다.
	if (Pc > maxLine || exit_Flg)
		return;

	code = save = firstCode(Pc);
	//제어 범위의 시작과 끝을 지정한다.	
	top_line = Pc;
	end_line = code.jmpAdrs;

	//if문일 때의 끝을 지정한다.
	if (code.kind == If)
		end_line = endline_of_If(Pc);

	switch (code.kind)
	{
	case If:
		//if
		if (get_expression(If, 0))
		{
			++Pc;
			block();
			Pc = end_line + 1;
			return;
		}
		//다음 처리를 위해 점프
		Pc = save.jmpAdrs;

		//elif
		while (lookCode(Pc) == Elif)
		{
			//앞에 문 별도로 저장하고 그 다음은 if의 시작과 유사하다.
			save = firstCode(Pc);
			code = nextCode();

			if (get_expression())
			{
				++Pc;
				block();
				Pc = end_line + 1;
				return;
			}
			//이 다음 동작을 확인하여 실행
			Pc = save.jmpAdrs;
		}

		//else
		if (lookCode(Pc) == Else)
		{
			//라인 실행 후 종료 동작은 같다.
			++Pc;
			block();
			Pc = end_line + 1;
			return;
		}

		//end
		++Pc;

		break;
	case While:
		//while의 경우에는 종료 처리를 확인하고 넘겨야 하기 때문에 일단 무한 루프 안에서 동작을 시켜야 한다.
		for (;;)
		{
			//false 처리일 경우 종료       
			if (!get_expression(While, 0))
				break;

			//우선 실행
			++Pc;
			block();

			//종료 관련 플래그 올라가면 종료
			if (break_Flg || return_Flg || exit_Flg)
			{
				break_Flg = false;
				break;
			}

			//while의 시작점으로 이동한다.
			Pc = top_line;
			code = firstCode(Pc);
		}
		Pc = end_line + 1;
		break;
	case For:
		//for문은 제어변수, 초기값, 최종값과 증분식까지 구성해야 한다.
			//그래서 코드 구조가 while보다 좀 더 복잡하다.
		save = nextCode();
		//제어 변수의 주소를 구한다.
		varAdrs = get_memAdrs(save);

		//초기값 설정
		expression('=', 0);
		//형을 확정한다.
		set_dtTyp(save, DBL_T);
		//초기값을 설정한다.
		Dmem.set(varAdrs, stk.pop());

		//최종값에 대해서 보존을 한다.
		endDt = get_expression(To, 0);

		//증분값에 대해서 보존한다. (이거 중요)
		if (code.kind == Step)
			stepDt = get_expression(Step, 0);
		else
			stepDt = 1.0;


		for (;; Pc = top_line)
		{
			//증가 루프   
			if (stepDt >= 0)
			{
				//거짓일 경우에는 종료를 한다.      
				if (Dmem.get(varAdrs) > endDt)
					break;
			}
			//감소 루프
			else
			{
				//거짓이면 종료한다.
				if (Dmem.get(varAdrs) < endDt)
					break;
			}

			//실행을 한다.
			++Pc;
			block();

			//실행 도중에 중단하는 플래그를 발견하면 중단한다.
			if (break_Flg || return_Flg || exit_Flg)
			{
				break_Flg = false;
				break;
			}

			//가상 메모리에 실행 스탭 추가
			Dmem.add(varAdrs, stepDt);
		}

		Pc = end_line + 1;
		break;
	case Fcall:
		//대입 없는 함술을 호출했을 경우
		//함수 호출 후에 그냥 리턴값 없이 void로 처리한다.
		fncCall(code.symNbr);
		(void)stk.pop();
		++Pc;
		break;
	case Func:
		//함수 정의일 경우
		Pc = end_line + 1;
		break;
	case Print:
	case Println:
		//출력문일 경우 -> 그냥 출력
		sysFncExec(code.kind);
		++Pc;
		break;
	case Gvar:
	case Lvar:
		//대입문의 경우
		varAdrs = get_memAdrs(code);
		expression('=', 0);
		//대입할 때의 형을 확정한다.
		set_dtTyp(save, DBL_T);
		Dmem.set(varAdrs, stk.pop());
		++Pc;
		break;
	case Return:
		wkVal = returnValue;
		code = nextCode();

		//식이 존재할 경우 -> 이때에는 별도의 반환값을 계산한다.
		if (code.kind != '?' && code.kind != EofLine)
			wkVal = get_expression();

		//?가 있을 경우에 처리를 진행한다.
		post_if_set(return_Flg);

		//리턴 플래그가 참일 경우
			//리턴값을 던진다.
		if (return_Flg)
			returnValue = wkVal;

		//리턴값이 거짓일 경우
			//계속 진행을 한다.
		if (!return_Flg)
			++Pc;
		break;
	case Break:
		code = nextCode();
		post_if_set(break_Flg);
		//?가 있을 경우에 처리를 해준다.
		if (!break_Flg)
			++Pc;
		break;
	case Exit:
		code = nextCode();
		exit_Flg = true;
		break;
		//여기 나머지는 그냥 실행되면 무시하면 된다.
	case Option:
	case Var:
	case EofLine:
		++Pc;
		break;
		//그 외 -> 전부 오류임.
	default:
		err_exit("잘못된 기술입니다: ", kind_to_s(code.kind));
	}
}

//블록 끝까지 문을 실행할 때 이용하는 함수
void block()
{
	TknKind k;

	//break, return, exit문일 경우에는 종료 처리를 한다.
	while (!break_Flg && !return_Flg && !exit_Flg)
	{
		//다음 시작 코드를 불러온다.
		k = lookCode(Pc);

		//다음 코드가 다음과 같은 경우에는 블록을 정상적으로 종료시킨다.
		if (k == Elif || k == Else || k == End)
			break;

		statement();
	}
}

//함수 선언에서 다음의 기본 인수를 지정
//int kind1=0, int kind2=0 이런 형태의 기본 인수들...
//이로 인해서 결과값을 던진다.
double get_expression(int kind1, int kind2)
{
	expression(kind1, kind2);
	return stk.pop();
}

//조건부 식처리를 처리하는 표현 - 인수 있음
void expression(int kind1, int kind2)
{
	if (kind1 != 0)
		code = chk_nextCode(code, kind1);

	expression();

	if (kind2 != 0)
		code = chk_nextCode(code, kind2);
}

//일반적인 식 처리를 진행 - 인수 없음
void expression()
{
	term(1);
}

//n에 대해서 우선 처리를 진행한다.
void term(int n)
{
	TknKind op;
	if (n == 7)
	{
		factor();
		return;
	}

	term(n + 1);

	//우선순위가 같을 경우에는 연산자가 연속해서 진행된다.
	while (n == opOrder(code.kind))
	{
		op = code.kind;
		code = nextCode();
		term(n + 1);

		//구문 체크를 하는 모드일 경우
		if (syntaxChk_mode)
		{
			stk.pop();
			stk.pop();
			stk.push(1.0);
		}
		else
			binaryExpr(op);
	}
}

//인자 처리 함수.
void factor()
{
	TknKind kd = code.kind;

	//구문 체크 처리를 한다.
	if (syntaxChk_mode)
	{
		//구문의 토큰 종류 처리
		switch (kd)
		{
		case Not:
		case Minus:
		case Plus:
			code = nextCode();
			factor();
			stk.pop();
			stk.push(1.0);
			break;
		case Lparen:
			expression('(', ')');
			break;
		case IntNum:
		case DblNum:
			stk.push(1.0);
			code = nextCode();
			break;
		case Gvar:
		case Lvar:
			(void)get_memAdrs(code);
			stk.push(1.0);
			break;
		case Toint:
		case Input:
			sysFncExec_syntax(kd);
			break;
		case Fcall:
			fncCall_syntax(code.symNbr);
			break;
		case EofLine:
			err_exit("식이 바르지 않습니다.");
		default:
			err_exit("식 오류:", kind_to_s(code));
		}
		return;
	}

	//구문 이외의 실행 토큰 처리
	switch (kd)
	{
	case Not:
	case Minus:
	case Plus:
		//다음 값을 획득하여 처리한다.
		code = nextCode();
		factor();

		if (kd == Not)
			stk.push(!stk.pop());

		if (kd == Minus)
			stk.push(-stk.pop());

		break;
	case Lparen:
		expression('(', ')');
		break;
	case IntNum:
	case DblNum:
		stk.push(code.dblVal);
		code = nextCode();
		break;
	case Gvar:
	case Lvar:
		//값 설정을 마친 변수인가 확인
		chk_dtTyp(code);
		stk.push(Dmem.get(get_memAdrs(code)));
		break;
	case Toint:
	case Input:
		sysFncExec(kd);
		break;
	case Fcall:
		fncCall(code.symNbr);
		break;
	}
}

//이항 연산자의 우선순위 처리
int opOrder(TknKind kd)
{
	switch (kd)
	{
		// *,  /,  %, \  
	case Multi:
	case Divi:
	case Mod:
	case IntDivi:
		return 6;

		// +,  - 
	case Plus:
	case Minus:
		return 5;

		// <,  <=, >, >= 
	case Less:
	case LessEq:
	case Great:
	case GreatEq:
		return 4;

		// == , !=     
	case Equal:
	case NotEq:
		return 3;

		// &&
	case And:
		return 2;

		// ||
	case Or:
		return 1;

		//기본적으로는 해당 없음.
	default:
		return 0;

	}
}

//이항 연산 처리
void binaryExpr(TknKind op)
{
	double d = 0, d2 = stk.pop(), d1 = stk.pop();

	if ((op == Divi || op == Mod || op == IntDivi) && d2 == 0)
		err_exit("0으로 나누었습니다.");

	switch (op)
	{
	case Plus:
		d = d1 + d2;
		break;
	case Minus:
		d = d1 - d2;
		break;
	case Multi:
		d = d1 * d2;
		break;
	case Divi:
		d = d1 / d2;
		break;
	case Mod:
		d = (int)d1 % (int)d2;
		break;
	case IntDivi:
		d = (int)d1 / (int)d2;
		break;
	case Less:
		d = d1 < d2;
		break;
	case LessEq:
		d = d1 <= d2;
		break;
	case Great:
		d = d1 > d2;
		break;
	case GreatEq:
		d = d1 >= d2;
		break;
	case Equal:
		d = d1 == d2;
		break;
	case NotEq:
		d = d1 != d2;
		break;
	case And:
		d = d1 && d2;
		break;
	case Or:
		d = d1 || d2;
		break;
	}

	stk.push(d);
}

//?식(삼항연산자)을 처리한다.
void post_if_set(bool& flg)
{
	//?가 없을 경우 플래그를 true로 변경
	if (code.kind == EofLine)
	{
		flg = true;
		return;
	}

	//발견될 경우 -> 조건식과 같이 처리
	if (get_expression('?', 0))
		flg = true;
}

//함수 호출을 검사하는 함수
void fncCall_syntax(int fncNbr)
{
	int argCt = 0;

	//실인수 저장
	code = nextCode();
	//함수명 뒤의 (를 체크
	code = chk_nextCode(code, '(');

	//인수가 있을 경우
	if (code.kind != ')')
	{
		for (;; code = nextCode())
		{
			//인수식을 처리하고 인수 갯수를 증가
			(void)get_expression();
			++argCt;

			//, 만나면 인수가 계속 되는 것이니 계속되게 처리
			if (code.kind != ',')
				break;
		}
	}
	//)가 나오니 넘기고 처리
	code = chk_nextCode(code, ')');

	//인수 갯수를 검사
	if (argCt != Gtable[fncNbr].args)
		err_exit(Gtable[fncNbr].name, " 함수의 인수 개수가 잘못되었습니다");

	//적당한 반환값을 넘김
	stk.push(1.0);
}

//함수 호출 처리하는 함수
void fncCall(int fncNbr)
{
	int  n, argCt = 0;
	vector<double> vc;

	// 실인수 저장
	nextCode();
	//함수명 뒤의 (를 건너뜀
	code = nextCode();

	//인수가 있을 경우
	if (code.kind != ')')
	{
		for (;; code = nextCode())
		{
			//인수식을 처리하고 인수 갯수를 늘린다.
			expression();
			++argCt;

			//,를 만나면 인수 처리를 계속 한다.
			if (code.kind != ',')
				break;
		}
	}

	//)를 건너뜀
	code = nextCode();

	//인수 저장 순서 변경
	//인수 저장을 뒤에서부터 인수를 저장하도록 수정함.
	for (n = 0; n < argCt; n++)
	{
		vc.push_back(stk.pop());
	}

	for (n = 0; n < argCt; n++)
	{
		stk.push(vc[n]);
	}

	//함수 실행
	fncExec(fncNbr);
}

//함수를 실행한다.
void fncExec(int fncNbr)
{
	//함수 입구를 처리 첫번째
	//현재 실행행의 프로그램 카운터 저장
	int save_Pc = Pc;
	//현재의 base register 저장
	int save_baseReg = baseReg;
	//현재의 sp register 저장
	int save_spReg = spReg;
	//현재 실행하는 행의 분석용 포인터 저장
	char* save_code_ptr = code_ptr;
	//현재 코드를 저장
	CodeSet save_code = code;

	//함수 입구를 처리 두번째
	//새로운 프로그램 카운터 설정
	Pc = Gtable[fncNbr].adrs;
	//새로운 베이스 레지스터 설정
	baseReg = spReg;
	//프레임을 새로 확보
	spReg += Gtable[fncNbr].frame;
	//메모리 유효 영역 확보
	Dmem.auto_resize(spReg);
	//반환 값의 기본값 임의 설정
	returnValue = 1.0;
	//시작 코드를 얻음
	code = firstCode(Pc);

	//인수 저장을 처리
	nextCode();
	//함수명 뒤의 (를 건너뜀 
	code = nextCode();

	//인수 있을 경우
	//) 만날 때까지 돌게된다. ㅇㅅㅇ
	if (code.kind != ')')
	{
		for (;; code = nextCode())
		{
			//대입 시 형을 확정함
			set_dtTyp(code, DBL_T);
			//실제 인수 값 저장
			Dmem.set(get_memAdrs(code), stk.pop());

			//인수 처리 종료
			if (code.kind != ',')
				break;
		}
	}

	//) 처리를 건너뜀
	code = nextCode();

	//함수 본체를 처리
	++Pc;
	block();
	return_Flg = false;

	//함수 출구를 처리
	//반환 값 설정
	stk.push(returnValue);
	//호출 이전 환경을 복원
	Pc = save_Pc;
	baseReg = save_baseReg;
	spReg = save_spReg;
	code_ptr = save_code_ptr;
	code = save_code;
}

//내장 함수를 검사한다.
void sysFncExec_syntax(TknKind kd)
{
	switch (kd)
	{
	case Toint:
		code = nextCode();
		(void)get_expression('(', ')');
		stk.push(1.0);
		break;
	case Input:
		code = nextCode();
		code = chk_nextCode(code, '(');
		code = chk_nextCode(code, ')');
		//적당한 값을 넣어준다.
		stk.push(1.0);
		break;
	case Print:
	case Println:
		do
		{
			code = nextCode();
			//문자열이면 출력 확인
			if (code.kind == String)
				code = nextCode();
			//그 외에는 값 출력 확인
			else
				(void)get_expression();
		}
		//,라면 파라미터 계속 진행.
			//단, 앞에것도 실행은 해야 하니 while이 뒤에 있음. 
		while (code.kind == ',');

		chk_EofLine();

		break;
	}
}

//내장 함수를 실행하는 함수
void sysFncExec(TknKind kd)
{
	double d;
	string s;

	switch (kd)
	{
	case Toint:
		code = nextCode();
		//끝에 수를 버린다.
		stk.push((int)get_expression('(', ')'));
		break;
	case Input:
		nextCode();
		nextCode();
		//input()을 건너뛴다.
		code = nextCode();
		//한 행을 읽음
		getline(cin, s);
		//숫자로 변환해서 저장함
		stk.push(atof(s.c_str()));
		break;
	case Print:
	case Println:
		do
		{
			code = nextCode();
			//문자열이면 문자열을 출력한다.
			if (code.kind == String)
			{
				cout << code.text; code = nextCode();
			}
			else
			{
				//함수 내에서 종료될 가능성이 있기 때문에 확인
				d = get_expression();

				//빠져나가는 플래그 아닐 경우, 수치 출력
				if (!exit_Flg)
					cout << d;
			}
		}
		//,가 있으면 파라미터가 계속 되어야 한다. 
			//근데 그 전에 있던 파라메터도 실행은 해야 하니 while을 뒤로 뺌 
		while (code.kind == ',');

		//println일 경우 -> 줄바꿈도 해줘야 함.
		if (kd == Println)
			cout << endl;
		break;
	}
}

// 단순 변수 또는 배열 요소의 주소를 반환한다
int get_memAdrs(const CodeSet& cd)
{
	int adr = 0, index, len;
	double d;

	adr = get_topAdrs(cd);
	len = tableP(cd)->aryLen;
	code = nextCode();

	//비배열 변수 -> 그냥 주소 변환
	if (len == 0)
		return adr;

	d = get_expression('[', ']');

	//첨자 오류
	if ((int)d != d)
		err_exit("첨자는 끝수가 없는 수치로 지정해 주세요.");

	//구문 검사 모드 -> 그냥 주소 반환
	if (syntaxChk_mode)
		return adr;

	index = (int)d;

	//첨자 범위 벗어날 경우
	if (index < 0 || len <= index)
		err_exit(index, "  는 첨자 범위 밖입니다(첨자범위:0-", len - 1, "Åj");

	return adr + index;
}

// 변수의 시작 주소(배열일 때도 그 시작 주소)를 반환한다
int get_topAdrs(const CodeSet& cd)
{
	switch (cd.kind)
	{
	case Gvar:
		return tableP(cd)->adrs;
	case Lvar:
		return tableP(cd)->adrs + baseReg;
	default:
		err_exit("변수명이 필요합니다: ", kind_to_s(cd));
	}

	// 이곳으론 오지 않는다
	return 0;
}

int endline_of_If(int line)
{
	CodeSet cd;
	char* save = code_ptr;

	cd = firstCode(line);

	for (;;)
	{
		line = cd.jmpAdrs;
		cd = firstCode(line);

		if (cd.kind == Elif || cd.kind == Else)
			continue;

		if (cd.kind == End)
			break;
	}

	code_ptr = save;

	return line;
}

//라인의 마지막인지 체크하는 코드 확인
void chk_EofLine()
{
	if (code.kind != EofLine)
		err_exit("잘못된 기술입니다: ", kind_to_s(code));
}

//특정 라인 행의 시작 코드를 진행
TknKind lookCode(int line)
{
	return (TknKind)(unsigned char)intercode[line][0];
}

//확인부쪽 코드를 확인 후에 다음을 진행.
CodeSet chk_nextCode(const CodeSet& cd, int kind2)
{
	if (cd.kind != kind2)
	{
		if (kind2 == EofLine)
			err_exit("잘못된 서술입니다: ", kind_to_s(cd));

		if (cd.kind == EofLine)
			err_exit(kind_to_s(kind2), " 가 필요합니다.");

		err_exit(kind_to_s(kind2) + " 가(이) " + kind_to_s(cd) + " 앞에 필요합니다.");
	}

	return nextCode();
}

//시작 코드를 획득하는 함수
CodeSet firstCode(int line)
{
	//분석용 포인터를 행의 시작으로 설정한다.
	code_ptr = intercode[line];
	return nextCode();
}

//다음 코드를 획득하는 함수
CodeSet nextCode()
{
	TknKind kd;
	short int jmpAdrs, tblNbr;

	//코드가 마지막이면 종료
	if (*code_ptr == '\0')
		return CodeSet(EofLine);

	kd = (TknKind)*UCHAR_P(code_ptr++);

	//이런 거 땜에 앞단계에서 전처리를....ㅠㅠㅠㅠ
	switch (kd)
	{
	case Func:
	case While:
	case For:
	case If:
	case Elif:
	case Else:
		//점프할 주소를 만들어서 리턴
		jmpAdrs = *SHORT_P(code_ptr);
		code_ptr += SHORT_SIZ;
		return CodeSet(kd, -1, jmpAdrs);
	case String:
		//다음 테이블 주소 포인터를 만들어서 리턴
		tblNbr = *SHORT_P(code_ptr);
		code_ptr += SHORT_SIZ;
		return CodeSet(kd, strLITERAL[tblNbr].c_str());
	case IntNum:
	case DblNum:
		//수치 리터럴값에 대한 포인터를 만들어 리턴
		tblNbr = *SHORT_P(code_ptr);
		code_ptr += SHORT_SIZ;
		return CodeSet(kd, nbrLITERAL[tblNbr]);
	case Fcall:
	case Gvar:
	case Lvar:
		//함수 콜에 대한 주소 포인터를 만들어 리턴
		tblNbr = *SHORT_P(code_ptr);
		code_ptr += SHORT_SIZ;
		return CodeSet(kd, tblNbr, -1);
	default:
		// 부속 정보가 없는 코드셋이 만들어져서 리턴
		return CodeSet(kd);
	}
}

//데이터 형을 확인하는 함수
void chk_dtTyp(const CodeSet& cd)
{
	if (tableP(cd)->dtTyp == NON_T)
		err_exit("초기화되지 않은 변수가 사용되었습니다: ", kind_to_s(cd));
}

//데이터 형을 설정하는 함수
void set_dtTyp(const CodeSet& cd, char typ)
{
	int memAdrs = get_topAdrs(cd);
	vector<SymTbl>::iterator p = tableP(cd);

	//이미 형이 결정되어 있으면 그냥 끝냄.
	if (p->dtTyp != NON_T)
		return;

	p->dtTyp = typ;
	//배열일 경우에는 해당 메모리 내용을 0으로 초기화
	if (p->aryLen != 0)
	{
		for (int n = 0; n < p->aryLen; n++)
		{
			Dmem.set(memAdrs + n, 0);
		}
	}
}

//수치값의 리터럴 설정
int set_LITERAL(double d)
{
	for (int n = 0; n < (int)nbrLITERAL.size(); n++)
	{
		//같은 첨자의 위치를 반환한다.
		if (nbrLITERAL[n] == d)
			return n;
	}

	//수치값의 리터럴을 저장한다.
	nbrLITERAL.push_back(d);

	//저장한 수치값 리터럴의 첨자 위치를 반환한다.
	return nbrLITERAL.size() - 1;
}

//문자열 리터럴 설정
int set_LITERAL(const string& s)
{
	for (int n = 0; n < (int)strLITERAL.size(); n++)
	{
		//같은 첨자의 위치를 반환한다.
		if (strLITERAL[n] == s)
			return n;
	}

	//문자열 리터럴을 저장한다.
	strLITERAL.push_back(s);

	//저장된 문자열 리터럴의 첨자 위치를 리턴한다.
	return strLITERAL.size() - 1;
}


/*
 * 생성자: Kyuling Lee
 * 언어 처리 이외에 별도의 작업들을 진행하는 소스코드 파일
 */

#include "kyuling-lang.h"
#include "kyuling-lang-prototype.h"

 //수치를 문자열로 바꾸는 함수
string dbl_to_s(double d)
{
	//출력용 스트림을 확보
	ostringstream ostr;
	//출력 스트림에 쓰기
	ostr << d;
	//버퍼의 내용을 반환
	return ostr.str();
}

//에러 메시지를 생성하는 함수
string err_msg(const string& a, const string& b)
{
	if (a == "")
		return b + " 가 필요합니다.";
	if (b == "")
		return a + " 가 바르지 않습니다.";

	return b + " 가 " + a + " 앞에 필요합니다.";
}

//오류 표시를 하는 함수
//기존 cout 처리를 수정할 수 있다. ㅇㅅㅇ;;
//함수 선언에서 디폴트 인수 지정을 하고 시작하였다.
void err_exit(Tobj a, Tobj b, Tobj c, Tobj d)
{
	Tobj ob[5];
	ob[1] = a;
	ob[2] = b;
	ob[3] = c;
	ob[4] = d;

	cerr << "line:" << get_lineNo() << " ERROR ";

	for (int i = 1; i <= 4 && ob[i].s != "\1"; i++)
	{
		if (ob[i].type == 'd')
			cout << ob[i].d;
		if (ob[i].type == 's')
			cout << ob[i].s;
	}

	cout << endl;
	exit(1);
}
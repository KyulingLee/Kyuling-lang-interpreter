/*
 * 생성자: Kyuling Lee
 * 메인 프로그램
 */

#include "kyuling-lang.h"
#include "kyuling-lang-prototype.h"

int main(int argc, char* argv[])
{
    //파일이 없을 경우 - 메시지 출력 후 종료
    if (argc == 1) 
    { 
        cout << "사용 방법: bbi filename\n"; 
        exit(1); 
    }

    //내부 코드로 변환
    convertToInternalCode(argv[1]);
    //구분 체크
    syntaxCheck();
    //실행
    execute();

    return 0;
}
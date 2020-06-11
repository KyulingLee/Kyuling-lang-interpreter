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
vector<SymbolTable> GlobalTable;           
//로컬용 테이블
vector<SymbolTable> LocalTable;           
//로컬 테이블 시작 위치
int startLocalTable;           		

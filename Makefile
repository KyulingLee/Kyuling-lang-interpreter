make:
	g++ -Wall -o kyuling-lang kyuling-lang.cpp kyuling-lang.h kyuling-lang-prototype.h kyuling-lang-token.cpp kyuling-lang-parser.cpp
clean:
	rm kyuling-lang
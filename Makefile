make:
	g++ -Wall -o kyuling-lang kyuling-lang.cpp kyuling-lang.h kyuling-lang-prototype.h kyuling-lang-token.cpp kyuling-lang-parser.cpp kyuling-lang-table.cpp kyuling-lang-misc.cpp kyuling-lang-code.cpp
clean:
	rm kyuling-lang
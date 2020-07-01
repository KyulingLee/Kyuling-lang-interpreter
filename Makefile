make:
	g++ -Wall -o kyuling-lang kyuling-lang.cpp kyuling-lang-parser.cpp kyuling-lang-token.cpp kyuling-lang-table.cpp kyuling-lang-code.cpp kyuling-lang-misc.cpp
clean:
	rm kyuling-lang
#pragma once
#include <cstdio>
#include <cstdlib>
#include <string>

void ASSERT(bool exp, std::string s) {
	if (!exp) {
		fprintf(stderr, "%s\n", s.c_str());
		exit(1);
	}
}

void ERROR(std::string s) {
	fprintf(stderr, "%s\n", s.c_str());
	exit(2);
}

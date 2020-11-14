#include <cstdio>
#include <cstdlib>

void ASSERT(bool exp, char *s) {
	if (!exp) {
		fprintf(stderr, "%s\n", s);
		exit(3);
	}
}
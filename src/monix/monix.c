extern int krn;

void handler_e00() {
	krn = 1;
	printf("e00\n");
	krn = 0;
}
void handler_e01() {
	krn = 1;
	printf("e01\n");
	krn = 0;
}
void handler_e02() {
	krn = 1;
	printf("e02\n");
	krn = 0;
}
void handler_e03() {
	krn = 1;
	printf("e03\n");
	krn = 0;
}
void handler_e04() {
	krn = 1;
	printf("e04\n");
	krn = 0;
}
void handler_e05() {
	krn = 1;
	printf("e05\n");
	krn = 0;
}
void handler_e06() {
	krn = 1;
	printf("e06\n");
	krn = 0;
}
void handler_e07() {
	krn = 1;
	printf("e07\n");
	krn = 0;
}
void handler_e08() {
	krn = 1;
	printf("e08\n");
	krn = 0;
}
void handler_e09() {
	krn = 1;
	printf("e09\n");
	krn = 0;
}
void handler_e0a() {
	krn = 1;
	printf("e0a\n");
	krn = 0;
}
void handler_e0b() {
	krn = 1;
	printf("e0b\n");
	krn = 0;
}
void handler_e0c() {
	krn = 1;
	printf("e0c\n");
	krn = 0;
}
void handler_e0d(int cs) {
	*(char*)(0xb8a12) = '0' + cs;
}
void handler_e0e() {
	*(char*)(0xb8a12) = 'P';
}

void handler_e10() {
	krn = 1;
	printf("e10\n");
	krn = 0;
}
void handler_e11() {
	krn = 1;
	printf("e11\n");
	krn = 0;
}
void handler_e12() {
	krn = 1;
	printf("e12\n");
	krn = 0;
}
void handler_e13() {
	krn = 1;
	printf("e13\n");
	krn = 0;
}
void handler_e14() {
	krn = 1;
	printf("e14\n");
	krn = 0;
}
void handler_e15() {
	krn = 1;
	printf("e15\n");
	krn = 0;
}
void handler_e16() {
	krn = 1;
	printf("e16\n");
	krn = 0;
}
void handler_e17() {
	krn = 1;
	printf("e17\n");
	krn = 0;
}
void handler_e18() {
	krn = 1;
	printf("e18\n");
	krn = 0;
}
void handler_e19() {
	krn = 1;
	printf("e19\n");
	krn = 0;
}
void handler_e1a() {
	krn = 1;
	printf("e1a\n");
	krn = 0;
}
void handler_e1b() {
	krn = 1;
	printf("e1b\n");
	krn = 0;
}
void handler_e1c() {
	krn = 1;
	printf("e1c\n");
	krn = 0;
}
void handler_e1d() {
	krn = 1;
	printf("e1d\n");
	krn = 0;
}
void handler_e1e() {
	krn = 1;
	printf("e1e\n");
	krn = 0;
}
void handler_e1f() {
	krn = 1;
	printf("e1f\n");
	krn = 0;
}
void handler_e0f() {
	krn = 1;
	printf("e0f\n");
	krn = 0;
}



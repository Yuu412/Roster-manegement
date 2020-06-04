
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

char dummy[5];

/*
===========================================================================
宣言部
===========================================================================
*/

#define MAX_LINE_LEN 1024
#define MAX_DATA_STORE 10000
int profile_data_nitems = 0;

struct date {
	int y; // year
	int m; // month
	int d; // day of month
};

struct profile {
	int ID;   //ID
	char school[70];   //学校名
	struct date d1;    //設立年月日
	char place[70];    //所在地
	char* etc;    //備考データ
};

struct profile profile_data_store[MAX_DATA_STORE];
struct profile* profile_data_pnt[MAX_DATA_STORE];

struct profile* new_profile(struct profile* p, char* line);

void exec_command(char cmd, char* num);

void make_profile_pnt(int i, int j) {
	//構造体をポインタに設定し直している
	//(ソート時に構造体全体を置き換えることによりメモリを多く使用してしまうことを防ぐ)
	profile_data_pnt[i] = &profile_data_store[j];
}


/*
===========================================================================
解析部
===========================================================================
*/

int subst(char* str, char c1, char c2) {
	//c1をc2に置き換える
	int num = 0;
	while (*str) {
		if (*str == c1) {
			*str = c2;
			num++;
		}
		str++;
	}
	return num;
}

int split(char* str, char* ret[], char sep, int max) {
	int num = 0;

	ret[num++] = str;

	while (*str && num < max) {
		if (*str == sep) {
			*str = '\0';
			ret[num++] = str + 1;
		}
		str++;
	}
	return num;
}

int get_line(FILE* abc, char* line) {
	//文字列を読み込む
	if (fgets(line, MAX_LINE_LEN + 1, abc) == NULL) {
		return 0;
	}
	subst(line, '\n', '\0');
	if (*line == '\0')
		return 0;
	//なにも打たれていなかったらゼロを返す。
	return 1;
}

struct profile* new_profile(struct profile* p, char* line) {
	//読み込んだ文字列を構造体に格納する関数
	char* ret[5], * ret2[3];
	int year, month, day;

	if (split(line, ret, ',', 5) != 5) {
		printf("Please input 5 data\n");
		profile_data_nitems--;
		return NULL;  //カンマの数が違っていたらエラー
	}
	if (split(ret[2], ret2, '-', 3) != 3) {
		printf("Please input corect date\n");
		profile_data_nitems--;
		return NULL;  //-の数が違っていたらエラー
	}

	year = atoi(ret2[0]);
	month = atoi(ret2[1]);
	day = atoi(ret2[2]);
	if (year < 0 || month > 13 || day > 31) {
		printf("Please input corect date\n");
		profile_data_nitems--;
		return NULL;  //年月日が異常な値ならエラー
	}

	p->ID = atoi(ret[0]);

	strncpy(p->school, ret[1], 69);
	p->school[69] = '\0';

	//設立年月日のret[2]をsplitで分割して、それぞれを下記で構造体に入れる
	p->d1.y = atoi(ret2[0]);
	p->d1.m = atoi(ret2[1]);
	p->d1.d = atoi(ret2[2]);

	strncpy(p->place, ret[3], 69);
	p->place[69] = '\0';

	p->etc = (char*)malloc(sizeof(char) * (strlen(ret[4]) + 1));
	strcpy(p->etc, ret[4]);

	return p;
}

void parse_line(char* line) {
	int i, count = 0;

	if (line[0] == '%') {
		exec_command(line[1], &line[3]);
	}
	else {
		for (i = 0; i < profile_data_nitems; i++) {
			if (profile_data_store[i].ID == '\0') {
				new_profile(&profile_data_store[i], line);
				make_profile_pnt(profile_data_nitems, i);
				profile_data_nitems++;
				count++;

			}
		}
		if (count == 0) {
			new_profile(&profile_data_store[profile_data_nitems], line);
			make_profile_pnt(profile_data_nitems, profile_data_nitems);
			profile_data_nitems++;
		}
	}
}


/*
===========================================================================
各種コマンド実現部
===========================================================================
*/
void cmd_quit() {
	/*
	Qコマンド :
	*/
	exit(0);
}


void cmd_check() {//保持データの項目数の表示
	printf("%d profile(s)\n", profile_data_nitems);
}


void print(int nitems, int num);
void cmd_print(int nitems) { //保持データの出力
	if (nitems > 0 && profile_data_nitems > nitems) {
		print(nitems, 0);
	}
	else if (nitems < 0 && profile_data_nitems > -nitems) {
		print(profile_data_nitems, profile_data_nitems + nitems);
	}
	else {
		print(profile_data_nitems, 0);
	}
}

void print(int nitems, int num) {
	//%P "数値"の正負によってnitemsによって場合分けし構造体を表示する関数
	int i;
	struct profile* p;

	for (i = num; i < nitems; i++) {
		p = profile_data_pnt[i];
		printf("No.%d\n", i + 1);
		printf("Id    : %d\n", p->ID);
		printf("Name  : %s\n", p->school);
		printf("Birth : %d-%d-%d\n", p->d1.y, p->d1.m, p->d1.d);
		printf("Addr  : %s\n", p->place);
		printf("Com   : %s\n\n", p->etc);
	}
}


int cmd_read(char* filename) {
	FILE* fp;
	char line[MAX_LINE_LEN + 1];

	fp = fopen(filename, "r");

	if (fp == NULL) { /* fp が NULL なら，オープン失敗 */
		fprintf(stderr, "Could not open file: %s\n", filename);
		return -1;
	}

	while (get_line(fp, line)) {
		parse_line(line);
	}
	fclose(fp);
	return 0;

}


void fprint(FILE* fp, struct profile* p);
int cmd_write(char* filename) {
	int i;
	char com[2];
	FILE* fp;

	if (profile_data_nitems == 0) {
		printf("There are no data.\n");
		printf("Please input Y or N depending on whether you want to output data (= Y) or not (=N).\n");
		scanf("%s", com);
		if (com[0] == 'Y') {
			printf("CAN DO IT!!\n");
		}
		else if (com[0] == 'N') {
			return -1;
		}
	}
	fp = fopen(filename, "w");

	for (i = 0; i < profile_data_nitems; i++) {
		fprint(fp, profile_data_pnt[i]);
	}
	fclose(fp);

	return 0;
}

void fprint(FILE* fp, struct profile* p) {
	fprintf(fp, "%d,", p->ID);
	fprintf(fp, "%s,", p->school);
	fprintf(fp, "%d-%d-%d,", p->d1.y, p->d1.m, p->d1.d);
	fprintf(fp, "%s,", p->place);
	fprintf(fp, "%s\n", p->etc);
}

int cmd_find_search(char* word, struct profile* p) {
	char ID[8], date[11];

	sprintf(ID, "%d", p->ID);
	sprintf(date, "%d-%d-%d", p->d1.y, p->d1.m, p->d1.d);

	if (strcmp(ID, word) == 0 ||
		strcmp(p->school, word) == 0 ||
		strcmp(date, word) == 0 ||
		strcmp(p->place, word) == 0 ||
		strcmp(p->etc, word) == 0) {
		return 0;
	}
	else {
		return 1;
	}
}

void cmd_find_print(struct profile* p) {
	printf("Id     : %d\n", p->ID);
	printf("Name   : %s\n", p->school);
	printf("Birth  : %d-%d-%d\n", p->d1.y, p->d1.m, p->d1.d);
	printf("Addr   : %s\n", p->place);
	printf("comment: %s\n", p->etc);
}


void cmd_find(char* word) {
	int i, j = 0;

	for (i = 0; i < profile_data_nitems; i++) {
		cmd_find_search(word, profile_data_pnt[i]);
		if (cmd_find_search(word, profile_data_pnt[i]) == 0) {
			printf("Profile No.%d\n", i + 1);
			cmd_find_print(profile_data_pnt[i]);
			j++;
		}
	}
	if (j == 0) {
		printf("No Data.\n");
	}
}

void cmd_sort_swap(int n1, int n2) {
	//ポインタを用いた置換
	struct profile* shelf;

	shelf = profile_data_pnt[n1];
	profile_data_pnt[n1] = profile_data_pnt[n2];
	profile_data_pnt[n2] = shelf;
}

void cmd_sort_1();
void cmd_sort_2();
void cmd_sort_3();
void cmd_sort_4();
void cmd_sort_5();

void cmd_sort(int num) {

	switch (num) {
	case 1: cmd_sort_1(); break;
	case 2: cmd_sort_2(); break;
	case 3: cmd_sort_3(); break;
	case 4: cmd_sort_4(); break;
	case 5: cmd_sort_5(); break;
	}
}

void cmd_sort_1() {
	int i, j;

	for (i = 0; i < profile_data_nitems - 1; i++) {
		for (j = 0; j < profile_data_nitems - 1; j++) {

			if (profile_data_pnt[j]->ID > profile_data_pnt[j + 1]->ID)
				cmd_sort_swap(j, j + 1);
		}
	}
}
void cmd_sort_2() {
	int i, j;
	for (i = 0; i < profile_data_nitems - 1; i++) {
		for (j = 0; j < profile_data_nitems - 1; j++) {
			if (strcmp(profile_data_pnt[j]->school, profile_data_pnt[j + 1]->school) > 0)
				cmd_sort_swap(j, j + 1);
		}
	}
}
void cmd_sort_3() {
	int i, j;

	for (i = 0; i < profile_data_nitems - 1; i++) {
		for (j = 0; j < profile_data_nitems - 1; j++) {
			if (profile_data_pnt[j]->d1.y > profile_data_pnt[j + 1]->d1.y) {
				cmd_sort_swap(j, j + 1);
			}
			else if (profile_data_pnt[j]->d1.y == profile_data_pnt[j + 1]->d1.y && profile_data_pnt[j]->d1.m > profile_data_pnt[j + 1]->d1.m) {
				cmd_sort_swap(j, j + 1);
			}
			else if (profile_data_pnt[j]->d1.y == profile_data_pnt[j + 1]->d1.y &&
				profile_data_pnt[j]->d1.m == profile_data_pnt[j + 1]->d1.m &&
				profile_data_pnt[j]->d1.d > profile_data_pnt[j + 1]->d1.d) {
				cmd_sort_swap(j, j + 1);
			}
		}
	}
}
void cmd_sort_4() {
	int i, j;
	for (i = 0; i < profile_data_nitems - 1; i++) {
		for (j = 0; j < profile_data_nitems - 1; j++) {
			if (strcmp(profile_data_pnt[j]->place, profile_data_pnt[j + 1]->place) > 0)
				cmd_sort_swap(j, j + 1);
		}
	}
}
void cmd_sort_5() {
	int i, j;
	for (i = 0; i < profile_data_nitems - 1; i++) {
		for (j = 0; j < profile_data_nitems - 1; j++) {
			if (strcmp(profile_data_pnt[j]->etc, profile_data_pnt[j + 1]->etc) > 0)
				cmd_sort_swap(j, j + 1);
		}
	}

}


void cmd_help() {
	printf("%%Q : Quitting this app.\n");
	printf("%%C : Indicating number of the data you have ever enrolled.\n");
	printf("%%P : Indicating data you have ever enrolled.\n");
	printf("%%R : Reading disignated data and enrolling.\n");
	printf("%%W : Writing out data you have ever enrolled to disignated file.\n");
	printf("%%F : Indicating data with the same word as the entered word\n");
	printf("%%S : Sort data in order of 'ID(1)','school name(2)','date of establishment','location','remarks data',\n");
	printf("%%H : Indicating each function.\n");

}

void make_profile_delete(struct profile* a);
struct profile* del_profile(struct profile* p, char* line);
void cmd_delete(int num) {
	int i;
	if (num == 0 || num > profile_data_nitems) {
		for (num = 0; num < profile_data_nitems; num++) {
			profile_data_nitems = 0;
		}
		printf("DELETE ALL DATA\n");
	}
	else if (num != 0 && num <= profile_data_nitems) {
		profile_data_pnt[num - 1]->ID = '\0';
		for (i = num - 1; i < profile_data_nitems - 1; i++) {
			cmd_sort_swap(i, i + 1);
		}
		profile_data_nitems--;
	}
}

struct profile* del_profile(struct profile* p, char* line) {
	//読み込んだ文字列を構造体に格納する関数
	char* ret[5], * ret2[3];
	split(line, ret, ',', 5);
	split(ret[2], ret2, '-', 3);

	p->ID = atoi(ret[0]);

	strncpy(p->school, ret[1], 69);
	p->school[69] = '\0';

	//設立年月日のret[2]をsplitで分割して、それぞれを下記で構造体に入れる
	p->d1.y = atoi(ret2[0]);
	p->d1.m = atoi(ret2[1]);
	p->d1.d = atoi(ret2[2]);

	strncpy(p->place, ret[3], 69);
	p->place[69] = '\0';

	p->etc = (char*)malloc(sizeof(char) * (strlen(ret[4]) + 1));
	strcpy(p->etc, ret[4]);

	return p;
}


void cmd_edit(int num) {
	char new_data[MAX_LINE_LEN + 1];
	get_line(stdin, new_data);
	new_profile(profile_data_pnt[num - 1], new_data);
}


void exec_command(char cmd, char* num) {
	switch (cmd) {
	case 'Q': cmd_quit(); break;
	case 'C': cmd_check(); break;
	case 'P': cmd_print(atoi(num)); break;
	case 'R': cmd_read(num); break;
	case 'W': cmd_write(num); break;
	case 'F': cmd_find(num); break;
	case 'S': cmd_sort(atoi(num)); break;
	case 'H': cmd_help(); break;
	case 'D': cmd_delete(atoi(num)); break;
	case 'E': cmd_edit(atoi(num)); break;

	default: fprintf(stderr, "Invalid command %c: ignored.\n", cmd);
		break;
	}
}


/*
===========================================================================
main関数
===========================================================================
*/

int main() {
	char line[MAX_LINE_LEN + 1];   //line:打ち込まれた文字列
	printf("If you don't know how to use this app, please input `%%H'\n");

	int t1, t2, dt;
	char hairetu[10000];

	t1 = clock();
	while (get_line(stdin, line)) {
		parse_line(line);
	}
	t2 = clock();
	dt = t2 - t1;

	printf("Processing time : %d[ms]\n", dt);

	printf("Size of profile_data_store is %d bite.\n", sizeof(profile_data_pnt));
	fgets(dummy, 5, stdin);

	return 0;
}


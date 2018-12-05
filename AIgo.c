#include <windows.h>
#include <stdio.h>
#include <conio.h>
#define up 'w'
#define left 'a'
#define down 's'
#define right 'd'
#define place 'p'
// w,a,s,d를 누르면 이동, p를 누르면 해당 위치에 둔다.
#define EMPTY 0
#define PLAYER 1
#define COMPUTER 2 
// 빈 위치는 0 ，Player의 위치는 1 ，컴퓨터의 위치는 2.
#define whitePlayer 1
#define blackPlayer 2
// player가 white면 1, black면 2
struct stu
{
	int x;
	int y;
} location;//좌표 구조체 정의

struct lim
{
	int x_lower;
	int x_upper;
	int y_lower;
	int y_upper;
} limited; // 탐색 범위 제한을 위한 구조체

int turn = 0; //turn 초기화, 0:player차례, 1: 컴퓨터 차레
int Q[19][19] = { 0 }; //게임 현황을 기록하기 위해 배열을 정의한다
int depth = 0; // 탐색 트리의 깊이

void gotoxy(int x, int y);//커서 위치 설정 함수
void drawBoard(); //board 초기화 및 출력 함수
void record(); //어떤 위치에 바둑돌을 두었으면 기록
void rmv(int x, int y); // 바둑돌 제거
void setlimit(); // 탐색 범위 제한 함수
int checkWin(); //승부 판단 함수
void move(int n, int *i, int *j); // n방향으로 위치(i,j)를 기준으로 이동
int type(int n, int p, int q); // 위치(p,q)가 n방향에 대한 type를 분석하고 return 하는 함수
int value(int p, int q); // 위치 (p,q)의 value를 계산하고 return하는 함수
int minimax(int depth, int turn, int alpha, int beta);
void AI(int colour);// 바둑판에 있는 모든 빈 위치의 value를 계산하고 value가 가장 큰 위치에 둔다.
void play(int colour); //게임 시작 함수

void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = 2 * x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}//커서 위치 설정 함수

void drawBoard() //board 초기화 및 출력
{
	int i, j;
	system("cls");
	for (i = 0; i<19; i++)
	{
		for (j = 0; j<19; j++)
		{
			Q[i][j] = 0;
			printf("十");
		}
		printf("\n");
	}
	location.x = 9; location.y = 9;
	gotoxy(location.x, location.y); //시작 위치 설정
}

void record()
{
	Q[location.x][location.y] = turn + 1;  //어떤 위치에 바둑돌을 두었으면 기록
	if (turn == 1) //player 전환
		turn = 0;
	else
		turn = 1;
}

void rmv(int x, int y) {
	location.x = x;
	location.y = y;
	Q[location.x][location.y] = 0; // 해당 위치의 바둑돌을 제거
	if (turn == 1) // record 함수로 돌을 놓으면서 전환된 player를 다시 되돌림
		turn = 0;
	else
		turn = 1;
}

void setlimit() {
	int i, j;
	int temp[4] = { -5 }; // temp[0] : x_lower, temp[1] : x_upper, temp[2] : y_lower, temp[3] : y_upper

	for (j = 0; j < 19; j++) {
		for (i = 0; i < 19; i++) {
			if (Q[i][j] != 0) {      // 해당 위치에 돌이 놓여있으면
				if (temp[0] == -5) {
					temp[0] = i - 2;
					temp[1] = i + 2;
					temp[2] = j - 2;
					temp[3] = j + 2; // 만약 초기 상태라면 돌 주변 2칸으로 탐색 범위를 제한함
				}
				else {
					if (temp[0] > i - 2)
						temp[0] = i - 2;
					if (temp[1] < i + 2)
						temp[1] = i + 2;
					if (temp[2] > j - 2)
						temp[2] = j - 2;
					if (temp[3] < j + 2)
						temp[3] = j + 2; // 돌이 놓임에 따라 탐색 범위를 갱신함
				}
			}
		}
	}
	
	if (temp[0] < 0)
		temp[0] = 0;
	if (temp[1] > 18)
		temp[1] = 18;
	if (temp[2] < 0)
		temp[2] = 0;
	if (temp[3] > 18)
		temp[3] = 18; // 바둑판 범위를 초과하면 범위 내로 들어오도록 조정

	limited.x_lower = temp[0];
	limited.x_upper = temp[1];
	limited.y_lower = temp[2];
	limited.y_upper = temp[3]; // 최종적으로 결정된 범위를 구조체에 저장
}

int checkWin() //승부 판단 함수
{
	int p;
	int r, c, rr, cc, count = 0;
	p = turn == 0 ? 2 : 1;
	for (c = 0; c<19; c++)
	{
		for (r = 0; r<19; r++)
		{
			if (Q[c][r] != p)
				continue;
			// '|' 방향 체크
			rr = r;
			cc = c;
			while (--cc >= 0 && Q[cc][rr] == p)
				count++;
			cc = c;
			while (++cc<19 && Q[cc][rr] == p)
				count++;
			cc = c;
			if (count + 1 >= 5)
				return p;
			// '—'방향 체크
			count = 0;
			while (--rr >= 0 && Q[cc][rr] == p)
				count++;
			rr = r;
			while (++rr<19 && Q[cc][rr] == p)
				count++;
			rr = r;
			if (count + 1 >= 5)
				return p;
			// '\'방향 체크
			count = 0;
			cc--;
			rr--;
			while ((cc >= 0 || rr >= 0) && Q[cc][rr] == p)
			{
				count++;
				cc--;
				rr--;
			}
			rr = r;
			cc = c;
			cc++;
			rr++;
			while ((cc<19 || rr<19) && Q[cc][rr] == p)
			{
				count++;
				cc++;
				rr++;
			}
			rr = r;
			cc = c;
			if (count + 1 >= 5)
				return p;
			// '/'방향 체크
			count = 0;
			cc++;
			rr--;
			while ((cc<19 || rr >= 0) && Q[cc][rr] == p)
			{
				count++;
				cc++;
				rr--;
			}
			rr = r;
			cc = c;
			cc--;
			rr++;
			while ((cc >= 0 || rr<19) && Q[cc][rr] == p)
			{
				count++;
				cc--;
				rr++;
			}
			rr = r;
			cc = c;
			if (count + 1 >= 5)
				return p;
			count = 0;
		}
	}
	return 0;
}

void move(int n, int *i, int *j)
{
	switch (n) //n는 방향 번호(총 8개 방향)
	{
	case 1: //→
		*i += 1;
		break;
	case 2: //↘
		*i += 1;
		*j += 1;
		break;
	case 3: //↓
		*j += 1;
		break;
	case 4: //↙
		*i -= 1;
		*j += 1;
		break;
	case 5: //←
		*i -= 1;
		break;
	case 6: //↖
		*i -= 1;
		*j -= 1;
		break;
	case 7: //↑
		*j -= 1;
		break;
	case 8: //↗
		*i += 1;
		*j -= 1;
		break;
	}
}
int type(int n, int p, int q) // □는 빈 위치, ┃는 벽, ○는 컴퓨터의 돌이 있는 위치, ●는 player의 돌이 있는 위치
{
	int k, m = 0;
	move(n, &p, &q);
	if (p<0 || p>18 || q<0 || q>18) // □→┃
		k = -2;
	switch (Q[p][q])
	{
	case COMPUTER:// □→○
	{
		m++;
		move(n, &p, &q);
		if (p < 0 || p>18 || q < 0 || q>18) // □→○┃
		{
			k = m + 9;
			return k;
		}
		while (Q[p][q] == COMPUTER) // □→○○, □→○○○, □→○○○○
		{
			m++;
			move(n, &p, &q);
			if (p < 0 || p>18 || q < 0 || q>18) // □→○○┃, □→○○○┃, □→○○○○┃
			{
				k = m + 9;
				return k;
			}
		}
		if (Q[p][q] == 0)
			k = m - 1; // □→○□, □→○○□, □→○○○□, □→○○○○□
		else
			k = m + 9; // □→○●, □→○○●, □→○○○●, □→○○○○●
	}
	break;
	case PLAYER: // □→●
	{
		m++;
		move(n, &p, &q);
		if (p < 0 || p>18 || q < 0 || q>18) // □→●┃
		{
			k = m + 109;
			return k;
		}
		while (Q[p][q] == PLAYER)// □→●●, □→●●●, □→●●●●
		{
			m++;
			move(n, &p, &q);
			if (p < 0 || p>18 || q < 0 || q>18)// □→●●┃ , // □→●●●┃ , // □→●●●●┃
			{
				k = m + 109;
				return k;
			}
		}
		if (Q[p][q] == EMPTY)
			k = m + 99; // □→●□, □→●●□, □→●●●□, □→●●●●□
		else
			k = m + 109; // □→●○, □→●●○, □→●●●○, □→●●●●○
	}
	break;
	case EMPTY:// □→□
	{
		move(n, &p, &q);
		if (p < 0 || p>18 || q < 0 || q>18)
		{
			k = -3; // □→□┃
			return k;
		}
		switch (Q[p][q])
		{
		case COMPUTER: // □→□○
		{
			m++;
			move(n, &p, &q);
			if (p < 0 || p>18 || q < 0 || q>18) // □→□○┃
			{
				k = m + 29;
				return k;
			}
			while (Q[p][q] == COMPUTER) // □→□○○ , □→□○○○, □→□○○○○
			{
				m++;
				move(n, &p, &q);
				if (p < 0 || p>18 || q < 0 || q>18) // □→□○○┃, □→□○○○┃, □→□○○○○┃ 
				{
					k = m + 29;
					return k;
				}
			}
			if (Q[p][q] == EMPTY)
				k = m + 19; // □→□○□, □→□○○□, □→□○○○□, □→□○○○○□ 
			else
				k = m + 29; // □→□○●, □→□○○●, □→□○○○●, □→□○○○○● 
		}
		break;
		case PLAYER: // □→□●
		{
			m++;
			move(n, &p, &q);
			if (p < 0 || p>18 || q < 0 || q>18) // □→□●┃
			{
				k = m + 129;
				return k;
			}
			while (Q[p][q] == PLAYER)// □→□●● , □→□●●●, □→□●●●●
			{
				m++;
				move(n, &p, &q);
				if (p < 0 || p>18 || q < 0 || q>18) // □→□●●┃, □→□●●●┃, □→□●●●●┃ 
				{
					k = m + 129;
					return k;
				}
			}
			if (Q[p][q] == EMPTY)
				k = m + 119; // □→□●□, □→□●●□, □→□●●●□, □→□●●●●□ 
			else
				k = m + 129; // □→□●○, □→□●●○, □→□●●●○, □→□●●●●○
		}
		break;
		case EMPTY:
			k = -1; // □→□□
			break;
		}
	}
	break;
	}
	return k;
}
int value(int p, int q)
{
	int n = 1, k = 0, k1, k2, K1, K2, X1, Y1, Z1, X2, Y2, Z2, temp;
	int a[2][4][4] = { 40,400,3000,10000,6,10,600,10000,20,120,200,0,6,10,500,0,30,300,2500,5000,2,8,300,8000,26,160,0,0,4,20,300,0 };
	// 배열 a는 총 32개 type의 값을 저장 
	while (n != 5)
	{
		k1 = type(n, p, q);
		n += 4;
		k2 = type(n, p, q);// k1,k2는 서로 반대인 2개 방향의 type 번호 
		n -= 3;
		if (k1>k2)
		{
			temp = k1;// 번호가 작으면→k1, 크면→k2 
			k1 = k2;
			k2 = temp;
		}
		K1 = k1;
		K2 = k2;// K1 K2는 k1 k2의 번호를 저장함
		Z1 = k1 % 10;
		Z2 = k2 % 10;
		k1 /= 10;
		k2 /= 10;
		Y1 = k1 % 10;
		Y2 = k2 % 10;
		k1 /= 10;
		k2 /= 10;
		X1 = k1 % 10;
		X2 = k2 % 10;
		/* 

		a[2][4][4] 대응 X , Y , Z
		X :  0 - 자신, 1 - 상대방
		Y :  0 - 2번 type, 1 - 3번 type , 2 - 4번 type, 3- 5번 type
		Z :  바둑돌 개수 0 - 1, 1 - 2, 2 - 3, 3 - 4

		*/
		if (K1 == -1)
		{// k1: □→□□ && k2: 기타
			if (K2<0)
			{
				k += 0;
				continue;
			}
			else
				k += a[X2][Y2][Z2] + 5;
			continue;
		};
		if (K1 == -2)
		{ // k1: □→┃ && k2: 기타
			if (K2<0)
			{
				k += 0;
				continue;
			}
			else
				k += a[X2][Y2][Z2] / 2;
			continue;
		};
		if (K1 == -3)
		{ //k1: □→□┃ && k2: 기타
			if (K2<0)
			{
				k += 0;
				continue;
			}
			else
				k += a[X2][Y2][Z2] / 3;
			continue;
		};

		if (((K1>-1 && K1<4) && ((K2>-1 && K2<4) || (K2>9 && K2<14))) || ((K1>99 && K1<104) && ((K2>99 && K2<104) || (K2>109 && K2<114))))
		{
			// 02 && 02 , 02 && 03, 12 && 12, 12 && 13 (첫번쨰 숫자는 0 - 자신, 1 - 상대방  두번째 숫자는 type번호)
			if (Z1 + Z2 >= 2)
			{
				k += a[X2][Y2][3];
				continue;
			}
			else
			{
				k += a[X2][Y2][Z1 + Z2 + 1];
				continue;
			}
		}
		if (((K1>9 && K1<14) && (K2>9 && K2<14)) || ((K1>109 && K1<114) && (K2>109 && K2<114)))
		{ //03 && 03, 13 && 13
			if (Z1 + Z2 >= 2)
			{
				k += 10000;
				continue;
			}
			else
			{
				k += 0;
				continue;
			}
		}
		if (((K1>-1 && K1<4) && ((K2>99 && K2<104) || (K2>109 && K2<114))) || ((K1>9 && K1<14) && ((K2>99 && K2<104) || (K2>109 && K2<114))))
		{ //02 && 12, 02 && 13, 03 && 12, 03 && 13
			if (Z1 == 3 || Z2 == 3)
			{
				k += 10000;
				continue;
			}
			else
			{
				k += a[X2][Y2][Z2] + a[X1][Y1][Z1] / 4;
				continue;
			}
		}
		else
		{ //기타
			k += a[X1][Y1][Z1] + a[X2][Y2][Z2];
			continue;
		}
	}
	return k;
}

int minimax(int depth, int turn, int alpha, int beta) {
	int e, i, j, n;
	int val = 0;

	if (depth == 0) {                   // 탐색 트리 끝까지 도달하면
		for (j = 0; j < 19; j++) {
			for (i = 0; i < 19; i++) {
				if (Q[i][j] == 0)
					val += value(i, j); // 각각의 빈 칸들이 가진 가치값을 전부 더한다.
			}
		}

		return val;
	}

	if (turn == 1) { // 플레이어 차례
		for (j = limited.y_lower; j <= limited.y_upper; j++)
		{
			for (i = limited.x_lower; i <= limited.x_upper; i++) // 제한된 범위 내에서 탐색한다
			{
				if (Q[i][j] == 0) {  // 빈 칸이면
					location.x = i;
					location.y = j;
					record();        // 돌을 놓아본다.
					e = checkWin();  // 승리조건을 만족하는지 체크한다.
					if (e == 1) {
						beta = -100000;
						rmv(i, j);   // 실제로 돌을 놓은 게 아니기 때문에 제거해준다.
						return beta; // 승리한다면 beta값을 리턴한다.
					}

					n = minimax(depth - 1, turn, alpha, beta); // 탐색 트리의 한 단계 밑으로 내려간다.

					if (n < beta)
						beta = n; // 최솟값이 되는 beta를 찾는다.

					if (beta <= alpha) {
						rmv(i, j);
						return beta; // alpha값보다 beta값이 작아지면 바로 beta 값을 리턴한다. (Alpha-Beta Pruning)
					}

					rmv(i, j);
				}
			}
		}
		return beta;
	}

	else { // AI 차례, 플레이어 차례일 때와 구조는 동일하나 여기서는 최댓값이 되는 alpha값을 찾는다.
		for (j = limited.y_lower; j <= limited.y_upper; j++)
		{
			for (i = limited.x_lower; i <= limited.x_upper; i++)
			{
				if (Q[i][j] == 0) {
					location.x = i;
					location.y = j;
					record();
					e = checkWin();
					if (e == 2) {
						alpha = 100000;
						rmv(i, j);
						return alpha;
					}

					n = minimax(depth - 1, turn, alpha, beta);

					if (n > alpha)
						alpha = n;

					if (beta <= alpha) {
						rmv(i, j);
						return alpha;
					}

					rmv(i, j);
				}
			}
		}
		return alpha;
	}

}

void AI(int colour)
{
	int max = 0;
	int val = -100001;
	int i, j, e, n, k;
	int I = -1;
	int J = -1;

	if (depth == 0) {
		for (j = 0; j < 19; j++)
		{
			for (i = 0; i < 19; i++)
			{
				if (Q[i][j] == 0)// 바둑판에 있는 모든 빈 위치의 value를 계산함
				{
					k = value(i, j);
					if (k >= max)
					{
						I = i;
						J = j;
						max = k;
					}
				}
			}
		}
	}

	else {
		setlimit(); // 탐색 범위를 제한한다.

		for (j = limited.y_lower; j <= limited.y_upper; j++)
		{
			for (i = limited.x_lower; i <= limited.x_upper; i++) // 제한된 범위 내에서 탐색
			{
				if (Q[i][j] == 0)   // 빈 칸이면
				{
					location.x = i;
					location.y = j;
					record();       // 돌을 놓아보고
					e = checkWin(); // 승리 여부를 따진다.
					if (e == 2) {
						I = i;
						J = j;
						rmv(i, j);
						break;      // 승리한다면 돌을 제거하고 해당 착수 위치를 다음에 둘 곳으로 지정한다.
					}
					n = minimax(depth - 1, turn, -100001, 100001); // min-max 알고리즘으로 다음 수를 찾는다.

					if (-n > val) {
						val = -n;
						I = i;
						J = j;  // val이 최댓값에 도달할 때의 착수 위치를 다음에 둘 곳으로 지정한다.
					}
					rmv(i, j);  // 실제로 둔 게 아니므로 돌을 제거해준다.
				}
			}
		}
	}
	location.x = I;
	location.y = J;
	gotoxy(location.x, location.y); // 다음에 둘 곳으로 지정된 곳으로 이동한다.
	record();
	if (colour == whitePlayer)
		printf("○");
	else
		printf("●");
	gotoxy(location.x, location.y); // 돌을 두고 커서를 옮긴다.
	}


void play(int colour)
{
	int temp = 0;
	char press;
	int winer = 0;
l:  drawBoard();
m:	press = getch();
	while (1)
	{
		switch (press)
		{
		case up:
			location.y--;
			gotoxy(location.x, location.y);
			break;
			//위로 이동
		case left:
			location.x--;
			gotoxy(location.x, location.y);
			break;
			//왼 쪽으로 이동
		case right:
			location.x++;
			gotoxy(location.x, location.y);
			break;
			//오른 쪽으로 이동
		case down:
			location.y++;
			gotoxy(location.x, location.y);
			break;
			//밑으로 이동
		case place:
			if (Q[location.x][location.y] == 0)
			{
				gotoxy(location.x, location.y);
				record();
				if (colour == whitePlayer)
					printf("●");
				else
					printf("○");
				gotoxy(location.x, location.y);
				break;
			}
			else
				goto m;
		}
		if (press == 'p')
		{
			winer = checkWin();
			if (colour == whitePlayer)
			{
				if (winer != 0)
				{
					gotoxy(0, 19);
					if (winer == 2)
						printf("the side of ○ wins\n");
					else if (winer == 1)
						printf("the side of ● wins\n");
				}
				else
				{
					AI(colour);
					winer = checkWin();
					if (winer != 0)
					{
						gotoxy(0, 19);
						if (winer == 2)
							printf("the side of ○ wins\n");
						else if (winer == 1)
							printf("the side of ● wins\n");
					}
					else temp = 1;
				}
			}
			else
			{
				if (winer != 0)
				{
					gotoxy(0, 19);
					if (winer == 2)
						printf("the side of ● wins\n");
					else if (winer == 1)
						printf("the side of ○ wins\n");
				}
				else
				{
					AI(colour);
					winer = checkWin();
					if (winer != 0)
					{
						gotoxy(0, 19);
						if (winer == 2)
							printf("the side of ● wins\n");
						else if (winer == 1)
							printf("the side of ○ wins\n");
					}
					else temp = 1;
				}
			}
		}
		if (press != 'p' || temp == 1)
		{
			press = getch();
			temp = 0;
			continue;
		}
		printf("do you want to play again with computer?(y/n):");
	p:      press = getch();
		if (press == 'n') {
			printf("\n");
			break;
		}
		else if (press == 'y')
		{
			winer = 0;
			goto l;
		}
		else
			goto p;
	}
}


void main()
{
	int choice1, choice2;
	printf("\t --------------------------------------------\n");
	printf("\t|                                             |\n");
	printf("\t|                                             |\n");
	printf("\t|                                             |\n");
	printf("\t|                    오 목                    |\n");
	printf("\t|                                             |\n");
	printf("\t|                                             |\n");
	printf("\t|                                             |\n");
	printf("\t|                                             |\n");
	printf("\t|           1. 게임 시작    2. 종료           |\n");
	printf("\t|                                             |\n");
	printf("\t|                                             |\n");
	printf("\t|                                             |\n");
	printf("\t ---------------------------------------------\n");
	printf("\t 1 혹은 2를 입력하세요: ");
	while (scanf_s("%d", &choice1), choice1 != 1 && choice1 != 2)
		printf("\t 1 혹은 2를 입력하세요：");
	system("cls");
	if (choice1 == 1) {
		printf("\n\n\n\t 1 혹은 2를 입력해서\n\t ● 혹은 ○ 를  선택하세요\n\t 1. ●  2. ○: ");
		while (scanf_s("%d", &choice2), choice2 != 1 && choice2 != 2)
			printf("\t 1 혹은 2를 입력하세요：");
		system("cls");
		if (choice2 == 1)
		{
			play(whitePlayer);
		}
		else
		{
			play(blackPlayer);

		}
	}
	else
		return;
}
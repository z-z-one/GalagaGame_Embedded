#include <stdio.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <asm/ioctls.h>
#include <math.h>

#define clcd "/dev/clcd"
#define dot "/dev/dot"
#define tact_d "/dev/tactsw"
#define fnd "/dev/fnd"

#define MAX_LEN 32

unsigned char blocks[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };//장애물
unsigned char spaceship[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x1C };//우주선
unsigned char missile[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };//발사체
unsigned char matrix[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };//최종 배열 <- 이걸 dotmatrix에 출력

int setMatrix(char d1[], char d2[], char d3[], int d); //장애물, 우주선, 발사체를 합해 dotmatrix에 출력
int Game(int play_time, int b_gen, int b_speed); //메인 게임
int setCLCD_Tact(); //게임 진행 선택
void setText(char data[]); //CLCD에 문구 출력
void ScoreToCLCD(); //점수 초기화
void setFnd(int k); //FND에 남은시간 출력
void init_matrix(); //스테이지

int HIGHT_SCORE = 0; //진행되는 게임의 최고점수
int CURRENT_SCORE = 0; //진행중인 게임의 점수


int main()
{
    int select, result, i;
    int time_list[3] = {10, 15, 20};  //스테이지별 시간
    int blocks_gen[3] = {3000000,2000000,1500000};
    int blocks_sp[3] = {900000,700000,500000};
    char text[MAX_LEN] = "Press 1 to StartPress 11 to End";
    setText(text);
    select = setCLCD_Tact();
    switch (select)
    {
        case 1:
        {
            CURRENT_SCORE = 0;
            for(i=0;i<(sizeof(time_list)/sizeof(time_list[0]));i++)  //배열 길이만큼 게임 반복
            {
                init_matrix();  //배열 초기상태로 초기화
                result = Game(time_list[i], blocks_gen[i], blocks_sp[i]);
                if(!result){
                    char text[MAX_LEN] = "  GAME OVER...  ";
                    setText(text);
                    usleep(5000000);
                    break;
                }
                else if(result == 2){
                    char text[MAX_LEN] = "   Game Quit    ";
                    setText(text);
                    return 0;
                }
                char text[MAX_LEN];
                sprintf(text, " STAGE %d CLEAR! ", i+1);
                setText(text);
                usleep(5000000);
            }
            if(result == 1)
            {
                char text[MAX_LEN] = "ALL STAGE CLEAR!";
                setText(text);
                return 0;
            }
            break;
        }
        case 11:
        {
            char text[MAX_LEN] = "   Game Quit    ";
            setText(text);
            return 0;
        }
    }
    while(1)  //11번 선택으로 Quit할때까지 반복
    {
        if (!result)
        {
            char text[MAX_LEN] = "Restart : 1     Quit    : 11";
            setText(text);
            select = setCLCD_Tact();
            switch (select)
            {
                case 1:
                {
                    CURRENT_SCORE = 0;
                    for(i=0;i<(sizeof(time_list)/sizeof(time_list[0]));i++)
                    {
                        init_matrix();
                        result = Game(time_list[i], blocks_gen[i], blocks_sp[i]);
                        if(!result){
                            char text[MAX_LEN] = "  GAME OVER...  ";
                            setText(text);
                            usleep(5000000);
                            break;
                        }
                        else if(result == 2){
                            char text[MAX_LEN] = "   Game Quit    ";
                            setText(text);
                            return 0;
                        }
                        char text[MAX_LEN];
                        sprintf(text, " STAGE %d CLEAR! ", i+1);
                        setText(text);
                        usleep(5000000);
                    }
                    if(result == 1)
                    {
                        char text[MAX_LEN] = "ALL STAGE CLEAR!";
                        setText(text);
                        return 0;
                    }
                    break;
                }
                case 11:
                {
                    char text[MAX_LEN] = "   Game Quit    ";
                    setText(text);
                    return 0;
                }
            }
        }
    }
}




int Game(int play_time, int b_gen, int b_speed) {
    struct timeval dotst, dotend, tactst, tactend, blockst, blockend, block_spst, block_spend, attackst, timest, timeend;
    int dot_d = 0;
    int tact = 0;
    unsigned char tact_select = 0;
    bool attack = false;
    int at, i;
    double block_p;    //블록 포지션 결정 변수
    int block_n;    //블록 길이 결정 변수

    ScoreToCLCD(); //현재 점수 데이터로 초기화

    srand(time(NULL));    //난수 생성

    gettimeofday(&blockst, NULL);  //장애물 생성 속도 조절 관련
    gettimeofday(&block_spst, NULL);  //장애물 내려오는 속도 조절 관련
    gettimeofday(&dotst, NULL);  //dotmatrix 출력 시간 조절 관련
    gettimeofday(&timest, NULL);  //전체 게임 진행 시간 조절 관련
    
    setFnd(play_time);  //게임 시작 시 남은 시간 출력

    while (1)
    {
        if (dot_d == 0)    //dot matrix에 접근하지 않은 경우만 open
        {
            dot_d = open(dot, O_RDWR);
        }
        gettimeofday(&dotend, NULL);  //dotmatrix 출력 시간 조절 관련
        gettimeofday(&blockend, NULL);  //장애물 생성 속도 조절 관련

        if (!setMatrix(blocks, spaceship, missile, dot_d))  //dotmatrix에 장애물, 우주선, 발사체 출력 및 충돌 판정
        {
            return 0;
        }

        if (attack)   //발사체 발사 부분, 0.1초마다 이동
        {
            if (abs(dotend.tv_usec - attackst.tv_usec) > 100000)
            {
                if (at > -1)
                {
                    gettimeofday(&attackst, NULL);  //발사체 속도 조절 관련
                    missile[at] = missile[at + 1];  //발사체가 날아가는 부분
                    missile[at + 1] = 0;
                    at--;
                    if (at == -1)  //끝까지 날아간 경우 삭제
                    {
                        missile[at + 1] = 0;
                        attack = false;
                    }
                }
            }
        } 

        //------------------------------------------------------------장애물------------------------------------------------------
        if ((blockend.tv_usec + b_gen - blockst.tv_usec > b_gen) && ((blockend.tv_sec - blockst.tv_sec) == (b_gen/1000000)))  //장애물 생성
        {
            block_n = rand() % 2;  //장애물 길이 2,3 랜덤 생성
            if (block_n == 0)
            {
                block_p = rand() % 6;     //블록 생성 위치 결정
                blocks[0] = pow(2.0, block_p) + pow(2.0, block_p + 1.0f);    //정해진 위치에 블록 생성
            }
            else
            {
                block_p = rand() % 5;
                blocks[0] = pow(2.0, block_p) + pow(2.0, block_p + 1.0f) + pow(2.0, block_p + 2.0f);
            }
            gettimeofday(&blockst, NULL);
        }
        
        gettimeofday(&block_spend, NULL);  //장애물 내려오는 속도 관련
        if ((block_spend.tv_usec - block_spst.tv_usec > b_speed) || (block_spend.tv_sec > block_spst.tv_sec && (block_spend.tv_usec + 1000000 - block_spst.tv_usec > b_speed)))
        {
            if (!setMatrix(blocks, spaceship, missile, dot_d))  //내려올 때마다 datmatrix에 출력 및 충돌 판정
            {
                return 0;
            }
            for (i = 6; i > -1; i--)     //떨어지는 모습 구현
            {
                blocks[i + 1] = blocks[i];
            }
            if (blocks[0] > 0)     //장애물 생성 전 기존 장애물 삭제
            {
                blocks[0] = 0;
            }
            gettimeofday(&block_spst, NULL);
        }
        //------------------------------------------------------------장애물 끝-------------------------------------------------------

        //dot matrix 접근 시간 제한, 제한한 시간 동안 tact switch와 fnd에 접근 동작
        if ((dotend.tv_usec - dotst.tv_usec > 100000) || (dotend.tv_sec > dotst.tv_sec && (dotend.tv_usec + 1000000 - dotst.tv_usec > 100000)))
        {
            dot_d = close(dot_d);
            if (tact == 0)     //tact switch에 접근하지 않은 경우만 open
            {
                tact = open(tact_d, O_RDWR);
            }
            read(tact, &tact_select, sizeof(tact_select));     //tact switch에 접근해 있는 동안 입력받음

            switch (tact_select)
            {
                case 4:
                    if (spaceship[6] != 0x40)    //4번 버튼 입력 시 왼쪽으로 우주선 이동
                    {
                        spaceship[6] = spaceship[6] << 1;
                        spaceship[7] = spaceship[7] << 1;
                    }
                    break;

                case 5:
                    if (!attack)  //5번 버튼 입력 시 발사체 발사
                    {
                        attack = true;
                        missile[5] = spaceship[6];  //우주선 머리 부분에서 부터 발사
                        at = 4;
                        gettimeofday(&attackst, NULL);
                    }
                    break;

                case 6:
                    if (spaceship[6] != 0x02)   //6번 버튼 입력 시 오른쪽으로 우주선 이동
                    {
                        spaceship[6] = spaceship[6] >> 1;
                        spaceship[7] = spaceship[7] >> 1;
                    }
                    break;

                case 11:    //11번 버튼 입력시 중지
                    tact = close(tact);
                    return 2;
            }
            tact = close(tact);
            setFnd(play_time);  //남은 시간 출력
            gettimeofday(&dotst, NULL);
        }
        gettimeofday(&timeend,NULL);
        //게임 진행 시간이 1초 지날 때 마다 play_time 초기화
        if ((timeend.tv_usec + 1000000 - timest.tv_usec > 1000000) && ((timeend.tv_sec-timest.tv_sec)==1))
        {
            play_time--;
            if(play_time == 0)  //해당 스테이지 클리어
            {
                dot_d = close(dot_d);
                return 1;
            }
            gettimeofday(&timest,NULL);
        }
    }
}

int setMatrix(char d1[], char d2[], char d3[], int d) {    //d1:장애물 d2:우주선 d3:발사체
    int h;
    int b = 1;
    for (h = 0; h < 8; h++)
    {
        if ((d3[h] & d1[h]) > 0)     //충돌처리 - 발사체와 장애물
        {
            d3[h] = 0;
            d1[h] = 0;
            CURRENT_SCORE +=10;  //발사체로 장애물 부수면 점수 추가
            d = close(d);
            ScoreToCLCD();  //점수 초기화
            d = open(dot, O_RDWR);
        }
        if ((d2[h] & d1[h]) > 0)    //충돌처리 - 우주선과 장애물
        {
            b = 0;
            CURRENT_SCORE = 0;  //게임 오버 시 현재 점수 0점으로 초기화
            close(d);
            return b;
        }
        matrix[h] = d1[h] + d2[h] + d3[h];  //matrix 배열에 3가지 요소 합함
    }
    write(d, &matrix, sizeof(matrix));   //dotmatrix에 출력
    return b;
}

int setCLCD_Tact() {
    int tact;
    char s;
    tact = open(tact_d, O_RDWR);
    int dot_d = 0;
    while (1)  //tact switch 입력 받을 때 까지 대기
    {
        read(tact, &s, sizeof(s));
        if (s){
            break;
        }
    }
    close(tact);
    return s;
}

void setText(char data[]) {
    int clcd_d;
    clcd_d = open(clcd, O_RDWR);
    write(clcd_d, data, 32);  //입력받은 data 문자열 CLCD에 출력
    close(clcd_d);
}

void ScoreToCLCD() {
    int clcd_d;
    char score[MAX_LEN];
    if(CURRENT_SCORE>HIGHT_SCORE){   //현재 점수가 최고 점수보다 높아지면 최고 점수 초기화
        HIGHT_SCORE = CURRENT_SCORE;
    }
    sprintf(score, "High Score:%4d Score     :%4d", HIGHT_SCORE, CURRENT_SCORE);
    clcd_d = open(clcd, O_RDWR);
    write(clcd_d, score, 32);  //점수 CLCD에 출력
    close(clcd_d);
}

void setFnd(int k) {
    int fnd_d, ten_num, one_num;
    unsigned char fnd_list[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};  //0~9표현 16진수
    unsigned char fnd_data[4] = {0xC0,0xC0,0xC0,0xC0};  //fnd에 출력할 배열
    if(k<10){  //남은시간 10의 자리와 1의 자리 구하기
        ten_num = 0;
    }
    else{
        ten_num = k / 10;
    }
    if(k<1){
        one_num = 0;
    }
    else{
        one_num = k % 10;
    }
    fnd_data[2] = fnd_list[ten_num];
    fnd_data[3] = fnd_list[one_num];
    fnd_d = open(fnd,O_RDWR);
    write(fnd_d,fnd_data,4);  //0.1초간 fnd에 남은시간 출력
    usleep(100000);
    close(fnd_d);
}

void init_matrix() {  //dotmatrix에 출력되는 배열 초기상태로 초기화
    int i;
    for(i=0;i<8;i++)
    {
        blocks[i] = 0x00;
        missile[i] = 0x00;
        matrix[i] = 0x00;
        if(i==6){
            spaceship[i] = 0x08;
        }
        else if(i==7){
            spaceship[i] = 0x1C;
        }
        else{
            spaceship[i] = 0x00;
        }
    }
}
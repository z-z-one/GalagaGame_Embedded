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

#define ONE 0xF9
#define TWO 0xA4
#define THREE 0xB0
#define FOUR 0x99
#define FIVE 0x92
#define SIX 0x82
#define SEVEN 0xF8
#define EIGHT 0x80
#define NINE 0x90
#define ZERO 0xC0

#define dot "/dev/dot"
#define tact_d "/dev/tactsw"
#define fnd "/dev/fnd"

unsigned char blocks[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };//장애물
unsigned char spaceship[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x1C };//우주선
unsigned char missile[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
unsigned char matrix[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };//최종 배열 <- 이걸 dotmatrix에 출력

//최종배열을 만드는 함수
int setMatrix(char d1[], char d2[], char d3[], int d)   //d1이 장애물
{
    //matrix배열에 인자로받은 두 배열 합치기
    int h;
    int b = 1;
    for (h = 0; h < 8; h++)
    {
        if((d3[h]&d1[h])>0)     //충돌처리
        {
            d3[h] = 0;
            d1[h] = 0;
        }
        if((d2[h]&d1[h])>0)
        {
            b = false;
            break;
        }
        matrix[h] = d1[h] + d2[h] + d3[h];
    }
    write(d, &matrix, sizeof(matrix));
    return b;  
}

int main()
{
    struct timeval dotst, dotend, tactst, tactend, blockst, blockend, block_spst, block_spend, attackst;
    int dot_d = 0;
    int tact = 0;
    int fnd_d = 0;
    unsigned char t = 0;
    bool b = true;
    bool attack = false;
    int at, k;
    double block_p;    //블록 포지션 결정 변수
    int block_n, i;    //블록 길이 결정 변수

    k = 10;
    srand(time(NULL));    //난수 생성

    gettimeofday(&blockst, NULL);
    gettimeofday(&block_spst, NULL);
    gettimeofday(&dotst, NULL);
    gettimeofday(&timest, NULL);
    while (b)
    {
        if (dot_d == 0)    //dot matrix에 접근하지 않은 경우만 open
        {
            dot_d = open(dot, O_RDWR);
        }
        gettimeofday(&dotend, NULL);
        gettimeofday(&blockend, NULL);

        if(!setMatrix(blocks, spaceship, missile, dot_d))
        {
            break;
        }
        
        if(attack)   //미사일 발사 부분, 0.1초마다 이동
        {
            if(abs(dotend.tv_usec-attackst.tv_usec)>100000)
            {
                if(at>-1)
                {
                    gettimeofday(&attackst, NULL);
                    missile[at] = missile[at+1];
                    missile[at+1] = 0;
                    
                    at--;
                    if(at==-1)
                    {
                        missile[at+1] = 0;
                        attack = false;
                    }
                }
            }
        }

        //------------------------------------------------------------장애물------------------------------------------------------
        if ((blockend.tv_usec + 2000000 - blockst.tv_usec > 2000000) && ((blockend.tv_sec-blockst.tv_sec)==2))
        {
            block_n = rand() % 2;
            if (block_n == 0)    //블록길이 2
            {
                block_p = rand() % 6;     //블록 생성 위치 결정
                blocks[0] = pow(2.0, block_p) + pow(2.0, block_p + 1.0f);    //정해진 위치에 블록 생성
            }
            else    //블록길이 3
            {
                block_p = rand() % 5;
                blocks[0] = pow(2.0, block_p) + pow(2.0, block_p + 1.0f) + pow(2.0, block_p + 2.0f);
            }
            gettimeofday(&blockst, NULL);
        }
        gettimeofday(&block_spend, NULL);
        if ((block_spend.tv_usec - block_spst.tv_usec > 800000) || (block_spend.tv_sec > block_spst.tv_sec && (block_spend.tv_usec + 1000000 - block_spst.tv_usec > 800000)))
        {
            if(!setMatrix(blocks, spaceship, missile, dot_d))
            {
                break;
            }
            for (i = 6; i > -1; i--)     //떨어지는 모습 구현
            {
                blocks[i + 1] = blocks[i];
            }
            if (blocks[0] > 0)     //블록 생성 전 삭제
            {
                blocks[0] = 0;
            }
            gettimeofday(&block_spst, NULL);
        }

        //------------------------------------------------------------장애물 끝-------------------------------------------------------


        //dot matrix와 tack switch를 0.2초마다 번갈아가면서 접근
        if ((dotend.tv_usec - dotst.tv_usec > 200000) || (dotend.tv_sec > dotst.tv_sec && (dotend.tv_usec + 1000000 - dotst.tv_usec > 200000)))
        {
            dot_d = close(dot_d);
            if (tact == 0)     //tact switch에 접근하지 않은 경우만 open
            {
                tact = open(tact_d, O_RDWR);
            }
            gettimeofday(&tactst, NULL);

            while (1)
            {
                gettimeofday(&tactend, NULL);
                read(tact, &t, sizeof(t));     //tact switch에 0.2초간 접근해있는 동안 입력받음

                switch (t)
                {
                    case 4:
                        if (spaceship[6] != 0x40)    //4번 버튼 입력시 왼쪽으로 우주선 이동
                        {
                            spaceship[6] = spaceship[6] << 1;
                            spaceship[7] = spaceship[7] << 1;
                        }
                        break;
                    
                    case 5:
                        if(!attack)
                        {
                            attack = true;
                            missile[5] = spaceship[6];
                            at = 4;
                            gettimeofday(&attackst,NULL);
                        }
                        break;

                    case 6:
                        if (spaceship[6] != 0x02)   //6번 버튼 입력시 오른쪽으로 우주선 이동
                        {
                            spaceship[6] = spaceship[6] >> 1;
                            spaceship[7] = spaceship[7] >> 1;
                        }
                        break;

                    case 11:    //11번 버튼 입력시 중지
                        b = false;
                        break;
                }

                //0.2초 경과 or tact switch 입력이 있는 경우 tact switch에 접근 해제
                if ((tactend.tv_usec - tactst.tv_usec > 200000) || (tactend.tv_sec > tactst.tv_sec && (tactend.tv_usec + 1000000 - tactst.tv_usec > 200000)) || t)
                {
                    tact = close(tact);
                    break;
                }
            }
            gettimeofday(&dotst, NULL);
        }        
    }
}

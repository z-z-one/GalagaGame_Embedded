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

#define dot "/dev/dot"


int main()
{
    struct timeval blockst, blockend, block_spst, block_spend;    //gettimeofday 사용하기 위한 구조체
    double block_p;    //블록 포지션 결정 변수
    int block_n, i;    //블록 길이 결정 변수
    int dot_d = 0;
    srand(time(NULL));    //난수 생성
    unsigned char c[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};    //dot matrix 배열
    dot_d = open(dot, O_RDWR);

    gettimeofday(&blockst, NULL);
    while(1)
    {
        gettimeofday(&blockend, NULL);
        if (blockend.tv_usec+1000000 - blockst.tv_usec > 1000000)     //밀리세컨드 단위, 1초 후 블록 생성(현재 상태에서는 제대로 동작x)
        {
            block_n = rand() % 2;
            if (block_n == 0)    //블록길이 2
            {
                block_p = rand() % 6;     //블록 생성 위치 결정
                for(i=0; i<8; i++)     //맨 윗줄부터 아래로 떨어지는 블록 구현
                {
                    gettimeofday(&block_spst, NULL);
                    while(1)
                    {
                        gettimeofday(&block_spend, NULL);
                        //0.5초마다 아래로 떨어짐(현재 시간 기준으로 측정하기 때문에 2.9초에서 3.4초로 측정된 경우도 고려)
                        if ((block_spend.tv_usec - block_spst.tv_usec > 500000) || (block_spend.tv_sec>block_spst.tv_sec && (block_spend.tv_usec+1000000-block_spst.tv_usec > 500000)))
                        {
                            c[i] = pow(2.0, block_p) + pow(2.0, block_p+1.0f);    //정해진 위치에 블록 생성
                            write(dot_d,&c,sizeof(c));
                            c[i] = 0;    //블록이 지나간 줄 초기화
                            break;
                        }
                    }
                }
            }
            else
            {
                block_p = rand() % 5;
                for(i=0; i<8; i++)
                {
                    gettimeofday(&block_spst, NULL);
                    while(1)
                    {
                        gettimeofday(&block_spend, NULL);
                        if ((block_spend.tv_usec - block_spst.tv_usec > 500000) || (block_spend.tv_sec>block_spst.tv_sec && (block_spend.tv_usec+1000000-block_spst.tv_usec > 500000)))
                        {
                            c[i] = pow(2.0, block_p) + pow(2.0, block_p+1.0f) + pow(2.0, block_p+2.0f);
                            write(dot_d,&c,sizeof(c));
                            c[i] = 0;
                            break;
                        }
                    }
                }
            }
            gettimeofday(&blockst, NULL);
        }
    }
}

#include <stdio.h>
#include <sys/time.h>
#include <stdbool.h>
#include <conio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <asm/ioctls.h>

#define dot "/dev/dot"
#define tact_d "/dev/tactsw"


int main()
{
    struct timeval dotst, dotend, tactst, tactend;
    int dot_d = 0;
    int tact = 0;
    unsigned char c[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x1C};     //우주선 시작 위치
    unsigned char t = 0;
    bool b = true;
    int a;

    gettimeofday(&dotst, NULL);
    while(b)
    {
        if (dot_d == 0)    //dot matrix에 접근하지 않은 경우만 open
        {
            dot_d = open(dot, O_RDWR);
        }
        gettimeofday(&dotend, NULL);
        write(dot_d,&c,sizeof(c));     //우주선 출력
        //dot matrix와 tack switch를 0.2초마다 번갈아가면서 접근
        if((dotend.tv_usec - dotst.tv_usec > 200000) || (dotend.tv_sec>dotst.tv_sec && (dotend.tv_usec+1000000-dotst.tv_usec > 200000)))
        {
            dot_d = close(dot_d);
            if (tact == 0)     //tact switch에 접근하지 않은 경우만 open
            {
                tact = open(tact_d,O_RDWR);
            }
            gettimeofday(&tactst, NULL);
            while(1)
            {
                gettimeofday(&tactend, NULL);
                read(tact,&t,sizeof(t));     //tact switch에 0.2초간 접근해있는 동안 입력받음
                switch(t)
                {
                    case 4:
                        if (c[6] != 0x40)    //4번 버튼 입력시 왼쪽으로 우주선 이동
                        {
                            c[6] = c[6]<<1;
                            c[7] = c[7]<<1;
                        }
                        break;
                    
                    case 6:
                        if (c[6] != 0x02)   //6번 버튼 입력시 오른쪽으로 우주선 이동
                        {
                            c[6] = c[6]>>1;
                            c[7] = c[7]>>1;
                        }
                        break;
                        
                    case 11:    //11번 버튼 입력시 중지
                        b = false;
                        break;
                }
                //0.2초 경과 or tact switch 입력이 있는 경우 tact switch에 접근 해제
                if((tactend.tv_usec - tactst.tv_usec > 200000) || (tactend.tv_sec>tactst.tv_sec && (tactend.tv_usec+1000000-tactst.tv_usec > 200000)) || t)
                {
                    tact = close(tact);
                    break;
                }
            }
            gettimeofday(&dotst, NULL);
        }
    }
}

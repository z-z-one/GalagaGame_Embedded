# 22.06.07 4차 발표
### 발표자 : 김민규
## 진행상황
* Dot Matrix
  - 우주선 - 좌우이동, 발사체 발사
  - 장애물 - 랜덤으로 여러개 생성
* Tact Switch
  - 우주선 이동, 미사일 발사
* FND
  - 현재 스테이지 남은시간 출력
* CLCD
  - 현재점수
  - 최고점수
  - 게임진행메시지 : 게임시작, 게임오버, 다시시작, 게임종료, 스테이지, 재시작
* 게임 진행
  - 난이도 조절 : 장애물 생성주기, 장애물 내려오는 속도, 스테이지별 시간

![Pic](./pic/message.png)

#### FND출력함수
``` C
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
```

#### CLCD출력함수
``` C
void setText(char data[]) {
    int clcd_d;
    clcd_d = open(clcd, O_RDWR);
    write(clcd_d, data, 32);  //입력받은 data 문자열 CLCD에 출력
    close(clcd_d);
}
```

#### 게임진행을 위한 case문
``` C
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
```

## 참고문헌
[FND 조작 참고문헌](https://comonyo.tistory.com/7)

[FND 16진수 참고문헌](https://zmade.tistory.com/18)

[CLCD 조작 참고문헌](https://cccding.tistory.com/67)

[sprintf 함수 참고문헌](https://jhnyang.tistory.com/314)

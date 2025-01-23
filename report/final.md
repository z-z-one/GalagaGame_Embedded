# 22.06.09 최종 발표
### 발표자 : 김민규

## 게임설명
* 플레이 방식
  - 좌우로 이동 가능한 우주선을 조작해 장애물 회피&미사일 발사로 파괴
* 게임 진행
  - 장애물 파괴 시 점수 증가
  - 우주선과 장애물 충돌 시 게임 오버
  - 각 스테이지마다 시간이 존재
  - 해당 시간동안 생존 시 다음 스테이지 진출
<br><br>

## 개발 과정
### 2주차
#### 1) 1주차에 구현계획한 기능
* Dot Matrix
  - 우주선 - 좌우이동
  - 장애물 - 랜덤으로 하나씩 생성
* Tact Switch
  - 우주선 이동

#### 2) 문제상황
우주선 이동 구현 중 여러 장치에 동시 접근이 불가능하다는 것을 알게 됨
 
#### 3) 해결한 방법
기존 프로젝트들의 코드를 살펴보니 장치마다 sleep함수로 딜레이를 주며 번갈아 접근하는 것을 알게 됨

하지만 sleep함수는 우리 게임 진행에 적합하지 않아 다른 방식으로 딜레이를 주는 방법을 찾다가 검색을 통해 gettimeofday함수를 알게 됨

이 후 코드 진행 중 시간 제한이 필요한 경우 대부분 gettimeofday함수를 이용하였음

#### 4) 핵심 코드
``` C
    gettimeofday(&dotst, NULL);
    while(b)
    {
        if (dot_d == 0)    //dot matrix에 접근하지 않은 경우만 open
        {
            dot_d = open(dot, O_RDWR);
        }
        
        gettimeofday(&dotend, NULL);
        
        write(dot_d,&c,sizeof(c));     //dot matrix 출력
        
        /***dot matrix와 tack switch를 0.2초마다 번갈아가면서 접근***/
        if((dotend.tv_usec - dotst.tv_usec > 200000) || (dotend.tv_sec>dotst.tv_sec && (dotend.tv_usec+1000000-dotst.tv_usec > 200000)))
        {
            close(dot_d)
            
            /**** tact switch 동작 부분 ****/
            
            gettimeofday(&dotst, NULL);
        }
    }
```
<br>

### 3주차
#### 1) 2주차에 구현계획한 기능 
* Dot Matrix
  - 장애물 - 랜덤으로 여러개 생성 
  - 미사일 - 장애물과 충돌시 삭제 
  - 우주선과 장애물 동시출력
* Tact Switch
  - 미사일 발사

#### 2) 문제상황
2주차에 따로 구현했던 장애물과 우주선, 미사일을 Dot Matrix에 동시출력하는데 어려움을 겪음 

#### 3) 해결한 방법
우주선, 장애물, 미사일을 각각의 배열로 만든 후 하나의 배열로 합쳐서 출력

#### 4) 핵심 코드
``` C
unsigned char blocks[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };//장애물
unsigned char spaceship[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x1C };//우주선
unsigned char missile[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };//미사일
unsigned char matrix[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };//최종 배열 <- 이걸 dotmatrix에 출력

int setMatrix(char d1[], char d2[], char d3[], int d)   //d1:장애물 d2:우주선 d3:미사일
{
    int h;
    int b = 1;
    for (h = 0; h < 8; h++)
    {
        if((d3[h]&d1[h])>0)     //비트연산으로 충돌여부 판단
        {
            d3[h] = 0;
            d1[h] = 0;
        }
        if((d2[h]&d1[h])>0)
        {
            b = 0;
            break;
        }
        matrix[h] = d1[h] + d2[h] + d3[h];
    }
    write(d, &matrix, sizeof(matrix));
    return b;   //장애물과 우주선이 충돌하면 false리턴으로 게임 종료
}
```
<br>

### 4주차
#### 1) 3주차에 구현계획한 기능
* FND
  - 현재 스테이지 남은시간 출력
* CLCD
  - 최고점수, 현재점수
  - 게임진행메시지 : 게임시작, 게임오버, 다시시작, 게임종료, 스테이지, 재시작
* 게임 진행
  - 난이도 조절 : 장애물 생성주기, 장애물 내려오는 속도, 스테이지별 시간


#### 2) 핵심 코드
- FND출력함수
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

- CLCD출력함수
``` C
void setText(char data[]) {
    int clcd_d;
    clcd_d = open(clcd, O_RDWR);
    write(clcd_d, data, 32);  //입력받은 data 문자열 CLCD에 출력
    close(clcd_d);
}
```
  
- 게임진행을 위한 case문
``` C
case 1:
    {
        CURRENT_SCORE = 0;
        for(i=0;i<(sizeof(time_list)/sizeof(time_list[0]));i++)    //스테이지 수 만큼 Game함수 반복하기 위함
        {
            init_matrix();
            result = Game(time_list[i], blocks_gen[i], blocks_sp[i]);  //클리어:1, 게임오버:0, 11번 스위치:2
            
            if(!result){       //게임오버한 상황
                char text[MAX_LEN] = "  GAME OVER...  ";
                setText(text);
                usleep(5000000);
                break;
            }
            else if(result == 2){       //게임진행 중 11번 스위치 입력한 상황
                char text[MAX_LEN] = "   Game Quit    ";
                setText(text);
                return 0;
            }
            char text[MAX_LEN];
            sprintf(text, " STAGE %d CLEAR! ", i+1);     //스테이지 클리어한 상황
            setText(text);
            usleep(5000000);
        }
        if(result == 1)    //result가 1인 상태로 for문을 빠져나오면 모든 스테이지 클리어
        {
            char text[MAX_LEN] = "ALL STAGE CLEAR!";
            setText(text);
            return 0;
        }
        break;
    }
```
<br><br>

## 독창성
- 기존에 있던 임베디드 시스템 프로젝트중 참고할 유사한 프로젝트가 없었음
- 장치 출력&동작방법, gettimeofday와 같은 함수 사용법만 참고하여 직접 코드 구현

## 참고문헌
[Snake 게임](https://github.com/jinwoo1225/SnakeGameWithSmart4412)

[테트리스 게임](https://github.com/raekim/embedded_tetris)

[dot matrix 조작](https://comonyo.tistory.com/16)

[tact switch 조작](https://hongci.tistory.com/85)

[gettimeofday 함수](https://bywords.tistory.com/entry/CLinux-gettimeofday%EB%A1%9C-%EB%A7%88%EC%9D%B4%ED%81%AC%EB%A1%9C%EC%B4%88-%EB%8B%A8%EC%9C%84-%EC%B8%A1%EC%A0%95%ED%95%98%EA%B8%B0)

[FND 조작](https://comonyo.tistory.com/7)

[FND 16진수](https://zmade.tistory.com/18)

[CLCD 조작](https://cccding.tistory.com/67)

[sprintf 함수](https://jhnyang.tistory.com/314)

## 실행 영상
[Galaga 게임](https://www.youtube.com/watch?v=yWGgyCQRmjs)



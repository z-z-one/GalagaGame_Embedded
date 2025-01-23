# 22.05.31 3차 발표
### 발표자 : 장지원
## 진행상황
* Dot Matrix
  - 우주선 - 좌우이동, 발사체 발사
  - 장애물 - 랜덤으로 여러개 생성
* Tact Switch
  - 우주선 이동, 미사일 발사

![Pic](./pic/dot_matrix.gif)

저번주에 구현 했던 우주선과 장애물, 그리고 이번에 새로 구현한 발사체를 각각의 배열로 만들었다. 그리고 3개의 배열을 하나의 배열로 합친 후 출력하는 함수를 작성하여 한번에 출력하도록 구현했다.

``` C
int setMatrix(char d1[], char d2[], char d3[], int d)   //d1:장애물 d2:우주선 d3:발사체
{
    //matrix배열에 인자로받은 두 배열 합치기
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
            b = false;
            break;
        }
        matrix[h] = d1[h] + d2[h] + d3[h];
    }
    write(d, &matrix, sizeof(matrix));
    return b;   //장애물과 우주선이 충돌하면 false리턴으로 게임 종료
}
```

## 다음 발표까지 진행목표 
- FND
  - 현재 스테이지 남은 시간 출력
- CLCD
  - 점수, 게임 진행 안내 출력
- 게임진행
  - 난이도 조절

# Embedded_rhythm_game

[![preview](https://img.youtube.com/vi/selpMuCNNnA/0.jpg)](https://youtu.be/selpMuCNNnA?t=0s)
[영상입니다. 누르면 유튜브로 연결됩니다.]

## 1. 프로젝트 주제 : 미니 리듬 게임

## 2. 개요

- 사용 부품 : Dot Matrix LED, Text LCD, Push Switch, Buzzer

![1](https://user-images.githubusercontent.com/46732674/106379485-b500e380-63ef-11eb-9404-7dc3ea0d0b92.PNG)

- Buzzer에서 노래가 재생되고, 노래의 박자에 맞게 Dot Matrix 의 상단에서 내려오는 *노트(LED)가 하단으로 사라지기 직전에 해당 위치의 Push Switch를 눌러야 하는 리듬 게임. Text LCD는 게임의 시작과 끝을 알리고, 현재 재생중인 노래와 점수가 나타난다. 

* 노트 : 리듬게임에서 타이밍에 맞추어 키보드나 버튼을 누르는 등의 행동으로 점수를 얻는 주요 구성 요소

## 3. 구체적인 동작 설명

* Dot Matrix에서 빨간색으로 칠한 부분은 실제로 LED가 들어오는 부분이고, 그 외에 색을 입힌 부분은 강조하기 위해 칠한 부분이다.

![2](https://user-images.githubusercontent.com/46732674/106379486-b6321080-63ef-11eb-928b-a7643b13d9dc.PNG)

- 기본 화면 : Switch의 맨 위 세 버튼중 왼쪽, 오른쪽 버튼으로 음악을 고르고 가운데 버튼으로 SELECT를 하면 게임이 시작된다.

![3](https://user-images.githubusercontent.com/46732674/106379487-b6caa700-63ef-11eb-91d4-054d741ec04e.PNG)

- 게임 진행 중 화면 : Dot Matrix 상단에서 노트(LED)가 떨어진다. 노트가 위에서 아래로 한칸씩 내려와 점점 하단으로 향하면서, 매트릭스 맨 밑 칸에 다다르게 되는데, 이 때 각각 칸에 대응하는 버튼을 누르면 점수를 얻게 된다.

![4](https://user-images.githubusercontent.com/46732674/106379489-b7633d80-63ef-11eb-82a0-d0bdc33df290.PNG)

- 게임 종료 후 화면 : 최고 기록과 나의 기록이 Text LCD에 표시되면서 게임이 종료된다. 9개의 버튼 중 아무 버튼이나 누르면 다시 기본화면으로 돌아간다.

## 4. 프로그래밍 계획

-  Buzzer로 음악 재생하기

![5](https://user-images.githubusercontent.com/46732674/106379490-b7633d80-63ef-11eb-9590-573009e5383f.PNG)

해당 Buzzer의 회로도를 살펴보면, Piezo Buzzer임을 확인할 수 있다. 신호의 주파수를 조절하여 원하는 음을 표현할 것인데, Buzzer의 Beep음 한계상 간단한 멜로디만 연주 할 계획이다.

- 음악에 맞추어 Dot Matrix에 노트 표현하기

![6](https://user-images.githubusercontent.com/46732674/106379491-b7fbd400-63ef-11eb-9951-8568d6294a30.PNG)

Dot Matrix의 LED를 ON 하는 것은 간단한 GPIO 작업이지만, 리듬게임 특성상 음악의 박자에 맞추어 노트를 등장시켜야 한다. 현재 음악과 노트를 자동으로 싱크 하는 것은 어려우므로, 음악을 우선 만들고, 음악에 맞게 노트를 직접 일일이 생성해야 한다. 노트가 Matrix의 상단에 출현해서, 최 하단까지 도달하려면 총 10칸을 거쳐야 하는데, 전체 시간을 1초로 하여 0.1초마다 아래로 내려오도록 한다. 즉 각 LED에 0.1초씩 머무르게 한다. 원래 박자보다 1초 빠르게 노트가 등장하여 1초 동안 Matrix를 내려오게 하여 원래 박자에 맞추어 노트가 최 하단에 도달하도록 한다. 모든 노트는 멀티 스레드를 통해 구현한다.

- 노트에 맞게 스위치를 눌렀는지 점수 판별하기

Matrix의 최 하단의 비트를 읽어 해당 비트에 LED가 들어왔는지 확인하고, LED가 들어왔을 때 버튼이 눌리면 점수를 1 증가시킨다. 그런데 최 하단의 LED가 들어오는 시간이 0.1초인데 그 안에 버튼을 정확히 누르는 것이 어려울 것이라고 예상이 된다. 그래서 노트가 최 하단보다 한 칸 위에 있을 때부터 최 하단에 있을 때, 그리고 사라진 직후 0.1초동안, 총 0.3초 내에 버튼 입력이 있으면 점수로 인정을 한다.


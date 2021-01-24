#include "../include/project_test.h"
#include "../include/dot_define.h"
#include "../include/music.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int note_on; //mode flag
unsigned char sw_buf[PUSH_SWITCH_MAX_BUTTON];
int score, end_flag = 0;
int high_score[3][2] = {{0,0},{0,0}, {0,0}};
int music_flag = 1, music_diff = 0;

unsigned char quit = 0;
void user_signal1(int sig) { quit = 1; }
void beep(int dev_buzzer, ssize_t ret_buzzer, int period, int time);
void off(int dev_buzzer, ssize_t ret_buzzer, int time);
void text_out(int dev, char* string1, char* string2);
void text_make(int num, char* string, char buf[16]);
void dot_clear(int dev, ssize_t ret);

//-----------------------------------------------------------------------------

void beep(int dev_buzzer, ssize_t ret_buzzer, int period, int time) //buzzer
{
//use period, make sound ( manipulation buzzer's on/off period fine, make pwm signal, and get the desired sound.
	unsigned char state;
	double real_time = time * 1000 / (period * 2);

	int i = 0;

	while(i<real_time){
	state = 1;
	ret_buzzer = write(dev_buzzer, &state, 1);
	usleep(period);
	state = 0;
	ret_buzzer = write(dev_buzzer, &state, 1);
	usleep(period);
	++i;}
}

void off(int dev_buzzer, ssize_t ret_buzzer, int time)
{
	unsigned char state = 0;

	ret_buzzer = write(dev_buzzer, &state, 1);
	usleep(time * 1000);
}

//text lcd write
void text_out(int dev, char* string1, char* string2)
{
	unsigned char lcd[32];
	int i = 0, j = 0;
	int len1 = strlen(string1);
	int len2 = strlen(string2);

	if(len1 < 16)
	{
		for(i=0; i<len1; i++)
			lcd[i] = string1[i];
		for(i=len1; i<16; i++)
			lcd[i] = ' ';

		for(i=0; i<len2; i++)
			lcd[i+16] = string2[i];
		for(i=len2; i<16; i++)
			lcd[i+16] = ' ';

		write(dev, lcd, 32);
	}
	else
	{
		for(i=0; i<len2; i++)
			lcd[i+16] = string2[i];
		for(i=len2; i<16; i++)
			lcd[i+16] = ' ';

		for(i=0; i<16; i++)
			lcd[i] = string1[i];
		write(dev, lcd, 32);
	}
}

//highscore(int) + textlcd => new string ( for write text lcd )
void text_make(int num, char* string, char buf[16])
{
	unsigned char n_buf[3] = {0,0,0};
	int i=0;
	int len = strlen(string);

	sprintf(n_buf,"%d",num);

	for(i=0; i<len; i++)
		buf[i] = string[i];

	buf[13] = n_buf[0];
	buf[14] = n_buf[1];
	buf[15] = n_buf[2];
}

//music end->dot matrix clear
void dot_clear(int dev, ssize_t ret)
{
	int i=0;
	for(i=0; i<10; i++)
	{
		ret = write(dev, fpga_full, i);
	}

	sleep(1);

	for(i=10; i>0; i--)
	{
		ret = write(dev, fpga_000, i-1);
	}	
}

//struct for thread
struct argstr
{
	int dev_buzzer, dev_dot, dev_switch, dev_fnd;
	ssize_t ret_buzzer, ret_dot, ret_fnd;
};

//thread for buzzer (music playing)
void *thread_music(void *arg)
{
	struct argstr *m_arg = (struct argstr *)arg; //argstr struct get from main.
	int i=0;

	while(note_on > -1) // when note_on flag < 0 , all threads and main function closed.
	{
		if(note_on == 1 && end_flag == 0) //note_on == 1, super mario mode. 
//end_flag : when music is finish but note is still falling , note_on flag is also 1. It makes music play again. so I made end_flag to prevent this. 
		{
			usleep(6000 * 9 * 9); //wait for dot falling at the end of matrix.

			for(i=0; i<SUPER_LEN; i++) //super mario music play.
			{
				beep(m_arg->dev_buzzer, m_arg->ret_buzzer, super_note[i], super_bit[i][0]);
				off(m_arg->dev_buzzer, m_arg->ret_buzzer, super_bit[i][1]);
			}
			sleep(2);
		}

		else if(note_on == 2 && end_flag == 0) //note_on == 2, tetris mode.
		{
			usleep(6000 * 9 * 10);

			for(i=0; i<TETRIS_LEN; i++)
			{
				beep(m_arg->dev_buzzer, m_arg->ret_buzzer, tetris_note[i], tetris_bit[i][0]);
				off(m_arg->dev_buzzer, m_arg->ret_buzzer, tetris_bit[i][1]);
			}
			sleep(2);
		}

		else if(note_on == 3 && end_flag == 0) //note_on == 3, maple story mode
		{
			usleep(6000 * 9 * 10);

			for(i=0; i<MAPLE_LEN; i++)
			{
				beep(m_arg->dev_buzzer, m_arg->ret_buzzer, maple_note[i], maple_bit[i][0]);
				off(m_arg->dev_buzzer, m_arg->ret_buzzer, maple_bit[i][1]);
			}
			sleep(2);
		}
	}
}

//to falling note in dot matrix thread 1
//I made three thread for dot matrix, so that there are three notes that can occur at one time. As it were, there are three handlers that generate notes. 
void *thread_dot1(void *arg)
{
//in dot[0][0~6], this thread makes note.
	struct argstr *m_arg = (struct argstr *)arg;

	int i;
	while(note_on > -1)
	{
		if(dot[0][0] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_100, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[0][0] = 0;
		}

		else if(dot[0][1] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_010, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[0][1] = 0;
		}

		else if(dot[0][2] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_001, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[0][2] = 0;
		}

		else if(dot[0][3] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_110, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[0][3] = 0;
		}

		else if(dot[0][4] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_101, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[0][4] = 0;
		}

		else if(dot[0][5] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_011, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[0][5] = 0;
		}

		else if(dot[0][6] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_111, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[0][6] = 0;
		}
	}
}

void *thread_dot2(void *arg)
{
//in dot[1][0~6], this thread makes note.
	struct argstr *m_arg = (struct argstr *)arg;

	int i;
	while(note_on > -1)
	{
		if(dot[1][0] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_100, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[1][0] = 0;
		}

		else if(dot[1][1] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_010, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[1][1] = 0;
		}

		else if(dot[1][2] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_001, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[1][2] = 0;
		}

		else if(dot[1][3] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_110, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[1][3] = 0;
		}

		else if(dot[1][4] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_101, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[1][4] = 0;
		}

		else if(dot[1][5] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_011, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[1][5] = 0;
		}

		else if(dot[1][6] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_111, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[1][6] = 0;
		}
	}
}

void *thread_dot3(void *arg)
{
//in dot[2][0~6], this thread makes note.
	struct argstr *m_arg = (struct argstr *)arg;
	int i;
	while(note_on > -1)
	{

		if(dot[2][0] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_100, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[2][0] = 0;
		}

		else if(dot[2][1] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_010, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[2][1] = 0;
		}

		else if(dot[2][2] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_001, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[2][2] = 0;
		}

		else if(dot[2][3] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_110, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[2][3] = 0;
		}

		else if(dot[2][4] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_101, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[2][4] = 0;
		}

		else if(dot[2][5] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_011, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[2][5] = 0;
		}

		else if(dot[2][6] == 1)
		{
			for(i=0; i<10; i++)
			{
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_111, i);
				usleep(30 * 1000);
				m_arg->ret_dot = write(m_arg->dev_dot, fpga_000, i);
				usleep(30 * 1000);
			}
			dot[2][6] = 0;
		}
	}
}

//thread for detect switch pushed and calculate score.
void *thread_switch(void *arg)
{
	struct argstr *m_arg = (struct argstr *)arg;
	unsigned char dot_buf[1]; //dot read
	unsigned char sw_buf[9]; //sw read
	unsigned char sw_flag[9] = {0,0,0,0,0,0,0,0,0}; //sw flag
	unsigned char flag[9] = {0,0,0,0,0,0,0,0,0}; //sw flag2
	unsigned char score_fnd[4] = {0,0,0,0}; //score for write fnd
	int i=0;

//music select mode
	while(note_on > -1)
	{
		while(note_on == 0)
		{
			score = 0; //end of music, score reset.
			read(m_arg->dev_switch, &sw_buf, sizeof(sw_buf)); //read switch

			if(sw_buf[0] == 1) //when sw1 pushed,
			{
				if(sw_flag[0] < 1) //and when sw flag1 is 0, 
				{
					sw_flag[0]++; // sw flag1 +1 ( make 0->1 )
					flag[0] = 1; //sw flag2 1
				}
			}
			else
				sw_flag[0] = 0;

			if(sw_buf[1] == 1)
			{
				if(sw_flag[1] < 1)
				{
					sw_flag[1]++;
					flag[1] = 1;
				}
			}
			else
				sw_flag[1] = 0;

			if(sw_buf[2] == 1)
			{
				if(sw_flag[2] < 1)
				{
					sw_flag[2]++;
					flag[2] = 1;
				}
			}
			else
				sw_flag[2] = 0;

			if(sw_buf[3] == 1)
			{
				if(sw_flag[3] < 1)
				{
					sw_flag[3]++;
					flag[3] = 1;
				}
			}
			else
				sw_flag[3] = 0;

			if(sw_buf[4] == 1)
			{
				if(sw_flag[4] < 1)
				{
					sw_flag[4]++;
					flag[4] = 1;
				}
			}
			else
				sw_flag[4] = 0;

			if(sw_buf[5] == 1)
			{
				if(sw_flag[5] < 1)
				{
					sw_flag[5]++;
					flag[5] = 1;
				}
			}
			else
				sw_flag[5] = 0;

			if(sw_buf[6] == 1)
			{
				if(sw_flag[6] < 1)
				{
					sw_flag[6]++;
					flag[6] = 1;
				}
			}
			else
				sw_flag[6] = 0;
//I made tis complicated flag to prevent duplicate input in playing music.

			if(flag[0] == 1)
			{
				music_flag = 1; //sw1, music_flag = 1 ( music = super mario )
				flag[0] = 0; //flag clear
			}

			if(flag[1] == 1)
			{
				music_flag = 2; //sw2, music_flag = 2 ( music = tetris )
				flag[1] = 0; //flag clear
			}

			if(flag[2] == 1)
			{
				music_flag = 3; //sw3, music_flag = 3 ( music = maple story )
				flag[2] = 0; //flag clear
			}

			if(flag[3] == 1)
			{
				music_diff = 0; //sw4, music_difficulty = 0 ( easy mode )
				flag[3] = 0; //flag clear
			}

			if(flag[4] == 1)
			{
				flag[4] = 0; //sw5, music select, so put music_flag in note_on, start play game.
				note_on = music_flag;
				printf("music : %d, diff : %d\n", music_flag, music_diff); //I want to check work well.
				break; //get out this loop
			}

			if(flag[5] == 1)
			{
				music_diff = 1; //sw6, music_difficulty = 1 ( hard mode )
				flag[5] = 0; //flag clear
			}

			if(flag[6] == 1)
			{
				flag[6] = 0; //flag clear
				note_on = -1; //sw7 in music select mode, it makes note_on = -1, so program is closed.
			}
		}

	//music playing mode
		while(note_on > 0)
		{
			read(m_arg->dev_dot, &dot_buf, 9); //read the last line of the dot matrix.
			read(m_arg->dev_switch, &sw_buf, sizeof(sw_buf)); //read switch

			flag[6] = 0; //flag clear
			flag[7] = 0;
			flag[8] = 0;

			if(sw_buf[6] == 1)
			{
				if(sw_flag[6] < 4)
				{
					sw_flag[6]++;
					flag[6] = 1;
				}
			}
			else
				sw_flag[6] = 0;

			if(sw_buf[7] == 1)
			{
				if(sw_flag[7] < 4)
				{
					sw_flag[7]++;
					flag[7] = 1;
				}
			}
			else
				sw_flag[7] = 0;

			if(sw_buf[8] == 1)
			{
				if(sw_flag[8] < 4)
				{
					sw_flag[8]++;
					flag[8] = 1;
				}
			}
			else
				sw_flag[8] = 0;

			switch(dot_buf[0]) //the last line of the dot matrix = ?
			{
				case 0x20 ://100
					if(flag[6] == 1 && end_flag == 0) // also use end_flag to prevent score up when game is finish.
						score++;
					break;
				case 0x08 ://010
					if(flag[7] == 1 && end_flag == 0)
						score++;
					break;
				case 0x02 ://001
					if(flag[8] == 1 && end_flag == 0)
						score++;
					break;
				case 0x28 ://110
					if(flag[6] == 1 && flag[7] == 1 && end_flag == 0)
						score++;
					break;
				case 0x0a ://011
					if(flag[7] == 1 && flag[8] == 1 && end_flag == 0)
						score++;
					break;
				case 0x22 ://101
					if(flag[6] == 1 && flag[8] == 1 && end_flag == 0)
						score++;
					break;
				case 0x2a ://111
					if(flag[6] == 1 && flag[7] == 1 && flag[8] == 1 && end_flag == 0)
						score++;
					break;
				default : break;
			}

			if(score > 0) //to prevent floating point exception.
			{
				score_fnd[2] = score/10;
				score_fnd[3] = score%10;
			}
			else
			{
				score_fnd[2] = 0;
				score_fnd[3] = 0;
			}

			m_arg->ret_fnd = write(m_arg->dev_fnd, score_fnd, FND_MAX_DIGIT); //write score in fnd

			if(score > high_score[note_on-1][music_diff]) //when score is higher then highscore, it is highscore.
				high_score[note_on-1][music_diff] = score;

			usleep(30 * 1000);
		}
	}

}

int main(void) 
{
	unsigned char state;
	int dev_buzzer, dev_dot, dev_text, dev_switch, dev_fnd;
	int status, i,j=0;
	ssize_t ret_buzzer, ret_dot, ret_text, ret_fnd;
	struct argstr arg;
	pthread_t m_thread, thread_1, thread_2, thread_3, thread_4;
	int mthr_id, thr_id1, thr_id2, thr_id3, thr_id4;
	unsigned char lcd_text[32] = "Linux RhythmGame2016132043 LMJ  ";
	unsigned char *lcd1, *lcd2;
	unsigned char lcd3[16], fnd[4];
	note_on = 0;
	score = 0;
//------------------------------------------------------------------------------
//buzzer open
	dev_buzzer = open(BUZZER_DEVICE, O_RDWR);
	assert2(dev_buzzer >= 0, "buzzer open error", BUZZER_DEVICE);
	(void)signal(SIGINT, user_signal1);
//matrix open
	dev_dot = open(DOT_DEVICE, O_RDWR);
	assert2(dev_dot >= 0, "dot matrix open error", DOT_DEVICE);
//matrix clear
	for(i=0; i<10; i++)
		ret_dot = write(dev_dot, fpga_000, i);
	assert2(ret_dot >= 0, "dot matrix write err", DOT_DEVICE);
//text_lcd_open
	dev_text = open(TEXT_LCD_DEVICE, O_WRONLY);
	assert2(dev_text>=0, "text lcd open error", TEXT_LCD_DEVICE);
	ret_text = write(dev_text, lcd_text, TEXT_LCD_MAX_BUF);
	assert2(ret_text >= 0, "dot lcd write err", TEXT_LCD_DEVICE);
//switch open
	dev_switch = open(PUSH_SWITCH_DEVICE, O_RDONLY);
	assert2(dev_switch >= 0, "switch open error", PUSH_SWITCH_DEVICE);
//fnd open
	dev_fnd = open(FND_DEVICE, O_RDWR);
	assert2(dev_fnd >= 0, "Device open error", FND_DEVICE);
	for(i=0; i<4; i++)
		fnd[i] = 0;
	ret_fnd = write(dev_fnd, fnd, FND_MAX_DIGIT);
//arg init
	arg.dev_buzzer = dev_buzzer;
	arg.dev_dot = dev_dot;
	arg.dev_switch = dev_switch;
	arg.dev_fnd = dev_fnd;
	arg.ret_buzzer = ret_buzzer;
	arg.ret_dot = ret_dot;
	arg.ret_fnd = ret_fnd;
//thread init
	mthr_id = pthread_create(&m_thread, NULL, thread_music, (void *)&arg);
	if(mthr_id < 0)
		exit(0);
	pthread_detach(m_thread);

	thr_id1 = pthread_create(&thread_1, NULL, thread_dot1, (void *)&arg);
	if(thr_id1 < 0)
		exit(0);
	pthread_detach(thread_1);

	thr_id2 = pthread_create(&thread_2, NULL, thread_dot2, (void *)&arg);
	if(thr_id2 < 0)
		exit(0);
	pthread_detach(thread_2);

	thr_id3 = pthread_create(&thread_3, NULL, thread_dot3, (void *)&arg);
	if(thr_id3 < 0)
		exit(0);
	pthread_detach(thread_3);

	thr_id4 = pthread_create(&thread_4, NULL, thread_switch, (void *)&arg);
	if(thr_id4 < 0)
		exit(0);
	pthread_detach(thread_4);

//change the mode of the game by note_on flag 

	while(note_on > -1)
	{
		if(note_on == 0) // music select mode
		{
			if(music_flag == 1)
				lcd1 = "Super Mario";
			else if(music_flag == 2)
				lcd1 = "Tetris";
			else if(music_flag == 3)
				lcd1 = "Maple Story";

			if(music_diff == 0)
			{
				lcd2 = "EASY    best:";
				text_make(high_score[music_flag-1][0], lcd2, lcd3); //combine the (int)highscore into string. 
					
			}
			else
			{
				lcd2 = "HARD    best:";
				text_make(high_score[music_flag-1][1], lcd2, lcd3);
					
			}

			text_out(dev_text, lcd1, lcd3);
			usleep(1);
		}

		else if(note_on == 1) //super mario music
		{
			if(music_diff == 0) //difficulty is easy
			{
				end_flag = 0;
				for(i=0; i<SUPER_LEN; i++)
				{
					dot[j][supere_dot[i]] = 1; //work three thread to make note ( 1->2->3->1->2->3 ... )
					if(super_note[i] < 700) //if the tone is too high, a slight timing error is created.
						usleep((super_bit[i][0]+super_bit[i][1] + 50) * 1000); //calibrate
					else
						usleep((super_bit[i][0]+super_bit[i][1]) * 1000); //calculate beep's on time + off time, make note at the intervals.

					if(j==2) // thread ( 1->2->3->1...)
						j=0;
					else
						j++;
				}
				sleep(1);
				end_flag = 1;
				dot_clear(dev_dot, ret_dot);
				note_on = 0;
			}
			else //music_diff == 1 : hard
			{
				end_flag = 0;
				for(i=0; i<SUPER_LEN; i++)
				{
					dot[j][superh_dot[i]] = 1;
					if(super_note[i] < 700)
						usleep((super_bit[i][0]+super_bit[i][1] + 50) * 1000);
					else
						usleep((super_bit[i][0]+super_bit[i][1]) * 1000);

					if(j==2)
						j=0;
					else
						j++;
				}
				sleep(1);
				end_flag = 1;
				dot_clear(dev_dot, ret_dot);
				note_on = 0;
			}
		}

		else if(note_on == 2) //tetris music
		{
			if(music_diff == 0)
			{
				end_flag = 0;
				for(i=0; i<TETRIS_LEN; i++)
				{
					dot[j][tetrise_dot[i]] = 1;
					if(tetris_note[i] < 700)
						usleep((tetris_bit[i][0]+tetris_bit[i][1] + 70) * 1000);
					else
						usleep((tetris_bit[i][0]+tetris_bit[i][1] + 20) * 1000);

					if(j==2)
						j=0;
					else
						j++;
				}
				sleep(1);
				end_flag = 1;
				dot_clear(dev_dot, ret_dot);
				note_on = 0;
			}
			else
			{
				end_flag = 0;
				for(i=0; i<TETRIS_LEN; i++)
				{
					dot[j][tetrish_dot[i]] = 1;
					if(tetris_note[i] < 700)
						usleep((tetris_bit[i][0]+tetris_bit[i][1] + 70) * 1000);
					else
						usleep((tetris_bit[i][0]+tetris_bit[i][1] + 20) * 1000);

					if(j==2)
						j=0;
					else
						j++;
				}
				sleep(1);
				end_flag = 1;
				dot_clear(dev_dot, ret_dot);
				note_on = 0;
			}
		}

		else if(note_on == 3) //maple story music
		{
			if(music_diff == 0)
			{
				end_flag = 0;
				for(i=0; i<MAPLE_LEN; i++)
				{
					dot[j][maplee_dot[i]] = 1;
					if(maple_note[i] < 700)
						usleep((maple_bit[i][0]+maple_bit[i][1] + 20) * 1000);
					else
						usleep((maple_bit[i][0]+maple_bit[i][1] + 20) * 1000);

					if(j==2)
						j=0;
					else
						j++;
				}
				sleep(1);
				end_flag = 1;
				dot_clear(dev_dot, ret_dot);
				note_on = 0;
			}
			else
			{
				end_flag = 0;
				for(i=0; i<MAPLE_LEN; i++)
				{
					dot[j][mapleh_dot[i]] = 1;
					if(maple_note[i] < 700)
						usleep((maple_bit[i][0]+maple_bit[i][1] + 20) * 1000);
					else
						usleep((maple_bit[i][0]+maple_bit[i][1] + 20) * 1000);

					if(j==2)
						j=0;
					else
						j++;
				}
				sleep(1);
				end_flag = 1;
				dot_clear(dev_dot, ret_dot);
				note_on = 0;
			}
		}
	}
//when note_on become -1, everything is over.

	state = BUZZER_OFF;
	ret_buzzer = write(dev_buzzer, &state, 1);
	close(dev_buzzer);
	close(dev_dot);
	close(dev_text);
	close(dev_switch);
	pthread_join(m_thread, (void **)&status);
	pthread_join(thread_1, (void **)&status);
	pthread_join(thread_2, (void **)&status);
	pthread_join(thread_3, (void **)&status);
	pthread_join(thread_4, (void **)&status);
	return 0;
}
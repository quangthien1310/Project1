#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <winbgim.h>

const int N = 512;
const int K = 256;

typedef struct header 
{
	char chunk_id[4];
	int chunk_size;
	char format[4];
	char subchunk1_id[4];
	int subchunk1_size;
	short int audio_format;
	short int num_channels;
	int sample_rate;
	int byte_rate;
	short int block_align;
	short int bits_per_sample;
	short int extra_param_size;
	char subchunk2_id[4];
	int subchunk2_size;
} header;

typedef struct header* header_p;

FILE *fp;
short inbuff16[1000000], inbuff16_max;
float scaleX, scaleY;
unsigned int sample_number = 0;
float r_data[K], max_abs_r;
bool inspect_end = false;
unsigned int backup_ptr[2][161];
int ptr_pos[2] = {-1, -1};
header_p meta = (header_p)malloc(sizeof(header));

int randm(int, int);
void delay(int ms);
void display();
void read(char*);
void draw_wave();
void draw_zoom(short*, int);
void draw_r(float*, int);
void draw_freq(int , int);
int R(short *data, float* r);
void set_ptr(int id, int pos, int color);
void *inspect(void*);
void key_listener();

int main() 
{
	display();
	read((char*)"xebesvexchef.wav");
	draw_wave();
	pthread_t inspect_thread;
	pthread_create(&inspect_thread, NULL, inspect, (void*) 1);
	key_listener();
	return 0;
}

int randm(int minN, int maxN)
{
	return minN + (rand() % (maxN + 1 - minN));
}

void delay(int ms) 
{
	clock_t t = clock();
	while(clock() < t + ms);
}

void display() 
{
	initwindow(1000, 650);
	setwindowtitle("Mach Quang Thien - 20215646 - PROJECT1");
	setbkcolor(7);	
	cleardevice();
	setfillstyle(SOLID_FILL, 0);
	
	bar(50, 30, 950, 190);
	setcolor(14); //yellow
	setlinestyle(1, 1, 1);
	line(50, 70, 950, 70);
	line(50, 110, 950, 110);
	line(50, 150, 950, 150);
	line(150, 30, 150, 190);
	line(250, 30, 250, 190);
	line(350, 30, 350, 190);
	line(450, 30, 450, 190);
	line(550, 30, 550, 190);
	line(650, 30, 650, 190);
	line(750, 30, 750, 190);
	line(850, 30, 850, 190);
	
	bar(50, 210, 490, 390);		// zoom
	bar(510, 210, 950, 390);	// r(x)
	bar(50, 410, 950, 590);		// frequency
	
	setlinestyle(1, 1, 1);
	setcolor(14);
	line(50, 455, 950, 455);
	line(50, 500, 950, 500);
	line(50, 545, 950, 545);
	line(150, 410, 150, 590);
	line(250, 410, 250, 590);
	line(350, 410, 350, 590);
	line(450, 410, 450, 590);
	line(550, 410, 550, 590);
	line(650, 410, 650, 590);
	line(750, 410, 750, 590);
	line(850, 410, 850, 590);
	
	setcolor(0);
	settextstyle(SMALL_FONT, 2, 4);
	outtextxy(45, 600, "80");
	outtextxy(45, 545, "160");
	outtextxy(45, 500, "240");
	outtextxy(45, 455, "320");
	outtextxy(45, 410, "400Hz");
	outtextxy(155, 205, "588.0");
    outtextxy(255, 205, "1176.0");
    outtextxy(355, 205, "1764.0");
    outtextxy(455, 205, "2352.0");
    outtextxy(555, 205, "2940.0");
    outtextxy(655, 205, "3528.0");
    outtextxy(755, 205, "4116.0");
    outtextxy(855, 205, "4704.0");
    outtextxy(155, 605, "588.0");
    outtextxy(255, 605, "1176.0");
    outtextxy(355, 605, "1764.0");
    outtextxy(455, 605, "2352.0");
    outtextxy(555, 605, "2940.0");
    outtextxy(655, 605, "3528.0");
    outtextxy(755, 605, "4116.0");
    outtextxy(855, 605, "4704.0");
    outtextxy(985, 300, "K = 256");
    outtextxy(985, 605, "5313 ms");
}

void read(char *filename) 
{
	fp = fopen(filename, "rb");
	
	fread(meta, 1, sizeof(header), fp);
	
	short tmp = 0;
	while(fread((char*) &tmp, 2, 1, fp)) 
	{
		inbuff16[sample_number] = tmp;
		sample_number++;
		if(inbuff16_max < abs(tmp))
		{
			inbuff16_max = abs(tmp);			
		}
	}
}

int R(short *x, float* r) 
{
	for(int k = 0; k < K; k++) 
	{
		r[k] = 0;
		for(int i = 0; i < N - k; i++) 
		{
			r[k] += x[i] * x[i + k];
		}	
	}
	
	float max_r = 0, max_i = 1;
	max_abs_r = 0;
	for(int k = 0; k < K; k++) 
	{
		if(k > 0 && k < K - 1 && max_r < r[k] && r[k] > r[k - 1] && r[k] > r[k + 1]) 
		{
			max_r = r[k];
			max_i = k;
		}	
	}
	return max_i;
}

void draw_wave() 
{
	setcolor(0);
	setbkcolor(7);
	settextstyle(SMALL_FONT, 2, 4);
	outtextxy(45, 115, "0");
	srand(time(0));
	int randint = randm(500, 10500);
	randint = randm(500, 10500);
	char maxY[20];
	sprintf(maxY, "%d", inbuff16_max + randint);
	outtextxy(45, 30, maxY);
	sprintf(maxY, "-%d", inbuff16_max + randint);
	outtextxy(45, 190, maxY);
	outtextxy(985, 115, "5313 ms");
	
	setlinestyle(0, 0, 1);
	setcolor(10);
	moveto(50, 110);
	scaleX = sample_number / 900.0;
	scaleY = (inbuff16_max + randint) / 80.0;
	
	for(int i = 0; i < sample_number; i++) 
	{
		lineto(i / scaleX + 50, 110 - inbuff16[i] / scaleY);
	}
}

void draw_zoom(short *data, int size) 
{
	// redraw the view for zoom wave form
	bar(50, 210, 490, 390);
	setlinestyle(1, 1, 1);
	setcolor(14);
	line(50, 300, 490, 300);
	line(50, 255, 490, 255);
	line(50, 345, 490, 345);
	line(150, 210, 150, 390);
	line(250, 210, 250, 390);
	line(350, 210, 350, 390);
	line(450, 210, 450, 390);
	
	short max_wave_data = 0;
	for(int i = 0; i < size; i++) 
	{
		if(max_wave_data < abs(data[i]))
		{
			max_wave_data = abs(data[i]);			
		}
	}
	
	// zoom wave form
	setlinestyle(0, 1, 1);
	setcolor(10);
	
	float scaleX = size / 440.0;
	float scaleY = (max_wave_data + 1000) / 90.0;
	moveto(50, 300);
	for(int i = 0; i < size; i++) 
	{
		lineto(i / scaleX + 50, 300 - data[i] / scaleY);
	}
}

void draw_r(float *r_data, int T) 
{
	bar(510, 210, 950, 390);
	setlinestyle(1, 1, 1);
	setcolor(14);
	line(510, 300, 950, 300);
	line(510, 255, 950, 255);
	line(510, 345, 950, 345);
	line(610, 210, 610, 390);
	line(710, 210, 710, 390);
	line(810, 210, 810, 390);
	line(910, 210, 910, 390);
	
	float max = 0;
	
	for(int i = 0; i < K; i++) 
	{
		if(max < fabs(r_data[i]))
			max = fabs(r_data[i]);
	}
	
	// R(x)
	setlinestyle(0, 1, 1);
	setcolor(10);
	float scaleX = 256.0 / 440;
	float scaleY = max != 0 ? max / 90.0 : 1;
	moveto(510, 210);
	
	for(int i = 0; i < K; i++) 
	{
		lineto(i / scaleX + 510, 300 - r_data[i] / scaleY);
	}
	
	setcolor(4);
	line(510 + T / scaleX, 210, 510 + T / scaleX, 390);
}

void draw_freq(int T, int i)
{
	float freq = 0;
	
	setcolor(10);
	scaleX = sample_number / 900.0;
	scaleY = 400.0 / 180;
	freq = 1.0 / ((float) T / meta->sample_rate);
	if(freq < 400)
	{
		circle(50 + i / scaleX, 590 - freq / scaleY, 3);
	}
}

void *inspect(void *inspect_id) 
{
	int T;
	float freq = 0;
	
	for(int i = 0; i + N < sample_number; i+= N / 2) 
	{
		set_ptr(0, i, 15);
		set_ptr(1, i + N, 15);
		
		T = R(inbuff16 + i, r_data);
		
		draw_zoom(inbuff16 + i, 512);	
		draw_r(r_data, T);
		draw_freq(T, i);
		delay(200);
	}
	
	float scaleX = sample_number / 900.0;
	
	set_ptr(0, 5000, 4);
	set_ptr(1, 9500, 1);
	inspect_end = true;
	pthread_exit(NULL);
}

void key_listener() 
{
	float scaleX = sample_number / 900.0;
	
	while(1) 
	{
		char key = getch();

		if(key == 't' && inspect_end == true) 
		{
			if(ptr_pos[0] > 0) 
			{
				set_ptr(0, ptr_pos[0] - scaleX, 4);				
			}
		}
		else if(key == 'T' && inspect_end == true) 
		{
			if(ptr_pos[1] > ptr_pos[0])
			{
				set_ptr(1, ptr_pos[1] - scaleX, 1);				
			}
		}
		else if(key == 'p' && inspect_end == true) 
		{
			if(ptr_pos[0] < ptr_pos[1])
			{
				set_ptr(0, ptr_pos[0] + scaleX, 4);				
			}
		}
		else if(key == 'P' && inspect_end == true) 
		{
			if(ptr_pos[1] + scaleX < sample_number)
			{
				set_ptr(1, ptr_pos[1] + scaleX, 1);	
			}
		}
		else if((key == 'i' || key == 'I') && inspect_end == true) 
		{
			draw_zoom(inbuff16 + ptr_pos[0], ptr_pos[1] - ptr_pos[0] + 1);
		}
		else if(key == 'x') 
		{
			break;
		}
	}
}

void set_ptr(int id, int pos, int color) 
{
	float scaleX = sample_number / 900.0;
	for(int i = 0; i <= 160; i++) 
	{
		if(ptr_pos[id] >= 0) 
		{
			putpixel(50 + ptr_pos[id] / scaleX, i + 30, backup_ptr[id][i]);
		}
		if(pos >= 0) 
		{
			backup_ptr[id][i] = getpixel(50 + pos / scaleX, i + 30);
		}
	}
	ptr_pos[id] = pos;
	
	if(pos >= 0) 
	{
		setlinestyle(0, 0, 1);
		if (color == 1)
		{
			setlinestyle(1, 1, 1);
			setbkcolor(7);
		}
		setcolor(color);
		line(50 + pos / scaleX, 30, 50 + pos / scaleX, 190);
	}
}


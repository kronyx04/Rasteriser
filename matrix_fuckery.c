#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <unistd.h>

#define CANVAS_WIDTH 960
#define CANVAS_HEIGHT 960
#define inf 9999999999
#define END 696969

//SDL do not touch
SDL_Event event;
SDL_Window *window;
SDL_Renderer *renderer;

//##### structs ################
struct rgb {
	int r;
	int g;
	int b;
};

struct point {
	float x;
	float y;
};

struct matrix_2d {
	float matrix[2][2];
};

//##### constants ##############

const struct rgb red = {255, 0, 0};
const struct rgb green = {0, 255, 0};
const struct rgb blue = {0, 0, 255};
const struct rgb yellow = {255, 255, 0};
const struct rgb Aradia = {160, 1, 3};
const struct rgb Nepeta = {65, 102, 0};
const struct rgb Vriska = {0, 65, 130};
const struct rgb Sollux = {161, 161, 0};
const struct rgb pawl = {0, 147, 205};
const struct rgb pink = {255, 158, 192};
const struct rgb black = {0, 0, 0};
const struct rgb white = {255, 255, 255};

const float pi = 3.141592;
float degree = 180/pi;
double radian = 0.01745329;

const struct rgb BACKGROUND = white;

float rot_matrix[4] = {
	0.9998477, 0.01745241,
	-0.01745241,  0.9998477
};

//##### the scene ##############

//##### functions ##############

//putpixel simplifies the SDL ordeal, its mission is to set the color of a pixel in the canvas using cartesian coordinates
int PutPixel(double x, double y, struct rgb color) {
	int sx, sy;
	float cw = CANVAS_WIDTH;
	float ch = CANVAS_HEIGHT;

	sx =cw/2 + x;
	sy = ch/2 - y;
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
	SDL_RenderDrawPoint(renderer, sx, sy);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	return 0;
}

struct point PointNumMul (struct point point, float n) {
	struct point result;

	result.x = point.x * n;
	result.y = point.y * n;

	return result;
};

struct point PointAdd (struct point a, struct point b) {
	struct point result;

	result.x = a.x + b.x;
	result.y = a.y + b.y;

	return result;
};

struct point Transform(struct point p, float * m) {
	struct point result;
	
	result.x = m[0] * p.x + m[2] * p.y;
	result.y = m[1] * p.x + m[3] * p.y;

	return result;
};

void Interpolate(float * values, float i0, float d0, float i1, float d1) {
	float a;
	float d;
	int i;
	int j = 0;

	if (i0 == i1) {
		values[0] = d0;
		values[1] = END;
		return;
	};

	a = (d1 - d0) / (i1 - i0);
	d = d0;

	for (i = i0 ; i <= i1 ; ++i) {

//		printf("j = %d\n", j);

		values[j++] = d;
		d += a;
	};

	values[j] = END;

	return;
};

void PointSwap(struct point * p0, struct point * p1) {
	struct point temp;
	temp = *p0;
	*p0 = *p1;
	*p1 = temp;
	return;
}


void DrawLine(struct point p0, struct point p1, struct rgb color) {

	float s[1000];
	int i;
	int arr;

	if (abs(p1.x - p0.x) > abs(p1.y - p0.y)) {
		// line is horizontal-ish
		// make sure p0.x < p1.x
		if (p0.x > p1.x) {
			PointSwap(&p0, &p1);
		};
		Interpolate(&s[0], p0.x, p0.y, p1.x, p1.y);
		for ( i = p0.x ; i <= p1.x ; ++i) {
			arr = i - p0.x;
			PutPixel(i, s[arr], color);
		};
	} else {
		// line is vertical-ish
		// make sure p0.y < p1.y
		if (p0.y > p1.y) {
			PointSwap(&p0, &p1);
		};
		Interpolate(&s[0], p0.y, p0.x, p1.y, p1.x);
		for (i = p0.y ; i <= p1.y ; ++i) {
			arr = i - p0.y;
			PutPixel(s[arr], i, color);
		};
	};
};

void DrawWireframeTriangle(struct point p0, struct point p1, struct point p2, 
						   struct rgb color) {
	DrawLine(p0, p1, color);
	DrawLine(p1, p2, color);
	DrawLine(p2, p0, color);
};

void DrawCircle(float r, struct rgb color) {
	int angle;
	struct point p = {r, 0};

	for(angle = 0 ; angle <= 360 ; angle += 1) {

		p = Transform(p, rot_matrix);
		PutPixel(p.x, p.y, color);
	};
	return;
};

//##### main function ##########
int main(void) {

//window name
	const char* title = "bepis";

//initialize SDL, the window and the renderer
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CANVAS_WIDTH, CANVAS_HEIGHT, 0);
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

//stuff
	struct point p = {420, 69};
	struct rgb color = black;

	float matrix[4] = {0,0,0,0};

	for(p.x=-CANVAS_WIDTH/2 ; p.x<=CANVAS_WIDTH/2 ; p.x++) {
		for(p.y=CANVAS_HEIGHT/2 ; p.y>=-CANVAS_HEIGHT/2 ; p.y--) {
			PutPixel(p.x, p.y, color);
		};
	};

	double q = 2.617994; 

	int r = 100;
	struct point p1 = {0, 100}, p2 = {-cos(q) * 100, -sin(q) * 100}, p3 = {cos(q) * 100, -sin(q) * 100};

	struct point p4 = p1, p5 = p2, p6 = p3;


	p.x = 0;
	p.y = 0;

	int i = 0;
	


	while(1) {

	p4 = Transform(p4, rot_matrix);
	p5 = Transform(p5, rot_matrix);
	p6 = Transform(p6, rot_matrix);

	DrawCircle(r, green);
	DrawWireframeTriangle(p4, p5, p6, green);

	i++;
	if (i == 360) {
		p4 = p1;
		p5 = p2;
		p6 = p3;
	};

// don't touch zone
		SDL_RenderPresent(renderer);
		SDL_RenderClear(renderer);
		
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
			break;
		SDL_Delay(16);

	};

	
//loop to keep the window open and additional stuff for gracefully exiting
//the render call is also here
//	while(1) {
//		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
//			break;
//		SDL_RenderPresent(renderer);
//		SDL_Delay(16);
//	};
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}


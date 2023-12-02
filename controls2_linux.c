#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#define CANVAS_WIDTH 960
#define CANVAS_HEIGHT 960
#define inf 9999999999
#define END 696969

#define CUBE_VERTS 8
#define CUBE_TRIANGLES 12

//SDL do not touch
SDL_Event event;
SDL_Window *window;
SDL_Renderer *renderer;


//##### structs ################
struct rgb {
	float r;
	float g;
	float b;
};

struct point {
	int x;
	int y;
	float h; // intensity, can represent many things, mainly shade of color
};

struct vector {
	float x;
	float y;
	float z;
};

struct triangle {
	int vert[3];
	struct rgb color;
};

struct model {
	struct vector v[100];
	int vc; // vector count
	struct triangle t[100];
	int tc; // triangle count
};

struct transform {
	float scale;
	int x_rotation; //degrees
	int y_rotation; //degrees
	int z_rotation; //degrees
	struct vector translation;
};

struct instance {
	struct model model;
	struct transform transform;
	struct rgb color;
};

struct scene {
	struct transform camera;
	struct instance instances[100];
	int instance_count;
};

struct plane {
	struct vector normal;
	float D;
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
const struct rgb Pawl = {0, 147, 205};
const struct rgb pink = {255, 158, 192};
const struct rgb pinker = {245, 44, 115};
const struct rgb purple = {124, 0, 164};
const struct rgb cyan = {0, 255, 255};
const struct rgb black = {0, 0, 0};
const struct rgb white = {255, 255, 255};

const double pi = 3.1415926535897932384626433832795028841971693993751058209749445923;
const double radian = 0.0174532925199432957692369076848861271344287188854172545609719144;
const double sqrt_2 = 1.4142135623730950488016887242096980785696718753769480731766797379;
const double one_over_sq2 = 0.7071067811865475244008443621048490392848359376884740365883398689;

const struct vector invalid_vector = {END, END, END};
struct vector leftn = {one_over_sq2, 0, one_over_sq2};
struct vector rightn = {-one_over_sq2, 0, one_over_sq2};

const struct model cube = {
	{
		1, 1, 1,  	// A - v[0]
		-1, 1, 1,	// B - v[1]
		-1, -1, 1,  // C - v[2]
		1, -1, 1,   // D - v[3]
		1, 1, -1,	// E - v[4]
		-1, 1, -1,	// F - v[5]
		-1, -1, -1,	// G - v[6]
		1, -1, -1	// H - v[7]
	},
	8,
	{
		0, 1, 2, red,
		0, 2, 3, red,
		4, 0, 3, green,
		4, 3, 7, green,
		5, 4, 7, blue,
		5, 7, 6, blue,
		1, 5, 6, yellow,
		1, 6, 2, yellow,
		4, 5, 1, purple,
		4, 1, 0, purple,
		2, 6, 7, cyan,
		2, 7, 3, cyan
	},
	12
};

const struct model pyramid = {
	{
		0, 1, 0,	// v[0]
		-1, -1, 1,  // v[1]
		1, -1, 1,   // v[2]
		-1, -1, -1,	// v[3]
		1, -1, -1	// v[4]
	},
	5,
	{
		0, 1, 2, red,
		0, 2, 4, red,
		0, 3, 4, green,
		0, 1, 3, green,
		1, 2, 3, cyan,
		3, 4, 2, cyan
	},
	6
};

struct plane left_plane;
struct plane right_plane;

const struct rgb BACKGROUND = white;

const double sin1 = 0.0174524064372835128194189785163161924722527203071396426836124276;
const double cos1 = 0.9998476951563912391570115588139148516927403105831859396583207145;

double x_rot[] = {
	1, 0, 0,
	0, cos1, sin1,
	0, -sin1, cos1
};

double y_rot[] = {
	cos1, 0, -sin1,
 	0, 1, 0,
	sin1, 0, cos1
};

double z_rot[] = {
	cos1, -sin1, 0,
	sin1, cos1,  0,
	0,    0,   1
};

double x_rot_neg[] = {
	1, 0, 0,
	0, cos1, -sin1,
	0, sin1, cos1
};

double y_rot_neg[] = {
	cos1,  0,  sin1,
 	0,     1,  0,
	-sin1, 0, cos1
};

double z_rot_neg[] = {
	-cos1, sin1, 0,
	-sin1, -cos1, 0,
	0, 0, 1
};

//##### the scenes ##############

//struct vector O = {0, 0, 0}; //assumed to be looking in the z+ axes
float Vw = 1; //viewport width
float Vh = 1; //viewport height
float Vd = 1; //viewport distance



//##### functions ##############

//putpixel simplifies the SDL ordeal, its mission is to set the color of a pixel in the canvas using cartesian coordinates
int PutPixel(int x, int y, struct rgb color) {
	int sx, sy;
	sx = CANVAS_WIDTH/2 + x;
	sy = CANVAS_HEIGHT/2 - y;
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
	SDL_RenderDrawPoint(renderer, sx, sy);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	return 0;
}

struct vector VectorAdd(struct vector va, struct vector vb) {
	struct vector result;

	result.x = va.x + vb.x;
	result.y = va.y + vb.y;
	result.z = va.z + vb.z;

	return result;
}

struct vector VectorSub(struct vector va, struct vector vb) {
	struct vector result;

	result.x = va.x - vb.x;
	result.y = va.y - vb.y;
	result.z = va.z - vb.z;

	return result;
};
;

struct vector VectorNumMul(struct vector va, float b) {
	struct vector result;

	result.x = va.x * b;
	result.y = va.y * b;
	result.z = va.z * b;

	return result;
};

// given a vector and a value in radians rotates the vector by that amount
struct vector VectorRot(struct vector v, float radians, char axes) {
	struct vector result;
	float true_angle;
	float x;
	float y;

	if (axes == 'y') {
		x = v.x;
		y = v.z;
	} else if (axes == 'x') {
		x = v.z;
		y = v.y;
	} else if (axes == 'z') {
		x = v.x;
		y = v.y;
	};

	float radius = sqrt(fabs(x) * fabs(x) + fabs(y) * fabs(y));
	float base_angle = atan2(fabs(y),fabs(x));

	if (y >= 0 && x >= 0)
		true_angle = base_angle;
	else if (y >= 0 && x <= 0)
		true_angle = pi - base_angle;
	else if (y <= 0 && x <= 0)
		true_angle = 4.712389 - base_angle;
	else
		true_angle = 2*pi - base_angle;

	if (axes == 'y') {
		result.x = radius * cos(true_angle + radians);
		result.y = v.y;
		result.z = radius * sin(true_angle + radians);
	} else if (axes == 'x') {
		result.x = v.x;
		result.y = radius * sin(true_angle + radians);
		result.z = radius * cos(true_angle + radians);
	} else if (axes == 'z') {
		result.x = radius * cos(true_angle + radians);
		result.y = radius * sin(true_angle + radians);
		result.z = v.z;
	};

	return result;
};

// applies 3d matrix to vector
struct vector Transform(struct vector v, double * m) {
	struct vector result;

	result.x = m[0] * v.x + m[1] * v.y + m[2] * v.z;
	result.y = m[3] * v.x + m[4] * v.y + m[5] * v.z;
	result.z = m[6] * v.x + m[7] * v.y + m[8] * v.z;

	return result;
};

// applies a matrix to a vector multiple times
struct vector MatrixMul(double * m, struct vector v, int n) {

	struct vector result = v;

	for (int i = 0 ; i < n ; i++) {
		result = Transform(result, m);
	};

	return result;
};

struct vector VectorRot2(struct vector v, int degrees, char axis) {
	struct vector result = v;

	// counter clockwise
	if (axis == 'x' && degrees >= 0) {
		result = MatrixMul(x_rot, v, degrees);
	} else if (axis == 'z' && degrees >= 0) {
		result = MatrixMul(z_rot, v, degrees);
	} else if (axis == 'y' && degrees >= 0){
		result = MatrixMul(y_rot, v, degrees);
	} 
	// clockwise
	else if (axis == 'x' && degrees <= 0) {
		result = MatrixMul(x_rot_neg, v, -degrees);
	} else if (axis == 'z' && degrees <= 0) {
		result = MatrixMul(z_rot_neg, v, -degrees);
	} else if (axis == 'y' && degrees <= 0){
		result = MatrixMul(y_rot_neg, v, -degrees);
	};

	return result;
};

struct vector VectorInv(struct vector v) {
	struct vector result;

	result.x = -1 * v.x;
	result.y = -1 * v.y;
	result.z = -1 * v.z;

	return result;
};

// checks if a vector is == invalid_vector
// 1 for yes 0 for no
char VectorCheck(struct vector v) {
	if (v.x == invalid_vector.x && v.y == invalid_vector.y && v.z == invalid_vector.z)
		return 1;
	else
		return 0;
};

struct rgb RgbNumMul(struct rgb a, float b) {
	struct rgb result;

	result.r = a.r * b;
	result.g = a.g * b;
	result.b = a.b * b;
	
	if (result.r > 255)
		result.r = 255;
	if (result.r < 0)
		result.r = 0;

	if (result.g > 255)
		result.g = 255;
	if (result.g < 0)
		result.g = 0;

	if (result.b > 255)
		result.b = 255;
	if (result.b < 0)
		result.b = 0;

	return result;
};

float Dot(struct vector va, struct vector vb) {
	return va.x*vb.x + va.y*vb.y + va.z*vb.z;
};

void PrintVector(struct vector v) {
	printf("%f,%f,%f\n", v.x, v.y, v.z);
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

//given a float array replaces the last value with END
void RemoveLast (float * array) {
	int i;
	for (i = 0 ; array[i] != END ; i++);
	array[i] = '\0';
	array[--i] = END;
	return;
};

//given three float arrays fuses the first two into the third
void Concatenate (float * array1, float * array2, float * arrayout) {
	int i;
	int j;
	for (i=0 ; array1[i] != END ; i++)
		arrayout[i] = array1[i];

	for (j=0 ; array2[j] != END ; j++)
		arrayout[i++] = array2[j];

	arrayout[i] = END;
	return;
};

//length of array
int Length(float * array) {
	int i;
	for(i=0 ; array[i] != END ; i++); 
	return i-1;
}

// copy first (from) array into second (to)
void ArrayCpy(float *f, float *t) {
	while ((*f = *t) != END) {
		f++;
		t++;
	};
	*t = END;
	return;
};

// prints contents of given array
void PrintArray(float * array) {
	for(int i = 0 ; array[i] != END ; i++) {
		printf("%f\n", array[i]);
	};
};

void PointSwap(struct point * p0, struct point * p1) {
	struct point temp;
	temp = *p0;
	*p0 = *p1;
	*p1 = temp;
	return;
}

void DrawLine(struct point p0, struct point p1, struct rgb color) {

	float s[2000];
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

//Draw Wireframe Triangle takes three points and a color and draws a hollow
//triangle
void DrawWireframeTriangle(struct point p0, struct point p1, struct point p2, 
						   struct rgb color) {
	DrawLine(p0, p1, color);
	DrawLine(p1, p2, color);
	DrawLine(p2, p0, color);
};

void DrawFilledTriangle (struct point p0, struct point p1, struct point p2,
						 struct rgb color) {
	
	float x01[1000];
	float x12[1000];
	float x02[1000];
	float x012[1000];
	float x_left[1000];
	float x_right[1000];
	int m;
	int y;
	int x;

	// sort the points so that p0.y <= p1.y <= p2.y
	if (p1.y < p0.y) { PointSwap(&p1, &p0);};
	if (p2.y < p0.y) { PointSwap(&p2, &p0);};
	if (p2.y < p1.y) { PointSwap(&p2, &p1);};

	// Compute the x coordinates of the triangle edges
	Interpolate(&x01[0], p0.y, p0.x, p1.y, p1.x);
	Interpolate(&x12[0], p1.y, p1.x, p2.y, p2.x);
	Interpolate(&x02[0], p0.y, p0.x, p2.y, p2.x);

	// concatenate the short sides
	RemoveLast(&x01[0]);
	Concatenate(&x01[0], &x12[0], &x012[0]);


	// determine which is left and which is right
	m = floor(Length(&x012[0]) / 2);
	if (x02[m] < x012[m]) {
		ArrayCpy(&x_left[0], &x02[0]);
		ArrayCpy(&x_right[0], &x012[0]);
	} else {
		ArrayCpy(&x_left[0], &x012[0]);
		ArrayCpy(&x_right[0], &x02[0]);
	};

	// draw the horizontal segments
	for(y = p0.y ; y <= p2.y ; y++) {
		for(x = x_left[y - p0.y] ; x <= x_right[y - p0.y] ; x++) {
			PutPixel(x, y, color);
		};
	};
	
	return;
	
};

void DrawShadedTriangle (struct point p0, struct point p1, struct point p2,
						 struct rgb color) {
	
	float x01[1000];
	float h01[1000];
	float x12[1000];
	float h12[1000];
	float x02[1000];
	float h02[1000];
	float x012[1000];
	float h012[1000];
	float x_left[1000];
	float h_left[1000];
	float x_right[1000];
	float h_right[1000];
	float h_segment[1000];
	int m;
	int y;
	int x;
	int x_l;
	int x_r;
	struct rgb shaded_color;

	// sort the points so that p0.y <= p1.y <= p2.y
	if (p1.y < p0.y) { PointSwap(&p1, &p0);};
	if (p2.y < p0.y) { PointSwap(&p2, &p0);};
	if (p2.y < p1.y) { PointSwap(&p2, &p1);};

	// Compute the x coordinates of the triangle edges
	Interpolate(&x01[0], p0.y, p0.x, p1.y, p1.x);
	Interpolate(&h01[0], p0.y, p0.h, p1.y, p1.h);

	Interpolate(&x12[0], p1.y, p1.x, p2.y, p2.x);
	Interpolate(&h12[0], p1.y, p1.h, p2.y, p2.h);

	Interpolate(&x02[0], p0.y, p0.x, p2.y, p2.x);
	Interpolate(&h02[0], p0.y, p0.h, p2.y, p2.h);

	// concatenate the short sides
	RemoveLast(&x01[0]);
	Concatenate(&x01[0], &x12[0], &x012[0]);

	RemoveLast(&h01[0]);
	Concatenate(&h01[0], &h12[0], &h012[0]);

	// determine which is left and which is right
	m = floor(Length(&x012[0]) / 2);
	if (x02[m] < x012[m]) {
		ArrayCpy(&x_left[0], &x02[0]);
		ArrayCpy(&h_left[0], &h02[0]);

		ArrayCpy(&x_right[0], &x012[0]);
		ArrayCpy(&h_right[0], &h012[0]);
	} else {
		ArrayCpy(&x_left[0], &x012[0]);
		ArrayCpy(&h_left[0], &h012[0]);

		ArrayCpy(&x_right[0], &x02[0]);
		ArrayCpy(&h_right[0], &h02[0]);
	};

	// draw the horizontal segments
	for(y = p0.y ; y <= p2.y ; y++) {
		x_l = x_left[y - p0.y];
		x_r = x_right[y - p0.y];
		
		Interpolate(&h_segment[0], x_l, h_left[y - p0.y], x_r, h_right[y - p0.y]);
		for(x = x_l ; x <= x_r ; x++) {
			shaded_color = RgbNumMul(color, h_segment[x - x_l]);
			PutPixel(x, y, shaded_color);
		};
	};
	
	return;
	
};

struct point ViewportToCanvas(float x, float y) {
	struct point p;

	p.x = x * CANVAS_WIDTH/Vw;
	p.y = y * CANVAS_HEIGHT/Vh;
	p.h = 1;

	return p;
};


struct point ProjectVertex(struct vector v) {
	struct point p;
	p = ViewportToCanvas(v.x * Vd / v.z, v.y * Vd / v.z);
	return p;
};


void RenderTriangle(struct triangle triangle, struct point * projected) {
	DrawWireframeTriangle(projected[triangle.vert[0]],
						  projected[triangle.vert[1]],
						  projected[triangle.vert[2]],
						  triangle.color
			);
	return;
};

struct vector ApplyTransform(struct vector vertex, struct transform transform) {
	struct vector scaled;
	struct vector rotated;
	struct vector translated;

	scaled = VectorNumMul(vertex, transform.scale);
	rotated = VectorRot2(scaled, transform.x_rotation, 'x');
	rotated = VectorRot2(rotated, transform.y_rotation, 'y');
	rotated = VectorRot2(rotated, transform.z_rotation, 'z');
	translated = VectorAdd(rotated, transform.translation);

	return translated;
};

struct vector ApplyCamera(struct vector vertex, struct transform transform) {
	struct vector rotated;
	struct vector translated;

	translated = VectorSub(vertex, transform.translation);
	rotated = VectorRot2(translated, -transform.x_rotation, 'x');
	rotated = VectorRot2(rotated, -transform.y_rotation, 'y');
	rotated = VectorRot2(rotated, -transform.z_rotation, 'z');

	return rotated;
};

float SignedDistance(struct plane plane, struct vector vertex) {
	struct vector normal = plane.normal;
	return Dot(vertex, normal) + plane.D;
};

//replaces bad vertexes in an instance with invalid_vector
void ClipInstance(struct vector * v, int vc) {
	int i;
	float dl;
	float dr;
	for (i=0 ; i < vc ; i++) {
		dl = SignedDistance(left_plane, v[i]);
		dr = SignedDistance(right_plane, v[i]);
		if (v[i].z <= Vd)
			v[i] = invalid_vector;
		if (dl <= 0)
			v[i] = invalid_vector;
		if (dr <= 0)
			v[i] = invalid_vector;
	};
};

void RenderInstance(struct instance instance, struct transform camera) {
	struct point projected[2000];
	struct vector transformed[2000];
	struct model model = instance.model;
	struct vector v;
	int i;
	char invalid = 0;

// makes an unprojected instance (array of vector)
	for (i=0 ; i < model.vc ; i++) {
		v = ApplyTransform(model.v[i], instance.transform);
		v = ApplyCamera(v, camera);
		*(transformed+i) = v;
	};

	ClipInstance(transformed, model.vc);

// check if all vertices are valid
	for (i=0 ; i < model.vc ; i++) {
		if (VectorCheck(transformed[i])){
			invalid = 1;
			break;
		};
	};

// if none of the vertices are bad, proceed
	if (!invalid) {;
		for (i=0 ; i < model.vc ; i++)
			*(projected+i) = ProjectVertex(*(transformed+i));

		for (i=0 ; i < model.tc ; i++) {
			model.t[i].color = instance.color;
			RenderTriangle(model.t[i], projected);
		};
	};

	return;
};

void RenderScene(struct scene scene) {
	for(int i = 0; i < scene.instance_count ; i++)
		RenderInstance(scene.instances[i], scene.camera);
};

//##### main function ##########
int main(void) {

//window name
	const char* title = "ඞ ඞ ඞ ඞ ඞ ඞ ඞ ඞ ඞ ඞ ඞ ඞ";

//initialize SDL, the window and the renderer
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CANVAS_WIDTH, CANVAS_HEIGHT, 0);
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

//stuff

	left_plane.normal = leftn;
	left_plane.D = 0;

	right_plane.normal = rightn;
	right_plane.D = 0;

	
	struct vector forwards = {0, 0, 1};
	struct vector backwards = {0, 0, -1};
	struct vector left = {-1, 0, 0};
	struct vector right = {1, 0, 0};

	struct vector yeya = {69, 420, 666};
	struct vector badbad = invalid_vector;

	struct scene scene1 = {
		1, 0, 0, 0, {0, 0, 0}, // camera: "scale" (scale does nothing on the camera), x_rotation, y_rot, z_rot, translation
		{
			cube, 1, 0, 0, 0, {-1.5, 0, 7}, Nepeta, //model, scale, x y and z rotation, position of center
			cube, 0.5, 0, 0, 0, {1.5, 1.5, 7}, Pawl,
			pyramid, 1, 0, 0, 0, {1.5, -0.5, 7}, pinker,
		},
		3
	};	

	int i = 0;
	while (1) {

		scene1.instances[0].transform.x_rotation = i*2;
		scene1.instances[0].transform.y_rotation = -i*2;
		scene1.instances[0].transform.z_rotation = i*2;

		scene1.instances[1].transform.z_rotation = i;
		scene1.instances[1].transform.x_rotation = i;

		scene1.instances[2].transform.y_rotation = i;
		
		RenderScene(scene1);
		SDL_RenderPresent(renderer);

		i += 1;
		if (i == 360)
			i = 0;

// more SDL stuff
//	while (1) {
		SDL_RenderClear(renderer);
		SDL_Delay(16);
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				goto QUIT;
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_RIGHT) {
					scene1.camera.translation = VectorAdd(scene1.camera.translation, VectorRot2(right, scene1.camera.y_rotation, 'y'));
					PrintVector(scene1.camera.translation);
				} else if (event.key.keysym.sym == SDLK_LEFT) {
					scene1.camera.translation = VectorAdd(scene1.camera.translation, VectorRot2(left, scene1.camera.y_rotation, 'y'));
					PrintVector(scene1.camera.translation);
				} else if (event.key.keysym.sym == SDLK_DOWN) {
					scene1.camera.translation = VectorAdd(scene1.camera.translation, VectorRot2(backwards, scene1.camera.y_rotation, 'y'));
					PrintVector(scene1.camera.translation);
				} else if (event.key.keysym.sym == SDLK_UP) {
					scene1.camera.translation = VectorAdd(scene1.camera.translation, VectorRot2(forwards, scene1.camera.y_rotation, 'y'));
					PrintVector(scene1.camera.translation);
				} else if (event.key.keysym.sym == SDLK_q) {
					scene1.camera.y_rotation += 3;
					printf("%d\n", scene1.camera.y_rotation);
				} else if (event.key.keysym.sym == SDLK_e) {
					scene1.camera.y_rotation -= 3;
					printf("%d\n", scene1.camera.y_rotation);
				};
			};
		};
		if (scene1.camera.y_rotation <= -360 || scene1.camera.y_rotation >= 360)
			scene1.camera.y_rotation = 0;
	};

// graceful exit
	QUIT:
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
};


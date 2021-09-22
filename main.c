#include <SDL2/SDL.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

void initSDL() {
	assert(SDL_Init(SDL_INIT_EVERYTHING) >= 0);
	window = SDL_CreateWindow("Rasterizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
	assert(window);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	assert(renderer);
}

void endSDL() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

struct shape {
	float vrt[12*3];
	int s;
	unsigned char colour[3];
};

struct shape *shapes[250] = {NULL};
int shapeNum = 0;

struct shape *addShape(int s, float *vrt, unsigned char *colour) {
	struct shape *sh = malloc(sizeof(struct shape));
	shapes[shapeNum] = sh;
	shapeNum++;
	sh->s = s;
	for(int i = 0; i < s*3; i++)
		sh->vrt[i] = vrt[i];
	for(int i = 0; i < 3; i++)
		sh->colour[i] = colour[i];
	return sh;
}

void moveShape(struct shape *s, float *mov) {
	for(int i = 0; i < s->s*3; i++)
		s->vrt[i] += mov[i%3];
}

void rotateShape(struct shape *s, float *rot) {
	for(int i = 0; i < s->s; i++) {
		float *vrt = s->vrt+i*3;
		float nvrt[3];

		nvrt[0] = vrt[0];
		nvrt[1] = vrt[1]*cosf(rot[0]) + vrt[2]*-sinf(rot[0]);
		nvrt[2] = vrt[1]*sinf(rot[0]) + vrt[2]*cosf(rot[0]);
		for(int j = 0; j < 3; j++)
			vrt[j] = nvrt[j];

		nvrt[0] = vrt[0]*cosf(rot[1]) + vrt[2]*sinf(rot[1]);
		nvrt[1] = vrt[1];
		nvrt[2] = vrt[0]*-sinf(rot[1]) + vrt[2]*cosf(rot[1]);
		for(int j = 0; j < 3; j++)
			vrt[j] = nvrt[j];

		nvrt[0] = vrt[0]*cosf(rot[2]) + vrt[1]*-sinf(rot[2]);
		nvrt[1] = vrt[0]*sinf(rot[2]) + vrt[1]*cosf(rot[2]);
		nvrt[2] = vrt[2];
		for(int j = 0; j < 3; j++)
			vrt[j] = nvrt[j];
	}
}

void transformShapes(float *mov, float *rot) {
	for(int i = 0; i < shapeNum; i++) {
		if(mov)
			moveShape(shapes[i], mov);
		if(rot)
			rotateShape(shapes[i], rot);
	}
}

void projectShapes() {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	for(int i = 0; i < shapeNum; i++) {
		struct shape *s = shapes[i];
		for(int j = 0; j < s->s; j++) {
			float *vrt = s->vrt+j*3;
			vrt[0] += 256/vrt[2];
			vrt[1] += 256/vrt[2];
		}
	}
}

float shapeCenter(struct shape *s, int c) {
	float vrt[3] = {0};
	for(int i = 0; i < s->s*3; i++)
		vrt[i%3] += s->vrt[i];
	return (vrt[c]/s->s);
}

void orderShapes() {
	struct shape *f;
	do {
		f = NULL;
		for(int i = 1; i < shapeNum; i++) {
			if(shapeCenter(shapes[i-1], 2) < shapeCenter(shapes[i], 2)) {
				f = shapes[i-1];
				shapes[i-1] = shapes[i];
				shapes[i] = f;
			}
		}
	} while(f != NULL);
}

void drawTriangle(float *vrt) {
	for(float i = 0; i <= 1.0; i += 0.001) {
		int x1, y1, x2, y2;
		x1 = vrt[0]+i*(vrt[3]-vrt[0]);
		y1 = vrt[1]+i*(vrt[4]-vrt[1]);
		x2 = vrt[0]+i*(vrt[6]-vrt[0]);
		y2 = vrt[1]+i*(vrt[7]-vrt[1]);
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}
}

void drawShapes() {
	for(int i = 0; i < shapeNum; i++) {
		struct shape *s = shapes[i];
		SDL_SetRenderDrawColor(renderer, s->colour[0], s->colour[1], s->colour[2], 0xff);
		for(int j = 0; j < s->s-3; j++) {
			drawTriangle(s->vrt+j*3);
		}
		if(s->s > 3) {
			float vrt[9];
			for(int j = s->s*3-6; j < s->s*3; j++)
				vrt[j-s->s*3+6] = s->vrt[j];
			for(int j = 0; j < 3; j++)
				vrt[6+j] = s->vrt[j];
			drawTriangle(vrt);
		}

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
		for(int j = 0; j < s->s; j++) {
			int x1, y1, x2, y2;
			x1 = s->vrt[j*3];
			y1 = s->vrt[j*3+1];
			if(j == s->s-1) {
				x2 = s->vrt[0];
				y2 = s->vrt[1];
			}
			else {
				x2 = s->vrt[j*3+3];
				y2 = s->vrt[j*3+4];
			}
			SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
		}
	}
}

void deleteShapes() {
	for(int i = 0; i < shapeNum; i++)
		free(shapes[i]);
	shapeNum = 0;
}

float cubeRot = 0;

void draw() {
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(renderer);

	float cube[] = {
		0, 0, 0,
		200, 0, 0,
		200, 200, 0,
		0, 200, 0,

		0, 0, 200,
		200, 0, 200,
		200, 200, 200,
		0, 200, 200,

		0, 0, 0,
		0, 200, 0,
		0, 200, 200,
		0, 0, 200,

		200, 0, 0,
		200, 200, 0,
		200, 200, 200,
		200, 0, 200,

		0, 0, 0,
		200, 0, 0,
		200, 0, 200,
		0, 0, 200,

		0, 200, 0,
		200, 200, 0,
		200, 200, 200,
		0, 200, 200,
	};
	unsigned char colour[] = {
		0xff, 0x00, 0x00,
		0x00, 0xff, 0x00,
		0x00, 0x00, 0xff,
		0xff, 0xff, 0x00,
		0xff, 0x00, 0xff,
		0x00, 0xff, 0xff,
	};
	for(int i = 0; i < 6; i++)
		addShape(4, cube+(i*4*3), colour+(i*3));

	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	float m1[3] = {-100, -100, -100};
	float r1[3] = {cubeRot/3, cubeRot, 0};
	float m2[3] = {w/2, h/2, 200};
	float r2[3] = {0};

	transformShapes(m1, r1);
	transformShapes(m2, r2);

	orderShapes();
	projectShapes();
	drawShapes();
	deleteShapes();

	SDL_RenderPresent(renderer);
}

int main() {
	initSDL();
	SDL_Event event;
	bool quit = false;
	int lastUpdate;
	while(!quit) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			}
		}
		int currentTime = SDL_GetTicks();
		cubeRot += 0.001*(currentTime-lastUpdate);
		lastUpdate = currentTime;
		draw();
	}
	endSDL();
	return 0;
}

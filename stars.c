#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

typedef struct sphere {
	double x;
	double y;
	double r;
	double vx;
	double vy;
	double mass;
}sphere;

Uint32 animation(Uint32 interval, void * param) {
		//функция будет добавлять событие в очередь сообщений
		SDL_Event event_to_push;         //событие, которое будет добавлено в очередь, представленное как union
		SDL_UserEvent user_event;        //структура, представляющая конкретное событие
		user_event.type = SDL_USEREVENT; //обязательно задать верный тип сообщения
		//данные, которые можно анализировать в основном цикле
		user_event.code = 0;             //целое число, которое может быть определено программистом
		user_event.data1 = NULL;         //указатель, который может быть задан программистом
		user_event.data2 = NULL;
		event_to_push.user = user_event; //копируем данные в union
		SDL_PushEvent(&event_to_push);   //добавляем сообщение в очередь
		return interval;
}

double GetAngle(sphere Star, sphere Planet, double dist) {
	double angle = acos((double) ((Star.x-Planet.x)/dist));
	if (Star.y > Planet.y) return -angle;
	else return angle;
};

void RenderFillCircle(SDL_Renderer * renderer, int cx, int cy, int r) {
    for (int line = r; line != -1; --line) {
        int l = (int)sqrt(r*r-line*line);
        SDL_RenderDrawLine(renderer,cx-l,cy+line,cx+l,cy+line);
        SDL_RenderDrawLine(renderer,cx-l,cy-line,cx+l,cy-line);
    }
}

bool Not_Hit(sphere Star, sphere Planet) {
	if (sqrt(pow(Star.x-Planet.x, 2) + pow(Star.y-Planet.y, 2)) > Star.r + Planet.r) return 1;
	else return 0;
};

int main(int argc, char **argv)
{
	
	double m1, m2, x0, y0, vx0, vy0, mp;
	double accx, accy, dist1, dist2;
	double prevx, prevy;
	double vy;
	printf ("Input star masses (in sun masses): ");
	fflush(stdout);
	scanf("%lf, %lf", &m1, &m2);
	printf ("Input planet parameters (x0 and y0 in pixels, vx0 and vy0 in km/s, mass in earth masses): ");
	fflush(stdout);
	scanf("%lf, %lf, %lf, %lf, %lf", &x0, &y0, &vx0, &vy0, &mp);
	sphere star1 = {400, 500, 10, 0, 0, m1 * 1.989 * pow(10, 30)};
	sphere star2 = {600, 500, 10, 0, 0, m2 * 1.989 * pow(10, 30)};
	sphere planet = {x0, y0, 5, (double) vx0/600000, (double) vy0/600000, (double) mp * 5.972 * pow(10, 24)};
	printf("star1 mass is %lf, star2 mass is %lf, planet mass is %lf\n", star1.mass, star2.mass, planet.mass);
	SDL_Window * window = NULL; //главное окно
	SDL_Renderer * renderer = NULL; //инструмент рисования
	SDL_Texture * texture = NULL; //текстура с основными элементами
	SDL_Texture * texture_graph = NULL; //текстура с графиком
	
	window = SDL_CreateWindow(
		"System",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        1000,                               // width, in pixels
        1000,                               // height, in pixels
        0
    );
	
	renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED
	);
	
	texture = SDL_CreateTexture(
		renderer,                      
        SDL_PIXELFORMAT_RGBA8888,      
        SDL_TEXTUREACCESS_TARGET,      
        1000,                           
        1000                            
    );
	texture_graph = SDL_CreateTexture(
		renderer,                      
        SDL_PIXELFORMAT_RGBA8888,      
        SDL_TEXTUREACCESS_TARGET,      
        1000,                           
        1000                            
    );
    SDL_TimerID timer = 0;
    bool proceed = true;
    bool running = false;
    double tik = 0.033;
    int counter = 0;
    double * optimal_v = malloc(sizeof(double));
    
    int num = 0;
	vy = 0;
	do{
		counter = 0;
		vy += 100000; 			
		planet.vx = 0;
		planet.vy = vy;
		planet.x = x0;
		planet.y = y0;
		dist1 = sqrt(pow(planet.x-star1.x, 2) + pow(planet.y-star1.y, 2));
		dist2 = sqrt(pow(planet.x-star2.x, 2) + pow(planet.y-star2.y, 2));
		planet.vx += 0.5*(6.67 * pow(10, -33) * star1.mass * planet.mass * pow(dist1*600000, -2) * cos(GetAngle(star1, planet, dist1)) + 6.67 * pow(10, -33) * star2.mass * planet.mass * pow(dist2*600000, -2) * cos(GetAngle(star2, planet, dist2)))*pow(600000, -1);
		planet.vy += 0.5*(6.67 * pow(10, -33) * star1.mass * planet.mass * pow(dist1*600000, -2) * sin(GetAngle(star1, planet, dist1)) + 6.67 * pow(10, -33) * star2.mass * planet.mass * pow(dist2*600000, -2) * sin(GetAngle(star2, planet, dist2)))*pow(600000, -1);
		do{
			dist1 = sqrt(pow(planet.x-star1.x, 2) + pow(planet.y-star1.y, 2));
			dist2 = sqrt(pow(planet.x-star2.x, 2) + pow(planet.y-star2.y, 2));
			
			accx = (6.67 * pow(10, -33) * star1.mass * planet.mass * pow(dist1*600000, -2) * cos(GetAngle(star1, planet, dist1)) + 6.67 * pow(10, -33) * star2.mass * planet.mass * pow(dist2*600000, -2) * cos(GetAngle(star2, planet, dist2)))*pow(600000, -1);
			accy = (6.67 * pow(10, -33) * star1.mass * planet.mass * pow(dist1*600000, -2) * sin(GetAngle(star1, planet, dist1)) + 6.67 * pow(10, -33) * star2.mass * planet.mass * pow(dist2*600000, -2) * sin(GetAngle(star2, planet, dist2)))*pow(600000, -1);
			planet.vx += accx;
			planet.vy += accy;
			planet.x += planet.vx;
			planet.y -= planet.vy;
			counter += 1;
		}while(Not_Hit(star1, planet) && Not_Hit(star2, planet) &&(counter<100000));
		if (counter = 100000) {
			optimal_v[num] = vy;
			num+=1;
			optimal_v = realloc(optimal_v, (num+1) * sizeof(double));
		};
	}while(vy<30000000);	
			optimal_v = realloc(optimal_v, (num-1)*sizeof(double));
			for(int i = 0; i < num-1; i++) printf("optimal_v is %lf\n",optimal_v[i]);
				
										//1 pixel equals 600 000 kilometers!!!!
    
    planet.vx = vx0;
    planet.vy = vy0;
    planet.x = x0;
    planet.y = y0;
    dist1 = sqrt(pow(planet.x-star1.x, 2) + pow(planet.y-star1.y, 2));
	dist2 = sqrt(pow(planet.x-star2.x, 2) + pow(planet.y-star2.y, 2));
    planet.vx += 0.5*(6.67 * pow(10, -33) * star1.mass * planet.mass * pow(dist1*600000, -2) * cos(GetAngle(star1, planet, dist1)) + 6.67 * pow(10, -33) * star2.mass * planet.mass * pow(dist2*600000, -2) * cos(GetAngle(star2, planet, dist2)))*pow(600000, -1);
	planet.vy += 0.5*(6.67 * pow(10, -33) * star1.mass * planet.mass * pow(dist1*600000, -2) * sin(GetAngle(star1, planet, dist1)) + 6.67 * pow(10, -33) * star2.mass * planet.mass * pow(dist2*600000, -2) * sin(GetAngle(star2, planet, dist2)))*pow(600000, -1);
    //Pre-drawing objects on texture
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0,0,0x22,255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer,0xff, 0xa5, 0x00, 255);
    RenderFillCircle(renderer, star1.x, star1.y, star1.r);
    RenderFillCircle(renderer, star2.x, star2.y, star2.r);
    RenderFillCircle(renderer, planet.x, planet.y, planet.r);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_Rect star1_in = {star1.x-star1.r, star1.y-star1.r, 2*star1.r, 2*star1.r};
    SDL_Rect star1_out = {star1.x-star1.r, star1.y-star1.r, 2*star1.r, 2*star1.r};
    SDL_Rect star2_in = {star2.x-star2.r, star2.y-star2.r, 2*star2.r, 2*star2.r};
    SDL_Rect star2_out = {star2.x-star2.r, star2.y-star2.r, 2*star2.r, 2*star2.r};
    SDL_Rect planet_in = {planet.x-planet.r, planet.y-planet.r, 2*planet.r, 2*planet.r};
    SDL_Rect planet_out = {planet.x-planet.r, planet.y-planet.r, 2*planet.r, 2*planet.r};
    SDL_Rect graph_in = {0,0,1000,1000};
    SDL_Rect graph_out = {0,0,1000,1000};
    do{
		SDL_Event ev;
			if (SDL_PollEvent(&ev)) {
				if (SDL_QUIT == ev.type) {
					proceed = false;
					goto END;
				};
				switch(ev.type) {
					case SDL_MOUSEBUTTONDOWN: {
						if (running) {              //если таймер запущен
							SDL_RemoveTimer(timer); //останавливаем таймер
							timer = 0;              //зануляем идентификатор таймера, чтобы не освободить его повторно
							running = !running;
						}
						else {
							timer = SDL_AddTimer((int) (tik*1000),animation,NULL); //запускаем таймер и сохраняем его идентификатор
							running = !running;
						};
					}break;
					case SDL_USEREVENT: {
					counter++;
					dist1 = sqrt(pow(planet.x-star1.x, 2) + pow(planet.y-star1.y, 2));
					dist2 = sqrt(pow(planet.x-star2.x, 2) + pow(planet.y-star2.y, 2));
					
					accx = (6.67 * pow(10, -33) * star1.mass * planet.mass * pow(dist1*600000, -2) * cos(GetAngle(star1, planet, dist1)) + 6.67 * pow(10, -33) * star2.mass * planet.mass * pow(dist2*600000, -2) * cos(GetAngle(star2, planet, dist2)))*pow(600000, -1);
					accy = (6.67 * pow(10, -33) * star1.mass * planet.mass * pow(dist1*600000, -2) * sin(GetAngle(star1, planet, dist1)) + 6.67 * pow(10, -33) * star2.mass * planet.mass * pow(dist2*600000, -2) * sin(GetAngle(star2, planet, dist2)))*pow(600000, -1);
					prevx = planet.x;
					prevy = planet.y;
					planet.vx += tik*accx;
					planet.vy += tik*accy;
					planet.x += planet.vx*tik;
					planet.y -= planet.vy*tik;
					SDL_SetRenderTarget(renderer, texture_graph);
					SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 255);
					SDL_RenderDrawLine(renderer, prevx,prevy,planet.x,planet.y);
					SDL_SetRenderTarget(renderer, NULL);
					planet_out.x = planet.x-planet.r;
					planet_out.y = planet.y-planet.r;
					printf("alpha is %lf, betta is %lf\n", GetAngle(star1, planet, dist1), GetAngle(star2, planet, dist2));
					printf("accx is %lf, accy is %lf\n", accx, accy);
					printf("x is %lf, y is %lf\n", planet.x, planet.y);
					}break;
				};
			};
		SDL_SetRenderDrawColor(renderer, 0,0,0x22,255);
		SDL_RenderClear(renderer);	
		SDL_RenderCopy(renderer, texture_graph, &graph_in, &graph_out);
		SDL_RenderCopy(renderer, texture, &star1_in, &star1_out);
		SDL_RenderCopy(renderer, texture, &star2_in, &star2_out);
		SDL_RenderCopy(renderer, texture, &planet_in, &planet_out);
		SDL_RenderPresent(renderer);
	}while(proceed);
	
	END:
	
	if (NULL != window) SDL_DestroyWindow(window);
    if (NULL != renderer) SDL_DestroyRenderer(renderer);
	
	SDL_Quit();
	
	return 0;
}


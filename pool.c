#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdbool.h> //для работы макросов bool true false
#include <math.h>

void RenderFillCircle(SDL_Renderer * renderer, int cx, int cy, int r) {
    for (int line = r; line != -1; --line) {
        int l = (int)sqrt(r*r-line*line);
        SDL_RenderDrawLine(renderer,cx-l,cy+line,cx+l,cy+line);
        SDL_RenderDrawLine(renderer,cx-l,cy-line,cx+l,cy-line);
    }
}

void BlankTexture(SDL_Renderer * renderer, SDL_Texture * texture) {
    if (0 == SDL_SetRenderTarget(renderer,texture)) {  //устанавливаем текстуру в качестве цели для рисования
        SDL_SetRenderDrawColor(renderer,0xFF,0xFF,0xFF,0xFF);             
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer,NULL); //возвращаем инструмент на окно
    }    
}

typedef struct _Ball { //структура _Ball содержит информацию о круге
    int px,py;    //предыдущее положение круга
    int cx,cy;    //текущее положение круга
    int vx,vy;    //текущая скорость круга
    int radius;
} Ball;

void DrawBall(SDL_Renderer * renderer, SDL_Texture * texture, Ball * ball) {
    if (0 == SDL_SetRenderTarget(renderer,texture)) {  
        SDL_SetRenderDrawColor(renderer,0xFF,0x99,0x33,0xFF);             
        RenderFillCircle(renderer,ball->cx,ball->cy,ball->radius);
        SDL_SetRenderTarget(renderer,NULL); 
    }
}

void DrawPath(SDL_Renderer * renderer, SDL_Texture * texture, Ball * ball) {
    if (0 == SDL_SetRenderTarget(renderer,texture)) {  
        SDL_SetRenderDrawColor(renderer,0xCC,0x00,0x66,0xFF);             
        SDL_RenderDrawLine(renderer,ball->px,ball->py,ball->cx,ball->cy);
        SDL_SetRenderTarget(renderer,NULL); 
    }
}

//callback функция, которая будет вызываться таймером
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

int main(int argc, char * args[]) {
    //Инициализация компонентов библиотеки SDL2
    //SDL_INIT_VIDEO - графическая подсистема (окна и рисование)
    //SDL_INIT_EVENTS - подсистема очереди сообщений (управление мышью и клавиатурой)
    //SDL_INIT_TIMER - подсистема периодических вызовов (таймеры) 
    if( 0 != SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_TIMER) ) {
        //Если SDL_Init возвращает не 0, то это сигнал об ошибке
        printf("Unable to start SDL: %s\n",SDL_GetError());
        return 0; //мгновенно завершаем программу
    }

    //Основные ресурсы
    SDL_Window * window = NULL; //главное окно
    SDL_Renderer * renderer = NULL; //инструмент рисования
    SDL_Texture * path_texture = NULL, * ball_texture = NULL; //текстуры для рисования

    //Создаём главное окно
    window = SDL_CreateWindow(
        "An SDL2 window",                  // название окна в кодировке UTF-8
        SDL_WINDOWPOS_UNDEFINED,           // начальное положение окна по x на экране; SDL_WINDOWPOS_UNDEFINED - означает, что положение определит ОС
        SDL_WINDOWPOS_UNDEFINED,           // начальное положение окна по y на экране
        640,                               // ширина в пикселях
        480,                               // высота в пикселях
        0                                  // дополнительный параметр (подробнее в SDL2 Wiki =)
    );
    //если окно не было создано, то сообщаем об ошибке
    if (NULL == window) {
         printf("Can't create window: %s\n", SDL_GetError());
        goto CLEANUP; //и переходим к коду очистки ресурсов
    }

    //Создаём инструмент рисования (требуется верный ресурс окна)
    renderer = SDL_CreateRenderer(
        window,                           //указатель на ресурс окна, в котором будем рисовать 
        -1,                               //номер устройства для ускорения графики (GPU); -1 означает, что будет выбран ресурс ОС по умолчанию
        SDL_RENDERER_ACCELERATED          //используем аппаратное ускорение: обсчёт на GPU все графические ресурсы будут загружые в память GPU
    );
    if (NULL == renderer) {
        printf("Can't create renderer %s\n",SDL_GetError());
        goto CLEANUP;
    }

    //Создаём текстуры для рисования (требуется верный инструмент рисования)
    ball_texture = SDL_CreateTexture(
        renderer,                      //инструмент двумерного рисования типа SDL_Renderer *
        SDL_PIXELFORMAT_RGBA8888,      //формат хранения цвета внутри текстуры
        SDL_TEXTUREACCESS_TARGET,      //специальный флаг указывает, что на данной текстуре можно рисовать
        300,                           //ширина в пикселях
        300                            //высота в пикселях
    );
    
    path_texture = SDL_CreateTexture(
        renderer,                      
        SDL_PIXELFORMAT_RGBA8888,      
        SDL_TEXTUREACCESS_TARGET,      
        300,                           
        300                            
    );

    if ( NULL == ball_texture || NULL == path_texture ) {
        printf("Can't create textures: %s\n",SDL_GetError());
        goto CLEANUP;
    }

    //Основная часть программы
    //Все необходимые ресурсы графического интерфейса созданы выше
    {
        //создаём дополнительные данные
        SDL_Rect ball_src_rect = {0,0,300,300}, path_src_rect = {0,0,300,300};         //прямоугольники внутри текстур откуда копируем содержимое
        SDL_Rect ball_dst_rect = {15,150,300,300}, path_dst_rect = {325,150,300,300};  //прямоугольники, в которые на окно копируется содержимое текстур


        Ball ball = {150,150,150,150,7,13,15}; //создаём структуру типа Ball в центре области текструры, скорость нулевая
        BlankTexture(renderer,ball_texture);
        BlankTexture(renderer,path_texture);
        DrawBall(renderer,ball_texture,&ball);
        DrawPath(renderer,path_texture,&ball);

        SDL_TimerID timer = 0; //переменная хранит номер таймера, т.к. в программе может быть запущено много таймеров
        bool running = false;  //переменная обозначает запущен ли таймер: если false - таймер не запущен, если true - таймер запущен

        //Блок с основным циклом обработки сообщений
        bool proceed = true;
        do {
            SDL_Event event; //union для хранения любого события
            if (SDL_PollEvent(&event)) { //пытаемся извлечь следующее событие, если оно есть, то SDL_PollEvent возвращает не ноль
                switch(event.type) {//каждое конкретное событие имеет свой тип, который хранится в поле .type
                    case SDL_QUIT: { //SDL_QUIT - событие нажатия кнопки закрытия окна
                        proceed = false;
                    } break;

                    case SDL_USEREVENT: {   //обрабатываем сообщение от таймера
                        //изменяем координаты круга согласно текущей скорости
                        ball.px = ball.cx;             
                        ball.py = ball.cy;
                        ball.cx = ball.px + ball.vx;
                        ball.cy = ball.py + ball.vy;

                        //если круг вышел за пределы квадрата, то меняем скрость на противоположную (отражение от стенки)
                        if ( (ball_src_rect.x+ball.radius) > ball.cx || (ball_src_rect.x-ball.radius + ball_src_rect.w) < ball.cx )
                            ball.vx *= -1;
                        if ( (ball_src_rect.y+ball.radius) > ball.cy || (ball_src_rect.y-ball.radius + ball_src_rect.h) < ball.cy )
                            ball.vy *= -1;

                        BlankTexture(renderer,ball_texture);    //очищаем текстуру с кругом
                        DrawBall(renderer,ball_texture,&ball);  //отрисовываем круг в новом месте
                        DrawPath(renderer,path_texture,&ball);  //добавляем линию движения на текстуру пройденного пути
                    } break;

                    case SDL_MOUSEBUTTONDOWN: { //нажатие мышки останавливает и запускает анимацию
                        if (running) {              //если таймер запущен
                            SDL_RemoveTimer(timer); //останавливаем таймер
                            timer = 0;              //зануляем идентификатор таймера, чтобы не освободить его повторно
                        }
                        else {
                            timer = SDL_AddTimer(33,animation,NULL); //запускаем таймер и сохраняем его идентификатор
                        }
                        running = !running; //меняем значение переменной running на обратное
                    } break;
                }
            }

        REDRAW:; //далее следует код отрисовки основного окна; отрисовка происходит после каждого события
            SDL_SetRenderDrawColor(renderer,0x33,0xCC,0xFF,0xFF);//задаём цвет пера для рисования у указанного первым инструмента
            SDL_RenderClear(renderer); //закрашиваем всё окно указанным выше цветом

            SDL_RenderCopy(renderer,ball_texture,&ball_src_rect,&ball_dst_rect); //копируем часть структуры из ball_src_rect на часть окна, заданную path_src_rect
            SDL_RenderCopy(renderer,path_texture,&path_src_rect,&path_dst_rect);

            SDL_RenderPresent(renderer); //представляем нарисованное 
        } while(proceed);

        SDL_RemoveTimer(timer);
    }

    //Освобождаем занятые ресурсы
    CLEANUP:;
        if (NULL != path_texture) SDL_DestroyTexture(path_texture);
        if (NULL != ball_texture) SDL_DestroyTexture(ball_texture); //Освобождение ресурса тектуры
        if (NULL != renderer) SDL_DestroyRenderer(renderer);        //Освобождение ресурса рисования
        if (NULL != window) SDL_DestroyWindow(window);              //Освобождение ресурса окна
        SDL_Quit();//освобождаем ресурсы подсистем SDL 

    return 0;
}
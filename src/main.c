
//Chao Yue 20008378
#include <driver/gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <esp_log.h>
#include <esp_timer.h>

#include "fonts.h"
#include "graphics.h"



static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    int val = gpio_get_level(gpio_num);
    static uint64_t lastkeytime;
    uint64_t time=esp_timer_get_time();
    uint64_t timesince=time-lastkeytime;
    ets_printf("gpio_isr_handler %d %d %lld\n",gpio_num, val,
    timesince);
    lastkeytime=time;
}

extern image_header spaceship_image;
typedef struct pos {
    int x;
    int y;
    int speed;
    int colour;
} pos;

void app_main() {
    bool hit=false;
    int count=1;
    int blink=0;
    int k=0;
    int l=0;
    int x1[100];
    float y1[100];
    float vol[100];
    int s=0;
    int d=0;
    int w=spaceship_image.width;
    int h=spaceship_image.height;
    char score[200];
    while(1){
        gpio_set_direction(35,GPIO_MODE_INPUT);
        gpio_set_direction(0,GPIO_MODE_INPUT);
        graphics_init();
        set_orientation(PORTRAIT);
        gpio_set_intr_type(0, GPIO_INTR_POSEDGE);
        gpio_set_intr_type(35, GPIO_INTR_ANYEDGE);
        gpio_install_isr_service(0);
        gpio_isr_handler_add(0, gpio_isr_handler, (void*) 0);
        gpio_isr_handler_add(35, gpio_isr_handler, (void*) 35);
        
        pos stars[100];
        for(int i=0;i<100;i++) {
            stars[i].x=rand()%display_width;
            stars[i].y=rand()%(display_height*256);
            stars[i].speed=rand()%512+64;
            stars[i].colour=rand();
        }
        for (int i=0;i<100;i++) {
            x1[i]=rand()%133;
            y1[i]=rand()%3+0.1;
            vol[i]=0.5;
        }
        float speed=1.0;

        setFont(FONT_UBUNTU16);
        setFontColour(216, 216, 0);
        cls(rgbToColour(0,0,30));
        print_xy("Dodge Rocket", 10, 60);
        setFont(FONT_SMALL);
        setFontColour(0, 200, 255);
        print_xy("Use left and right", 15, 90);
        print_xy("botton to avoid", 20, 105);
        print_xy("rocket being hit ", 18, 120);
        print_xy("by yellow lights", 18, 135);
        while(gpio_get_level(35)) {
            if ((blink%35)==0) {
                setFont(FONT_SMALL);
                setFontColour(0, 0, 30);
                print_xy("START", 46, 165);
                print_xy("Press right button", 12, 180);
            }
            if ((blink%10)==0){
                setFont(FONT_SMALL);
                setFontColour(255, 255, 255);
                print_xy("START", 46, 165);
                print_xy("Press right button", 12, 180);
            }
            flip_frame();
            blink++;
        }
        flip_frame();

        while(s<=20000) {
            setFont(FONT_SMALL);
            setFontColour(255, 255, 255);
            sprintf(score, "%d", s);
            cls(rgbToColour(0,0,30));
            print_xy("Score:", 55, 3);
            print_xy(score, 100, 3);
            for(int i=0;i<100;i++) {
                draw_pixel(stars[i].x,stars[i].y>>8,stars[i].colour);
                stars[i].y += stars[i].speed;
                if(stars[i].y>=display_height*256) {
                    stars[i].x=rand()%display_width;
                    stars[i].y=0;
                    stars[i].speed=rand()%512+64;
                }
            }
            if (count%1200==0) {
                speed+=0.3;
            }
            draw_image(&spaceship_image, d+w/2,display_height-h/2);
            if(!gpio_get_level(0)) {
                d-=2;
                if(d<0) d=0;
            }
            if(!gpio_get_level(35)) {
                d+=2;
                if(d>display_width-w) d=display_width-w;
            }
            if(s>=0 && s<=2000){
                l=2;
            }else if(s>2000 && s<=10000){
                l=3;
            }else if(s>10000 && s<=15000){
                l=4;
            }else{
                l=5;
            }
            for(k=0; k<l; k++){
                draw_rectangle(x1[k], y1[k], 3, 15, rgbToColour(255,216,0));
                y1[k]+=vol[k]+speed;
                if(y1[k] > 235) {
                    x1[k]=rand()%133;
                    y1[k]=0;
                    vol[k]=rand()%3+0.3;
                    s+=100;
                }
                if((x1[k]>d) && (x1[k]<(d+w)) && (y1[k]>(240-h))) {
                    hit=true;
                    break;
                }
            }
            if(hit) break;
            flip_frame();
            count++;
        }
        if(hit) {
            setFont(FONT_UBUNTU16);
            setFontColour(255, 64, 0);
            cls(rgbToColour(0,0,30));
            print_xy("GAME OVER", 19, 85);
            print_xy("___________", 19, 89);
            setFont(FONT_SMALL);
            setFontColour(216, 216, 0);
            sprintf(score, "%d", s);
            print_xy("Score:", 30, 115);
            print_xy(score, 75, 115);
            blink=0;
            while(1) {
                if ((blink>100)&&((blink%35)==0)) {
                    setFont(FONT_SMALL);
                    setFontColour(0, 0, 30);
                    print_xy("RESTART", 44, 150);
                    print_xy("Press right button", 14, 165);
                }
                if ((blink>100)&&((blink%10)==0)){
                    setFont(FONT_SMALL);
                    setFontColour(255, 255, 255);
                    print_xy("RESTART", 44, 150);
                    print_xy("Press right button", 14, 165);
                }
                if ((!gpio_get_level(35)&&(blink>150))) break;
                flip_frame();
                blink++;
            }
        }else{
            setFont(FONT_UBUNTU16);
            setFontColour(255, 64, 0);
            cls(rgbToColour(0,0,30));
            print_xy(" WINNER! ", 19, 65);
            print_xy("___________", 19, 69);
            setFont(FONT_SMALL);
            setFontColour(216, 216, 0);
            sprintf(score, "%d", s);
            print_xy("Score:", 30, 95);
            print_xy(score, 75, 95);
            print_xy(" You have reached ", 14, 110);
            print_xy("  the max score!  ", 14, 125);
            blink=0;
            while(1) {
                if ((blink>100)&&((blink%35)==0)) {
                    setFont(FONT_SMALL);
                    setFontColour(0, 0, 30);
                    print_xy("RESTART", 44, 160);
                    print_xy("Press right button", 14, 175);
                }
                if ((blink>100)&&((blink%10)==0)){
                    setFont(FONT_SMALL);
                    setFontColour(255, 255, 255);
                    print_xy("RESTART", 44, 160);
                    print_xy("Press right button", 14, 175);
                }
                if ((!gpio_get_level(35)&&(blink>150))) break;
                flip_frame();
                blink++;
            }
        }
        flip_frame();
    }
}


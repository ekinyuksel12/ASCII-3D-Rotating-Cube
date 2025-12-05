#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 320
#define HEIGHT 88

//Character that is used for the background
int backgroundASCII = ' ';
float incrementSpeed = 0.6;
int distanceFromCam = 100;
float K1 = 40;

//Degrees for rotation
float A,B,C;
float x,y,z;
float ooz; //one over z
int xProjected, yProjected;
int idx;

float cubeWidth = 20;
int width = WIDTH;
int height = HEIGHT;

//Creating buffers for charecters on the screen and depth map.
char screenBuffer[WIDTH * HEIGHT];
float zBuffer[WIDTH * HEIGHT];

//Rotation matrix calculations for the points
float calculateX(float i, float j, float k) {
    return i * (cos(B) * cos(C)) 
        + j * (sin(A) * sin(B) * cos (C) + cos(A) * sin(C))
        + k * (-sin(B) * cos(A) * cos(C) + sin(A) * sin(C));
}

float calculateY(float i, float j, float k) {
    return -i * (cos(B) * sin(C)) 
        - j * (sin(A) * sin(B) * sin (C) - cos(A) * cos(C))
        + k * (sin(B) * cos(A) * sin(C) + sin(A) * cos(C));
}

float calculateZ(float i, float j, float k) {
    return i * (sin(B)) 
        - j * (sin(A) * cos(B))
        + k * (cos(A) * cos(B));
}

void calculatePoints (float cubeX, float cubeY, float cubeZ, int ch){
    x = calculateX(cubeX, cubeY, cubeZ);
    y = calculateY(cubeX, cubeY, cubeZ);
    z = calculateZ(cubeX, cubeY, cubeZ) + distanceFromCam;

    ooz = 1/z;

    xProjected = (int)(width / 2 + K1 * ooz * x * 2);
    yProjected = (int)(height / 2 + K1 * ooz * y);

    idx = xProjected + yProjected * width;
    if (idx >= 0 && idx < width * height) {
        if (ooz > zBuffer[idx]){
            zBuffer[idx] = ooz;
            screenBuffer[idx] = ch;
        }
    }
}

int main () {
    printf("\x1b[2J"); //Clear Screen ANSII code
    
    //Main loop
    while (1) {
        //Clear buffers
        memset(screenBuffer, backgroundASCII, width * height);
        memset(zBuffer, 0, width * height * 4);

        for (float cubeX = -cubeWidth; cubeX < cubeWidth; cubeX += incrementSpeed){
            for (float cubeY = -cubeWidth; cubeY < cubeWidth; cubeY += incrementSpeed){
                calculatePoints(cubeX, cubeY, -cubeWidth, '@');
                calculatePoints(cubeWidth, cubeY, cubeX, '$');
                calculatePoints(-cubeWidth, cubeY, -cubeX, '~');
                calculatePoints(-cubeX, cubeY, cubeWidth, '#');
                calculatePoints(cubeX, -cubeWidth, -cubeY, ';');
                calculatePoints(cubeX, cubeWidth, cubeY, '+');
            }
        }

        printf("\x1b[H");
        for (int k = 0; k < width * height; k++){
            putchar(k % width ? screenBuffer[k]: 10);
        }      
        
        A += 0.05;
        B += 0.05;
        C += 0.05;
        usleep(8000 * 2);
    }

    return 0;
}
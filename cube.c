#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 320
#define HEIGHT 88

const char *luminance = ".,-~:;=!*#$@";

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

float lightX = 0, lightY = -1, lightZ = -1;
float magnitudeOfLight = sqrt(2);

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

char calculateShade(int normalX, int normalY, int normalZ){
    float rotatedNormalX = calculateX(normalX, normalY, normalZ);
    float rotatedNormalY = calculateY(normalX, normalY, normalZ);
    float rotatedNormalZ = calculateZ(normalX, normalY, normalZ);

    // Dot product of normal vector and light vector tells us how aligned the surface is with the light
    float lightAllignment = (rotatedNormalX * lightX + rotatedNormalY * lightY + rotatedNormalZ * lightZ);

    // Normalize L by dividing by the magnitude of the light vector. 
    // Now L is guaranteed to be in the range [-1.0, 1.0]
    lightAllignment = lightAllignment / magnitudeOfLight;

    // Map the L value to our ASCII palette. L is now in range [0, 11]
    int index = (int)((lightAllignment + 1.0f) * 5.5f);

    // Clamp the index
    if (index < 0) index = 0;
    if (index > 11) index = 11;

    return luminance[index];
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

        char shadeBack  = calculateShade(0, 0, -1);
        char shadeRight = calculateShade(1, 0, 0);
        char shadeLeft  = calculateShade(-1, 0, 0);
        char shadeFront = calculateShade(0, 0, 1);
        char shadeTop   = calculateShade(0, -1, 0);
        char shadeBottom= calculateShade(0, 1, 0);

        for (float cubeX = -cubeWidth; cubeX < cubeWidth; cubeX += incrementSpeed){
            for (float cubeY = -cubeWidth; cubeY < cubeWidth; cubeY += incrementSpeed){
                calculatePoints(cubeX, cubeY, -cubeWidth, shadeBack);
                calculatePoints(cubeWidth, cubeY, cubeX, shadeRight);
                calculatePoints(-cubeWidth, cubeY, -cubeX, shadeLeft);
                calculatePoints(-cubeX, cubeY, cubeWidth, shadeFront);
                calculatePoints(cubeX, -cubeWidth, -cubeY, shadeTop);
                calculatePoints(cubeX, cubeWidth, cubeY, shadeBottom);
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
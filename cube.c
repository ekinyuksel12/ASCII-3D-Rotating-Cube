#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

//Console Size
#define WIDTH 320
#define HEIGHT 88

#define TWO_PI 6.283185307f

const char *luminance = ".,-~:;=!*#$@";

//Character that is used for the background
int backgroundASCII = ' ';
float incrementSpeed = 0.8f;
int distanceFromCam = 110;
float K1 = 40;

//Degrees for rotation
float A, B, C;
float lightAngleIncrement = 0.0f;
float x,y,z;
float ooz; //one over z
int xProjected, yProjected;
int idx;

float lightX = -30, lightY = 0, lightZ = -110;
float magnitudeOfLight = sqrt(2);

float cubeWidth = 35;
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

float vectorMagnitude (float  x, float y, float z){
    return sqrt(x * x + y * y + z * z);
}

char calculateShade(float pixelX, float pixelY, float pixelZ, int normalX, int normalY, int normalZ){
    float rotatedNormalX = calculateX(normalX, normalY, normalZ);
    float rotatedNormalY = calculateY(normalX, normalY, normalZ);
    float rotatedNormalZ = calculateZ(normalX, normalY, normalZ);

    // Calculate the Vector from the Pixel to the Light Source
    float lx = lightX - pixelX;
    float ly = lightY - pixelY;
    float lz = lightZ - pixelZ;

    // Normalize light vectors by dividing by the magnitude of the light vector. 
    // Now light vectors are guaranteed to be in the range [-1.0, 1.0]
    float distance = vectorMagnitude(lx, ly, lz);
    lx /= distance;
    ly /= distance;
    lz /= distance;

    // Calculate Dot Product (Normal . LightVector)
    float lightAlignment = (rotatedNormalX * lx + rotatedNormalY * ly + rotatedNormalZ * lz);

    // If the dot product is < 0, the face is pointing away from the light
    // Make them the darkest ASCII character
    if (lightAlignment < 0) return '.';

    float ambient = 0.2; 
    float intensity = lightAlignment + ambient;

    // Clamp intensity strictly between 0 and 1
    // This handles the shadows gracefully without a hard 'if' check
    if (intensity < 0) intensity = 0;
    if (intensity > 1) intensity = 1;

    float noise = ((float)(rand() % 100) / 100.0f) * 0.1f - 0.05f;

    intensity += noise;
    
    // Map the L value to our ASCII palette. L is now in range [0, 11]
    int index = (int)((intensity + 1.0f) * 5.5f);

    // Clamp the index
    if (index < 0) index = 0;
    if (index > 11) index = 11;

    return luminance[index];
}

void calculatePoints (float cubeX, float cubeY, float cubeZ, int normalX, int normalY, int normalZ){
    x = calculateX(cubeX, cubeY, cubeZ);
    y = calculateY(cubeX, cubeY, cubeZ);

    float rawZ = calculateZ(cubeX, cubeY, cubeZ);

    z = rawZ + distanceFromCam;

    char ch = calculateShade(x, y, rawZ, normalX, normalY, normalZ);

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

        magnitudeOfLight = vectorMagnitude(lightX, lightY, lightZ);

        for (float cubeX = -cubeWidth; cubeX < cubeWidth; cubeX += incrementSpeed){
            for (float cubeY = -cubeWidth; cubeY < cubeWidth; cubeY += incrementSpeed){
                calculatePoints(cubeX, cubeY, -cubeWidth, 0, 0, -1);
                calculatePoints(cubeWidth, cubeY, cubeX, 1, 0, 0);
                calculatePoints(-cubeWidth, cubeY, -cubeX, -1, 0, 0);
                calculatePoints(-cubeX, cubeY, cubeWidth, 0, 0, 1);
                calculatePoints(cubeX, -cubeWidth, -cubeY, 0, -1, 0);
                calculatePoints(cubeX, cubeWidth, cubeY, 0, 1, 0);
            }
        }

        printf("\x1b[H");
        for (int k = 0; k < width * height; k++){
            putchar(k % width ? screenBuffer[k]: 10);
        }      
        
        A += 0.05; if(A > TWO_PI) A -= TWO_PI;
        B += 0.05; if(B > TWO_PI) B -= TWO_PI;
        C += 0.05; if(C > TWO_PI) C -= TWO_PI;

        float tempX = lightX;
        float tempZ = lightZ;
        
        lightX = tempX * cos(lightAngleIncrement) + tempZ * sin(lightAngleIncrement);
        lightZ = -tempX * sin(lightAngleIncrement) + tempZ * cos(lightAngleIncrement);

        usleep(8000 * 2);
    }

    return 0;
}
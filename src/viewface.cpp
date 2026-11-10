#include "raylib.h"
#include "raymath.h"
#include <random>
#include <bits/stdc++.h>

class ViewFace{
    private:
        Vector2 facePoints[4];
        //int pointCount = 4;

        Color color;
        float lightMult;
    public:
        ViewFace (Vector2* points[4], float lightMultiplier)
        :
        facePoints{*points[0],*points[1],*points[2],*points[3]},
        lightMult(lightMultiplier)
        {

            std::random_device dev;
            std::mt19937 r1(dev());
            std::mt19937 r2(dev());
            std::mt19937 r3(dev());

            std::uniform_int_distribution<std::mt19937::result_type> dist(190,215);
            color =  {
                (unsigned char)(dist(r1)),
                (unsigned char)(dist(r2)),
                (unsigned char)(dist(r3)),
                (unsigned char)(255)
            };
        }

        void renderViewFace(){
            Color finalCol({
                (unsigned char)(std::clamp(color.r*lightMult,0.0f,255.0f)),//raylib does have a clamp function
                (unsigned char)(std::clamp(color.g*lightMult,0.0f,255.0f)),
                (unsigned char)(std::clamp(color.b*lightMult,0.0f,255.0f)),
                color.a
            });
            //std::cout << "rendering face";
            //printf("hello");
            //Vector2 fp* = facePoints;
            Vector2 *firstPointer = facePoints;
            //DrawCircleV(*firstPointer, 10, RED);
            //DrawCircleV(*(firstPointer+1), 10, ORANGE);
            //DrawCircleV(*(firstPointer+2), 10, GREEN);
            //DrawCircleV(*(firstPointer+3), 10, BLUE);

            //DrawLineV(*firstPointer, *(firstPointer+1), BLACK); //facepoints should be Vector2 *points
            //DrawTriangleLines(*firstPointer, *(firstPointer+1),*(firstPointer+2), BLUE);
            //DrawTriangle(*firstPointer, *(firstPointer+2),*(firstPointer+1), MAGENTA);

            DrawTriangleFan(firstPointer, 4, finalCol); //facepoints should be Vector2 *points

            //drawLinesFromPoints(facePoints);
            //color.Brightness(lightMult-1.0f)
            //DrawTriangle({100,100}, {100,200}, {200, 100}, RED);

        }
};

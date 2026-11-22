#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

class ViewFace{
    private:
        Vector2 facePoints[4];
        int pointCount;

        //Color color;
        float lightMult;
        struct TextureVertex{
            Vector2 position;
            Vector2 uv;
        };
        std::vector<TextureVertex> textureVertices;
        Color applyLighting(Color color) const{
            return Color({
                (unsigned char)(std::clamp(color.r*lightMult,0.0f,255.0f)),//raylib does have a clamp function
                (unsigned char)(std::clamp(color.g*lightMult,0.0f,255.0f)),
                (unsigned char)(std::clamp(color.b*lightMult,0.0f,255.0f)),
                color.a
            });
        }

        void cacheTextureVertices(){
            // Cache the texture mapping for the fixed face geometry.
            if (pointCount == 4){
                Vector2 d1 = Vector2Subtract(facePoints[1], facePoints[2]);
                Vector2 d2 = Vector2Subtract(facePoints[3], facePoints[2]);
                Vector2 d3 = Vector2Add(Vector2Subtract(facePoints[0], facePoints[1]), Vector2Subtract(facePoints[2], facePoints[3]));
                float determinant = d1.x*d2.y - d2.x*d1.y;
                float g = 0, h = 0;
                if (std::abs(determinant) > 0.000001f){
                    g = (d3.x*d2.y - d2.x*d3.y)/determinant;
                    h = (d1.x*d3.y - d3.x*d1.y)/determinant;
                }
                Vector2 a = Vector2Add(Vector2Subtract(facePoints[1], facePoints[0]), Vector2Scale(facePoints[1], g));
                Vector2 b = Vector2Add(Vector2Subtract(facePoints[3], facePoints[0]), Vector2Scale(facePoints[3], h));
                auto addVertex = [&](float u, float v){
                    float w = g*u + h*v + 1;
                    textureVertices.push_back({{(a.x*u + b.x*v + facePoints[0].x)/w, (a.y*u + b.y*v + facePoints[0].y)/w}, {u,v}});
                };
                const int divisions = 12;
                textureVertices.reserve(divisions*divisions*6);
                for (int y = 0; y < divisions; y++){
                    for (int x = 0; x < divisions; x++){
                        float u0 = (float)x/divisions, u1 = (float)(x+1)/divisions;
                        float v0 = (float)y/divisions, v1 = (float)(y+1)/divisions;
                        addVertex(u0,v0); addVertex(u1,v0); addVertex(u1,v1);
                        addVertex(u0,v0); addVertex(u1,v1); addVertex(u0,v1);
                    }
                }
            }
            else{
                Vector2 minimum = facePoints[0], maximum = facePoints[0];
                for (int i = 1; i < pointCount; i++){
                    minimum = Vector2Min(minimum, facePoints[i]);
                    maximum = Vector2Max(maximum, facePoints[i]);
                }
                Vector2 size = Vector2Max(Vector2Subtract(maximum, minimum), {1,1});
                for (int i = 0; i < pointCount; i++){
                    textureVertices.push_back({facePoints[i], {(facePoints[i].x-minimum.x)/size.x, (facePoints[i].y-minimum.y)/size.y}});
                }
            }
        }

        void renderTexturedFace(Color finalCol, Texture2D texture){
            if (textureVertices.empty()) cacheTextureVertices();
            rlCheckRenderBatchLimit((int)textureVertices.size());
            rlSetTexture(texture.id);
            rlBegin(RL_TRIANGLES);
            rlColor4ub(finalCol.r, finalCol.g, finalCol.b, finalCol.a);
            for (const TextureVertex& vertex : textureVertices){
                rlTexCoord2f(vertex.uv.x, vertex.uv.y);
                rlVertex2f(vertex.position.x, vertex.position.y);
            }
            rlEnd();
            rlSetTexture(0);
        }
    public:
        ViewFace (Vector2* points[], float lightMultiplier, int point_count = 4)
        :
        facePoints{},
        pointCount(point_count),
        lightMult(lightMultiplier)
        {
            for (int i = 0; i < pointCount; i++){
                facePoints[i] = *points[i];
            }

            std::random_device dev;// make static
            std::mt19937 r1(dev());
            std::mt19937 r2(dev());
            std::mt19937 r3(dev());

            std::uniform_int_distribution<std::mt19937::result_type> dist(190,215); // make static
            Color color =  {
                (unsigned char)(dist(r1)),
                (unsigned char)(dist(r2)),
                (unsigned char)(dist(r3)),
                (unsigned char)(255)
            };
        }

        void renderViewFace(Color color, Texture2D texture = {}){
            Color finalCol = applyLighting(color);
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

            if (texture.id == 0){
                DrawTriangleFan(firstPointer, pointCount, finalCol);
            }
            else{
                renderTexturedFace(finalCol, texture);
            }

            //drawLinesFromPoints(facePoints);
            //color.Brightness(lightMult-1.0f)
            //DrawTriangle({100,100}, {100,200}, {200, 100}, RED);

        }
};

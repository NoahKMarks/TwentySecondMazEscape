/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/
//./build/external/raylib-master/src/raylib.h
//./build/external/raylib-master/src/raymath.h
//./src/resource_dir.h
#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
//#include <Color.hpp>
//#include "stdafx.h"
#include <string>
#include <iostream>

#include <vector>
#include <unordered_map>

#include "viewface.cpp"
#include "block.cpp"
#include "board.cpp"

std::ostream& operator<<(std::ostream& os, const Vector2& v) { //overload << so it can print Vector2
    return os << "(" << v.x << ", " << v.y << ")";
}


const int screenWidth = 900;
const int screenHeight = 600;

const int block_separation = 1.0; //3d separation between each grid cell in the world
const float light_factor = 3.0; //stops further cells getting dark too quickly; might be similar to where light starts dropping off
const float light_scale = block_separation/light_factor;

void drawLinesFromPoints(const Vector2 (&points)[4]){
    for (int i = 0; i<4; i++){

		    DrawLineV(points[i%4], points[(i+1)%4], LIME);
	}
}

std::vector<ViewFace> generateDisplay(const int tunnel_depth){

    //-----Some Constants For The Function-----
    const Vector2 center = {screenWidth/2,screenHeight/2};
    //const int tunnel_depth = 4;
	const float ring_separation = 50.0f;
	const float display_size = 500.0f;
	const float display_radius = display_size/2.0;

	const int ring_count = tunnel_depth+1;
	const int total_points = ring_count*4;
	const int face_count = tunnel_depth*4+1; //one center and four per ring

	//-----Genarates Points for the Quads Faces-----
	// starts at most outer points and goes in so loop backwards
	std::vector<Vector2> displayPoints(total_points);
	for (int i=ring_count-1; i>=0; i--){

		displayPoints[i*4] = {-display_radius/(i+1),display_radius/(i+1)};
		displayPoints[i*4+1] = {display_radius/(i+1),display_radius/(i+1)};
		displayPoints[i*4+2] = {display_radius/(i+1),-display_radius/(i+1)};
        displayPoints[i*4+3] = {-display_radius/(i+1),-display_radius/(i+1)};

	}

	for (int i=0; i<total_points; i++){//shift points to center
	    displayPoints[i]= Vector2Add(displayPoints[i], center);
	}

	std::vector<ViewFace> displayFaces; //Array Of Faces that will be returned
	displayFaces.reserve(face_count); // Save contiguous Memory for when objects are created


	//This Loop makes faces from the point and adds new objects to the array
	for (int i=0; i<face_count-1; i++){
	    //Makes The faces
		Vector2 *zone_points[4]= {
					&displayPoints[i],
					((i%4)==3)? &displayPoints[i-3] : &displayPoints[i+1],
					((i%4)==3)? &displayPoints[i+1] : &displayPoints[i+5], // Ternary Operator checks if at ring wraparound case
					&displayPoints[i+4],
		};//there is always 3 more verticies than faces(including centre so no out of bounds)

		//calculate faces Lighting Factor
		int row_num = i/4;
		float factored_distance_squared = (row_num*light_scale*row_num*light_scale) + (light_scale/2)*(light_scale/2);
		float lightFactor = 1.0f/(1.0f+factored_distance_squared);
		//std::cout << "row_num: " <<row_num<< "  |  face_num: "<< i << " |  lightFactor: "<< lightFactor << "\n";
		//adds Face
		displayFaces.emplace_back(zone_points, (float)(lightFactor));
	}
	//Make the Centre end Face and put at end
	Vector2 *center_face_points[4] = {
				&displayPoints[tunnel_depth*4],
				&displayPoints[tunnel_depth*4+1],
				&displayPoints[tunnel_depth*4+2],
				&displayPoints[tunnel_depth*4+3],
	};
	displayFaces.emplace_back(center_face_points, 1/(1.0+tunnel_depth*light_scale*tunnel_depth*light_scale));//Also calculates face light factor

	return displayFaces;
}

class Block{
    private:
        Block* back2;
        Block* front3;
        Block* left4;
        Block* right1;
        Block* bottom5;
        Block* top0;

    public:
        Block()
        {

        }

        int add_connection(Block from, Vector2 dir){
            return 0
        }
};


void makeMazeBoard(){
    int steps_to_finish = 10;
    Vector2 position = {0,0};
    Vector2 direction = {0,1};
    std::vector<Block> block_array;
    std::unordered_map<Vector2, &Block> positions_to_blocks;
    int block_num = 0;
    Block& prev_block;
    Block& new_block;
    while (steps_to_finish > 0){
        block_array.emplace_back()
        new_block = &block_array[block_num];
        block_num++;

        new_block.add_connection(prev_block, direction);
        positions_to_blocks[position] = newBlock;
        prev_block = new_block;
        //change direction (and maybe make it an enum or int instead of vector)
        position = Vector2Add(position,direction);
        steps_to_finish --
        std::cout << position << "\n";
    }
    std::cout << "done \n";


}


int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Load a texture from the resources directory
	Texture wabbit = LoadTexture("wabbit_alpha.png");



	InitWindow(screenWidth, screenHeight, "Hello Raylib, This is noah");
// points must be in counter clockwise order

	//std::vector<ViewFace> display_faces = generateDisplay(5);
	makeMazeBoard();


	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(MAGENTA);

		// draw some text using the default font
		//DrawText("Hello Raylib, Hello Noah", 200,200,20,LIME);

		// draw our texture to the screen
		//DrawTexture(wabbit, 800, 200, RED);

		for(int i=0; i < display_faces.size(); i++){
            display_faces[i].renderViewFace();
        }


		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();

		WindowShouldClose();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(wabbit);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}

/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "raymath.h"
//#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
//#include "stdafx.h"
#include <string>
#include <iostream>
#include <random>


#include <vector>
#include <map>
#include <unordered_map>

#include "viewface.cpp"
#include "block.cpp"
#include "board.cpp"

const int screenWidth = 900;
const int screenHeight = 600;
const Vector2 screenCentre((float)screenWidth/2, (float)screenHeight/2);

const int block_separation = 1.0; //3d separation between each grid cell in the world
const float light_factor = 3.0; //stops further cells getting dark too quickly; might be similar to where light starts dropping off
const float light_scale = block_separation/light_factor;

const float map_scale = 30;

struct IntVector{
    int x,y;

    IntVector(int _x, int _y)
    :
    x(_x),
    y(_y)
    {}

    static IntVector addIntVector(IntVector v1,IntVector v2){
        IntVector result = {v1.x + v2.x, v1.y+v2.y};
        return result;
    }
    void displayVector(float size=1, Vector2 offset = screenCentre){
        Vector2 converted(x*map_scale, y*map_scale);
        Vector2 drawpos = Vector2Add(offset, converted);
        DrawCircleV(drawpos, size*map_scale/2, WHITE);
    }

    bool operator==(const IntVector& other) const {
            return ((x == other.x) && (y == other.y)) ;
    }

    static IntVector rotateVector(IntVector v, bool anticlockwise){
      if (anticlockwise){
        IntVector result(v.y,-v.x);
        return result;
      }
      else{
        IntVector result(-v.y,v.x);
        return result;
      }
    }
};

template<> struct std::hash<IntVector> {
    std::size_t operator()(IntVector const& s) const noexcept {
        std::size_t x = std::hash<int>{}(s.x);
        std::size_t y = std::hash<int>{}(s.y);
        return x ^ (y << 1); // or use boost::hash_combine (see Discussion) https://en.cppreference.com/w/Talk:cpp/utility/hash
    }
};

int randomInteger(int min = 0, int max = 10){
	std::random_device dev;
    std::mt19937 randomNum(dev());        
    std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
    return dist(randomNum);
}

bool randomBool(){
	std::random_device dev;
    std::mt19937 randomNum(dev());        
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 1);
    int result = dist(randomNum);
    return ((result==1)? true : false);
}

std::ostream& operator<<(std::ostream& os, const Vector2& v) { //overload << so it can print Vector2
    return os << "(" << v.x << ", " << v.y << ")";
}

std::ostream& operator<<(std::ostream& Stream,const IntVector vec) {
  Stream << "x: "<<vec.x<< " y: " << vec.y << "\n";
  return Stream;
}

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
class Face{
	private:
		Block* next_block;
		Color color;
	
	public:
		Face()
		:
		next_block(nullptr)
		{
			std::random_device dev;
            std::mt19937 r1(dev());
            //std::mt19937 r2(dev());
            ////std::mt19937 r3(dev());

            std::uniform_int_distribution<std::mt19937::result_type> dist(150,250);
            this.color =  {
                (unsigned char)(dist(r1)),
                (unsigned char)(200),
                (unsigned char)(200),
                (unsigned char)(255)
            };
		}
		void setNextBlock(Block* to_block){
			this.next_block = to_block;
		}
		
		Block* getNextBlock(){
			return next_block;
		}
};

class Block{
    bool operator==(const Block& other) const {
            return blockID == other.blockID;
    }

    private:
		std::unordered_map<IntVector, Face> direction_to_face;
        
        //Face back2;
        //Face front3;
        //Face left4;
        //Face right1;
        //Face bottom5;
        //Face top0;

    public:
        int blockID;
        IntVector pos;

        Block()
        :
        pos(0,0)
        
        //    back2(nullptr),
        //    front3(nullptr),
        //    left4(nullptr),
        //    right1(nullptr),
        //    bottom5(nullptr),
        //    top0(nullptr),
        {
			direction_to_face[IntVector{1,0}] = Face();
			direction_to_face[IntVector{-1,0}] = Face();
			direction_to_face[IntVector{0,1}] = Face();
			direction_to_face[IntVector{0,-1}] = Face();
			//direction_to_face[IntVector{0,0}] = Face();
			//direction_to_face[IntVector{0,0}] = Face();        
		}

        int add_connection(Block from, IntVector dir){
            return 0;
        }
        Face get_next_block(IntVector dir){
			return direction_to_face[dir]; 
        }
};


std::vector<Block> makeMazeBoard(const int maze_steps = 10){
    int steps_to_finish = maze_steps+1;
    IntVector position = {0, 0};
    IntVector direction = {1, 0};
    std::vector<Block> block_array;
    std::unordered_map<IntVector, Block*> positions_to_blocks;

    int block_num = 0;
    Block* prev_block = nullptr;
    Block* new_block;
    while (steps_to_finish > 0){
        block_array.emplace_back();
        new_block = &block_array[block_num];
        (*new_block).pos = position;

        block_num++;
        if (prev_block != nullptr){
            (*new_block).add_connection((*prev_block), direction);
            //(*prev_block).add_connection((*new_block), -direction);
        }
        positions_to_blocks[position] = new_block;
        //std::cout << position << "\n";
        //positions_to_blocks.emplace(position, *new_block);
        prev_block = new_block;
        //change direction (and maybe make it an enum or int instead of vector)
        bool shouldTurn = randomBool();
        if (shouldTurn){
			bool isRight = randomBool();
			if (isRight){
				direction = IntVector::rotateVector(direction,false);
			}
			else{
			direction = IntVector::rotateVector(direction,true);
			}	
		}
        position = IntVector::addIntVector(position,direction);
        steps_to_finish --;
    }
    for (const Block& block : block_array){
        //std::cout << " pos " << block.pos << "\n";
		;
    }

        std::cout << "done \n";
	return block_array;

}

class Player{
  private:
	IntVector direction;
	IntVector position;
  public:
	Player()
	:
		direction{0,1},
		position{0,0}
	{}
	void renderPlayer(Vector2 draw_offset = screenCentre){

		Vector2 scaledPos(position.x*map_scale, position.y*map_scale);
        Vector2 drawpos = Vector2Add(draw_offset, scaledPos);
        DrawCircleV(drawpos, 0.75*map_scale/2, RED);

        Vector2 headOffset(direction.x*map_scale, direction.y*map_scale/2);
		Vector2 head_pos = Vector2Add(drawpos,headOffset);
		DrawCircleV(head_pos, 0.5*map_scale/2, PURPLE);
	}
};

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	//SearchAndSetResourceDir("resources");

	// Load a texture from the resources directory
	Texture wabbit = LoadTexture("wabbit_alpha.png");

	//Make Window
	InitWindow(screenWidth, screenHeight, "Hello Raylib, This is noah");


	//Initialise main variables
	std::vector<ViewFace> display_faces = generateDisplay(5);
	std::vector<Block> mazeBoardBlocks = makeMazeBoard(20);
	Player player;
	std::cout<<"hello \n";
	//std::cout<<"there \n";

	Vector2 projectionOffset{10,10};
	Vector2 mapOffset{500,10};

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(SKYBLUE);

		// draw some text using the default font
		//DrawText("Hello Raylib, Hello Noah", 200,200,20,LIME);

		// draw our texture to the screen
		//DrawTexture(wabbit, 800, 200, RED);

	for(int i=0; i < display_faces.size(); i++){
            display_faces[i].renderViewFace();
        }
        for(int i=0; i < mazeBoardBlocks.size(); i++){
            mazeBoardBlocks[i].pos.displayVector();
        }
        player.renderPlayer();


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

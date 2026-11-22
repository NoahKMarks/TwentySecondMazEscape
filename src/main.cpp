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
const Vector2 screenCentre{(float)screenWidth/2, (float)screenHeight/2};

const int block_separation = 1.0; //3d separation between each grid cell in the world
const float light_factor = 3.0; //stops further cells getting dark too quickly; might be similar to where light starts dropping off
const float light_scale = block_separation/light_factor;

const float map_scale = 30;

//maze genaration
//
const int maze_steps = 10;

const int turn_rate = 3; //chance to make a turn in correct maze path per step
const int portal_rate = 6; //chance to attempt to spawn a portal is 1/portal_rate

// Order matches the seven faces generated for each ring.
enum class Direction {
    Bottom,
    Right,
    Top,
    Left,
    Front,
    FrontLeft,
    FrontRight
};

const int fpr = 7; //number of faces per ring

struct IntVector{
    int x,y,z;

    IntVector(int _x, int _y, int _z)
    :
    x(_x),
    y(_y),
    z(_z)
    {}

    static IntVector addIntVector(IntVector v1,IntVector v2){
        IntVector result = {v1.x + v2.x, v1.y+v2.y, v1.z+v2.z};
        return result;
    }
    IntVector reverseVector(){
        IntVector result = {-x,-y,-z};
        return result;
	}
    void displayVector(float size=1, Vector2 offset = screenCentre){
        Vector2 converted(x*map_scale, y*map_scale);
        Vector2 drawpos = Vector2Add(offset, converted);
        DrawCircleV(drawpos, size*map_scale/2, WHITE);
    }

    bool operator==(const IntVector& other) const {
            bool result = ((x == other.x) && (y == other.y) && (z == other.z));
            //std::cout << this << (IntVector)other << result << "\n";
            return result;
    }

    static IntVector rotateVectorZ(IntVector v, bool anticlockwise){
      if (anticlockwise){
        IntVector result(v.y,-+v.x,v.z);
        return result;
      }
      else{
        IntVector result(-v.y,v.x,v.z);
        return result;
      }
    }
    static IntVector rotateVectorX(IntVector v, bool anticlockwise){
      if (anticlockwise){
        IntVector result(v.y,-+v.x,v.z);
        return result;
      }
      else{
        IntVector result(-v.y,v.x,v.z);
        return result;
      }
    }
    //make versions for other direction rotations


};

template<> struct std::hash<IntVector> {
    std::size_t operator()(IntVector const& s) const noexcept {
        std::size_t x = std::hash<int>{}(s.x);
        std::size_t y = std::hash<int>{}(s.y);
        std::size_t z = std::hash<int>{}(s.z);
        return z * x ^ (y << 1); // or use boost::hash_combine (see Discussion) https://en.cppreference.com/w/Talk:cpp/utility/hash
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
    os << "x: "<<v.x<< " y: " << v.y << "\n";
    return os;

}

std::ostream& operator<<(std::ostream& os,const IntVector v) {
  return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
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
	const int face_count = tunnel_depth*fpr;

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

	for (int rn=0; rn<tunnel_depth; rn++){
    	for (int fn=0; fn<fpr; fn++){
            const Direction face_direction = static_cast<Direction>(fn);
        	int i = rn*4 + fn;
        	Vector2 *zone_points[4];
            Vector2 side_points[2];
       	    //Makes The faces, bottom, right, top, left
            if (fn < static_cast<int>(Direction::Front)){
          		zone_points[0] = &displayPoints[i];
          		zone_points[1] = (face_direction == Direction::Left)? &displayPoints[i-3] : &displayPoints[i+1];
                zone_points[2] = (face_direction == Direction::Left)? &displayPoints[i+1] : &displayPoints[i+5]; // Ternary Operator checks if at ring wraparound case
                zone_points[3] = &displayPoints[i+4];
            }
            else if (face_direction == Direction::Front){ // back face
     			zone_points[0] = &displayPoints[i];
          		zone_points[1] = &displayPoints[i+1];
                zone_points[2] = &displayPoints[i+2]; // Ternary Operator checks if at ring wraparound case
                zone_points[3] = &displayPoints[i+3];
      		}
            else if (face_direction == Direction::FrontLeft){
                const int outer = rn*4;
                const int inner = outer+4;
                side_points[0] = {displayPoints[outer].x, displayPoints[inner].y};
                side_points[1] = {displayPoints[outer+3].x, displayPoints[inner+3].y};
                zone_points[0] = &side_points[0];
                zone_points[1] = &displayPoints[inner];
                zone_points[2] = &displayPoints[inner+3];
                zone_points[3] = &side_points[1];
            }
            else if (face_direction == Direction::FrontRight){
                const int outer = rn*4;
                const int inner = outer+4;
                side_points[0] = {displayPoints[outer+1].x, displayPoints[inner+1].y};
                side_points[1] = {displayPoints[outer+2].x, displayPoints[inner+2].y};
                zone_points[0] = &displayPoints[inner+1];
                zone_points[1] = &side_points[0];
                zone_points[2] = &side_points[1];
                zone_points[3] = &displayPoints[inner+2];
            }

      		//calculate faces Lighting Factor
      		float factored_distance_squared = (rn*light_scale*rn*light_scale) + (light_scale/2)*(light_scale/2);
      		float lightFactor = 1.0f/(1.0f+factored_distance_squared);
      		//std::cout << "row_num: " <<row_num<< "  |  face_num: "<< i << " |  lightFactor: "<< lightFactor << "\n";
      		//adds Face
      		displayFaces.emplace_back(zone_points, (float)(lightFactor));
	    }
	}
	return displayFaces;
}


class Block; //Forward declaration so face knows it exists and can point to it
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
            std::mt19937 rng(dev());
            std::uniform_real_distribution<float> random(0.0f, 1.0f);

            float hue = random(rng)*360.0f;
            float saturation = random(rng)*0.05f;

            color = ColorFromHSV(hue, saturation, 0.82f);
		}
		void setNextBlock(Block* to_block){
			next_block = to_block;
		}

		Block* getNextBlock(){
			return next_block;
		}
		Color getFaceColor(){
		    return color;
		}

};

class Block{
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

        Block(int block_id = 0)
        :
        pos(0,0,0),
        blockID(block_id)
        //    back2(nullptr),
        //    front3(nullptr),
        //    left4(nullptr),
        //    right1(nullptr),
        //    bottom5(nullptr),
        //    top0(nullptr),
        {
            direction_to_face.reserve(6);
			direction_to_face[IntVector{1,0,0}] = Face();
			direction_to_face[IntVector{-1,0,0}] = Face();
			direction_to_face[IntVector{0,1,0}] = Face();
			direction_to_face[IntVector{0,-1,0}] = Face();

			direction_to_face[IntVector{0,0,1}] = Face();
			direction_to_face[IntVector{0,0,-1}] = Face();
		}

        Face* get_face_from_dir(IntVector dir){
/*
            std::cout << dir << "\n";

            std::cout << &direction_to_face[IntVector{1,0,0}] << "\n";
			std::cout << &direction_to_face[IntVector{-1,0,0}] << "\n";
			std::cout << &direction_to_face[IntVector{0,1,0}] << "\n";
			std::cout << &direction_to_face[IntVector{0,-1,0}] << "\n";
			std::cout << &direction_to_face[IntVector{0,0,1}] << "\n";
			std::cout << &direction_to_face[IntVector{0,0,-1}] << "\n";
*/
			return &(direction_to_face[dir]);
        }

		Block* get_next_block(IntVector dir){
            Face* next_face = get_face_from_dir(dir);
            std::cout << "next face " << next_face << " \n";
			return (*next_face).getNextBlock();
		}

        int add_connection(Block *toBlock, IntVector dir){
            Face& next_face = *(get_face_from_dir(dir));
            if (next_face.getNextBlock() == nullptr){
                std::cout << "connecting \n";
                next_face.setNextBlock(toBlock);
                return 0;
            }
            else{
                return 1;
            }
        }

		bool operator==(const Block& other) const {
            return blockID == other.blockID;
        }
};


std::vector<Block> makeMazeBoard(){
    int steps_to_finish = maze_steps+1;
    IntVector position = {0, 0, 0}; //Just used for visualisation Not actual game
    IntVector direction = {1, 0, 0};
    bool can_turn_left = true;
    bool can_turn_right = true;
    //int max_block_count = 20; // must be bigger than steps to finish
    std::vector<Block> block_array;
    block_array.reserve(steps_to_finish);
    //block_array.reserve(max_block_count);

    //std::unordered_map<IntVector, Block*> positions_to_blocks;

    int block_num = 0;
    Block* prev_block = nullptr;
    Block* new_block = nullptr;
    while (steps_to_finish > 0){
        prev_block = new_block;

        block_array.emplace_back(block_num);//use block_num as its ID
        new_block = &block_array[block_num];
        (*new_block).pos = position;

        block_num++;

        // connect blocks
        if (prev_block != nullptr){
            (*new_block).add_connection(prev_block, direction.reverseVector());
            (*prev_block).add_connection(new_block, direction);
        }

        //RANDOMLY ADD PORTAL ON ONE SIDE TO PREVIOUS BLOCK
        if (randomInteger(0, portal_rate)==0){
            //IntVector portal_dir = IntVector.rotateVector();
            ;//make a portal (ie connection to previous block)
        }
        // add to dictionary
        //positions_to_blocks[position] = new_block;

        //change direction (and maybe make it an enum or int instead of vector)
        bool shouldTurn = randomBool();
        bool isRight = randomBool();
        if (shouldTurn && isRight && can_turn_right){
			direction = IntVector::rotateVectorZ(direction,false);
			can_turn_left = false;
		}
		else if(shouldTurn && !isRight && can_turn_left){
		    direction = IntVector::rotateVectorZ(direction,true);
			can_turn_right = false;
		}
        else {
            can_turn_left = true;
            can_turn_right = true;
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
  public:
    IntVector direction;
	//IntVector position;
	Block* current_block;// pointer to block in maze

    Player(Block* start_block)
	:
		current_block(start_block),
		direction{0,1,0}
		//position{0,0,0}
	{}
	void renderPlayer(Vector2 draw_offset = screenCentre){
		IntVector position = (*current_block).pos;
        //std::cout << position << "\n";
		Vector2 scaledPos(position.x*map_scale, position.y*map_scale);
        Vector2 drawpos = Vector2Add(draw_offset, scaledPos);
        DrawCircleV(drawpos, 0.75*map_scale/2, RED);

        Vector2 headOffset(direction.x*map_scale/2, direction.y*map_scale/2);
		Vector2 head_pos = Vector2Add(drawpos,headOffset);
		DrawCircleV(head_pos, 0.5*map_scale/2, BLUE);
	}
	void turn_player(bool is_right){
	    direction = IntVector::rotateVectorZ(direction, !is_right);
	}
	void move_forward(){
		Block* next_block = (*current_block).get_next_block(direction);
        std::cout << "current block : "<< current_block << "\n";
		if (next_block != nullptr){
            std::cout << "moving \n";
			current_block = next_block;
		}
		else{
            std::cout << "blocked \n";
        }
	}
};

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	//SearchAndSetResourceDir("resources");

	//Make Window
	InitWindow(screenWidth, screenHeight, "Hello Raylib, This is noah");


	//Initialise main variables
	std::vector<ViewFace> display_faces = generateDisplay(7);
	std::vector<Block> mazeBoardBlocks = makeMazeBoard();
	Player player(&mazeBoardBlocks.front());

	Vector2 projectionOffset{10,10};
	Vector2 mapOffset{700,450};

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
    	if (IsKeyPressed(KEY_RIGHT)){
            player.turn_player(true);
            std::cout << "right \n";
    	}
        if (IsKeyPressed(KEY_LEFT)){
            player.turn_player(false);
            std::cout << "left \n";
    	}
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_UP)){
            player.move_forward();
            std::cout << "forwards \n";
    	}
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(PURPLE);

		// draw some text using the default font
		//DrawText("Hello Raylib, Hello Noah", 200,200,20,LIME);

		// draw our texture to the screen
		//DrawTexture(wabbit, 800, 200, RED);
		Block* viewBlock = player.current_block;
		for(int i=0; i < display_faces.size(); i++){
		    const Direction face_direction = static_cast<Direction>(i % fpr);
		    Face* face = nullptr;

			switch (face_direction){
			    case Direction::Left:
			        face = viewBlock->get_face_from_dir(IntVector::rotateVectorZ(player.direction, true));
			        break;
			    case Direction::Right:
			        face = viewBlock->get_face_from_dir(IntVector::rotateVectorZ(player.direction, false));
			        break;
			    case Direction::Front:
			        face = viewBlock->get_face_from_dir(player.direction);
			        break;
			    case Direction::Bottom:
			    case Direction::Top:
			        display_faces[i].renderViewFace(GRAY);
			        break;
			    case Direction::FrontLeft:
			        if (viewBlock->get_face_from_dir(IntVector::rotateVectorZ(player.direction, true))->getNextBlock() != nullptr){
			            display_faces[i].renderViewFace(GRAY);
			        }
			        break;
			    case Direction::FrontRight:
			        if (viewBlock->get_face_from_dir(IntVector::rotateVectorZ(player.direction, false))->getNextBlock() != nullptr){
			            display_faces[i].renderViewFace(GRAY);
			        }
			        break;
			}

			if (face != nullptr && face->getNextBlock()==nullptr){
			    display_faces[i].renderViewFace(face->getFaceColor());
			}

			if (i % fpr == fpr-1){//finished processing faces in a ring
			    viewBlock = viewBlock->get_face_from_dir(player.direction)->getNextBlock();
				if (viewBlock==nullptr){
				    break;
				}
			}
		}

        for(int i=0; i < mazeBoardBlocks.size(); i++){
            mazeBoardBlocks[i].pos.displayVector(1, mapOffset);
        }
        player.renderPlayer(mapOffset);


		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();

		WindowShouldClose();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	//UnloadTexture(wabbit);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}

/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "raymath.h"
#include <cmath>
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
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
#include "audio.h"

static GameAudio audio;

#if defined(__EMSCRIPTEN__)
// Keep the browser's sound control in sync with the game's M-key mute state.
extern "C" {
EMSCRIPTEN_KEEPALIVE int IsGameAudioReady() { return audio.isReady(); }
EMSCRIPTEN_KEEPALIVE int IsGameAudioMuted() { return audio.isMuted(); }
EMSCRIPTEN_KEEPALIVE void SetGameAudioMuted(int muted) { audio.setMuted(muted != 0); }
}
#endif

const int screenWidth = 900;
const int screenHeight = 600;
const Vector2 screenCentre{(float)screenWidth/2, (float)screenHeight/2};

const int block_separation = 1.0; //3d separation between each grid cell in the world
const float light_factor = 3; //stops further cells getting dark too quickly; might be similar to where light starts dropping off
const float light_scale = block_separation/light_factor;
const bool use_textures = false; // Set false for flat face colours.

const float map_scale = 30;
constexpr double move_interval_seconds = 2.0;

//Timing variables
const double gameLength = 20.0;
const maze_steps = 10;
constepr move_time = gameLength/moveCount
static_assert(maze_steps > 0);
static_assert(gameLength) > 0);

const int turn_rate = 3; //chance to make a turn in correct maze path per step
const int portal_rate = 6; //chance to attempt to spawn a portal is 1/portal_rate

// Order matches the eleven faces generated for each ring.
enum class Direction { // in future rename roof or top to be consistant
    Bottom,
    Right,
    Top,
    Left,
    Front,
    FrontLeft,
    FrontRight,
    RoofLeft,
    RoofRight,
    FloorLeft,
    FloorRight
};

const int fpr = 11; //number of faces per ring

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
        IntVector result(v.y,-v.x,v.z);
        return result;
      }
      else{
        IntVector result(-v.y,v.x,v.z);
        return result;
      }
    }
    /*
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
    */
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

	//-----Genarates Points for the Faces-----
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
        const int outer = rn*4;
        const int inner = outer+4;
    	for (int fn=0; fn<fpr; fn++){
            const Direction face_direction = static_cast<Direction>(fn);
        	int i = rn*4 + fn;
        	Vector2 *zone_points[4];
            int point_count = 4;
            Vector2 side_points[2];
            float side_dist = 0;
            float forward_dist = 0;
            float up_dist = 1/2;
       	    //Makes The faces, bottom, right, top, left
            if (fn < static_cast<int>(Direction::Front)){
          		zone_points[0] = &displayPoints[i];
          		zone_points[1] = (face_direction == Direction::Left)? &displayPoints[i-3] : &displayPoints[i+1];
                zone_points[2] = (face_direction == Direction::Left)? &displayPoints[i+1] : &displayPoints[i+5]; // Ternary Operator checks if at ring wraparound case
                zone_points[3] = &displayPoints[i+4];
                forward_dist = rn;
                side_dist = 1/2;

            }
            else if (face_direction == Direction::Front){ // back face
     			zone_points[0] = &displayPoints[i];
          		zone_points[1] = &displayPoints[i+1];
                zone_points[2] = &displayPoints[i+2]; // Ternary Operator checks if at ring wraparound case
                zone_points[3] = &displayPoints[i+3];
                forward_dist = rn + 1/2;

      		}
            //note side dist should be 1 not 1/2 on faces but have made it less as you see half of face
            else if (face_direction == Direction::FrontLeft){
                side_points[0] = {displayPoints[outer].x, displayPoints[inner].y};
                side_points[1] = {displayPoints[outer+3].x, displayPoints[inner+3].y};
                zone_points[0] = &side_points[0];
                zone_points[1] = &displayPoints[inner];
                zone_points[2] = &displayPoints[inner+3];
                zone_points[3] = &side_points[1];
                forward_dist = rn + 1/2;
                side_dist = 1/2;

            }
            else if (face_direction == Direction::FrontRight){
                side_points[0] = {displayPoints[outer+1].x, displayPoints[inner+1].y};
                side_points[1] = {displayPoints[outer+2].x, displayPoints[inner+2].y};
                zone_points[0] = &displayPoints[inner+1];
                zone_points[1] = &side_points[0];
                zone_points[2] = &side_points[1];
                zone_points[3] = &displayPoints[inner+2];
                forward_dist = rn + 1/2;
                side_dist = 1/2;
            }
            else if (face_direction == Direction::RoofLeft){
                side_points[0] = {displayPoints[outer+3].x, displayPoints[inner+3].y};
                zone_points[0] = &side_points[0];
                zone_points[1] = &displayPoints[inner+3];
                zone_points[2] = &displayPoints[outer+3];
                point_count = 3;
                forward_dist = rn;
                side_dist = 1/2;
                up_dist = 1/2;
            }
            else if (face_direction == Direction::RoofRight){
                side_points[0] = {displayPoints[outer+2].x, displayPoints[inner+2].y};
                zone_points[0] = &displayPoints[inner+2];
                zone_points[1] = &side_points[0];
                zone_points[2] = &displayPoints[outer+2];
                point_count = 3;
                forward_dist = rn;
                side_dist = 1/2;
                up_dist = 1/2;
            }
            else if (face_direction == Direction::FloorLeft){
                side_points[0] = {displayPoints[outer].x, displayPoints[inner].y};
                zone_points[0] = &displayPoints[outer];
                zone_points[1] = &displayPoints[inner];
                zone_points[2] = &side_points[0];
                point_count = 3;
                forward_dist = rn;
                side_dist = 1/2;//should be 1 if calculating from centre
                up_dist = 1/2;
            }
            else if (face_direction == Direction::FloorRight){
                side_points[0] = {displayPoints[outer+1].x, displayPoints[inner+1].y};
                zone_points[0] = &displayPoints[outer+1];
                zone_points[1] = &side_points[0];
                zone_points[2] = &displayPoints[inner+1];
                point_count = 3;
                forward_dist = rn;
                side_dist = 1/2;
                up_dist = 1/2;
            }

      		  //calculate faces Lighting Factor
      		  float factored_distance_squared = (light_scale*light_scale)*(forward_dist*forward_dist + side_dist*side_dist+up_dist*up_dist);
      		  float lightFactor = 1.0f/(1.0f+factored_distance_squared);
      		  //std::cout << "row_num: " <<row_num<< "  |  face_num: "<< i << " |  lightFactor: "<< lightFactor << "\n";
      		  //adds Face
			      displayFaces.emplace_back(zone_points, (float)(lightFactor), point_count);
	    }
	}
	return displayFaces;
}


class Block; //Forward declaration so face knows it exists and can point to it
class Face{
	private:
		Block* next_block;
		bool portal = false;
		Color color;

	public:
		Face()
		:
		next_block(nullptr)
		{
			std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_real_distribution<float> random(0.0f, 1.0f);

            float hue = random(rng)*360.0f; //Check this
            float saturation = random(rng)*0.1f;

            color = ColorFromHSV(hue, saturation, 0.82f);
		}
		void setColorOverride(Color col){
		    color = col;
		}

		void setNextBlock(Block* to_block, bool is_portal = false){
			next_block = to_block;
			portal = is_portal;
		}

		bool isPortal() const{
			return portal;
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
    public:
        int blockID;
        IntVector pos;

        Block(int block_id = 0)
          :
          pos(0,0,0),
          blockID(block_id)
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
			       return &(direction_to_face[dir]);
        }

		    Block* get_next_block(IntVector dir){
            Face* next_face = get_face_from_dir(dir);
            return (*next_face).getNextBlock();
		    }

        int add_connection(Block *toBlock, IntVector dir, bool is_portal = true){
            Face& next_face = *(get_face_from_dir(dir));
            if (next_face.getNextBlock() == nullptr){
                std::cout << "connecting \n";
                next_face.setNextBlock(toBlock, is_portal);
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
    block_array.reserve(steps_to_finish+2); // Keep existing links valid when adding the start and end rooms.
    //block_array.reserve(max_block_count);

    //std::unordered_map<IntVector, Block*> positions_to_blocks;

    int block_num = 0;
    Block* prev_block = nullptr;
    Block* new_block = nullptr;
    while (steps_to_finish > 0){
        prev_block = new_block;

        block_array.emplace_back(block_num);//use block_num as its ID
        new_block = &block_array[block_num];
        new_block->get_face_from_dir(IntVector{0,0,1})->setColorOverride(GetColor(0x455B61FF));//Set roof and floor colors
        new_block->get_face_from_dir(IntVector{0,0,-1})->setColorOverride(GetColor(0x806F5BFF));
        new_block->pos = position;
        block_num++;

        // connect blocks
        if (prev_block != nullptr){
            new_block->add_connection(prev_block, direction.reverseVector());
            prev_block->add_connection(new_block, direction);
        }

        IntVector in_direction = direction;
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

        //RANDOMLY ADD PORTAL ON ONE SIDE TO PREVIOUS BLOCK
        std::vector<IntVector> directions = {
            { 1,  0, 0}, // right
            {-1,  0, 0}, // left
            { 0,  1, 0}, // down
            { 0, -1, 0}  // up
        };
        std::erase(directions, in_direction);
        std::erase(directions, direction);
        int index = randomInteger(0, static_cast<int>(directions.size()) - 1);
        IntVector portal_dir = directions[index];

        for (int i=0; i<block_array.size()-1; i++){
            bool has_place = (block_array[i].get_face_from_dir(portal_dir.reverseVector())->getNextBlock()==nullptr);
            if (randomInteger(0, portal_rate)==0 and has_place){
                // A portal connection uses its own movement sound.
                new_block->add_connection(&block_array[i], portal_dir, true);
                block_array[i].add_connection(new_block, portal_dir.reverseVector(), false)// cannot go through this portal backwards
            }
        }
        //generate next block
        position = IntVector::addIntVector(position,direction);
        steps_to_finish --;
    }

    // Add the endpoint rooms after all random portals have been generated.
    Block* final_block = new_block;
    block_array.emplace_back(block_num);
    block_num++;
    Block* end_block = &block_array.back();
    end_block->pos = position;
    final_block->add_connection(end_block, direction);

    block_array.emplace_back(block_num++);
    Block* start_block = &block_array.back();
    IntVector start_direction{0,1,0};
    start_block->pos = IntVector::addIntVector(block_array.front().pos, start_direction.reverseVector());
    start_block->add_connection(&block_array.front(), start_direction, true);
    std::cout << "start is : " << start_block->position;
    const IntVector room_directions[] = {{1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};

    //setting start and ends custom colors
    for (IntVector room_direction : room_directions){
        start_block->get_face_from_dir(room_direction)->setColorOverride(GetColor(0xBCB782FF));
        end_block->get_face_from_dir(room_direction)->setColorOverride(GetColor(0xDDBC58FF));
    }
    start_block->get_face_from_dir({0,0,-1})->setColorOverride(GetColor(0x776131FF));

        std::cout << "done \n";
	  return block_array;
}

enum class MoveResult { Blocked, Moved, Portal};
enum class GameState { Waiting, Playing, Won, Dead};

class Player{
  public:
    IntVector direction;
	  Block* current_block;// pointer to block in maze

    Player(Block* start_block)
	  :
		current_block(start_block),
		direction{0,1,0}
		//position{0,0,0}
	  {}

  void renderPlayer(Vector2 draw_offset = screenCentre){
		IntVector position = (*current_block).pos;
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

    MoveResult move_forward(){
  		Face* exit_face = current_block->get_face_from_dir(direction);
  		Block* next_block = exit_face->getNextBlock();
        std::cout << "current block : "<< current_block << "\n";
		if (next_block != nullptr){
            std::cout << "moving \n";
			current_block = next_block;
			return exit_face->isPortal() ? MoveResult::Portal : MoveResult::Moved;
		}
		else{
            std::cout << "blocked \n";
        }
		return MoveResult::Blocked;
	}
};

Texture2D createPerlinTexture(){
	Image noise = GenImagePerlinNoise(512,512,17,91,5.0f);
	Color* noise_pixels = static_cast<Color*>(noise.data);
	double noise_mean = 0;
	for (int i = 0; i < noise.width*noise.height; i++) noise_mean += noise_pixels[i].r;
	noise_mean /= noise.width*noise.height;
	for (int i = 0; i < noise.width*noise.height; i++){
	    unsigned char channel = (unsigned char)std::clamp(244.0 + (noise_pixels[i].r-noise_mean)*0.42, 220.0, 255.0);
	    noise_pixels[i] = {channel,channel,channel,255};
	}
	Texture2D face_texture = LoadTextureFromImage(noise);
	UnloadImage(noise);
	GenTextureMipmaps(&face_texture);
	SetTextureFilter(face_texture, TEXTURE_FILTER_TRILINEAR);
	SetTextureWrap(face_texture, TEXTURE_WRAP_CLAMP);
	return face_texture;
}

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	//SearchAndSetResourceDir("resources");

	//Make Window
	InitWindow(screenWidth, screenHeight, "One Way Out");
	SetExitKey(KEY_NULL); // Any key can start, including Escape.
	audio.initialize();
	// Keep frame state alive when the browser takes over the main loop.
	static Texture2D face_texture = use_textures ? createPerlinTexture() : Texture2D{};


	//Initialise main variables
	static std::vector<ViewFace> display_faces = generateDisplay(7);
	static std::vector<Block> mazeBoardBlocks = makeMazeBoard();
	static Block* startBlock = &mazeBoardBlocks.back();
	static Block* endBlock = &mazeBoardBlocks[mazeBoardBlocks.size()-2];
	static Player player(startBlock);

	static GameState gameState = GameState::Waiting;
	static double secondsRemaining = round_duration_seconds;
	static int movesRemaining = moves_per_round;
	static double roundStartedAt = 0;
	static double nextMoveAt = 0;
	static const IntVector up_direction{0,0,1};
	static const IntVector down_direction{0,0,-1};

	Vector2 projectionOffset{10,10};
	static Vector2 mapOffset{700,450};
	static bool showMap = false;

	// One frame, shared by the desktop loop and the browser callback.
	const auto updateFrame = []()
	{
        if (IsKeyPressed(KEY_X)) showMap = !showMap;
        if (IsKeyPressed(KEY_M)) audio.toggleMute();

        audio.update();
        const double now = GetTime();
        // Raylib maps the primary touch to the left mouse button.
        const bool tapped = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        const bool tappedLeft = tapped && GetMouseX() < screenWidth/2;
        if (gameState == GameState::Dead){
            if (tapped || GetKeyPressed() != 0){
                player = Player(startBlock);
                secondsRemaining = round_duration_seconds;
                movesRemaining = moves_per_round;
                gameState = GameState::Waiting;
            }
        }
        else if (gameState == GameState::Waiting){
            if (tapped || GetKeyPressed() != 0){
                player.direction = {0,1,0};
                player.move_forward();
                audio.startMusic();
                audio.play(SoundEffect::Portal);
                roundStartedAt = now;
                nextMoveAt = now + move_interval_seconds;
                gameState = GameState::Playing;
                SetExitKey(KEY_ESCAPE);
            }
        }
        else if (gameState == GameState::Playing){
            secondsRemaining = round_duration_seconds - (now-roundStartedAt);
            if (secondsRemaining < 0) secondsRemaining = 0;
            if (secondsRemaining > 0){
                if (IsKeyPressed(KEY_RIGHT) || (tapped && !tappedLeft)){
                    player.turn_player(true);
                    audio.play(SoundEffect::Turn);
                }
                if (IsKeyPressed(KEY_LEFT) || tappedLeft){
                    player.turn_player(false);
                    audio.play(SoundEffect::Turn);
                }
            }
            while (now >= nextMoveAt && movesRemaining > 0 && gameState == GameState::Playing){
                switch (player.move_forward()) {
                    case MoveResult::Moved: audio.play(SoundEffect::Move); break;
                    case MoveResult::Blocked: audio.play(SoundEffect::Blocked); break;
                    case MoveResult::Portal: audio.play(SoundEffect::Move); break;
                }
                --movesRemaining;
                nextMoveAt += move_interval_seconds;
                if (player.current_block == endBlock){
                    gameState = GameState::Won;
                    audio.play(SoundEffect::Win);
                }
            }
            if (secondsRemaining <= 0 && gameState == GameState::Playing){
                movesRemaining = 0;
                gameState = GameState::Dead;
                SetExitKey(KEY_NULL); // Any key, including Escape, can retry after death.
            }
        }
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		// draw some text using the default font
		//DrawText("Hello Raylib, Hello Noah", 200,200,20,LIME);

		// draw our texture to the screen
		//DrawTexture(wabbit, 800, 200, RED);
		const auto wallTextureFor = [](Block* block){
		    return (block == startBlock || block == endBlock) ? Texture2D{} : face_texture;
		};
		Block* viewBlock = player.current_block;
    //loop through all possible face directions and check if the
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
			        face = viewBlock->get_face_from_dir(down_direction);
			        break;
			    case Direction::Top:
			        face = viewBlock->get_face_from_dir(up_direction);
			        break;
			    case Direction::FrontLeft:
			    case Direction::RoofLeft:
			    case Direction::FloorLeft:{
			        Block* sideBlock = viewBlock->get_face_from_dir(IntVector::rotateVectorZ(player.direction, true))->getNextBlock();
			        if (sideBlock != nullptr){
			            if (face_direction == Direction::FrontLeft){
			                display_faces[i].renderViewFace(sideBlock->get_face_from_dir(player.direction)->getFaceColor(), wallTextureFor(sideBlock));
			            }
			            else{
			                face = sideBlock->get_face_from_dir(face_direction == Direction::RoofLeft ? up_direction : down_direction);
			            }
			        }
			        break;
			    }
			    case Direction::FrontRight:
			    case Direction::RoofRight:
			    case Direction::FloorRight:{
			        Block* sideBlock = viewBlock->get_face_from_dir(IntVector::rotateVectorZ(player.direction, false))->getNextBlock();
			        if (sideBlock != nullptr){
			            if (face_direction == Direction::FrontRight){
			                display_faces[i].renderViewFace(sideBlock->get_face_from_dir(player.direction)->getFaceColor(), wallTextureFor(sideBlock));
			            }
			            else{
			                face = sideBlock->get_face_from_dir(face_direction == Direction::RoofRight ? up_direction : down_direction);
			            }
			        }
			        break;
			    }
			}

			if (face != nullptr && face->getNextBlock()==nullptr){
			    const bool isWall = face_direction == Direction::Left ||
			                        face_direction == Direction::Right ||
			                        face_direction == Direction::Front;
			    display_faces[i].renderViewFace(face->getFaceColor(), isWall ? wallTextureFor(viewBlock) : Texture2D{});
			}

			if (i % fpr == fpr-1){//finished processing faces in a ring
			    viewBlock = viewBlock->get_face_from_dir(player.direction)->getNextBlock();
				if (viewBlock==nullptr){
				    break;
				}
			}
		}

        if (showMap){
            for(int i=0; i < mazeBoardBlocks.size(); i++){
                mazeBoardBlocks[i].pos.displayVector(1, mapOffset);
            }
            player.renderPlayer(mapOffset);
        }

        const int hundredthsRemaining = static_cast<int>(std::ceil(secondsRemaining*100));
        const char* countdown = TextFormat("%02d:%02d", hundredthsRemaining/100, hundredthsRemaining%100);
        DrawText(countdown, 12, 12, 32, WHITE);
        const char* movesLeft = TextFormat("Moves: %02d", movesRemaining);
        DrawText(movesLeft, screenWidth-12-MeasureText(movesLeft,32), 12, 32, WHITE);

        if (gameState == GameState::Waiting){
            const char* firstLine = "Tap, Click or Press";
            const char* secondLine = "Any Key to Start";
            DrawText(firstLine, (screenWidth-MeasureText(firstLine,32))/2, 96, 32, BLACK);
            DrawText(secondLine, (screenWidth-MeasureText(secondLine,32))/2, 136, 32, BLACK);
        }
        else if (gameState == GameState::Won){
            const char* text = "You Win";
            DrawText(text, (screenWidth-MeasureText(text,48))/2, (screenHeight-48)/2, 48, BLACK);
        }
        else if (gameState == GameState::Dead){
            const char* text = "You Died";
            DrawText(text, (screenWidth-MeasureText(text,48))/2, (screenHeight-48)/2, 48, RED);
            const char* retry = "Tap or press any key to try again";
            DrawText(retry, (screenWidth-MeasureText(retry,24))/2, screenHeight/2+40, 24, WHITE);
        }

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();

	};

#if defined(__EMSCRIPTEN__)
	// Use requestAnimationFrame so the browser can draw and process input.
	emscripten_set_main_loop(updateFrame, 0, true);
#else
	while (!WindowShouldClose()) updateFrame();
#endif

	// cleanup
	// unload our texture so it can be cleaned up
	//UnloadTexture(wabbit);

	// destroy the window and cleanup the OpenGL context
	if (face_texture.id != 0) UnloadTexture(face_texture);
	audio.shutdown();
	    CloseWindow();
		return 0;
}

#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <string>
#include <vector>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TILESIZE 32
 
using namespace std;

void load_map(const char * filename, vector< vector<int> > & map)
{
	ifstream open_file(filename);
	if (open_file.is_open())
	{
		string line, value;
		int space;
		
		while (!open_file.eof())
		{
			getline(open_file, line);
			
			stringstream str(line);
			vector<int> temp_vec;
			
			while (!str.eof())
			{
				getline(str, value, ' ');
				int i = atoi(value.c_str());
				
				if (value.length() > 0)
					temp_vec.push_back(i);
			}
			
			map.push_back(temp_vec);
		}
	}
	else
	{
		cerr << "ERROR: Unable to find file.\n";
	}
}

void draw_map(vector< vector<int> > & map);

bool collision(float &x, float &y, float ex, float ey, int w, int h)
{
	return !(x + w < ex || x > w + ex || y + h < ey || y > h + ey);
}

bool pyt_collision(float &x, float &y, float ex, float ey, int pradius, int eradius)
{
	float dx = x - ex;
	float dy = y - ey;
	float dist = sqrt(dx*dx + dy*dy);
	int rad = pradius + eradius;
	
	return (dist < rad);
}

void camera_update(float * campos, float x, float y, int w, int h) 
{
	// center the map to the screen
	campos[0] = -(WINDOW_WIDTH / 2) + (x + w / 2);
	campos[1] = -(WINDOW_HEIGHT / 2) + (y + h / 2);
	
	// set to zero if position is below zero
	if (campos[0] < 0)
		campos[0] = 0;

	if (campos[1] < 0)
		campos[1] = 0;
}

int main()
{
	// ALLEGRO DISPLAY
	ALLEGRO_DISPLAY * display;

	// MAP VARIABLES
	vector< vector<int> > & _map;

	// FRAMES PER SECOND
	const float FPS = 60.0f;
	const float frame_FPS = 12.0f;

	// CAMERA EFFECTS
	float _degrees = 0.0f;
	float _scale = 1.0f;

	// MAIN LOOP
	bool _game = true;

	// DIRECTION ENUM
	enum Direction { DOWN, LEFT, RIGHT, UP };

	// DRAWING FLAG
	bool _draw = true;

	// PLAYER VARIABLES
	float _x = 300;
	float _y = 200;
	float _speed = 4;
	int _sprx = 32;
	int _spry = 0;
	int _dir = DOWN;
	int _prevdir;
	int _index = 0;
	bool _startanim = false;
	bool _active = false;
	bool _pcirc = true; // toggles bounding circle
	
	// BOX VARIABLES
	float _ex = 200;
	float _ey = 200;
	int _erad = 10;
	int _esprx = 32;
	int _espry = 0;
	int _edir = DOWN;
	int _eprevdir;
	
	// CAMERA VARIABLES
	float _camerapos[2] = { 0, 0 };
	
	// WINDOW VARIABLES
	int _winx = 200;
	int _winy = 100;

	// Check if library initialized
	if (!al_init())
	{
		al_show_native_message_box(NULL, NULL, NULL, "Unable to load Allegro 5", NULL, NULL);
		return 1;
	}

	// Set and create window
	al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
	display = al_create_display(WINDOW_WIDTH, WINDOW_HEIGHT);

	al_set_window_position(display, _winx, _winy);
	al_set_window_title(display, "Sample Window");

	// Prompt error if display window could not be created
	if (!display)
	{
		al_show_native_message_box(NULL, "Display Error", NULL, "Could not create display window", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return 1;
	}

	// Install ADDONS
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	al_init_image_addon();
	al_init_acodec_addon();

	al_install_keyboard();
	al_install_audio();
	
	// Reserve audio samples
	al_reserve_samples(2);
	
	// COLORS
	ALLEGRO_COLOR black = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR red = al_map_rgb(255, 0, 0);
	ALLEGRO_COLOR green = al_map_rgb(0, 255, 0);
	/*ALLEGRO_COLOR blue = al_map_rgb(0, 0, 255);
	ALLEGRO_COLOR magenta = al_map_rgb(255, 0, 255);
	ALLEGRO_COLOR cyan = al_map_rgb(0, 255, 255);
	ALLEGRO_COLOR yellow = al_map_rgb(255, 255, 0);

	// SECONDARY COLORS
	ALLEGRO_COLOR teal_purple = al_map_rgb(168, 44, 155);
	ALLEGRO_COLOR elec_blue = al_map_rgb(44, 117, 168);
	ALLEGRO_COLOR wood_green = al_map_rgb(44, 168, 117);*/
	
	// KEY STATE
	ALLEGRO_KEYBOARD_STATE key_state;

	// CAMERA TRANSFORM
	ALLEGRO_TRANSFORM camera;
	
	// PLAYER IMAGE
	//ALLEGRO_BITMAP * player = al_load_bitmap("img/animation.png");
	/*ALLEGRO_BITMAP * person = al_load_bitmap("img/person.png");
	ALLEGRO_BITMAP * hair = al_load_bitmap("img/bluehair.png");
	ALLEGRO_BITMAP * player = al_create_bitmap(al_get_bitmap_width(person), al_get_bitmap_height(person));*/
	ALLEGRO_BITMAP * player = al_load_bitmap("img/celine.png");
	
	// BOX IMAGE
	ALLEGRO_BITMAP * box = al_load_bitmap("img/box.png");
	
	// BACKGROUND IMAGE
	ALLEGRO_BITMAP * background = al_load_bitmap("img/bg.png");
	
	// Set target to player loaded in backbuffer
	/*al_set_target_bitmap(player);
	
	// Draw player with hair and body
	al_draw_bitmap(person, 0, 0, NULL);
	al_draw_bitmap(hair, 0, 0, NULL);
	
	// Save new player image
	al_save_bitmap("img/cp1.png");*/
	
	// Set drawing target to display's backbuffer
	//al_set_target_bitmap(al_get_backbuffer(display));
	
	//ALLEGRO_BITMAP * playerWalk[12];
	
	// Pixel indicator for sprites
	ALLEGRO_COLOR pixel, last_pixel, color;
	
	vector<int> source;
	vector<int> width;
	
	source.push_back(0);
	
	for (int i = 0; i < al_get_player_width(player); i++)
	{
		/*stringstream str;
		str << "Sprites: " << i + 1 << ".png";
		playerWalk[i] = al_load_bitmap(str.str().c_str());*/
		
		color = al_map_rgba(255, 0, 0, 255);
		pixel = al_get_pixel(player, i, 0);
		
		if (memcmp(&pixel, &last_pixel, sizeof(ALLEGRO_COLOR)))
		{
			if (!memcmp(&pixel, &color, sizeof(ALLEGRO_COLOR)))
			{
				source.push_back(i);
				if (source.size() == 2)
					width.push_back(i);
				else
					width.push_back(i - width[width.size() - 1]);
			}
		} 
		else if (i == al_get_bitmap_width(player) - 1)
		{	
			width.push_back(i - width[width.size() - 1]);
		}
			
		last_pixel = pixel;
	}
	
	// take out opaque background box on image
	al_convert_mask_to_alpha(player, black);
	
	// FONTS.
	ALLEGRO_FONT * font = al_load_ttf_font("pala.ttf", 20, NULL);

	ALLEGRO_TIMER * timer = al_create_timer(1.0 / FPS);
	ALLEGRO_TIMER * frame_timer = al_create_timer(1.0 / frame_FPS);

	ALLEGRO_EVENT_QUEUE * event_queue = al_create_event_queue();

	// SOUND EFFECT
	ALLEGRO_SAMPLE * sfx = al_load_sample("audio/sfx/sfx.wav");
	
	// MUSIC
	ALLEGRO_SAMPLE * music = al_load_sample("audio/bgm/song.ogg");
	ALLEGRO_SAMPLE_INSTANCE * music_instance = al_create_sample_instance(music);
	
	al_set_sample_instance_playmode(music_instance, ALLEGRO_PLAYMODE_LOOP);
	
	al_attach_sample_instance_to_mixer(music_instance, al_get_default_mixer());
	
	// Register events
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_timer_event_source(frame_timer));
	al_register_event_source(event_queue, al_get_display_event_source(display));

	// Play music
	al_play_sample_instance(music_instance);
	
	// Start timers
	al_start_timer(timer);
	al_start_timer(frame_timer);

    // tile width and height
    int tw = al_get_bitmap_width(player) / TILESIZE;
    int th = al_get_bitmap_height(player) / TILESIZE;

	// player bounding radius
	int _prad = (TILESIZE/2);
	
	// load tile map
	load_map("img/maps/sample001.txt", _map);
	
    // game loop
	while (_game)
	{
		ALLEGRO_EVENT events;
		al_wait_for_event(event_queue, &events);

		if (events.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			_game = false;
		}
		else if (events.type == ALLEGRO_EVENT_TIMER)
		{
			al_get_keyboard_state(&key_state);
			
			if (events.timer.source == timer)
			{
				_active = true;
				_prevdir = _dir;

				if (al_key_down(&key_state, ALLEGRO_KEY_UP)) 
				{
					_y -= _speed;
					_dir = UP;
				} 
				else if (al_key_down(&key_state, ALLEGRO_KEY_DOWN)) 
				{
					_y += _speed;
					_dir = DOWN;
					//al_play_sample(sfx, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, 0);
				} 
				else if (al_key_down(&key_state, ALLEGRO_KEY_LEFT)) 
				{
					_x -= _speed;
					_dir = LEFT;
				} 
				else if (al_key_down(&key_state, ALLEGRO_KEY_RIGHT)) 
				{
					_x += _speed;
					_dir = RIGHT;
				}
				else if (al_key_press(&key_state, ALLEGRO_KEY_ENTER))
				{
					_pcirc = !_pcirc;
				}
				/*else if (al_key_down(&key_state, ALLEGRO_KEY_A) && !_startanim) 
				{
					_startanim = true;
				}*/
				else if (al_key_down(&key_state, ALLEGRO_KEY_W))
				{
					_degrees++;
				}
				else if (al_key_down(&key_state, ALLEGRO_KEY_S))
				{
					_degrees--;
				}
				else if (al_key_down(&key_state, ALLEGRO_KEY_E))
				{
					_scale++;
				}
				else if (al_key_down(&key_state, ALLEGRO_KEY_D))
				{
					_scale--;
				}
				else if (al_key_down(&key_state, ALLEGRO_KEY_ESCAPE)) 
				{
					_game = false;
				} 
				else 
				{
					_active = false;
				}
				
				if (collision(_x, _y, _ex, _ey, TILESIZE, TILESIZE))
				{
					if (_dir == DOWN)
						y -= _speed;
					else if (_dir == LEFT)
						x += _speed;
					else if (_dir == RIGHT)
						x -= _speed;
					else if (_dir == UP)
						y += _speed;
				}
				
				// update camera position and move world
				camera_update(_camerapos, _x, _y, TILESIZE, TILESIZE);
				
				al_identity_transform(&camera);
				
				al_translate_transform(&camera, -(_x+_prad), -(_y+_prad));
				al_rotate_transform(&camera, _degrees * 3.1415 / 180);
				al_scale_transform(&camera, _scale, _scale);
				al_translate_transform(&camera, -_camerapos[0] + (_x+_prad), -_camerapos[1] + (_y+_prad));
				al_use_transform(&camera);
				
				/*if (_startanim)
				{
					if (_index > source.size() - 2)
					{
						_index = -1;
						_startanim = false;
					}
					else
					{
						_index++;
					}
				}*/
			}
			else if (events.timer.source == frame_timer)
			{
				if (_active)
					_sprx += al_get_bitmap_width(player) / tw;
				else
					_sprx = 32;


				if (_sprx >= al_get_bitmap_width(player))
					_sprx = 0;

				_spry = _dir * al_get_bitmap_height(player) / th;
			}
			
			_draw = true;
		}

		// draw stuff here
		if (_draw)
		{
			// set draw flag off
			//_draw = false;

			al_draw_textf(font, white, _prad, 10, ALLEGRO_ALIGN_LEFT, "_x: %.02f _y: %.02f", _x, _y);
			
			//ALLEGRO_BITMAP * sub_bitmap = al_create_sub_bitmap(player, _sprx, _spry * TILESIZE, TILESIZE, TILESIZE);
			
			al_draw_bitmap_region(player, _sprx, _spry, TILESIZE, TILESIZE, _x, _y, NULL);
			
			//al_draw_bitmap(sub_bitmap, _x-_prad, _y-_prad, NULL);
			
			if (_pcirc)
				al_draw_rectangle(_x, _y, _x+TILESIZE, _y+TILESIZE, green, 2);
				//al_draw_circle(_x, _y, _prad, green, 2);
			
			al_draw_rectangle(_ex, _ey, _ex+TILESIZE, _ey+TILESIZE, red, 2);
			//al_draw_circle(_ex, _ey, _erad, red, 2);
			
			//al_draw_bitmap_region(box, _ex, _ey, TILESIZE, TILESIZE, _ex, _ey, NULL);
			
			/*if (!_startanim)
			{
				al_draw_bitmap(player, _x, _y, NULL);
			}
			else
			{
				al_draw_bitmap_region(player, source[_index], 0, width[_index], 
					al_get_bitmap_height(player), _x, _y, NULL);
			}*/

			/*al_draw_filled_rectangle(90, 190, 100, 200, wood_green);

			al_draw_rounded_rectangle(570, 210, 650, 290, 5, 5, yellow, 1.25f);

			al_draw_triangle(200, 250, 300, 250, 250, 350, teal_purple, 0.75f);

			al_draw_circle(_winwidth / 2, _winheight / 2, 40, red, 2.0);

			al_draw_ellipse(_winwidth / 2 - 100, _winheight / 2 - 150,  140, 90, blue, 1.0);

			al_draw_pixel(90, 500, green);

			al_draw_line(35, 50, 450, 50, cyan, 1.0);

			float pts[8] = {0, 0, 400, 100, 50, 200, (float)_winwidth, (float)_winheight};

			al_draw_spline(pts, elec_blue, 1.0);*/

			// draw tile map
			draw_map(_map);
			
			// flip display to make it visible
			al_flip_display();

			al_clear_to_color(black);
		}
	}

	al_rest(0.25f);

	al_destroy_font(font);
	al_destroy_timer(timer);
	al_destroy_timer(frame_timer);
	al_destroy_bitmap(player);
	/*for (int i = 0;  i < 12; i++)
		al_destroy_bitmap(playerWalk[i]);*/
	al_destroy_sample_instance(music_instance);
	al_destroy_sample(music);
	al_destroy_sample(sfx);
	al_destroy_event_queue(event_queue);
	al_destroy_display(display);

	return 0;
}

void draw_map(vector< vector<int> > & map)
{
	for (int row = 0; row < map.size(); row++)
	{
		for (int col = 0; col < map[row].size(); col++)
		{
			/*if (map[row][col] == 0)
			{
				/*al_draw_filled_rectangle(col * TILESIZE, row * TILESIZE,
					col * TILESIZE + TILESIZE, row * TILESIZE + TILESIZE, al_map_rgb(0, 0, 255));
			}
			else if (map[row][col] == 1)
			{
				al_draw_filled_rectangle(col * TILESIZE, row * TILESIZE,
					col * TILESIZE + TILESIZE, row * TILESIZE + TILESIZE, al_map_rgb(0, 255, 0));
			
			}*/
			
			al_draw_bitmap_region(tileset, map[row][col] * TILESIZE, 0, TILESIZE, TILESIZE, 
				col * TILESIZE,
				row * TILESIZE, 
				NULL);
		}
	}
}

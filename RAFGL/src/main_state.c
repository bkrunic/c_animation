#include <main_state.h>
#include <glad/glad.h>
#include <math.h>
#include <limits.h>


#include <rafgl.h>

#include <game_constants.h>

static rafgl_raster_t doge;
static rafgl_raster_t upscaled_doge;
static rafgl_raster_t raster, raster2;
static rafgl_raster_t checker;

static rafgl_texture_t texture;

static rafgl_spritesheet_t hero;



static int raster_width = RASTER_WIDTH, raster_height = RASTER_HEIGHT;

int pressed;
float location = 0;
float selector = 0;

int animation_running = 0;
int animation_frame = 0;
int direction = 0;

int hero_pos_x = RASTER_WIDTH / 2;
int hero_pos_y = RASTER_HEIGHT / 2;

int hero_speed = 50;

int hover_frames = 0;

static char save_file[256];
int save_file_no = 0;


typedef struct _particle_t
{
    float x, y, dx, dy;
    int life;


} particle_t;


#define MAX_PARTICLES 100000

particle_t particles[MAX_PARTICLES];

void draw_particles(rafgl_raster_t *raster)
{
    int i;
    particle_t p;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        p = particles[i];
        if(p.life <= 0)
            continue;
        rafgl_raster_draw_line(raster, p.x - p.dx, p.y - p.dy, p.x, p.y, rafgl_RGB(226, 88,  34));
    }
}

static float elasticity = 1.1;

void update_particles(float delta_time)
{
    int i;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        if(particles[i].life <= 0)
            continue;

        particles[i].life--;

        particles[i].x += particles[i].dx;
        particles[i].y += particles[i].dy;
        particles[i].dx *= 0.995f;
        particles[i].dy *= 0.995f;
        particles[i].dy += 0.05;

        if(particles[i].x < 0)
        {
            particles[i].x = 0;
            particles[i].dx = (rafgl_abs_m(particles[i].dx)) * randf() * elasticity;
        }

        if(particles[i].y < 0)
        {
            particles[i].y = 0;
            particles[i].dy = (rafgl_abs_m(particles[i].dy)) * randf() * elasticity;
        }

        if(particles[i].x >= raster_width)
        {
            particles[i].x = raster_width - 1;
            particles[i].dx = (rafgl_abs_m(particles[i].dx)) * randf() * (-elasticity);
        }

        if(particles[i].y >= raster_height)
        {
            particles[i].y = raster_height - 1;
            particles[i].dy = (rafgl_abs_m(particles[i].dy)) * randf() * (-elasticity);
        }



    }
}

void main_state_init(GLFWwindow *window, void *args)
{
    /* inicijalizacija */
    /* raster init nam nije potreban ako radimo load from image */
    rafgl_raster_load_from_image(&doge, "res/images/doge.png");

    rafgl_raster_init(&upscaled_doge, raster_width, raster_height);
    rafgl_raster_bilinear_upsample(&upscaled_doge, &doge);


    rafgl_raster_init(&raster, raster_width, raster_height);
    rafgl_raster_init(&raster2, raster_width, raster_height);

    rafgl_spritesheet_init(&hero, "res/images/sprite.png", 10, 4);

    rafgl_texture_init(&texture);
}



void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{



    int i, gen = 20, radius = 1.3;
    float angle, speed;

    for(i = 0; (i < MAX_PARTICLES) && gen; i++)
    {
        if(particles[i].life <= 0 && animation_running)
        {
            if(hero_speed>1000)
                hero_speed=1000;
            particles[i].life = 100 * randf() + 100;
            particles[i].x = hero_pos_x+80;
            particles[i].y = hero_pos_y+90;

            angle = randf() * M_PI *  2.0f;
            speed = ( 2.0f + 0.7 * randf()) * radius;
            particles[i].dx = cosf(angle) * speed;
            particles[i].dy = sinf(angle) * speed;
            gen--;

        }
    }



    update_particles(delta_time);

    hero_speed++;

    animation_running = 1;
    if(game_data->keys_down[RAFGL_KEY_W])
    {
        hero_pos_y = hero_pos_y - hero_speed * delta_time;
        if(hero_pos_y<0)
            hero_pos_y =raster_height + hero_speed * delta_time;

        direction = 2;
    }
    else if(game_data->keys_down[RAFGL_KEY_S])
    {
        hero_pos_y = hero_pos_y + hero_speed * delta_time;
        if(hero_pos_y>RASTER_HEIGHT)
            hero_pos_y =0 + hero_speed * delta_time;

        direction = 0;
    }
    else if(game_data->keys_down[RAFGL_KEY_A])
    {
        hero_pos_x = hero_pos_x - hero_speed * delta_time;
        if(hero_pos_x<0)
            hero_pos_x =raster_height + hero_speed * delta_time;

        direction = 1;
    }
    else if(game_data->keys_down[RAFGL_KEY_D])
    {
        hero_pos_x = hero_pos_x + hero_speed * delta_time;
        if(hero_pos_x>=raster_width)
            hero_pos_x = 0 + hero_speed * delta_time;


        direction = 3;
    }
    else
    {
        animation_running = 0;
    }

    if(animation_running)
    {
        if(hover_frames == 0)
        {
            //animation_frame = (animation_frame + 1) % 10;
            hover_frames = 5;
        }
        else
        {
            hover_frames--;
        }

    }



    /* izmeni raster */

    int x, y;

    float xn, yn;

    rafgl_pixel_rgb_t sampled, sampled2, resulting, resulting2;


    for(y = 0; y < raster_height; y++)
    {
        yn = 1.0f * y / raster_height;
        for(x = 0; x < raster_width; x++)
        {
            xn = 1.0f * x / raster_width;

            sampled = pixel_at_m(upscaled_doge, x, y);
            sampled2 = rafgl_point_sample(&doge, xn, yn);

            resulting = sampled;
            resulting2 = sampled2;

            resulting.rgba = rafgl_RGB(0, 0, 0);

            pixel_at_m(raster, x, y) = resulting;
            pixel_at_m(raster2, x, y) = resulting2;


            if(pressed && rafgl_distance1D(location, y) < 3 && x > raster_width - 15)
            {
                pixel_at_m(raster, x, y).rgba = rafgl_RGB(255, 0, 0);
            }

        }
    }





    rafgl_raster_draw_spritesheet(&raster, &hero, animation_frame, direction, hero_pos_x, hero_pos_y);

    draw_particles(&raster);









    /* shift + s snima raster */
    if(game_data->keys_pressed[RAFGL_KEY_S] && game_data->keys_down[RAFGL_KEY_LEFT_SHIFT])
    {
        sprintf(save_file, "save%d.png", save_file_no++);
        rafgl_raster_save_to_png(&raster, save_file);
    }


    /* update-uj teksturu*/
    if(!game_data->keys_down[RAFGL_KEY_SPACE])
        rafgl_texture_load_from_raster(&texture, &raster);
    else
        rafgl_texture_load_from_raster(&texture, &raster2);
}




void main_state_render(GLFWwindow *window, void *args)
{
    /* prikazi teksturu */
    rafgl_texture_show(&texture);
}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    rafgl_raster_cleanup(&raster);
    rafgl_raster_cleanup(&raster2);
    rafgl_texture_cleanup(&texture);

}

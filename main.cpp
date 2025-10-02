
#include "renderer.h"
#include "microui_flex.h"


#include <GL/gl.h>    // This is often included by GLEW, but it's good practice to include it
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <array>

static float bg[3] = { 5, 5, 5 };

unsigned int width=800;
unsigned int height=480;

unsigned int botbarheight=30;
unsigned int topbarheight=botbarheight;

unsigned int leftbarwidth=100;
unsigned int rightbarwidth=100;

mu_Font q_font;

static mu_Animatable newstyle = {

  { 230, 200, 0, 255 }, /* border_color */
  { 20, 20, 20, 255 },  /* bg_color */
  1,                    /* border_size */
  5,                    /* gap */
  5,                    /* padding */
  { 230, 200, 0, 255 }, /* text_color */
  &q_font,              /* font*/
  MU_ALIGN_MIDDLE|MU_ALIGN_CENTER, /* text_align*/
  { 180, 200, 0, 255 }, /* hover_color */
  { 130, 200, 230, 255 }, /* focus_color */
  {0,0}
};

static mu_Animatable itemstyle = {

  { 230, 200, 0, 0 }, /* border_color */
  { 20, 20, 20, 255 },  /* bg_color */
  1,                    /* border_size */
  5,                    /* gap */
  
  5,                    /* padding */
  { 230, 200, 0, 255 }, /* text_color */
  &q_font,              /* font*/
  MU_ALIGN_MIDDLE|MU_ALIGN_CENTER, /* text_align*/
  { 180, 200, 0, 255 }, /* hover_color */
  { 130, 200, 230, 255 }, /* focus_color */
};

static mu_Animatable isostyle = {

  { 230, 200, 0, 255 }, /* border_color */
  { 20, 20, 20, 255 },  /* bg_color */
  1,                    /* border_size */
  5,                    /* gap */
  
  7,                    /* padding */
  { 230, 200, 0, 255 }, /* text_color */
  &q_font,              /* font*/
  MU_ALIGN_TOP|MU_ALIGN_LEFT, /* text_align*/
  { 180, 200, 0, 255 }, /* hover_color */
  { 130, 200, 230, 255 }, /* focus_color */
};




void cooldown(mu_Elem* elem) {
      int mov=0;
      if (elem->direction==DIR_X){
        int relativesize= elem->content_size+elem->animatable.padding*2+(elem->tree.count-1)*elem->animatable.gap;
        mov=mu_clamp(elem->animatable.scroll.x,0,elem->rect.w-relativesize);
        // mu_animation_start(elem->hash,);
      } else {
        int relativesize= elem->content_size+elem->animatable.padding*2+(elem->tree.count-1)*elem->animatable.gap;
        mov=mu_clamp(elem->animatable.scroll.y,elem->rect.h-relativesize,0);
        mov-=elem->animatable.scroll.y;

      }
    
}


static void layout(mu_Context *ctx) {
  
  if (mu_begin_elem_window_ex(ctx,"MAIN LAYOUT",mu_rect(0,0,width,height),MU_OPT_NOTITLE|MU_OPT_NORESIZE|MU_OPT_NOFRAME)){
    mu_begin_elem(ctx,0,30);
      mu_begin_elem_ex(ctx,-1,1,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);
        mu_add_text_to_elem(ctx,"REC");
      mu_end_elem(ctx);
      mu_begin_elem_ex(ctx,0,0,DIR_Y,0,0);
      mu_end_elem(ctx);
      mu_begin_elem_ex(ctx,-1,1,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);
        mu_add_text_to_elem(ctx,"00:00:00");
      mu_end_elem(ctx);
      mu_begin_elem_ex(ctx,0,0,DIR_Y,0,0);
      mu_end_elem(ctx);
      mu_begin_elem_ex(ctx,50,1,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);
        mu_add_text_to_elem(ctx,"Battery 67%");
      mu_end_elem(ctx);
      
    mu_end_elem(ctx);
    mu_begin_elem_ex(ctx,1,0,DIR_X,(MU_ALIGN_BOTTOM|MU_ALIGN_LEFT),0);
      mu_begin_elem_ex(ctx,90,1,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_CENTER),0);

        ctx->animatable=&isostyle;
        switch(mu_begin_elem_ex(ctx,0.9,80,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),MU_EL_CLICKABLE|MU_EL_STUTTER)){
          case MU_STATE_UNFOCUSED:
            printf("UNFOCUSED\n");
            mu_animation_set(ctx,cooldown);
            break;
        }
          ctx->animatable=&itemstyle;

          mu_add_text_to_elem(ctx,"ISO");
          mu_begin_elem_ex(ctx,1,30,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);
            mu_add_text_to_elem(ctx,"400");
          mu_end_elem(ctx);
          mu_begin_elem_ex(ctx,1,30,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);
            mu_add_text_to_elem(ctx,"800");
          mu_end_elem(ctx);
          mu_begin_elem_ex(ctx,1,30,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);
            mu_add_text_to_elem(ctx,"1600");
            ctx->animatable=&newstyle;

          mu_end_elem(ctx);
        mu_end_elem(ctx);
        mu_begin_elem_ex(ctx,0.9,120,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),MU_EL_CLICKABLE);
                                    mu_add_text_to_elem(ctx,"HEY");
        mu_end_elem(ctx);  

        mu_begin_elem_ex(ctx,0.9,150,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),MU_EL_CLICKABLE);
                                            mu_add_text_to_elem(ctx,"hello");
        mu_end_elem(ctx);  
        mu_begin_elem_ex(ctx,0.9,120,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),MU_EL_CLICKABLE);
        mu_end_elem(ctx);  
      mu_end_elem(ctx);


      mu_begin_elem_ex(ctx,0,0,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);
      mu_end_elem(ctx); 

      mu_begin_elem_ex(ctx,90,1,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);
      mu_end_elem(ctx); 

    mu_end_elem(ctx);

    mu_begin_elem(ctx,0,20);
    mu_end_elem(ctx);

  mu_end_elem_window(ctx);
  }
}



static void process_frame(mu_Context *ctx) {
  
  mu_begin(ctx);
  layout(ctx);
  mu_end(ctx);
}



constexpr std::array<char, 256> create_button_map() {
    std::array<char, 256> map{};
    map[SDL_BUTTON_LEFT & 0xff]   = MU_MOUSE_LEFT;
    map[SDL_BUTTON_RIGHT & 0xff]  = MU_MOUSE_RIGHT;
    map[SDL_BUTTON_MIDDLE & 0xff] = MU_MOUSE_MIDDLE;
    return map;
}
static constexpr auto button_map = create_button_map();

constexpr std::array<char, 256> create_key_map() {
    std::array<char, 256> map{};
    map[SDLK_LSHIFT & 0xff]    = MU_KEY_SHIFT;
    map[SDLK_RSHIFT & 0xff]    = MU_KEY_SHIFT;
    map[SDLK_LCTRL & 0xff]     = MU_KEY_CTRL;
    map[SDLK_RCTRL & 0xff]     = MU_KEY_CTRL;
    map[SDLK_LALT & 0xff]      = MU_KEY_ALT;
    map[SDLK_RALT & 0xff]      = MU_KEY_ALT;
    map[SDLK_RETURN & 0xff]    = MU_KEY_RETURN;
    map[SDLK_BACKSPACE & 0xff] = MU_KEY_BACKSPACE;
    return map;
}
static constexpr auto key_map = create_key_map();

static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return r_get_text_width(font, text, len);
}

static int text_height(mu_Font font) {

  return r_get_text_height(font);
}
// 


int main (int argc, char *argv[]) {
    (void)argc; 
    (void)argv; 

    SDL_Init(SDL_INIT_EVERYTHING);
    r_init();
    r_load_font(&q_font, "C:/Users/ed/Documents/camera/camera microui/assets/fonts/ZCOOL_QingKe_HuangYou/ZCOOLQingKeHuangYou-Regular.ttf", 16);
      /* init microui */
    mu_Context *ctx =(mu_Context*) malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->animatable=&newstyle;
    ctx->text_width = text_width;
    ctx->text_height = text_height;


    bool quit = false;
    while (!quit) {
  /* main loop */

        /* handle SDL events */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                quit=true;
                break;
            case SDL_MOUSEMOTION: mu_input_mousemove(ctx, e.motion.x, e.motion.y); break;
            case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, e.wheel.y * -30); break;
            case SDL_TEXTINPUT: mu_input_text(ctx, e.text.text); break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
            int b = button_map[e.button.button & 0xff];
            if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
            if (b && e.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b);   }
            break;
            }

            case SDL_KEYDOWN:
            case SDL_KEYUP: {
            int c = key_map[e.key.keysym.sym & 0xff];
            if (c && e.type == SDL_KEYDOWN) { mu_input_keydown(ctx, c); }
            if (c && e.type ==   SDL_KEYUP) { mu_input_keyup(ctx, c);   }
            break;
            }
        }
        }

        /* process frame */
        process_frame(ctx);

        /* render */
        r_clear(mu_color(bg[0], bg[1], bg[2], 255));
        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd)) {
          switch (cmd->type) {
              case MU_COMMAND_TEXT: r_draw_text(cmd->text.str,cmd->text.font, cmd->text.pos, cmd->text.color); break;
              case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
              case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
              case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
          }
        }
        r_present();    
      //  quit=1;
    }
    return 0;
}
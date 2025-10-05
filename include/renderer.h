#ifndef RENDERER_H
#define RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif



#include "micro_flexbox.h"
void r_init(void);
void r_draw_rect(mu_Rect rect, mu_Color color);
void r_draw_text(const char *text,mu_Font font, mu_Vec2 pos, mu_Color color);
void r_draw_icon(int id, mu_Rect rect, mu_Color color);
 int r_get_text_width(mu_Font font, const char *text, int len);
 int r_get_text_height(mu_Font font);
void r_set_clip_rect(mu_Rect rect);
void r_clear(mu_Color color);
void r_present(void);
void r_load_font(mu_Font *font, const char* path, unsigned char size);


#ifdef __cplusplus
}
#endif


#endif
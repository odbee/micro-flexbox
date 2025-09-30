/*
** Copyright (c) 2024 rxi
**
** This library is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See `microui.c` for details.
*/

#ifndef MICROUI_H
#define MICROUI_H


#ifdef __cplusplus
extern "C" {
#endif




#define MU_VERSION "2.02"

#define MU_COMMANDLIST_SIZE     (256 * 1024)
#define MU_ROOTLIST_SIZE        32
#define MU_CONTAINERSTACK_SIZE  32
#define MU_CLIPSTACK_SIZE       32
#define MU_IDSTACK_SIZE         32
#define MU_LAYOUTSTACK_SIZE     16
#define MU_ELEMENTSTACK_SIZE    256
#define MU_ANIMSTACK_SIZE        256
#define MU_CONTAINERPOOL_SIZE   48
#define MU_TREENODEPOOL_SIZE    48
#define MU_MAX_WIDTHS           16
#define MU_MAX_CHILDREN         16

#define MU_REAL                 float
#define MU_REAL_FMT             "%.3g"
#define MU_SLIDER_FMT           "%.2f"
#define MU_MAX_FMT              127

#define mu_stack(T, n)          struct { int idx; T items[n]; }
#define mu_min(a, b)            ((a) < (b) ? (a) : (b))
#define mu_max(a, b)            ((a) > (b) ? (a) : (b))
#define mu_clamp(x, a, b)       mu_min(b, mu_max(a, x))
#define mu_absmin(a, b)            (abs(a) < abs(b) ? (a) : (b))
enum {
  MU_CLIP_PART = 1,
  MU_CLIP_ALL
};




enum {
  MU_COMMAND_JUMP = 1,
  MU_COMMAND_CLIP,
  MU_COMMAND_RECT,
  MU_COMMAND_TEXT,
  MU_COMMAND_ICON,
  MU_COMMAND_MAX
};

enum {
  MU_COLOR_TEXT,
  MU_COLOR_BORDER,
  MU_COLOR_WINDOWBG,
  MU_COLOR_TITLEBG,
  MU_COLOR_TITLETEXT,
  MU_COLOR_PANELBG,
  MU_COLOR_BUTTON,
  MU_COLOR_BUTTONHOVER,
  MU_COLOR_BUTTONFOCUS,
  MU_COLOR_BASE,
  MU_COLOR_BASEHOVER,
  MU_COLOR_BASEFOCUS,
  MU_COLOR_SCROLLBASE,
  MU_COLOR_SCROLLTHUMB,
  MU_COLOR_MAX
};

enum {
  MU_ICON_CLOSE = 1,
  MU_ICON_CHECK,
  MU_ICON_COLLAPSED,
  MU_ICON_EXPANDED,
  MU_ICON_MAX
};

enum {
  MU_RES_ACTIVE       = (1 << 0),
  MU_RES_SUBMIT       = (1 << 1),
  MU_RES_CHANGE       = (1 << 2)
};

enum {
  MU_OPT_ALIGNCENTER  = (1 << 0),
  MU_OPT_ALIGNRIGHT   = (1 << 1),
  MU_OPT_NOINTERACT   = (1 << 2),
  MU_OPT_NOFRAME      = (1 << 3),
  MU_OPT_NORESIZE     = (1 << 4),
  MU_OPT_NOSCROLL     = (1 << 5),
  MU_OPT_NOCLOSE      = (1 << 6),
  MU_OPT_NOTITLE      = (1 << 7),
  MU_OPT_HOLDFOCUS    = (1 << 8),
  MU_OPT_AUTOSIZE     = (1 << 9),
  MU_OPT_POPUP        = (1 << 10),
  MU_OPT_CLOSED       = (1 << 11),
  MU_OPT_EXPANDED     = (1 << 12)
};

enum {
  MU_EL_CLICKABLE  = (1 << 0),
  MU_EL_SCROLLABLE = (1 << 1),
  MU_EL_DRAGGABLE  = (1 << 2),
  MU_EL_DEBUG      = (1 << 3),
  MU_EL_STUTTER    = (1 << 4),

  
};


enum {
  MU_STATE_INACTIVE     ,
  MU_STATE_ACTIVE       ,
  MU_STATE_HOVERED      ,
  MU_STATE_JUSTHOVERED  ,
  MU_STATE_FOCUSED      ,
  MU_STATE_JUSTFOCUSED  ,
  MU_STATE_UNFOCUSED    ,
  MU_STATE_UNHOVERED    ,
  
    
};


enum {
  MU_MOUSE_LEFT       = (1 << 0),
  MU_MOUSE_RIGHT      = (1 << 1),
  MU_MOUSE_MIDDLE     = (1 << 2)
};

enum {
  MU_KEY_SHIFT        = (1 << 0),
  MU_KEY_CTRL         = (1 << 1),
  MU_KEY_ALT          = (1 << 2),
  MU_KEY_BACKSPACE    = (1 << 3),
  MU_KEY_RETURN       = (1 << 4)
};
 
 enum {
    MU_ALIGN_LEFT   = 1 << 0,  // 0001
    MU_ALIGN_CENTER = 1 << 1,  // 0010
    MU_ALIGN_RIGHT  = 1 << 2,  // 0100

    MU_ALIGN_TOP    = 1 << 3,  // 1000
    MU_ALIGN_MIDDLE = 1 << 4,  // 1 0000
    MU_ALIGN_BOTTOM = 1 << 5   // 10 0000
};


typedef struct mu_Context mu_Context;
typedef unsigned mu_Id;
typedef MU_REAL mu_Real;
typedef void* mu_Font;

typedef struct { int x, y; } mu_Vec2;
typedef struct { float x, y; } mu_fVec2;

typedef struct { int x, y, w, h; } mu_Rect;
typedef struct { unsigned char r, g, b, a; } mu_Color;
typedef struct { mu_Id id; int last_update; } mu_PoolItem;


typedef struct { int type, size; } mu_BaseCommand;
typedef struct { mu_BaseCommand base; void *dst; } mu_JumpCommand;
typedef struct { mu_BaseCommand base; mu_Rect rect; } mu_ClipCommand;
typedef struct { mu_BaseCommand base; mu_Rect rect; mu_Color color; } mu_RectCommand;
typedef struct { mu_BaseCommand base; mu_Font font; mu_Vec2 pos; mu_Color color; char str[1]; } mu_TextCommand;
typedef struct { mu_BaseCommand base; mu_Rect rect; int id; mu_Color color; } mu_IconCommand;

typedef union {
  int type;
  mu_BaseCommand base;
  mu_JumpCommand jump;
  mu_ClipCommand clip;
  mu_RectCommand rect;
  mu_TextCommand text;
  mu_IconCommand icon;
} mu_Command;


typedef int mu_Bool;
#define true 1
#define false 0


typedef enum {
    DIR_X = 1,
    DIR_Y = 0
} mu_Dir;

typedef enum {
    PERMANENT = 1,
    TEMPORARY = 0
} mu_AnimType;

typedef struct {
  int children[MU_MAX_CHILDREN]; // id of children
  int count;
  int parent;
} mu_Tree;

typedef struct {
  const char *str;
  int fontsize;
  const char *font;
} mu_Text;

typedef struct {
  int id;
  double progress, time, initial, prev;
} mu_Animation;



typedef struct {
  //fixed styling
  mu_Color border_color;
  mu_Color bg_color;
  
  signed char border_size;
  signed char gap;
  signed char padding;
  //text styling
  mu_Color text_color;
  mu_Font font;
  int text_align;
  //anim styling
  mu_Color hover_color;
  mu_Color focus_color;
  
  mu_Vec2 scroll;

} mu_Animatable;

#define MU_STYLE_BORDER_COLOR  (1<<0)
#define MU_STYLE_BG_COLOR      (1<<1)
#define MU_STYLE_BORDER_SIZE   (1<<2)
#define MU_STYLE_GAP           (1<<3)
#define MU_STYLE_PADDING       (1<<4)
#define MU_STYLE_TEXT_COLOR    (1<<5)
#define MU_STYLE_FONT          (1<<6)
#define MU_STYLE_TEXT_ALIGN    (1<<7)
#define MU_STYLE_HOVER_COLOR   (1<<8)
#define MU_STYLE_SCROLL_X      (1<<10)
#define MU_STYLE_SCROLL_Y      (1<<11)



typedef struct {
  unsigned short set_flags; // bitmask indicating which fields are active
  //fixed styling
  mu_Color border_color;
  mu_Color bg_color;
  signed char border_size;
  signed char gap;
  signed char padding;
  //text styling
  mu_Color text_color;
  mu_Font font;
  int text_align;
  //anim styling
  mu_Color hover_color;
  mu_Color focus_color;
  
  mu_Vec2 scroll;

} mu_AnimatableOverride;


typedef struct {
  int childAlignment;// align top, center, bottom. left middle right
  mu_Dir direction;
  mu_fVec2 sizing; // 0 to 1 values are for percent. 1 to n values are for fixed, 0 is for grow,-1 is for fit.
  mu_Vec2 min; // minimum size for box
  mu_Rect rect;
  mu_Tree tree;
  mu_Text text;
  mu_Rect clip;
  int childrensize; //TOTAL SIZE OF ALL CHILDREN ELEMENTS TOGETHER (without padding or gap)
  int idx;
  int state;
  unsigned int hash;
  signed char tier;
  int settings;
  signed char cooldown;
  mu_Animatable style;

} mu_Elem;



typedef struct {
  mu_AnimType type;
  mu_AnimatableOverride animable;
  int hash;
  int (*tween)(int t);
  double progress, time, initial,prev;
} mu_Anim;

typedef struct {
  mu_Command *head, *tail;
  mu_Rect rect;
  mu_Rect body;
  mu_Vec2 content_size;
  mu_Vec2 scroll;
  int zindex;
  int open;
} mu_Container;

typedef struct {
  mu_Font font;
  mu_Vec2 size;
  int padding;
  int spacing;
  int indent;
  int title_height;
  int scrollbar_size;
  int thumb_size;
  mu_Color colors[MU_COLOR_MAX];
} mu_Style;

struct mu_Context {
  /* callbacks */
  int (*text_width)(mu_Font font, const char *str, int len);
  int (*text_height)(mu_Font font);
  void (*draw_frame)(mu_Context *ctx, mu_Rect rect, int colorid);
  /* core state */
  mu_Style _style;
  mu_Style *style;

  mu_Animatable _animatable;
  mu_Animatable *animatable;
  
  mu_Id hover;
  mu_Id focus;
  mu_Id last_id;
  mu_Rect last_rect;
  int last_zindex;
  int updated_focus;
  int frame;
  int tier;
  int last_time;
  int dt; // DELTA TIME
  mu_Container *hover_root;
  mu_Container *next_hover_root;
  mu_Container *scroll_target;
  char number_edit_buf[MU_MAX_FMT];
  mu_Id number_edit;
  
  /* stacks */
  mu_stack(char, MU_COMMANDLIST_SIZE) command_list;
  mu_stack(mu_Container*, MU_ROOTLIST_SIZE) root_list;
  mu_stack(mu_Container*, MU_CONTAINERSTACK_SIZE) container_stack;
  mu_stack(mu_Rect, MU_CLIPSTACK_SIZE) clip_stack;
  mu_stack(mu_Id, MU_IDSTACK_SIZE) id_stack;
  mu_stack(mu_Elem, MU_ELEMENTSTACK_SIZE) element_stack;
  mu_stack(mu_Anim, MU_ANIMSTACK_SIZE) anim_stack;
  /* retained state pools */
  mu_PoolItem container_pool[MU_CONTAINERPOOL_SIZE];
  mu_Container containers[MU_CONTAINERPOOL_SIZE];
  /* input state */
  mu_Vec2 mouse_pos;
  mu_Vec2 last_mouse_pos;
  mu_Vec2 mouse_delta;
  mu_Vec2 scroll_delta;
  int mouse_down;
  int mouse_pressed;
  int key_down;
  int key_pressed;
  mu_Elem* current_parent;

  char input_text[32];
};


mu_Vec2 mu_vec2(int x, int y);
mu_Rect mu_rect(int x, int y, int w, int h);
mu_Color mu_color(int r, int g, int b, int a);

void mu_init(mu_Context *ctx);
void mu_begin(mu_Context *ctx);
void mu_end(mu_Context *ctx);
void mu_set_focus(mu_Context *ctx, mu_Id id);
mu_Id mu_get_id(mu_Context *ctx, const void *data, int size);
void mu_push_id(mu_Context *ctx, const void *data, int size);
void mu_pop_id(mu_Context *ctx);
void mu_push_clip_rect(mu_Context *ctx, mu_Rect rect);
void mu_pop_clip_rect(mu_Context *ctx);
mu_Rect mu_get_clip_rect(mu_Context *ctx);
int mu_check_clip_ex(mu_Rect r,mu_Rect cr);
mu_Container* mu_get_current_container(mu_Context *ctx);
mu_Container* mu_get_container(mu_Context *ctx, const char *name);
void mu_bring_to_front(mu_Context *ctx, mu_Container *cnt);

int mu_pool_init(mu_Context *ctx, mu_PoolItem *items, int len, mu_Id id);
int mu_pool_get(mu_Context *ctx, mu_PoolItem *items, int len, mu_Id id);
void mu_pool_update(mu_Context *ctx, mu_PoolItem *items, int idx);

void mu_input_mousemove(mu_Context *ctx, int x, int y);
void mu_input_mousedown(mu_Context *ctx, int x, int y, int btn);
void mu_input_mouseup(mu_Context *ctx, int x, int y, int btn);
void mu_input_scroll(mu_Context *ctx, int x, int y);
void mu_input_keydown(mu_Context *ctx, int key);
void mu_input_keyup(mu_Context *ctx, int key);
void mu_input_text(mu_Context *ctx, const char *text);

mu_Command* mu_push_command(mu_Context *ctx, int type, int size);
int mu_next_command(mu_Context *ctx, mu_Command **cmd);
void mu_set_clip(mu_Context *ctx, mu_Rect rect);
void mu_draw_rect(mu_Context *ctx, mu_Rect rect, mu_Color color);

void mu_draw_outline_ex(mu_Context *ctx, mu_Rect rect, mu_Color color, int t);

void mu_draw_text(mu_Context *ctx, mu_Font font, const char *str, int len, mu_Vec2 pos, mu_Color color);
void mu_draw_icon(mu_Context *ctx, int id, mu_Rect rect, mu_Color color);
void mu_draw_point(mu_Context *ctx, mu_Vec2, mu_Color color);
void mu_layout_row_ex(mu_Context *ctx, int items, const int *widths, int height,mu_Dir direction);
void mu_layout_width(mu_Context *ctx, int width);
void mu_layout_height(mu_Context *ctx, int height);
void mu_layout_begin_column(mu_Context *ctx);
void mu_layout_end_column(mu_Context *ctx);
void mu_layout_set_next(mu_Context *ctx, mu_Rect r, int relative);
void mu_layout_set_squares(mu_Context *ctx, mu_Bool isSquare);

mu_Rect mu_layout_next(mu_Context *ctx);

void mu_draw_control_frame(mu_Context *ctx, mu_Id id, mu_Rect rect, int colorid, int opt);
void mu_draw_control_text(mu_Context *ctx, const char *str, mu_Rect rect, int colorid, int opt);
int mu_mouse_over(mu_Context *ctx, mu_Rect rect);

#define mu_button(ctx, label)             mu_button_ex(ctx, label, 0, MU_OPT_ALIGNCENTER)
#define mu_textbox(ctx, buf, bufsz)       mu_textbox_ex(ctx, buf, bufsz, 0)
#define mu_slider(ctx, value, lo, hi)     mu_slider_ex(ctx, value, lo, hi, 0, MU_SLIDER_FMT, MU_OPT_ALIGNCENTER)
#define mu_number(ctx, value, step)       mu_number_ex(ctx, value, step, MU_SLIDER_FMT, MU_OPT_ALIGNCENTER)
#define mu_header(ctx, label)             mu_header_ex(ctx, label, 0)
#define mu_begin_treenode(ctx, label)     mu_begin_treenode_ex(ctx, label, 0)
#define mu_begin_window(ctx, title, rect) mu_begin_window_ex(ctx, title, rect, 0)
#define mu_begin_panel(ctx, name)         mu_begin_panel_ex(ctx, name, 0)
#define mu_begin_elem(ctx, sizex, sizey)  mu_begin_elem_ex(ctx,sizex,sizey,DIR_X,MU_ALIGN_CENTER,0)

#define mu_check_clip(ctx, r)          mu_check_clip_ex(r,mu_get_clip_rect(ctx)) 


#define mu_draw_outline(ctx, rect, color) mu_draw_outline_ex(ctx, rect, color,1)

void mu_label(mu_Context *ctx, const char *text);
int mu_button_ex(mu_Context *ctx, const char *label, int icon, int opt);
int mu_textbox_raw(mu_Context *ctx, char *buf, int bufsz, mu_Id id, mu_Rect r, int opt);
int mu_textbox_ex(mu_Context *ctx, char *buf, int bufsz, int opt);
int mu_slider_ex(mu_Context *ctx, mu_Real *value, mu_Real low, mu_Real high, mu_Real step, const char *fmt, int opt);
int mu_number_ex(mu_Context *ctx, mu_Real *value, mu_Real step, const char *fmt, int opt);
int mu_header_ex(mu_Context *ctx, const char *label, int opt);
int mu_begin_treenode_ex(mu_Context *ctx, const char *label, int opt);
void mu_end_treenode(mu_Context *ctx);

void mu_begin_panel_ex(mu_Context *ctx, const char *name, int opt);
void mu_end_panel(mu_Context *ctx);
void mu_layout_row(mu_Context *ctx, int items, const int *widths, int height);



// FLEX  FUNCTIONS
int mu_begin_elem_ex(mu_Context *ctx, float sizex, float sizey, mu_Dir direction,int alignopts, int settings);
void mu_end_elem(mu_Context *ctx);
void mu_resize(mu_Context *ctx);
void mu_apply_size(mu_Context *ctx);
void mu_adjust_elem_positions(mu_Context *ctx);
void mu_draw_debug_elems(mu_Context *ctx);
void mu_print_debug_tree(mu_Context *ctx);
int mu_begin_elem_window_ex(mu_Context *ctx, const char *title, mu_Rect rect, int opt);
void mu_end_elem_window(mu_Context *ctx);
void mu_handle_interaction(mu_Context *ctx);
void mu_add_text_to_elem(mu_Context *ctx,const char* text);
void mu_set_global_style(mu_Context *ctx,mu_Animatable style);
void mu_animation_set(mu_Context *ctx,void (*anim)(mu_Elem* elem));

#ifdef __cplusplus
}
#endif


#endif
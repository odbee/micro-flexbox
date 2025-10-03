#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "microui_flex.h"

#define unused(x) ((void) (x))

#define expect(x) do {                                               \
    if (!(x)) {                                                      \
      fprintf(stderr, "Fatal error: %s:%d: assertion '%s' failed\n", \
        __FILE__, __LINE__, #x);                                     \
      abort();                                                       \
    }                                                                \
  } while (0)

#define push(stk, val) do {                                                 \
    expect((stk).idx < (int) (sizeof((stk).items) / sizeof(*(stk).items))); \
    (stk).items[(stk).idx] = (val);                                         \
    (stk).idx++; /* incremented after incase `val` uses this value */       \
  } while (0)

#define pop(stk) do {      \
    expect((stk).idx > 0); \
    (stk).idx--;           \
  } while (0)


static mu_Rect unclipped_rect = { 0, 0, 0x1000000, 0x1000000 };

static mu_Style default_style = {
  /* font | size | padding | spacing | indent */
  NULL, { 68, 10 }, 5, 5, 24,
  /* title_height | scrollbar_size | thumb_size */
  24, 12, 8,
  {
    { 230, 230, 230, 255 }, /* MU_COLOR_TEXT */
    { 200,  25,  25,  255 }, /* MU_COLOR_BORDER */
    { 50,  50,  50,  255 }, /* MU_COLOR_WINDOWBG */
    { 25,  25,  25,  255 }, /* MU_COLOR_TITLEBG */
    { 240, 240, 240, 255 }, /* MU_COLOR_TITLETEXT */
    { 0,   0,   0,   0   }, /* MU_COLOR_PANELBG */
    { 75,  75,  75,  255 }, /* MU_COLOR_BUTTON */
    { 95,  95,  95,  255 }, /* MU_COLOR_BUTTONHOVER */
    { 115, 115, 115, 255 }, /* MU_COLOR_BUTTONFOCUS */
    { 30,  30,  30,  255 }, /* MU_COLOR_BASE */
    { 35,  35,  35,  255 }, /* MU_COLOR_BASEHOVER */
    { 40,  40,  40,  255 }, /* MU_COLOR_BASEFOCUS */
    { 43,  43,  43,  255 }, /* MU_COLOR_SCROLLBASE */
    { 30,  30,  30,  255 }  /* MU_COLOR_SCROLLTHUMB */
  }
};


static mu_Animatable default_animatable = {
  { 230, 200, 0, 255 }, /* border_color */
  { 20, 20, 20, 255 }, /* bg_color */
  1,                  /* border_size */
  5,                  /* gap */
  5,                    /* padding */
  { 230, 200, 0, 255 }, /* text_color */
  NULL, /* font*/
  10010, /* text_align*/
  { 180, 200, 0, 255 }, /* hover_color */
  { 130, 200, 230, 255 }, /* focus_color */
  {0,0}
};

/// @brief Initializes and returns a new 2D vector.
///
/// This function returns a new mu_Vec2 structure and sets its x and y
/// coordinates to the values provided.
///
/// @param x The x-coordinate for the vector.
/// @param y The y-coordinate for the vector.
/// @return A mu_Vec2 structure with the specified coordinates.
mu_Vec2 mu_vec2(int x, int y) {
  mu_Vec2 res;
  res.x = x; res.y = y;
  return res;
}

float mu_fabs(float x) {
    union {
        float f;
        unsigned int u;
    } v;

    v.f = x;
    v.u &= 0x7FFFFFFF;  // clear the sign bit
    return v.f;
}

float mu_fabsmin(float a, float b){
  return (mu_fabs(a) < mu_fabs(b) ? (a) : (b));
}

/// @brief Initializes and returns a new rectangle.
///
/// This function returns a new mu_Rect structure and sets its position
/// (x, y) and dimensions (width, height) with the given values.
///
/// @param x The x-coordinate of the rectangle's top-left corner.
/// @param y The y-coordinate of the rectangle's top-left corner.
/// @param w The width of the rectangle.
/// @param h The height of the rectangle.
/// @return A mu_Rect structure with the specified position and dimensions.
mu_Rect mu_rect(int x, int y, int w, int h) {
  mu_Rect res;
  res.x = x; res.y = y; res.w = w; res.h = h;
  return res;
}

/// @brief Initializes and returns a new color.
///
/// This function returns a new mu_Color structure and fills it
/// with the provided red, green, blue, and alpha value.
///
/// @param r The red value (0-255).
/// @param g The green value (0-255).
/// @param b The blue value (0-255).
/// @param a The alpha (opacity) value (0-255).
/// @return A mu_Color structure with the specified color value.
mu_Color mu_color(int r, int g, int b, int a) {
  mu_Color res;
  res.r = r; res.g = g; res.b = b; res.a = a;
  return res;
}

/// @brief Calculates the intersection of two rectangles.
///
/// This function determines the overlapping area between two rectangles
/// and returns a new rectangle of common area. If the
/// two rectangles do not overlap, it returns a rectangle with a width
/// OR height of zero. You have to check both if you want to make sure there is no overlap
///
/// @param r1 The first rectangle.
/// @param r2 The second rectangle.
/// @return A new mu_Rect representing the overlapping area.
static mu_Rect intersect_rects(mu_Rect r1, mu_Rect r2) {
  int x1 = mu_max(r1.x, r2.x);
  int y1 = mu_max(r1.y, r2.y);
  int x2 = mu_min(r1.x + r1.w, r2.x + r2.w);
  int y2 = mu_min(r1.y + r1.h, r2.y + r2.h);
  if (x2 < x1) { x2 = x1; }
  if (y2 < y1) { y2 = y1; }
  return mu_rect(x1, y1, x2 - x1, y2 - y1);
}



/// @brief Checks if a point overlaps with a rectangle.
///
/// This function tests whether a given 2D vector (point) is located
/// inside the boundaries of a rectangle. The check is inclusive of the
/// top and left edges, and exclusive of the bottom and right edges.
/// User to check mouse on/mouse hover
///
/// @param r The rectangle to check against.
/// @param p The 2D vector (point) to check for overlap.
/// @return Returns 1 if the point is inside the rectangle, otherwise returns 0.
static int rect_overlaps_vec2(mu_Rect r, mu_Vec2 p) {
  return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h;
}



/// @brief Initializes a MicroUI context.
/// @param ctx The context to initialize.
///
/// This function clears the context state, sets up the default frame
/// drawing function, and configures the default style.
void mu_init(mu_Context *ctx) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->_style = default_style;
  ctx->style = &ctx->_style;
  ctx->_animatable = default_animatable;
  ctx->animatable = &ctx->_animatable;
  
  ctx->tier=0;


}

/// @brief Starts a new UI frame.
/// @param ctx The context to prepare for the new frame.
///
/// This function is called at the beginning of each frame. It
/// resets internal command and root lists, updates the hover state,
/// calculates the mouse movement delta, and increments the frame counter.
void mu_begin(mu_Context *ctx) {
  expect(ctx->text_width && ctx->text_height);
  ctx->command_list.idx = 0;
  ctx->root_list.idx = 0;
  ctx->element_stack.idx=0;
  ctx->current_parent=NULL;

  ctx->mouse_delta.x = ctx->mouse_pos.x - ctx->last_mouse_pos.x;
  ctx->mouse_delta.y = ctx->mouse_pos.y - ctx->last_mouse_pos.y;
  ctx->frame++;
}


/// @brief Finalizes a UI frame.
/// @param ctx The context to finalize.
///
/// This function is called at the end of each frame's update. It performs
/// final cleanup and prepares the context for rendering. It verifies
/// stack consistency, handles scroll input, manages focus and hover states,
/// resets input variables, sorts all root containers by z-index, and
/// links their command lists to ensure a correct drawing order.
void mu_end(mu_Context *ctx) {
  int i, n;
  /* check stacks */
  expect(ctx->container_stack.idx == 0);
  expect(ctx->clip_stack.idx      == 0);
  expect(ctx->id_stack.idx        == 0);

  /* STORE TIME*/
  ctx->dt = (SDL_GetTicks() - ctx->last_time);
  ctx->last_time = SDL_GetTicks();
  // printf("dt %d, last_time %d\n", ctx->dt, ctx->last_time);
  /* reset input state */
  ctx->key_pressed = 0;
  ctx->input_text[0] = '\0';
  ctx->mouse_pressed = 0;
  ctx->scroll_delta = mu_vec2(0, 0);
  ctx->last_mouse_pos = ctx->mouse_pos;
  
  /* sort root containers by zindex */
  n = ctx->root_list.idx;

  /* set root container jump commands */ //TODO CHECK THIS
  for (i = 0; i < n; i++) {
    mu_Container *cnt = ctx->root_list.items[i];
    /* if this is the first container then make the first command jump to it.
    ** otherwise set the previous container's tail to jump to this one */
    if (i == 0) {
      mu_Command *cmd = (mu_Command*) ctx->command_list.items;
      cmd->jump.dst = (char*) cnt->head + sizeof(mu_JumpCommand);
    } else {
      mu_Container *prev = ctx->root_list.items[i - 1];
      prev->tail->jump.dst = (char*) cnt->head + sizeof(mu_JumpCommand);
    }
    /* make the last container's tail jump to the end of command list */
    if (i == n - 1) {
      cnt->tail->jump.dst = ctx->command_list.items + ctx->command_list.idx;
    }
  }

  
}

/// @brief Sets the input focus to a specific UI element.
/// @param ctx The MicroUI context.
/// @param id The unique identifier of the UI element to receive focus.
///
/// This function assigns the given ID as the currently focused element. The focused
/// element is the one that receives keyboard input and other events. It also sets
/// a flag to signal that an element has actively requested focus during this frame,
/// preventing the focus from being cleared at the end of the frame by mu_end.
void mu_set_focus(mu_Context *ctx, mu_Id id) {
  ctx->focus = id;
  ctx->updated_focus = 1;
}


/* 32bit fnv-1a hash */
#define HASH_INITIAL 2166136261

/// @brief Hashes a block of data using the 32-bit FNV-1a algorithm.
/// @param hash A pointer to the hash value to be updated.
/// @param data A pointer to the data to hash.
/// @param size The size of the data block in bytes.
///
/// This function incrementally updates a hash value based on the provided data.
static void hash(mu_Id *hash, const void *data, int size) {
  const unsigned char *p = data;
  while (size--) {
    *hash = (*hash ^ *p++) * 16777619;
  }
}

/// @brief Generates a unique ID for a UI element.
/// @param ctx The MicroUI context.
/// @param data A pointer to the data to hash (e.g., a widget label or name).
/// @param size The size of the data block in bytes.
/// @return The unique ID generated for the element.
///
/// This function creates a unique ID by combining the ID of the parent container
/// (from the ID stack) with the hash of the provided data. This ensures that
/// element IDs are unique within their hierarchical context. 
/// @warning If multiple root-level elements are given the same name, they will
///          generate identical IDs, which can lead to unpredictable behavior.
mu_Id mu_get_id(mu_Context *ctx, const void *data, int size) { 
  int idx = ctx->id_stack.idx;
  mu_Id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : HASH_INITIAL;
  hash(&res, data, size);
  ctx->last_id = res;
  return res;
}


/// @brief Pops an ID from the ID stack.
/// @param ctx The MicroUI context.
///
/// This function is used to end a hierarchical ID scope by removing the
/// last ID from the stack. Subsequent IDs will be generated in the parent's scope.
void mu_pop_id(mu_Context *ctx) {
  pop(ctx->id_stack);
}

/// @brief Pushes a new clipping rectangle onto the clip stack.
/// @param ctx The MicroUI context.
/// @param rect The new rectangle to apply.
///
/// This function constrains all subsequent drawing to a new, smaller area.
/// It calculates the intersection of the provided rectangle with the current
/// clipping rectangle on the stack and pushes the result. This ensures that
/// drawing remains confined within the bounds of all parent clipping regions.
void mu_push_clip_rect(mu_Context *ctx, mu_Rect rect) {
  mu_Rect last = mu_get_clip_rect(ctx);
  push(ctx->clip_stack, intersect_rects(rect, last));
}

/// @brief Pops a clipping rectangle from the clip stack.
/// @param ctx The MicroUI context.
///
/// This function is used to restore the previous clipping rectangle, effectively
/// "un-nesting" the drawing area. It is the counterpart to `mu_push_clip_rect`
/// and should be called after drawing within a scoped area.
void mu_pop_clip_rect(mu_Context *ctx) {
  pop(ctx->clip_stack);
}

/// @brief Returns the current active clipping rectangle.
/// @param ctx The MicroUI context.
/// @return The mu_Rect representing the current clipping area.
///
/// This function retrieves and returns the top-most clipping rectangle from the
/// clip stack. The function asserts that the stack is not empty.
mu_Rect mu_get_clip_rect(mu_Context *ctx) {
  expect(ctx->clip_stack.idx > 0);
  return ctx->clip_stack.items[ctx->clip_stack.idx - 1];
}

/// @brief Determines if a rectangle is visible, partially visible, or invisible in relation on the current clipping rectangle.
/// @param ctx The MicroUI context.
/// @param r The rectangle to check against the current clip rectangle.
/// @param cr  the current clip rectangle.

/// @return 
///         - `MU_CLIP_ALL` if the rectangle is entirely outside the clip rect.
///
///         - `0` if the rectangle is entirely inside the clip rect.
///
///         - `MU_CLIP_PART` if the rectangle is partially inside the clip rect.
///
/// This function is a utility for determining whether a given rectangle needs
/// to be clipped, is fully visible, or is not visible at all.

int mu_check_clip_ex(mu_Rect r, mu_Rect cr) {
  if (r.x > cr.x + cr.w || r.x + r.w < cr.x ||
      r.y > cr.y + cr.h || r.y + r.h < cr.y   ) { return MU_CLIP_ALL; }
  if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w &&
      r.y >= cr.y && r.y + r.h <= cr.y + cr.h ) { return 0; }
  return MU_CLIP_PART;
}





/// @brief Finalizes and ends the scope of the current container.
/// @param ctx The MicroUI context.
///
/// This function calculates the container's total content size based on the
/// dimensions of the widgets added to it. It then ends the container's scope by
/// removing the container, its layout, and its ID from their respective stacks.
///
/// Calculates content size: This is done to determine the total space
/// needed by all widgets within the container. This information is
/// used to correctly render scrollbars and for auto-sizing the container
/// to fit its content.
static void pop_container(mu_Context *ctx) {

  /* pop container, layout and id */
  pop(ctx->container_stack);
  // pop(ctx->layout_stack);
  mu_pop_id(ctx);
}

/// @brief Returns a pointer to the current container on the stack.
/// @param ctx The MicroUI context.
/// @return A pointer to the top-most mu_Container on the stack.
///
/// This function provides access to the currently active container. It asserts
/// that the container stack is not empty before returning the top-most
/// container. The function does not modify the stack.
mu_Container* mu_get_current_container(mu_Context *ctx) {
  expect(ctx->container_stack.idx > 0);
  return ctx->container_stack.items[ ctx->container_stack.idx - 1 ];
}

/// @brief Retrieves or creates a container from the context's container pool.
/// @param ctx The MicroUI context.
/// @param id The unique identifier of the container.
/// @param opt A bitmask of options, such as MU_OPT_CLOSED.
/// @return A pointer to the requested mu_Container, or NULL if not found and
///         MU_OPT_CLOSED is set.
///
/// This function first attempts to find an existing container in the context's
/// pool. If found, it updates the container's status as recently used and
/// returns it. If a container with the given ID is not found, it checks the
/// options. If the MU_OPT_CLOSED flag is set, it returns NULL. Otherwise, it
/// initializes a new container from the pool, marks it as open, and returns
/// a pointer to it.
static mu_Container* get_container(mu_Context *ctx, mu_Id id, int opt) {
  mu_Container *cnt;
  /* try to get existing container from pool */
  int idx = mu_pool_get(ctx, ctx->container_pool, MU_CONTAINERPOOL_SIZE, id);
  if (idx >= 0) {
    if (ctx->containers[idx].open || ~opt & MU_OPT_CLOSED) {
      mu_pool_update(ctx, ctx->container_pool, idx);
    }
    return &ctx->containers[idx];
  }
  if (opt & MU_OPT_CLOSED) { return NULL; }
  /* container not found in pool: init new container */
  idx = mu_pool_init(ctx, ctx->container_pool, MU_CONTAINERPOOL_SIZE, id);
  cnt = &ctx->containers[idx];
  memset(cnt, 0, sizeof(*cnt));
  cnt->open = 1;
  return cnt;
}

/// @brief Retrieves or creates a container using a string name.
/// @param ctx The MicroUI context.
/// @param name The string name of the container.
/// @return A pointer to the requested mu_Container.
///
/// This  provides a public interface for
/// getting a container. It automatically generates a unique ID from the
/// provided name and then calls the internal `get_container` function to
/// perform the retrieval or creation.

mu_Container* mu_get_container(mu_Context *ctx, const char *name) {
  mu_Id id = mu_get_id(ctx, name, strlen(name));
  return get_container(ctx, id, 0);
}



/*============================================================================
** pool
**============================================================================*/

/// @brief Initializes an item in a memory pool by recycling the least recently used item.
/// @param ctx The MicroUI context.
/// @param items A pointer to the pool of items.
/// @param len The size of the pool.
/// @param id The unique identifier to associate with the new item.
/// @return The index of the initialized item in the pool.
///
/// This function finds a free slot in a memory pool by iterating through
/// all items and finding the one with the oldest last_update frame number.
/// It then assigns the provided ID to this slot and updates its last_update
/// time to the current frame. The function asserts that a free slot was found.
int mu_pool_init(mu_Context *ctx, mu_PoolItem *items, int len, mu_Id id) {
  int i, n = -1, f = ctx->frame;
  for (i = 0; i < len; i++) {
    if (items[i].last_update < f) {
      f = items[i].last_update;
      n = i;
    }
  }
  expect(n > -1);
  items[n].id = id;
  mu_pool_update(ctx, items, n);
  return n;
}

/// @brief Performs a linear search for a pool item with a matching ID.
/// @param ctx The MicroUI context.
/// @param items A pointer to the pool of items to search.
/// @param len The size of the pool.
/// @param id The unique identifier to search for.
/// @return The index of the item if found, otherwise -1.
///
/// This function iterates through a pool and returns the index of the first
/// item whose ID matches the provided ID.
int mu_pool_get(mu_Context *ctx, mu_PoolItem *items, int len, mu_Id id) {
  int i;
  unused(ctx);
  for (i = 0; i < len; i++) {
    if (items[i].id == id) { return i; }
  }
  return -1;
}

/// @brief Updates a pool item's last-used timestamp.
/// @param ctx The MicroUI context.
/// @param items A pointer to the pool of items.
/// @param idx The index of the item to update.
///
/// This function sets the specified pool item's last_update frame number
/// to the current frame. This marks the item as recently used, which is a key
/// part of the pool's mechanism for recycling inactive items.
void mu_pool_update(mu_Context *ctx, mu_PoolItem *items, int idx) {
  items[idx].last_update = ctx->frame;
}


/*============================================================================
** input handlers
**============================================================================*/

void mu_input_mousemove(mu_Context *ctx, int x, int y) {
  ctx->mouse_pos = mu_vec2(x, y);
}


void mu_input_mousedown(mu_Context *ctx, int x, int y, int btn) {
  mu_input_mousemove(ctx, x, y);
  ctx->mouse_down |= btn;
  ctx->mouse_pressed |= btn;
}


void mu_input_mouseup(mu_Context *ctx, int x, int y, int btn) {
  mu_input_mousemove(ctx, x, y);
  ctx->mouse_down &= ~btn;
}


void mu_input_scroll(mu_Context *ctx, int x, int y) {
  ctx->scroll_delta.x += x;
  ctx->scroll_delta.y += y;
}


void mu_input_keydown(mu_Context *ctx, int key) {
  ctx->key_pressed |= key;
  ctx->key_down |= key;
}


void mu_input_keyup(mu_Context *ctx, int key) {
  ctx->key_down &= ~key;
}


void mu_input_text(mu_Context *ctx, const char *text) {
  int len = strlen(ctx->input_text);
  int size = strlen(text) + 1;
  expect(len + size <= (int) sizeof(ctx->input_text));
  memcpy(ctx->input_text + len, text, size);
}


/*============================================================================
** commandlist
**============================================================================*/
/// @brief Pushes a new command onto the command list.
/// @param ctx The MicroUI context.
/// @param type The type of command to push.
/// @param size The total size of the command in bytes.
/// @return A pointer to the newly pushed command.
///
/// This function adds a new command of the specified type and size to the
/// context's command list buffer. It handles the necessary pointer arithmetic
/// and checks for buffer overflow before returning a pointer to the new command.
mu_Command* mu_push_command(mu_Context *ctx, int type, int size) {
  mu_Command *cmd = (mu_Command*) (ctx->command_list.items + ctx->command_list.idx);
  expect(ctx->command_list.idx + size < MU_COMMANDLIST_SIZE);
  cmd->base.type = type;
  cmd->base.size = size;
  ctx->command_list.idx += size;
  return cmd;
}

/// @brief Iterates to the next command in the command list.
/// @param ctx The MicroUI context.
/// @param cmd A pointer to a mu_Command pointer. On the first call, this
///        should be NULL. On subsequent calls, it should be the address of the
///        last command returned.
/// @return Returns 1 if a command was found, or 0 if the end of the list
///         has been reached.
///
/// This function acts as an iterator for the context's command list. It
/// advances the command pointer to the next valid command, automatically
/// handling MU_COMMAND_JUMP commands by skipping to their destination.
int mu_next_command(mu_Context *ctx, mu_Command **cmd) {
  if (*cmd) {
    *cmd = (mu_Command*) (((char*) *cmd) + (*cmd)->base.size);
  } else {
    *cmd = (mu_Command*) ctx->command_list.items;
  }
  while ((char*) *cmd != ctx->command_list.items + ctx->command_list.idx) {
    if ((*cmd)->type != MU_COMMAND_JUMP) { return 1; }
    *cmd = (*cmd)->jump.dst;
  }
  return 0;
}

/// @brief Pushes a jump command onto the command list.
/// @param ctx The MicroUI context.
/// @param dst A pointer to the destination command to jump to.
/// @return A pointer to the newly created jump command.
///
/// This is a utility function that pushes a `MU_COMMAND_JUMP` onto the command
/// list. A jump command is used by the renderer to efficiently skip over
/// sections of the command list (e.g., commands for clipped widgets) by
/// redirecting the command list iterator to a new location.
static mu_Command* push_jump(mu_Context *ctx, mu_Command *dst) {
  mu_Command *cmd;
  cmd = mu_push_command(ctx, MU_COMMAND_JUMP, sizeof(mu_JumpCommand));
  cmd->jump.dst = dst;
  return cmd;
}

/// @brief Adds a clipping command to the command list.
/// @param ctx The MicroUI context.
/// @param rect The clipping rectangle to set.
///
/// This function queues a command to set the drawing clip rectangle. The
/// command does not immediately apply the clipping but stores it for later
/// processing by the renderer.
void mu_set_clip(mu_Context *ctx, mu_Rect rect) {
  mu_Command *cmd;
  cmd = mu_push_command(ctx, MU_COMMAND_CLIP, sizeof(mu_ClipCommand));
  cmd->clip.rect = rect;
}

/// @brief Adds a command to draw a rectangle. 
/// @param ctx The MicroUI context.
/// @param rect The rectangle to draw.
/// @param color The color to fill the rectangle.
///
/// This function first clips the given rectangle against the current clipping
/// rectangle. If the resulting rectangle has a positive width and height, a
/// command to draw the rectangle is added to the command list.
void mu_draw_rect(mu_Context *ctx, mu_Rect rect, mu_Color color) {
  mu_Command *cmd;
  rect = intersect_rects(rect, mu_get_clip_rect(ctx));
  if (rect.w > 0 && rect.h > 0) {
    cmd = mu_push_command(ctx, MU_COMMAND_RECT, sizeof(mu_RectCommand));
    cmd->rect.rect = rect;
    cmd->rect.color = color;
  }
}

void mu_draw_debug_clip_rect(mu_Context *ctx, mu_Rect rect, mu_Rect clip_rect, mu_Color color) {
  mu_Command *cmd;
  rect = intersect_rects(rect, clip_rect);
  if (rect.w > 0 && rect.h > 0) {
    cmd = mu_push_command(ctx, MU_COMMAND_RECT, sizeof(mu_RectCommand));
    cmd->rect.rect = rect;
    cmd->rect.color = color;
  }
}


/// @brief Adds a command to draw a rectangular outline around a given rectangle with a specified thickness. A negative thickness will cause the outline to be drawn inward.
/// @param ctx The MicroUI context.
/// @param rect The inner rectangle around which the outline will be drawn.
/// @param color The color of the outline.
/// @param t The thickness of the outline in pixels. Negative values
///                  will cause the outline to be drawn inward.
///
/// This function draws a border that expands outward from the given rectangle,
/// increasing the total size of the drawn area. It achieves this by drawing
/// four separate, filled rectangles for each side of the outline.
void mu_draw_outline_ex(mu_Context *ctx, mu_Rect rect, mu_Color color, int t) {
  mu_draw_rect(ctx, mu_rect(rect.x - t, rect.y - t, rect.w + (t * 2), t), color); // top line
  mu_draw_rect(ctx, mu_rect(rect.x - t, rect.y + rect.h, rect.w + (t * 2), t), color); // bottom line
  mu_draw_rect(ctx, mu_rect(rect.x - t, rect.y, t, rect.h), color); // left line
  mu_draw_rect(ctx, mu_rect(rect.x + rect.w, rect.y, t, rect.h), color); // right line
}


void mu_draw_debug_clip_outline_ex(mu_Context *ctx, mu_Rect rect,  mu_Rect clip_rect, mu_Color color, int t) {
  mu_draw_debug_clip_rect(ctx, mu_rect(rect.x - t, rect.y - t, rect.w + (t * 2), t),clip_rect, color); // top line
  mu_draw_debug_clip_rect(ctx, mu_rect(rect.x - t, rect.y + rect.h, rect.w + (t * 2), t),clip_rect, color); // bottom line
  mu_draw_debug_clip_rect(ctx, mu_rect(rect.x - t, rect.y, t, rect.h),clip_rect, color); // left line
  mu_draw_debug_clip_rect(ctx, mu_rect(rect.x + rect.w, rect.y, t, rect.h),clip_rect, color); // right line
}



/// @brief Adds a command to draw text.
/// @param ctx The MicroUI context.
/// @param font The font to use for drawing the text.
/// @param str The string to draw.
/// @param len The length of the string, or a negative value to use `strlen`.
/// @param pos The position to draw the text at.
/// @param color The color of the text.
///
/// This function calculates the bounding box of the text and checks if it needs
/// to be clipped. It returns early if the text is completely outside the clip
/// rectangle. If the text is visible, it adds a text drawing command to the
/// command list. The command includes the string data itself, which is copied
/// into the command buffer. The function also handles resetting the clipping
/// state if it was temporarily modified
void mu_draw_text(mu_Context *ctx, mu_Font font, const char *str, int len,
  mu_Vec2 pos, mu_Color color)
{
  mu_Command *cmd;
  mu_Rect rect = mu_rect(
    pos.x, pos.y, ctx->text_width(font, str, len), ctx->text_height(font));
  int clipped = mu_check_clip(ctx, rect);
  if (clipped == MU_CLIP_ALL ) { return; }
  if (clipped == MU_CLIP_PART) { mu_set_clip(ctx, mu_get_clip_rect(ctx)); }
  /* add command */
  if (len < 0) { len = strlen(str); }
  cmd = mu_push_command(ctx, MU_COMMAND_TEXT, sizeof(mu_TextCommand) + len);
  memcpy(cmd->text.str, str, len);
  cmd->text.str[len] = '\0';
  cmd->text.pos = pos;
  cmd->text.color = color;
  cmd->text.font = font;
  /* reset clipping if it was set */
  if (clipped) { mu_set_clip(ctx, unclipped_rect); }
}




void mu_draw_text_ex(mu_Context *ctx, mu_Font font, const char *str, int len,
  mu_Vec2 pos, mu_Color color, mu_Rect clip,mu_Rect parent,int textAlignment,int padding)
{

  mu_Command *cmd;

    mu_fVec2 m;
  if (textAlignment & MU_ALIGN_LEFT)   m.x = 0.0f;
  if (textAlignment & MU_ALIGN_CENTER) m.x = 0.5f;
  if (textAlignment & MU_ALIGN_RIGHT)  m.x = 1.0f;
  if (textAlignment & MU_ALIGN_TOP)    m.y = 0.0f;
  if (textAlignment & MU_ALIGN_MIDDLE) m.y = 0.5f;
  if (textAlignment & MU_ALIGN_BOTTOM) m.y = 1.0f;
  
  pos.x+= (parent.w - (ctx->text_width(font, str, len)+2*padding))*m.x+padding;
  pos.y+= (parent.h - (ctx->text_height(font)         +2*padding))*m.y + padding;


  mu_Rect rect = mu_rect(
    pos.x, pos.y, ctx->text_width(font, str, len), ctx->text_height(font));
  int clipped = mu_check_clip_ex(rect, clip);
  // printf("checking clip of text %s in rect %d %d %d %d against clip: %d %d %d %d. returned %d \n ", str,rect.x,rect.y,rect.w,rect.h,clip.x,clip.y,clip.w,clip.h,clipped);
  // mu_draw_rect(ctx,rect,mu_color(255,0,0,50));

  if (clipped == MU_CLIP_ALL ) { return; }
  /* add command */
  if (clipped == MU_CLIP_PART) { mu_set_clip(ctx, clip); }

  if (len < 0) { len = strlen(str); }
  cmd = mu_push_command(ctx, MU_COMMAND_TEXT, sizeof(mu_TextCommand) + len);
  memcpy(cmd->text.str, str, len);
  cmd->text.str[len] = '\0';
  cmd->text.pos = pos;
  cmd->text.color = color;
  cmd->text.font = font;
  /* reset clipping if it was set */
  if (clipped) { mu_set_clip(ctx, unclipped_rect); }
}

/// @brief Adds a command to draw an icon.
/// @param ctx The MicroUI context.
/// @param id The unique identifier of the icon to draw.
/// @param rect The rectangle where the icon should be drawn.
/// @param color The color to apply to the icon.
///
/// This function first checks if the icon's rectangle is fully or partially
/// clipped. If it's completely outside the clipping area, the function returns.
/// If it is partially clipped, a temporary clip command is queued to ensure the
/// icon is drawn correctly. Finally, a command to draw the icon is pushed to
/// the command list, and the original clipping state is restored.
void mu_draw_icon(mu_Context *ctx, int id, mu_Rect rect, mu_Color color) {
  mu_Command *cmd;
  /* do clip command if the rect isn't fully contained within the cliprect */
  int clipped = mu_check_clip(ctx, rect);
  if (clipped == MU_CLIP_ALL ) { return; }
  if (clipped == MU_CLIP_PART) { mu_set_clip(ctx, mu_get_clip_rect(ctx)); }
  /* do icon command */
  cmd = mu_push_command(ctx, MU_COMMAND_ICON, sizeof(mu_IconCommand));
  cmd->icon.id = id;
  cmd->icon.rect = rect;
  cmd->icon.color = color;
  /* reset clipping if it was set */
  if (clipped) { mu_set_clip(ctx, unclipped_rect); }
}


/*============================================================================
** layout
**============================================================================*/

enum { RELATIVE = 1, ABSOLUTE = 2 };


/*============================================================================
** controls
**============================================================================*/




/// @brief Checks if the mouse is currently hovering over a rectangle.
/// @param ctx The MicroUI context.
/// @param rect The rectangle to check for mouse hover.
/// @return Returns 1 if the mouse is over the rectangle, otherwise 0.
///
/// This function determines if the mouse cursor is hovering over the provided
/// rectangle. It returns true only if the following three conditions are met:
/// - The mouse position is within the `rect`.
/// - The mouse position is also within the current clipping rectangle.
/// - The hover root is active, ensuring that the widget is not occluded by
///   another window or container.
int mu_mouse_over(mu_Context *ctx, mu_Rect rect) {
  return rect_overlaps_vec2(rect, ctx->mouse_pos) &&
    rect_overlaps_vec2(mu_get_clip_rect(ctx), ctx->mouse_pos);
}



/// @brief Updates the hover and focus state for a control.
/// @param ctx The MicroUI context.
/// @param elem gui element
/// This function is a core part of the input handling system. It updates the
/// context's `hover` and `focus` fields for a given control based on mouse
/// position and button state. It handles conditions for gaining and losing focus,
/// and it respects the `MU_OPT_NOINTERACT` flag, which prevents any state
/// changes. The `MU_OPT_HOLDFOCUS` option is also handled, which allows a
/// control to retain focus even when the mouse button is released.
void mu_update_element_control(mu_Context *ctx, mu_Elem* elem) {
  ctx->updated_focus = 0;

  unsigned int id = elem->hash;
  mu_Rect rect = elem->rect;
  int mouseover = mu_mouse_over(ctx, rect);
  elem->state=MU_STATE_ACTIVE;
  if (ctx->focus == id) {
    ctx->hover = 0;

    ctx->updated_focus = 1;
    elem->state=MU_STATE_FOCUSED;// this means its still focused from last frame
    if (ctx->mouse_pressed && !mouseover) { // no longer over object but still down
      mu_set_focus(ctx, 0);
      elem->state=MU_STATE_FOCUSED;
    }
    if (!ctx->mouse_down) { // no longer clicking
      mu_set_focus(ctx, 0);
      printf("NO LONGER FOCUSING\n");
      elem->state=MU_STATE_UNFOCUSED;
      elem->cooldown=1;
    }
  }
  if (mouseover && !ctx->mouse_down) { //TO HOVER WE NEED TO BE !MOUSEDOWN
    ctx->hover = id;
    elem->state=MU_STATE_JUSTHOVERED;
    
  }

  if (ctx->hover == id) { 
    elem->state=MU_STATE_HOVERED;
    if (ctx->mouse_pressed) {// TO TOGGLE FOCUS WE NEED TO BE HOVERED FIRST
      mu_set_focus(ctx, id);
      elem->state=MU_STATE_JUSTFOCUSED;
    } else if (!mouseover) { //this means was hovered last frame and is no longer
      ctx->hover = 0;
      elem->state=MU_STATE_UNHOVERED;
    }
  }
}

const char *int_to_str(int value) {
    static char buffer[12];  // reused on each call
    char *p = buffer;
    char *p1, *p2;
    unsigned int u = value;

    if (value < 0) {
        *p++ = '-';
        u = -value;
    }

    p1 = p;
    do {
        *p++ = (u % 10) + '0';
        u /= 10;
    } while (u > 0);

    *p = '\0';

    // reverse the digits
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
    return buffer;  // caller gets pointer
}



int mu_begin_elem_ex(mu_Context *ctx, float sizex,float sizey, mu_Dir direction,int alignopts, int settings) {
  // push(ctx->element_stack,emptyelem); // THIS BREAKS THINGS

  int newindex=ctx->element_stack.idx++;
  mu_Elem*new_elem=&ctx->element_stack.items[newindex];
    // fill with values

  new_elem->tree.count=0;
  new_elem->tree.parent=-1;
  new_elem->idx=newindex; //set element id after we pushed it
  new_elem->animatable=*ctx->animatable;
  new_elem->tier=ctx->tier++;
  new_elem->childAlignment=alignopts;
  new_elem->settings=settings;
  new_elem->direction=direction;
  new_elem->sizing=(mu_fVec2){sizex,sizey};
  new_elem->clip=(mu_Rect){0,0,0,0};
  const void *data = int_to_str(new_elem->idx);
  const char *s = (const char *)data;  // "12345"
  new_elem->hash=mu_get_id(ctx,s , strlen(s));

  if (new_elem->tier!=0){
    new_elem->tree.parent= ctx->current_parent->idx;
    ctx->current_parent->tree.children[ctx->current_parent->tree.count++]=new_elem->idx;
  } {
    ctx->current_parent=new_elem;
  }
  if (new_elem->hash==ctx->focus){
      // printf("FOCUS ID, %d %d\n",new_elem->idx, new_elem->state);
  }
  return new_elem->state;
}

void mu_end_elem(mu_Context *ctx) {
  // TODO ADD FIT ALGORITHM BY ADDING UP CHILDREN SIZES
  mu_Elem*new_elem=&ctx->element_stack.items[ctx->element_stack.idx-1];
  if (new_elem->sizing.x==-1) {
    if (new_elem->text.str) {
      new_elem->sizing.x=(float)(ctx->text_width(new_elem->animatable.font,new_elem->text.str,-1)+new_elem->animatable.padding*2);
    }
  } else if (new_elem->sizing.y==-1) {
      new_elem->sizing.y=(float)(ctx->text_height(new_elem->animatable.font)+new_elem->animatable.padding*2);

  }
  ctx->tier--;
  ctx->current_parent = &ctx->element_stack.items[ctx->current_parent->tree.parent];

}

void mu_resize_children(mu_Context *ctx,mu_Elem* elem) {
  if (elem->tree.count){
    float totalChildSize = 0;
    int growChildren = 0;
    mu_Elem* listofgrowers[elem->tree.count];
    for (int i = 0; i < elem->tree.count; i++) {
      mu_Elem* child = &ctx->element_stack.items[elem->tree.children[i]];
      // FIND GROWERS
      if ((child->sizing.x==0&&elem->direction==DIR_X) || (child->sizing.y==0&&elem->direction==DIR_Y))
        listofgrowers[growChildren++]=child;
      // if sizing is set to grow across the weak axis we set it to PERCENTAGE
      if (elem->direction==DIR_X && child->sizing.y==0) 
        child->sizing.y=1;
      if (elem->direction==DIR_Y && child->sizing.x==0) 
        child->sizing.x=1;
      
      //HANDLE PERCENTAGE
      if (child->sizing.x <= 1&&child->sizing.x>0)
      {
        child->sizing.x = (child->sizing.x * elem->sizing.x); 
        child->sizing.x -= 2*elem->animatable.padding;
        child->sizing.x -= elem->animatable.gap*(elem->tree.count-1)*(elem->direction); //ONLY ADD GAPS TO THE ACTIVE AXIS
      }
      if (child->sizing.y <= 1&&child->sizing.y>0){
        child->sizing.y = (child->sizing.y * elem->sizing.y);
        child->sizing.y -= 2*elem->animatable.padding;
        child->sizing.y -= elem->animatable.gap*(elem->tree.count-1)*((elem->direction+1)%2);//ONLY ADD GAPS TO THE ACTIVE AXIS
      }
      //ADD TO TOTAL CHILD SIZE
      totalChildSize += child->sizing.x*((elem->direction +0)%2);
      totalChildSize += child->sizing.y*((elem->direction +1)%2);
    }
    float adjustsize=totalChildSize;
    for (int j =0;j<growChildren;j++){
        mu_Elem* child = listofgrowers[j];
        //TODO STILL HAVE TO CHECK IF MIN SIZE IS BIGGER THAN GROW SIZE
        child->sizing.x+=((elem->sizing.x - adjustsize-elem->animatable.gap*(elem->tree.count-1)-elem->animatable.padding*2)/growChildren )*(elem->direction);
        child->sizing.y+=((elem->sizing.y - adjustsize)/growChildren-elem->animatable.gap*(elem->tree.count-1)-elem->animatable.padding*2 )*((elem->direction +1)%2);
        totalChildSize+=child->sizing.x*(elem->direction);
        totalChildSize+=child->sizing.y*((elem->direction +1)%2);
    }
    elem->content_size=totalChildSize;
  } 
}

void mu_resize(mu_Context *ctx) {
  for (int i = 0; i < ctx->element_stack.idx; i++)
  {
    mu_resize_children(ctx, &ctx->element_stack.items[i]);
  }
  
}
void mu_apply_size(mu_Context *ctx)
{
  for (int i = 0; i < ctx->element_stack.idx; i++)
  {
    mu_Elem*elem=&ctx->element_stack.items[i];
    if (elem->sizing.x>1){
      elem->rect.w=(int)elem->sizing.x;
    }
    if (elem->sizing.y>1){
      elem->rect.h=(int)elem->sizing.y;
    }
  }
}

void mu_adjust_children_positions(mu_Context *ctx,mu_Elem* elem){
  mu_push_clip_rect(ctx,elem->rect);
  if (elem->tree.count>0){
    mu_fVec2 m;
    if (elem->childAlignment & MU_ALIGN_LEFT)   m.x = 0.0f;
    if (elem->childAlignment & MU_ALIGN_CENTER) m.x = 0.5f;
    if (elem->childAlignment & MU_ALIGN_RIGHT)  m.x = 1.0f;
    if (elem->childAlignment & MU_ALIGN_TOP)    m.y = 0.0f;
    if (elem->childAlignment & MU_ALIGN_MIDDLE) m.y = 0.5f;
    if (elem->childAlignment & MU_ALIGN_BOTTOM) m.y = 1.0f;
    mu_Elem* child;
    int compoundx=0;
    int compoundy=0;
    for (int i = 0; i < elem->tree.count; i++)  {
      child  =  &ctx->element_stack.items[elem->tree.children[i]];
      child->rect.x = elem->rect.x;
      child->rect.y = elem->rect.y;
      child->rect.x += elem->animatable.padding;
      child->rect.y += elem->animatable.padding;
      if (elem->direction==DIR_X){
        child->rect.x +=(elem->rect.w-elem->content_size)* m.x;
        child->rect.x -=(2*elem->animatable.padding+(elem->tree.count-1)*elem->animatable.gap)*m.x;
        child->rect.y +=(elem->rect.h-child->rect.h)*m.y;
        child->rect.y -=(2*elem->animatable.padding)*m.y;
      } else {
        child->rect.x +=(elem->rect.w-child->rect.w)*m.x;  
        child->rect.x -=2*elem->animatable.padding*m.x;

        child->rect.y +=(elem->rect.h-elem->content_size)* m.y;
        child->rect.y -=(2*elem->animatable.padding+(elem->tree.count-1)*elem->animatable.gap)*m.y;

      }
      child->rect.x += compoundx;
      child->rect.y += compoundy;
      child->rect.x += elem->animatable.scroll.x;
      child->rect.y += elem->animatable.scroll.y;
      compoundx     += (child->rect.w +elem->animatable.gap)*((elem->direction +0)%2);
      compoundy     += (child->rect.h +elem->animatable.gap)*((elem->direction +1)%2);

      child->clip=mu_get_clip_rect(ctx);

      if (child->settings&(MU_EL_CLICKABLE|MU_EL_DRAGGABLE|MU_EL_STUTTER)){
        mu_update_element_control(ctx,child);
      }
      mu_adjust_children_positions(ctx,child);

      
    }
  }
  mu_pop_clip_rect(ctx);
}



void mu_adjust_elem_positions(mu_Context *ctx)
{
  mu_adjust_children_positions(ctx,&ctx->element_stack.items[0]);
  
}

static inline float lerp_float(float a, float b, float t) {
    return a + t * (b - a);
}

static inline signed char lerp_char(signed char a, signed char b, float t) {
    int v = (int)(a + t * (b - a));
    if (v > 127) v = 127;
    if (v < -128) v = -128;
    return (signed char)v;
}

static inline mu_Color lerp_color(mu_Color a, mu_Color b, float t) {
    mu_Color c;
    c.r = (unsigned char)(a.r + t * (b.r - a.r));
    c.g = (unsigned char)(a.g + t * (b.g - a.g));
    c.b = (unsigned char)(a.b + t * (b.b - a.b));
    c.a = (unsigned char)(a.a + t * (b.a - a.a));
    return c;
}
#define MU_INTERPOLATABLE_FIELDS \
    X(MU_STYLE_BORDER_COLOR, border_color, lerp_color) \
    X(MU_STYLE_BG_COLOR,     bg_color,    lerp_color) \
    X(MU_STYLE_BORDER_SIZE,  border_size, lerp_char) \
    X(MU_STYLE_GAP,          gap,         lerp_char) \
    X(MU_STYLE_PADDING,      padding,     lerp_char) \
    X(MU_STYLE_TEXT_COLOR,   text_color,  lerp_color) \
    /* font and text_align don’t really lerp meaningfully */ \
    X(MU_STYLE_HOVER_COLOR,  hover_color, lerp_color) \
    X(MU_STYLE_SCROLL_X,     scroll.x,    lerp_float) \
    X(MU_STYLE_SCROLL_Y,     scroll.y,    lerp_float)

mu_AnimatableOverride mu_interp_animatable(mu_AnimatableOverride initial,
                                           mu_AnimatableOverride target,
                                           float p) {
    mu_AnimatableOverride result = initial;

    #define X(FLAG, FIELD, LERP) \
        if (target.set_flags & FLAG) { \
            result.FIELD = LERP(initial.FIELD, target.FIELD, p); \
        }
    MU_INTERPOLATABLE_FIELDS
    #undef X

    return result;
}

#define MU_APPLY_FIELDS \
    APPLY_FIELD(MU_STYLE_BORDER_COLOR, border_color) \
    APPLY_FIELD(MU_STYLE_BG_COLOR,     bg_color) \
    APPLY_FIELD(MU_STYLE_BORDER_SIZE,  border_size) \
    APPLY_FIELD(MU_STYLE_GAP,          gap) \
    APPLY_FIELD(MU_STYLE_PADDING,      padding) \
    APPLY_FIELD(MU_STYLE_TEXT_COLOR,   text_color) \
    APPLY_FIELD(MU_STYLE_FONT,         font) \
    APPLY_FIELD(MU_STYLE_TEXT_ALIGN,   text_align) \
    APPLY_FIELD(MU_STYLE_HOVER_COLOR,  hover_color) \
    /* focus_color not in override flags? If needed, add here */ \
    APPLY_FIELD(MU_STYLE_SCROLL_X,     scroll.x) \
    APPLY_FIELD(MU_STYLE_SCROLL_Y,     scroll.y)

void mu_apply_override(mu_Animatable *dst, const mu_AnimatableOverride *ovr) {
    #define APPLY_FIELD(FLAG, FIELD) \
        if (ovr->set_flags & FLAG) { \
            dst->FIELD = ovr->FIELD; \
        }
    MU_APPLY_FIELDS
    #undef APPLY_FIELD
}

mu_AnimatableOverride mu_apply_animation(mu_Context *ctx, mu_Elem* elem, mu_Elem* helper){
  for (int i = 0; i < ctx->anim_stack.idx; i++) 
  {
    mu_Anim* it =&ctx->anim_stack.items[i];
    if (it->hash==elem->hash){
      if (it->progress==0){
        it->initial=(mu_AnimatableOverride){it->animable.set_flags,elem->animatable.border_color,elem->animatable.bg_color,elem->animatable.border_size,elem->animatable.gap,elem->animatable.padding,elem->animatable.text_color,elem->animatable.font,elem->animatable.text_align,elem->animatable.hover_color,elem->animatable.focus_color};
        it->prev=it->initial;
      }
      double p = it->progress;
      p = 1 - (1-p) * (1-p); // this is a east out quad tween. we can also use different tweens
      it->prev=mu_interp_animatable(it->initial,it->animable,p);

      switch (it->type)
      {
      case PERMANENT:
        mu_apply_override(&elem->animatable,&it->prev);
        break;
      case TEMPORARY:
        mu_apply_override(&helper->animatable,&it->prev);
        elem=helper;
      }

      return it->prev;

    }
  }
  
  return (mu_AnimatableOverride){};
}


void mu_draw_debug_elems(mu_Context *ctx){
  for (int i = 0; i < ctx->element_stack.idx; i++)
  {
    mu_Elem*elem=&ctx->element_stack.items[i];
    mu_Elem helper=ctx->element_stack.items[i];
    mu_AnimatableOverride newoverride;
    mu_apply_animation(ctx,elem,&helper);

    mu_draw_debug_clip_outline_ex(ctx, elem->rect, elem->clip,elem->animatable.border_color, elem->animatable.border_size);
    if (elem->settings&MU_EL_DEBUG){
      mu_draw_rect(ctx,elem->clip,mu_color(0,0,255,50));
      mu_draw_rect(ctx,intersect_rects(elem->clip,elem->rect),mu_color(0,255,0,50));
    }
    
    if (elem->text.str) {
      mu_draw_text_ex(ctx,elem->animatable.font,elem->text.str,sizeof(elem->text.str),mu_vec2(elem->rect.x,elem->rect.y),elem->animatable.text_color,intersect_rects(elem->clip,elem->rect),elem->rect,elem->animatable.text_align,elem->animatable.padding );
    }
  }
}


void mu_handle_interaction(mu_Context *ctx){
    for (int i = 0; i < ctx->element_stack.idx; i++)
  {
    mu_Elem*elem=&ctx->element_stack.items[i];
    if (ctx->hover == elem->hash) { elem->animatable.border_color = mu_color(0,0,255,255); }
    if (ctx->focus == elem->hash) { 
      elem->animatable.border_color = mu_color(0,255,0,255); 
      if(elem->settings&MU_EL_DRAGGABLE||elem->settings&MU_EL_STUTTER){
        if (ctx->mouse_down== MU_MOUSE_LEFT){
          elem->animatable.scroll.y+=ctx->mouse_delta.y;
          // printf("motion");
        }
      }
    }
  }
}



void mu_handle_animation(mu_Context *ctx) {
  printf("handlig animation\n");
  for (int i = 0; i < ctx->element_stack.idx; i++){
    mu_Elem* elem=&ctx->element_stack.items[i];
    
    if (elem->settings&MU_EL_DRAGGABLE&&(!(ctx->focus == elem->hash))&&(elem->animatable.scroll.x!=0||elem->animatable.scroll.y!=0)) {
      int mov=0;
      if (elem->direction==DIR_X){
        int relativesize= elem->content_size+elem->animatable.padding*2+(elem->tree.count-1)*elem->animatable.gap;
        mov=mu_clamp(elem->animatable.scroll.x,0,elem->rect.w-relativesize);
        mov-=elem->animatable.scroll.x;
        elem->animatable.scroll.x+=mov*0.1;
      } else {
        int relativesize= elem->content_size+elem->animatable.padding*2+(elem->tree.count-1)*elem->animatable.gap;
        mov=mu_clamp(elem->animatable.scroll.y,elem->rect.h-relativesize,0);
        mov-=elem->animatable.scroll.y;
        elem->animatable.scroll.y+=mov*0.2;
      }
    }
    if (elem->settings&MU_EL_STUTTER&&(!(ctx->focus == elem->hash)&&elem->cooldown)) {
      if (elem->direction==DIR_X){
        float mov=100000;
        for (int i = 0; i < elem->tree.count; i++)
        {
          float pos=ctx->element_stack.items[elem->tree.children[i]].rect.x-elem->rect.x;
          pos+=ctx->element_stack.items[elem->tree.children[i]].rect.w/2;
          pos-=elem->rect.w/2;
          mov= mu_fabsmin(pos,mov);
        }
        mov*=0.5;
        if ((int)mov==0) elem->cooldown=0;
        elem->animatable.scroll.x-=(int)mov;
      } else {
        float mov=100000;
        for (int i = 0; i < elem->tree.count; i++)
        {

          float pos=ctx->element_stack.items[elem->tree.children[i]].rect.y-elem->rect.y;
          pos+=ctx->element_stack.items[elem->tree.children[i]].rect.h/2;
          pos-=elem->rect.h/2;
          mov= mu_fabsmin(pos,mov);
        }
        mov*=0.5;

        if ((int)mov==0) elem->cooldown=0;
        elem->animatable.scroll.y-=(int)mov;
      }
    }
  }
}


void mu_animation_update(mu_Context *ctx, int dt) { //dt is delta time
  printf("animstack size  %d\n", ctx->anim_stack.idx);

  for (int i = ctx->anim_stack.idx-1; i >=0 ; i--)
  {
    mu_Anim *it = &ctx->anim_stack.items[i];
    
    it->progress+= (double)dt / it->time; 
    printf("updating animation, anim index %d dt %f, time %f \n",i,(double)dt, it->time);
    if (it->time==0){
      it->progress=1;
    }
    if (it->progress >=1.0){
      printf("anim finished");
      *it = ctx->anim_stack.items[--ctx->anim_stack.idx]; // if the animation is over we copy the last animation slot into the current one
    }

  }
}




void mu_print_debug_tree(mu_Context *ctx){
    for (int i = 0; i < ctx->element_stack.idx; i++)
  {
    mu_Elem*elem=&ctx->element_stack.items[i];
    
    printf("ELEMENT %03d: ID %02d, hASH %d x %03d, y %03d,h %03d, w %03d,  tier %03d,  number of children %d, parent %d", i, elem->idx, elem->hash, elem->rect.x,elem->rect.y,elem->rect.h,elem->rect.w, elem->tier, elem->tree.count,elem->tree.parent);
    printf("chlidren: ");
    for (int i = 0; i < elem->tree.count; i++)
    {
      printf("%d,",elem->tree.children[i]);
    }
    printf(".\n");
  }
}

void mu_add_text_to_elem(mu_Context *ctx,const char* text) {
  mu_Elem* elem= &ctx->element_stack.items[ctx->element_stack.idx-1];
  elem->text.str=text;
}

void mu_animation_set(mu_Context *ctx, void (*anim)(mu_Context *ctx,mu_Elem* elem))
{
  mu_Elem* elem= &ctx->element_stack.items[ctx->element_stack.idx-1];
  anim(ctx,elem);
  
}

void mu_animation_add(mu_Context *ctx,int (*tween)(int* t),
                      int time, 
                      mu_AnimType animtype,
                      mu_AnimatableOverride animable,
                      int hash
                    ){
  for (int i = 0; i < ctx->anim_stack.idx; i++)
  {
    mu_Anim *anim = &ctx->anim_stack.items[i];
    if (anim->hash==hash){
      anim->initial= anim->prev;
      anim->time=time;
      anim->progress=0;
      return;
      // if (animable.set_flags != anim->animable.set_flags){
        
      // }
    }
  }
  if (ctx->anim_stack.idx<MU_ANIMSTACK_SIZE){
    printf("adding animaiton");
    ctx->anim_stack.items[ctx->anim_stack.idx++]=(mu_Anim) {
      animtype,
      animable,
      hash,
      NULL,
      0,
      time,

    };
  }

  
}

/// @brief Begins a new root-level container.
/// @param ctx The MicroUI context.
/// @param cnt A pointer to the container to be set up as a root container.
///
/// This private helper function performs the initial setup for a root-level
/// container like a window. It pushes the container onto both the main container
/// stack and a separate root list for Z-order management. It also queues a
/// special drawing command that allows its contents to be reordered based on
/// Z-index. The function checks if the mouse is over the container and, if it
/// has a higher Z-index than the current hover root, sets it as the new hover
/// root. Finally, it resets the clipping for the container to prevent it from
/// being clipped by any parent containers.
static void begin_root_container(mu_Context *ctx, mu_Container *cnt) {
  push(ctx->container_stack, cnt);
  /* push container to roots list and push head command */
  push(ctx->root_list, cnt);
  cnt->head = push_jump(ctx, NULL);
  /* clipping is reset here in case a root-container is made within
  ** another root-containers's begin/end block; this prevents the inner
  ** root-container being clipped to the outer */
  push(ctx->clip_stack, unclipped_rect);
}

/// @brief Finalizes a root-level container scope.
/// @param ctx The MicroUI context.
///
/// This private helper function is the counterpart to `begin_root_container`.
/// It finalizes the drawing and state management for a root container. It adds
/// a "tail" jump command to the command list and updates the "head" jump
/// command's destination. This mechanism is crucial for enabling the library's
/// Z-order drawing system. The function also restores the clipping and container
/// stacks to their state before `begin_root_container` was called.
static void end_root_container(mu_Context *ctx) {
  /* push tail 'goto' jump command and set head 'skip' command. the final steps
  ** on initing these are done in mu_end() */
  mu_Container *cnt = mu_get_current_container(ctx);
  cnt->tail = push_jump(ctx, NULL);
  cnt->head->jump.dst = ctx->command_list.items + ctx->command_list.idx;
  /* pop base clip rect and container */
  mu_pop_clip_rect(ctx);
  pop_container(ctx);
}


int mu_begin_elem_window_ex(mu_Context *ctx, const char *title, mu_Rect rect, int opt) {
  mu_Rect body;
  mu_Id id = mu_get_id(ctx, title, strlen(title));
  mu_Container *cnt = get_container(ctx, id, opt);
  if (!cnt || !cnt->open) { return 0; }
  push(ctx->id_stack, id);

  if (cnt->rect.w == 0) { cnt->rect = rect; }
  begin_root_container(ctx, cnt);
  rect = body = cnt->rect;
  mu_begin_elem_ex(ctx,cnt->rect.w,cnt->rect.h,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);

  return MU_RES_ACTIVE;
}

/// @brief Ends the current window scope.
/// @param ctx The MicroUI context.
///
/// This function is the required counterpart to `mu_begin_window_ex`. It
/// properly **ends** the **window scope** by restoring the previous clipping
/// rectangle and calling the internal function to finalize the window's
/// state and drawing order. It is crucial to call this function after
/// `mu_begin_window_ex` to ensure correct UI behavior.

void mu_end_elem_window(mu_Context *ctx) {
  mu_end_elem(ctx);
  mu_resize(ctx);
  mu_apply_size(ctx);
  mu_adjust_elem_positions(ctx);
  mu_handle_interaction(ctx);
  mu_handle_animation(ctx);
  mu_draw_debug_elems(ctx);
  mu_animation_update(ctx, ctx->dt);

  // mu_print_debug_tree(ctx);
  end_root_container(ctx);
}
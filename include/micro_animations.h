#ifndef MICROUI_ANIMATIONS_H
#define MICROUI_ANIMATIONS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <micro_flexbox.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cooldown(mu_Context *ctx, mu_Elem* elem) {
      mu_StyleOverride anim;
      int mov=0;
      if (elem->direction==DIR_X){
        int relativesize= elem->content_size+elem->style.padding*2+(elem->tree.count-1)*elem->style.gap;
        mov=mu_clamp(elem->anim_override->scroll.x,0,elem->rect.w-relativesize);
        anim.set_flags=MU_STYLE_SCROLL_X;
        anim.scroll.x=mov;
        mu_animation_add(ctx,0,100,anim,elem->hash);
      } else {
        int relativesize= elem->content_size+elem->style.padding*2+(elem->tree.count-1)*elem->style.gap;
        mov=mu_clamp(elem->anim_override->scroll.y,elem->rect.h-relativesize,0);
        // mov-=elem->anim_override->scroll.y;
        anim.set_flags=MU_STYLE_SCROLL_Y;
        anim.scroll.y=mov;
        mu_animation_add(ctx,0,100,anim,elem->hash);
      }
}

void snaptoclosestchild(mu_Context *ctx, mu_Elem* elem) {
  mu_StyleOverride anim;
  if (elem->direction==DIR_X){
    int mov=100000;
    for (int i = 0; i < elem->tree.count; i++)
    {
      int pos=ctx->element_stack.items[elem->tree.children[i]].rect.x-elem->rect.x;
      pos+=ctx->element_stack.items[elem->tree.children[i]].rect.w/2;
      pos-=elem->rect.w/2;
      mov= (abs(mov) < abs(pos) ? (mov) : (pos));
    }
    anim.set_flags=MU_STYLE_SCROLL_X;
    anim.scroll.x=0;
    mu_animation_add(ctx,0,1000,anim,elem->hash);
  } else {
    int mov=100000;
    for (int i = 0; i < elem->tree.count; i++)
    {
      mu_Elem* child= &ctx->element_stack.items[elem->tree.children[i]];
      
      int pos = (child->rect.y + child->rect.h / 2)
                - (elem->rect.y + elem->rect.h / 2);
      // printf("pos %d\n", pos);
      mov= (abs(pos) < abs(mov) ? (pos) : (mov));
    }
    anim.set_flags=MU_STYLE_SCROLL_Y;
    anim.scroll.y=elem->anim_override->scroll.y-mov;
    // printf("moving to %d\n", anim.scroll.y);

    mu_animation_add(ctx,0,300,anim,elem->hash);
  }
}

void drag(mu_Context *ctx, mu_Elem* elem) {
      mu_StyleOverride anim;
      int mov=0;
      if (ctx->mouse_down== MU_MOUSE_LEFT|| ctx->finger_down) {
        mov=elem->anim_override->scroll.y+ctx->mouse_delta.y+ctx->finger_delta.y;

        // printf("motion\n");

        anim.set_flags=MU_STYLE_SCROLL_Y;
        anim.scroll.y=mov;
        mu_animation_add(ctx,0,0,anim,elem->hash);
      }
    
}

#ifdef __cplusplus
}
#endif


#endif
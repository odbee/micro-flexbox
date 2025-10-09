#ifndef MICROUI_WIDGETS_H
#define MICROUI_WIDGETS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <micro_flexbox.h>
#include <micro_animations.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void mu_scroller(mu_Context *ctx, const char * title, const char *entries[],int size) {
    mu_adjust_style(ctx,
        (mu_StyleOverride){
            .set_flags =   MU_STYLE_TEXT_ALIGN| MU_STYLE_GAP | MU_STYLE_BORDER_SIZE,
            .border_size=1,

            .gap = 5,
            .text_align = 9,

    });
    switch(mu_begin_elem_ex(ctx,0.9,75,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),MU_EL_CLICKABLE|MU_EL_STUTTER)){
        case MU_STATE_UNFOCUSED:
            mu_animation_set(ctx,snaptoclosestchild);
            break;
        case MU_STATE_FOCUSED:
            mu_animation_set(ctx,drag);
            break;

        case MU_STATE_JUSTFOCUSED:
            break;
        case MU_STATE_JUSTHOVERED:
            mu_animation_set(ctx,snaptoclosestchild);
            break;
    }
    mu_add_text_to_elem(ctx,title);
    mu_pop_style(ctx);
    mu_adjust_style(ctx,
        (mu_StyleOverride){
            .set_flags = MU_STYLE_BORDER_SIZE | MU_STYLE_PADDING| MU_STYLE_GAP,
            .border_size=0,
            .gap = 0,
            .padding = 5,
    });

    for (int i = 0; i <size; i++)
    {
        mu_begin_elem_ex(ctx,1,30,DIR_Y,(MU_ALIGN_TOP|MU_ALIGN_LEFT),0);
            mu_add_text_to_elem(ctx,entries[i]);
        mu_end_elem(ctx);
    }
    mu_pop_style(ctx);
    mu_end_elem(ctx);
}



#ifdef __cplusplus
}
#endif


#endif
#include <stdio.h>
#include "../src/hoviz.h"

int main()
{
    hoviz_init("/usr/share/fonts/truetype/freefont/FreeMono.ttf", 20);

    while(!hoviz_should_close())
    {
        hoviz_render_point((vec3){1,1,1}, hoviz_color_red);
        hoviz_render_text((vec2){0,0}, "hello", 5, hoviz_color_green);
        hoviz_flush();
    }
    return 0;
}
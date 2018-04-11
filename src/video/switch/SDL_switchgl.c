/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"
#include "../SDL_sysvideo.h"

#if SDL_VIDEO_DRIVER_SWITCH_GL

#include "SDL_switchgl.h"

#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720

int
SWITCH_GL_LoadLibrary(_THIS, const char *path)
{
    gfxSetMode(GfxMode_LinearDouble);

    return 0;
}

void
SWITCH_GL_UnloadLibrary(_THIS)
{
}

void *
SWITCH_GL_GetProcAddress(_THIS, const char *proc)
{
    return OSMesaGetProcAddress(proc);
}

SDL_GLContext
SWITCH_GL_CreateContext(_THIS, SDL_Window *window)
{
    const GLint z = _this->gl_config.depth_size;
    const GLint stencil = _this->gl_config.stencil_size;
    const GLint accum = 0;

    _this->gl_config.red_size = 8;
    _this->gl_config.green_size = 8;
    _this->gl_config.blue_size = 8;
    _this->gl_config.alpha_size = 8;

    _this->gl_config.accelerated = 1;

    _this->gl_data->context = OSMesaCreateContextExt(OSMESA_RGBA, z, stencil, accum, NULL);
    if (!_this->gl_data->context) {
        SDL_SetError("OSMesaCreateContextExt() failed!\n");
        return 0;
    }

    gfxConfigureResolution((s32) window->w, (s32) window->h);
    _this->gl_data->fb = gfxGetFramebuffer(NULL, NULL);

    if (!OSMesaMakeCurrent(_this->gl_data->context, _this->gl_data->fb, GL_UNSIGNED_BYTE, window->w, window->h)) {
        SDL_SetError("Unable to make GL context current (OSMesaMakeCurrent)");
        return 0;
    }

    window->flags |= SDL_WINDOW_FULLSCREEN;

    return _this->gl_data->context;
}

int
SWITCH_GL_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    if (!OSMesaMakeCurrent(_this->gl_data->context, _this->gl_data->fb, GL_UNSIGNED_BYTE, window->w, window->h)) {
        return SDL_SetError("Unable to make GL context current (OSMesaMakeCurrent)");
    }

    return 0;
}

int
SWITCH_GL_SetSwapInterval(_THIS, int interval)
{
    _this->gl_data->swap = interval;

    return 0;
}

int
SWITCH_GL_GetSwapInterval(_THIS)
{
    return _this->gl_data->swap;
}

int
SWITCH_GL_SwapWindow(_THIS, SDL_Window *window)
{

    glFinish();

    gfxFlushBuffers();
    gfxSwapBuffers();
    if (_this->gl_data->swap) {
        gfxWaitForVsync();
    }

    _this->gl_data->fb = gfxGetFramebuffer(NULL, NULL);

    return 0;
}

void
SWITCH_GL_DeleteContext(_THIS, SDL_GLContext context)
{
    OSMesaDestroyContext(_this->gl_data->context);
}

#endif /* SDL_VIDEO_DRIVER_SWITCH_GL */

/* vi: set ts=4 sw=4 expandtab: */

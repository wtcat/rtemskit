/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** GUIX Component                                                        */
/**                                                                       */
/**   Progress Bar Management (Progress Bar)                              */
/**                                                                       */
/**************************************************************************/

#define GX_SOURCE_CODE

/* Include necessary system files.  */

#include "gx_api.h"
#include "gx_context.h"
#include "gx_widget.h"
#include "gx_canvas.h"
#include "gx_utility.h"
#include "gx_progress_bar.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _gx_progress_bar_draw                               PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Kenneth Maxwell, Microsoft Corporation                              */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service draws the specified progress bar.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    progress_bar                          Progress Bar control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _gx_widget_border_draw                Draw border                   */
/*    _gx_context_pixelmap_get              Retrieve pixelmap image       */
/*    _gx_widget_client_get                 Find the client area of a     */
/*                                            widget                      */
/*    _gx_canvas_pixelmap_tile              Tile a pixelmap               */
/*    _gx_widget_width_get                  Find width of a widget        */
/*    _gx_context_fill_color_set            Set the draw context fill     */
/*                                            color                       */
/*    _gx_widget_children_draw              Draw children widgets         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    GUIX Internal Code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Kenneth Maxwell          Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
VOID  _gx_progress_bar_draw(GX_PROGRESS_BAR *progress_bar)
{
    /* Draw progress bar background. */
    _gx_progress_bar_background_draw(progress_bar);

    /* Draw text on the progress bar widget.  */
    if (progress_bar -> gx_widget_style & GX_STYLE_PROGRESS_TEXT_DRAW)
    {
        _gx_progress_bar_text_draw(progress_bar);
    }

    /* Draw children widgets of progress bar widget.  */
    _gx_widget_children_draw((GX_WIDGET *)progress_bar);
}


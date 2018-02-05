#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

#include "libuvc/libuvc.h"
#include "fl_imgtk.h"

////////////////////////////////////////////////////////////////////////////////

Fl_Window*      window = NULL;
Fl_Box*         boxDisp = NULL;
Fl_RGB_Image*   putImage = NULL;

uvc_context_t*          ctx = NULL;
uvc_device_t*           dev = NULL;
uvc_device_handle_t*    devh = NULL;
uvc_stream_ctrl_t       ctrl;

////////////////////////////////////////////////////////////////////////////////

void cb(uvc_frame_t *frame, void *ptr)
{
    uvc_frame_t* bgr = NULL;
    uvc_error_t ret;

    printf("callback! length = %u, ptr = 0x%X\n", frame->data_bytes, ptr);

    bgr = uvc_allocate_frame(frame->width * frame->height * 3);
    if (!bgr)
    {
        printf("unable to allocate bgr frame!");
        return;
    }

    ret = uvc_any2bgr(frame, bgr);

    if (ret)
    {
        uvc_perror(ret, "uvc_any2bgr");
        uvc_free_frame(bgr);
        return;
    }

    Fl_RGB_Image* aimg = new Fl_RGB_Image( (uchar*)bgr->data, bgr->width, bgr->height, 3 );
    if ( aimg != NULL )
    {
        if ( putImage != NULL )
        {
            fl_imgtk::discard_user_rgb_image( putImage );
        }

        putImage = (Fl_RGB_Image*)aimg->copy();
        fl_imgtk::discard_user_rgb_image( aimg );
    }

    uvc_free_frame(bgr);

    if ( boxDisp != NULL )
    {
        boxDisp->image( putImage );
        window->redraw();
    }
}

void print_formats_iterator( const uvc_format_desc *next )
{
    printf( "Format index : %d\n", next->bFormatIndex );
    printf( "Descriptors : %d\n", next->bNumFrameDescriptors );
    printf( "GUID : " );
    for( int cnt=0; cnt<16; cnt++ )
    {
        char tmpstr[32]={0};
        sprintf( tmpstr, "%X", (uchar)next->guidFormat[cnt] );
    }
    printf( "\n" );
    printf( "FourCC : %s\n", next->fourccFormat );
}

void print_formats( const uvc_format_desc_t* fmt )
{
    if ( fmt != NULL )
    {
        const uvc_format_desc* cur_desc = fmt;

        while( cur_desc == NULL )
        {
            print_formats_iterator( cur_desc );
            cur_desc = cur_desc->next;
        }
    }
}

int init_uvc()
{
    uvc_error_t res;

    res = uvc_init(&ctx, NULL);

    if (res < 0)
    {
        uvc_perror(res, "uvc_init");
        return res;
    }

    puts("UVC initialized");

    res = uvc_find_device( ctx, &dev, 0, 0, NULL);

    if (res < 0)
    {
        uvc_perror(res, "uvc_find_device");
    }
    else
    {
        puts("Device found");

        res = uvc_open(dev, &devh);

        if (res < 0)
        {
            const char* outerrstr = uvc_strerror( res );
            if ( ( boxDisp != NULL ) && ( outerrstr != NULL ) )
            {
                static char errstr[ 1024 ] = { 0 };
                sprintf( errstr,
                         "libuvc error ( %d ) :\n%s",
                         res,
                         outerrstr );
                boxDisp->label( errstr );
            }
            uvc_perror(res, "uvc_open");
        }
        else
        {
            puts("Device opened");

            const uvc_format_desc_t* fmt = uvc_get_format_descs( devh );
            print_formats( fmt );

            uvc_print_diag(devh, stderr);

            res = uvc_get_stream_ctrl_format_size( devh,
                                                   &ctrl,
                                                   UVC_FRAME_FORMAT_YUYV,
                                                   640, 480, 30 );

            uvc_print_stream_ctrl(&ctrl, stderr);

            if (res < 0)
            {
                uvc_perror(res, "get_mode");
            }
            else
            {
                res = uvc_start_streaming(devh, &ctrl, cb, NULL, 0);

                if (res < 0)
                {
                    uvc_perror(res, "start_streaming");
                }
                else
                {
                    puts("Streaming for 10 seconds...");
                    uvc_error_t resAEMODE = uvc_set_ae_mode(devh, 1);
                    uvc_perror(resAEMODE, "set_ae_mode");

                    for ( int i = 1; i <= 10; i++)
                    {
                        /* uvc_error_t resPT = uvc_set_pantilt_abs(devh, i * 20 * 3600, 0); */
                        /* uvc_perror(resPT, "set_pt_abs"); */
                        uvc_error_t resEXP = uvc_set_exposure_abs(devh, 20 + i * 5);
                        uvc_perror(resEXP, "set_exp_abs");

                        Sleep(1);
                    }

                    Sleep(10);
                    uvc_stop_streaming(devh);
                    puts("Done streaming.");
                }
            }

            uvc_close(devh);
            puts("Device closed");
        }

        uvc_unref_device(dev);
    }

    uvc_exit(ctx);
    puts("UVC exited");

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main (int argc, char ** argv)
{
    int retI = 0;

    window = new Fl_Window ( 600,400, "libuvc test" );
    if ( window != NULL )
    {
        boxDisp = new Fl_Box ( 5, 5, 590, 390 );
        if ( boxDisp != NULL )
        {
            boxDisp->box ( FL_DOWN_BOX );
            boxDisp->labelsize (36);
            boxDisp->labelfont (FL_BOLD+FL_ITALIC);
            boxDisp->labeltype (FL_SHADOW_LABEL);
        }

        window->end();
        window->show();

        init_uvc();

        retI = Fl::run();
    }

    return retI;
}

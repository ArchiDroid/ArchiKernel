/*
  Program : Screen Shot

  Author : khlee

  Date : 2010.01.26
*/
/* ==========================================================================*/
#include <linux/module.h>
#include <lg_diag_screen_shot.h>
#include <linux/fcntl.h> 
#include <linux/fs.h>
#include <lg_diagcmd.h>
#include <linux/uaccess.h>
#include <linux/delay.h>  //jihye.ahn
#include <linux/vmalloc.h>
#if 1
#include <linux/fb.h> /* to handle framebuffer ioctls */
#include <linux/ioctl.h>
#include <linux/syscalls.h> //for sys operations
#endif



/* ==========================================================================
===========================================================================*/
/*  jihye.ahn   2010-10-01    convert RGBA8888 to RGB565 */
#define LCD_BUFFER_SIZE LCD_MAIN_WIDTH * LCD_MAIN_HEIGHT * 4
/* ==========================================================================
===========================================================================*/
extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
/*==========================================================================*/
/*==========================================================================*/

lcd_buf_info_type lcd_buf_info;
/* jihye.ahn    2010-11-13    BLT mode enables for capturing video preview*/
extern int blt_mode_enable(void);
extern int blt_mode_disable(void);

void *vmalloc_buf;
void *vmalloc_buftmp;


/*==========================================================================*/
#if 1 // BPP_32
unsigned long convert_32_to_16_bpp(byte* target_buf, byte* src_buf, struct fb_var_screeninfo * fb_varinfo_p)
{
  int x, y, start_pos;
  byte* src_buf_ptr;
  unsigned long dst_cnt = 0;
  uint8 red, green, blue;

  printk(KERN_INFO "%s", __func__);

  //memset((void *)target_buf, 0, LCD_BUFFER_SIZE);
  
  for (y = 0; y < fb_varinfo_p->yres; y++) 
  {
    start_pos = y*fb_varinfo_p->xres*(fb_varinfo_p->bits_per_pixel / 8);
    src_buf_ptr = (byte *)src_buf + start_pos;

    for (x = 0; x < fb_varinfo_p->xres*(fb_varinfo_p->bits_per_pixel / 8); x += (fb_varinfo_p->bits_per_pixel / 8)) 
    {
      red = ((src_buf_ptr[x]>>3)&0x1F);
      green = ((src_buf_ptr[x+1]>>2)&0x3F);
      blue = ((src_buf_ptr[x+2]>>3)&0x1F);
      // ignore alpa (every 4th bit)
      target_buf[dst_cnt++] = ((green&0x07)<<5)|blue;
      target_buf[dst_cnt++] = ((red<<3)&0xF8)|((green>>3)&0x07);					
    }
  }
  printk(KERN_INFO "%s, translated count : %ld", __func__, dst_cnt);

  
  return dst_cnt;
}
#endif
#if 0
/* LGE_S     jihye.ahn   2010-10-01    convert RGBA8888 to RGB565 */
static void to_rgb565(byte* from, u16* to)
{
	int i;
	int r,g,b;
	u16 h;

	for(i=0; i<LCD_BUFFER_SIZE; i+=4){
		r=from[i+3];
		g=from[i+2];
		b=from[i+1];
		
		h=(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3));

		to[i/4]=h;
	
	}
}
/* LGE_E     jihye.ahn   2010-10-01    convert RGBA8888 to RGB565 */
#endif

static void read_framebuffer(byte* pBuf)
{
  struct file *phMscd_Filp = NULL;
  struct fb_var_screeninfo fb_varinfo;
  int fbfd;
  mm_segment_t old_fs=get_fs();
/* jihye.ahn    2010-11-13    BLT mode enables for capturing video preview*/
  //printk("\n jihye.ahn  [%s]\n",__func__);
//  blt_mode_enable();
 // mdelay(100);
  if ((fbfd = sys_open ("/dev/graphics/fb0", O_RDWR, 0)) == -1)
  {
    printk(KERN_ERR "%s Can't open %s\n", __func__, "/dev/graphics/fb0");
    return;
  }
  memset((void *)&fb_varinfo, 0, sizeof(struct fb_var_screeninfo));
  if (sys_ioctl (fbfd, FBIOGET_VSCREENINFO, (long unsigned int)&fb_varinfo) < 0)
  {
    printk(KERN_ERR "%s, ioctl failed\n", __func__);
    return;
  }
  sys_close(fbfd);

  set_fs(get_ds());

  phMscd_Filp = filp_open("/dev/graphics/fb0", O_RDONLY |O_LARGEFILE, 0);

  if( !phMscd_Filp) {
		printk("open fail screen capture \n" );
		return;
  	}

  phMscd_Filp->f_op->read(phMscd_Filp, pBuf, LCD_BUFFER_SIZE, &phMscd_Filp->f_pos);
  filp_close(phMscd_Filp,NULL);

  set_fs(old_fs);
  //to_rgb565(lcd_buf_info.buftmp,(u16 *)lcd_buf_info.buf);
  convert_32_to_16_bpp(lcd_buf_info.buf, pBuf,&fb_varinfo);

/* jihye.ahn    2010-11-13    BLT mode disables*/
  //  blt_mode_disable();

}

/*
int removefile( char const *filename )
{
  char *argv[4] = { NULL, NULL, NULL, NULL };
  char *envp[] = {
		"HOME=/",
		"TERM=linux",
		NULL,
	};
  if ( !filename )
  return -EINVAL;

  argv[0] = "/system/bin/rm";
  argv[1] = "-f";
  argv[2] = (char *)filename;

  return call_usermodehelper( argv[0], argv, envp, 0 );
}
*/

PACK (void *)LGF_ScreenShot (
        PACK (void	*)req_pkt_ptr,	/* pointer to request packet  */
        uint16		pkt_len )		      /* length of request packet   */
{
  diag_screen_shot_type *req_ptr = (diag_screen_shot_type *)req_pkt_ptr;
  diag_screen_shot_type *rsp_ptr = 0;
  int rsp_len;
  
  //printk(KERN_ERR "[screen shot] SubCmd=<%d>\n",req_ptr->lcd_bk_ctrl.sub_cmd_code);

  switch(req_ptr->lcd_bk_ctrl.sub_cmd_code)
  {
    case SCREEN_SHOT_BK_CTRL:
	  break;
    case SCREEN_SHOT_LCD_BUF:
      switch(req_ptr->lcd_buf.seq_flow)
      {
        case SEQ_START:
          rsp_len = sizeof(diag_lcd_get_buf_req_type);
          rsp_ptr = (diag_screen_shot_type *)diagpkt_alloc(DIAG_LGF_SCREEN_SHOT_F, rsp_len - SCREEN_SHOT_PACK_LEN);

		  if (!rsp_ptr)
		  	return 0;
		  	
          rsp_ptr->lcd_buf.seq_flow = SEQ_START;
          //printk(KERN_ERR "[screen shot] start\n");

          //LG_FW khlee - make the Image file in APP
          lcd_buf_info.is_fast_mode = req_ptr->lcd_buf.is_fast_mode;
          lcd_buf_info.full_draw = req_ptr->lcd_buf.full_draw;

          lcd_buf_info.update = TRUE;
          lcd_buf_info.updated = FALSE;
          lcd_buf_info.width = LCD_MAIN_WIDTH;
          lcd_buf_info.height = LCD_MAIN_HEIGHT;
                      
          lcd_buf_info.total_bytes = lcd_buf_info.width * lcd_buf_info.height * 2;
          lcd_buf_info.sended_bytes = 0;

          lcd_buf_info.update = FALSE;
          lcd_buf_info.updated = TRUE;
          rsp_ptr->lcd_buf.sub_cmd_code = SCREEN_SHOT_LCD_BUF;
          rsp_ptr->lcd_buf.ok = TRUE;
          
          rsp_ptr->lcd_buf.is_main_lcd = FALSE;
          rsp_ptr->lcd_buf.x = lcd_buf_info.x;
          rsp_ptr->lcd_buf.y = lcd_buf_info.y;
          rsp_ptr->lcd_buf.width = lcd_buf_info.width;
          rsp_ptr->lcd_buf.height = lcd_buf_info.height;

          vmalloc_buf = vmalloc(LCD_MAIN_WIDTH * LCD_MAIN_HEIGHT * 2);
          vmalloc_buftmp = vmalloc(LCD_MAIN_WIDTH * LCD_MAIN_HEIGHT * 4);
          lcd_buf_info.buf = vmalloc_buf;
          lcd_buf_info.buftmp= vmalloc_buftmp ;
          /*  jihye.ahn   2010-10-01	convert RGBA8888 to RGB565 */
          read_framebuffer(lcd_buf_info.buftmp );    // read file
	      //to_rgb565(lcd_buf_info.buftmp,(u16 *)lcd_buf_info.buf);
          break;
        case SEQ_REGET_BUF:
          lcd_buf_info.sended_bytes = 0;
        case SEQ_GET_BUF:
          if(lcd_buf_info.updated == TRUE)
          {
              //printk(KERN_ERR "[screen shot] getbuf %d  %d\n",(int)lcd_buf_info.total_bytes,(int)lcd_buf_info.sended_bytes );
              rsp_len = sizeof(diag_lcd_get_buf_req_type);
              rsp_ptr = (diag_screen_shot_type *)diagpkt_alloc(DIAG_LGF_SCREEN_SHOT_F, rsp_len);
              rsp_ptr->lcd_buf.is_main_lcd = TRUE;
              rsp_ptr->lcd_buf.x = lcd_buf_info.x;
              rsp_ptr->lcd_buf.y = lcd_buf_info.y;
              rsp_ptr->lcd_buf.width = lcd_buf_info.width;
              rsp_ptr->lcd_buf.height = lcd_buf_info.height;

              rsp_ptr->lcd_buf.total_bytes = lcd_buf_info.total_bytes;
              rsp_ptr->lcd_buf.sended_bytes = lcd_buf_info.sended_bytes;
              rsp_ptr->lcd_buf.packed = FALSE;
              rsp_ptr->lcd_buf.is_fast_mode = lcd_buf_info.is_fast_mode;
              rsp_ptr->lcd_buf.full_draw = lcd_buf_info.full_draw;

              if(lcd_buf_info.total_bytes < SCREEN_SHOT_PACK_LEN)
              {
                        // completed
                  lcd_buf_info.sended_bytes = 0;
                  rsp_ptr->lcd_buf.sended_bytes = lcd_buf_info.sended_bytes;

                  memcpy((void *)&(rsp_ptr->lcd_buf.buf[0]), (void *)&(lcd_buf_info.buf[lcd_buf_info.sended_bytes]), lcd_buf_info.total_bytes);
                  rsp_ptr->lcd_buf.seq_flow = SEQ_GET_BUF_COMPLETED;
                  lcd_buf_info.update = TRUE;
                  lcd_buf_info.updated = FALSE;
                }
              else if(lcd_buf_info.total_bytes <= lcd_buf_info.sended_bytes)
              {
                    // completed
                lcd_buf_info.sended_bytes -= SCREEN_SHOT_PACK_LEN;
                rsp_ptr->lcd_buf.sended_bytes = lcd_buf_info.sended_bytes;

                //printk(KERN_ERR "[screen shot] copy before\n");
                memcpy((void *)&(rsp_ptr->lcd_buf.buf[0]), (void *)&(lcd_buf_info.buf[lcd_buf_info.sended_bytes]), lcd_buf_info.total_bytes - lcd_buf_info.sended_bytes);
                //printk(KERN_ERR "[screen shot] copy after\n");

                rsp_ptr->lcd_buf.seq_flow = SEQ_GET_BUF_COMPLETED;
                lcd_buf_info.update = TRUE;
                lcd_buf_info.updated = FALSE;
              }
              else
              {
                    // getting
                memcpy((void *)&(rsp_ptr->lcd_buf.buf[0]), (void *)&(lcd_buf_info.buf[lcd_buf_info.sended_bytes]), SCREEN_SHOT_PACK_LEN);
                lcd_buf_info.sended_bytes += SCREEN_SHOT_PACK_LEN;
                rsp_ptr->lcd_buf.seq_flow = SEQ_GET_BUF;
              }
            }
            else
            {
              rsp_len = sizeof(diag_lcd_get_buf_req_type);
              rsp_ptr = (diag_screen_shot_type *)diagpkt_alloc(DIAG_LGF_SCREEN_SHOT_F, rsp_len - SCREEN_SHOT_PACK_LEN);

              rsp_ptr->lcd_buf.is_main_lcd = TRUE;
              rsp_ptr->lcd_buf.x = lcd_buf_info.x;
              rsp_ptr->lcd_buf.y = lcd_buf_info.y;
              rsp_ptr->lcd_buf.width = lcd_buf_info.width;
              rsp_ptr->lcd_buf.height = lcd_buf_info.height;

              rsp_ptr->lcd_buf.total_bytes = lcd_buf_info.total_bytes;
              rsp_ptr->lcd_buf.sended_bytes = lcd_buf_info.sended_bytes;
              rsp_ptr->lcd_buf.packed = FALSE;
              rsp_ptr->lcd_buf.is_fast_mode = lcd_buf_info.is_fast_mode;
              rsp_ptr->lcd_buf.full_draw = lcd_buf_info.full_draw;

              rsp_ptr->lcd_buf.seq_flow = SEQ_GET_BUF_SUSPEND;
            }
            rsp_ptr->lcd_buf.sub_cmd_code = SCREEN_SHOT_LCD_BUF;
            rsp_ptr->lcd_buf.ok = TRUE;

 			break;
        case SEQ_STOP:
           rsp_len = sizeof(diag_lcd_get_buf_req_type);
           rsp_ptr = (diag_screen_shot_type *)diagpkt_alloc(DIAG_LGF_SCREEN_SHOT_F, rsp_len - SCREEN_SHOT_PACK_LEN);
           rsp_ptr->lcd_buf.seq_flow = SEQ_STOP;

           lcd_buf_info.update = FALSE;
           lcd_buf_info.updated = FALSE;          
           rsp_ptr->lcd_buf.sub_cmd_code = SCREEN_SHOT_LCD_BUF;
           rsp_ptr->lcd_buf.ok = TRUE;
           
           vfree(vmalloc_buf);
           vfree(vmalloc_buftmp);
           break;		
      }
      break;  		
	}	

  return (rsp_ptr);	
}        

EXPORT_SYMBOL(LGF_ScreenShot);        

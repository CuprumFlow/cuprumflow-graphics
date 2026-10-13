//line drawing accelerator with alpha blending (bresenham algorithm)
//this can be compiled as software or automatically converted to Verilog to target the FPGA
#include "hdl.h"

MODULE line32a(
  bus_master(bus),
  const int16&	dx,
  const int16&	dy,
  const uint32&	rgba, //color to add
  const uint32&	tint, //source color multiply factor
  uint32 *dst_base,
  const int16& dst_xstride, //normally 1, but can run backwards
  const int16& dst_ystride, //pixels to skip for next line (usually the framebuffer width)
  const uint32 *src_base,
  const int16& src_xstride,
  const int16& src_ystride
  )
{

  const uint32* src = src_base;
  uint32 *dst = dst_base;

  int16 dxa =  iabs (dx);
  int16 dya = -iabs (dy);
  int2 sx = dx < 0 ? -1 : 1;
  int2 sy = dy < 0 ? -1 : 1;
  int16 err = dxa + dya; // error value e_xy
  int16 x = 0;
  int16 y = 0;
  int16 e2;

  uint8 r, g, b, a;
  uint9 inv_a;

  uint8 tr, tg, tb, ta;

  union {
    uint32 pix;
    struct { uint32 r:8; uint32 g:8; uint32 b:8; uint32 a:8; } p_rgba;
  };

  union {
    uint32 color; //union allows to access data as bitfields
    struct { uint32 r:8; uint32 g:8; uint32 b:8; uint32 a:8; } s_rgba;
  };

  r = rgba;
  g = rgba>>8;
  b = rgba>>16;
  a = rgba>>24;

  tr = tint;
  tg = tint>>8;
  tb = tint>>16;
  ta = tint>>24;
 
  while(x != dx || y != dy)
  {
    if(tint == 0) //transparent sources skip reads
    {
      *dst = rgba;
    }
    else
    {
		  pix = *src; //read pixel, assumes premultiplied alpha
		  
		  if(tint == 0xFFFFFFFF && rgba == 0)
		  {
		    color = pix; //copy
		  }
		  else
		  {
			  uint9 pix_a = alpha_mul(p_rgba.a, ta+1);
			  inv_a = 256-pix_a;
			  s_rgba.r = alpha_mul(r, inv_a) + alpha_mul(p_rgba.r, tr+1);
			  s_rgba.g = alpha_mul(g, inv_a) + alpha_mul(p_rgba.g, tg+1);
			  s_rgba.b = alpha_mul(b, inv_a) + alpha_mul(p_rgba.b, tb+1);
			  s_rgba.a = alpha_mul(a, inv_a) + pix_a;
		  }
		  *dst = color;
	}

      //these variables needs to be signed
      busaddr_diff_t dst_xincaddr = 0; 
      busaddr_diff_t dst_yincaddr = 0;
      busaddr_diff_t src_xincaddr = 0; 
      busaddr_diff_t src_yincaddr = 0;
      
      e2 = err << 1;

      int16 dxe = 0;
      if (e2 <= dxa) // e_xy+e_y < 0
      {
        dxe = dxa;
        y = y + sy;
        dst_yincaddr = dst_ystride;
        src_yincaddr = src_ystride;
      } 

      int16 dye = 0;
      if (e2 >= dya) // e_xy+e_x > 0
      {
        dye = dya;
        x = x + sx;
        dst_xincaddr = dst_xstride;
        src_xincaddr = src_xstride;
      }
      dst = dst + (dst_xincaddr + dst_yincaddr);
      src = src + (src_xincaddr + src_yincaddr);
      err = err + dxe + dye;
  }
}

typedef struct _wminfo {
  WORD CDECL (*get_mouse_event)(EVNTREC *er);
  WORD CDECL (*write_msg)(void *msg);
  WORD CDECL (*draw_object)(OBJECT *tree,WORD start,WORD depth,
			    WORD cx,WORD cy,WORD cwidth,WORD cheight);
}WMINFO;

typedef struct _wm {
  WORD CDECL   (*init)(WMINFO *wminfo);
  WORD CDECL   (*exit)(void);
  
  void * CDECL (*create)(WORD elements,WORD x,WORD y,WORD width,WORD height);
  WORD CDECL   (*set)(void *win,WORD mode,WORD parm1,WORD parm2,WORD parm3,WORD parm4);
  WORD CDECL   (*delete)(void *win);
  WORD CDECL   (*calc)(WORD dir,WORD elements,
	                  WORD x1,WORD y1,WORD width1,WORD height1,
	                  WORD *x2,WORD *y2,WORD *width2,WORD *height2);
  WORD CDECL   (*draw)(void *win,WORD cx,WORD cy,WORD cwidth,WORD cheight);
  WORD CDECL   (*click_handler)(void *win,WORD id,WORD but_state);
}WM;

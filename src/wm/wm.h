typedef struct _wminfo {
  WORD CDECL (*get_mouse_event)(EVNTREC *er);
  WORD CDECL (*write_msg)(void *msg);
  WORD CDECL (*draw_object)(OBJECT *tree,WORD start,WORD depth,RECT *clip);
}WMINFO;

typedef struct _wm {
  WORD CDECL   (*init)(WMINFO *wminfo);
  WORD CDECL   (*exit)(void);
  
  void * CDECL (*create)(WORD elements,RECT *size);
  WORD CDECL   (*set)(void *win,WORD mode,
		      WORD parm1,WORD parm2,WORD parm3,WORD parm4);
  WORD CDECL   (*delete)(void *win);
  WORD CDECL   (*calc)(WORD dir,WORD elements,RECT *r1,RECT *r2);
  WORD CDECL   (*draw)(void *win,RECT *clip);
  WORD CDECL   (*click_handler)(void *win,WORD id,WORD but_state);
}WM;

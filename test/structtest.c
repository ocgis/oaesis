#include <stdio.h>

#define PRINT_OFFSET(var, attrstr, attr) printf ("%s offset: %d\n", attrstr, (int)&var.attr - (int)&var);

typedef struct {
  short   w1;
  short   w2;
  short   w3;
  char    c1;
  short * p1;
  char    c2;
  char    c3;
} STRUCT_1;

typedef struct {
  short   w1;
  short   w2;
  short   w3;
  char    c1;
  short * p1;
  char    c2;
  char    c3;
} __attribute__ ((packed)) STRUCT_1p;


int main () {
  STRUCT_1 struct_1;
  STRUCT_1p struct_1p;

  printf ("STRUCT_1:\n");
  printf ("size: %d\n", sizeof (STRUCT_1));
  PRINT_OFFSET (struct_1, "w1", w1);
  PRINT_OFFSET (struct_1, "w2", w2);
  PRINT_OFFSET (struct_1, "w3", w3);
  PRINT_OFFSET (struct_1, "c1", c1);
  PRINT_OFFSET (struct_1, "p1", p1);
  PRINT_OFFSET (struct_1, "c2", c2);
  PRINT_OFFSET (struct_1, "c3", c3);

  printf ("STRUCT_1p:\n");
  printf ("size: %d\n", sizeof (STRUCT_1p));
  PRINT_OFFSET (struct_1p, "w1", w1);
  PRINT_OFFSET (struct_1p, "w2", w2);
  PRINT_OFFSET (struct_1p, "w3", w3);
  PRINT_OFFSET (struct_1p, "c1", c1);
  PRINT_OFFSET (struct_1p, "p1", p1);
  PRINT_OFFSET (struct_1p, "c2", c2);
  PRINT_OFFSET (struct_1p, "c3", c3);

  return 0;
}

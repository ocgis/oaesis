#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define TRUE	1

char * lower (char * str) {
  char * tmp = str;
  
  while (*tmp) {
    *tmp = tolower (*tmp);
    tmp++;
  }

  return str;
}


typedef struct treeinfo
{
  long	tree;
  
  char	*name;
  
  struct	treeinfo	*next;
}TREEINFO;

static unsigned char *memory;


char	*stripext(char *instring)
{
  static char	outstring[100];
  
  static int	i;
  
  i = 0;
  
  while((instring[i] != 0) & (instring[i] != '.'))
    {
      outstring[i] = instring[i];
      i++;
    };
  
  outstring[i] = 0;
  
  return(outstring);
};


void	insert(long tree,char *name,TREEINFO	**treeinf)
{
  TREEINFO	*treedum;
  
  treedum = (TREEINFO *)malloc(sizeof(TREEINFO));
  treedum->next = *treeinf;
  *treeinf = treedum;
  treedum->name = (char *)malloc(strlen(name)+1);
  strcpy(treedum->name,name);
  treedum->tree = tree;
};

void	delete(TREEINFO **treeinf)
{
  TREEINFO	*treedum;
  
  while(*treeinf != NULL) {
    free((*treeinf)->name);
    treedum = (*treeinf)->next;
    free(*treeinf);
    *treeinf = treedum;
  };
};

char	*find(long tree,TREEINFO *treeinf)
{
  TREEINFO	*treedum;
  
  treedum = treeinf;
  
  while(treedum != NULL) {
    if(tree == treedum->tree) {
      return(treedum->name);
    };
    
    treedum = treedum->next;
  };
  
  return(NULL);
};

void	printwords(FILE *fput,long mem,long nr)
{
  short	i;
  
  fprintf(fput,"{\n\t");
  
  for(i = 0; i < nr; i++) {
    if(i)
      fprintf(fput,",");
    
    fprintf(fput,"0x%04x",((short *)mem)[i]);
  };
  
  fprintf(fput,"\n}");
};

void	nfsprint(FILE *fput,char *pc)
{
  while (*pc != '\0')
    if ((unsigned int) *pc < 32) {
      fprintf (fput, "\\%u", *pc);
      pc++;
    }
    else {
      fprintf (fput, "%c", *pc);
      pc++;
    };
};		


/*
** Description
** r2c_raw converts an .rsc and .hrd pair of files into .c and .h files
**
** 1999-01-06 CG
*/
int
main (int    argc,
      char * argv[]) {
  char	c;
  char	infile[200];
  char	orgfile[200],outfile[200];
  char	dumstring[200];
  char	type;
  
  FILE	*fput;
  
  int	fpin;
  int	i;
  short number;
  short tree;
  
  
  TREEINFO	*treeindx = NULL;
		
  
  strcpy(orgfile,argv[1]);
  
  strcpy(infile,stripext(orgfile));
  strcat(infile,".hrd");
  fpin = (short)open(infile, 0);
  
  strcpy(outfile,stripext(orgfile));
  
  strcat(outfile,".h");
  
  fput = fopen(outfile,"w");
  
  printf("Converting file %s -> %s\n",infile,outfile);
  
  fprintf(fput,"extern char %s[];\n\n",stripext(orgfile));
  
  read(fpin, dumstring, 8);
  
  read(fpin, &type, 1);
  
  while(type != 6) {
    i = 0;
    
    fprintf(fput,"#define	");
    
    read(fpin, dumstring, 1);
    
    read(fpin, &tree, 2);
    
    read(fpin, &number, 2);
    
    read(fpin, &c, 1);
    
    while(c) {
      dumstring[i] = c;
      i++;
      fprintf(fput,"%c",c);
      read(fpin, &c, 1);
    } while(c);
    
    dumstring[i] = 0;
    
    fprintf(fput,"	");
    
    switch(type) {
    case	0:
    case	1:		
      fprintf(fput,"%d\n",tree);
      lower(dumstring);
      strcat(dumstring,"tad");
      insert(tree,dumstring,&treeindx);
      break;
      
    case	2:	
    case	3:
      fprintf(fput,"%d\n",tree);
      break;
      
    default:
      fprintf(fput,"%d\n",number);
    };
    
    read(fpin, &type, 1);
  };	
  
  fprintf(fput,"\n");
  
  close(fpin);
  fclose(fput);
  
  strcpy(outfile,stripext(orgfile));
  strcat(outfile,".c");
  
  
  strcpy(infile,stripext(orgfile));
  
  strcat(infile,".rsc");
  printf("Converting file %s -> %s\n",infile,outfile);
  
  {
    long size;
    
    fpin = (short)open(infile,0);
    size = lseek (fpin, 0, SEEK_END);
    lseek(fpin, 0, SEEK_SET);
    
    memory = (unsigned char *)malloc(size);
    
    if(memory != 0) {
      long i = 0;
      
      read(fpin, memory, size);
      close (fpin);
      
      fput = fopen(outfile,"w");
      
      fprintf(fput,"char %s[] = {\n",stripext(orgfile));
      
      while(i < size) {
	if((i % 8) == 0) {
	  fprintf(fput,"/* 0x%06lx */ ", i);
	}
	
	fprintf(fput,"0x%02x",memory[i]);
	
	if(i < (size - 1)) {
	  fprintf(fput,", ");
	}
	
	if((i % 8) == 7) {
	  fprintf(fput,"\n");
	}
	
	i++;
      }
      
      fprintf(fput,"\n};\n");
      
      fclose (fput);
      
      free ((void *)memory);
    } else {
      printf("Not enough memory to run!\nPress return to continue.\n");
      getchar();
    } 
  }
  
  delete(&treeindx);

  return 0;
}

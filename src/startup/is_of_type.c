
#include <string.h>

/****************************************************************************
 * is_of_type                                                               *
 *  Tells if a string is in a list of values                                *
 ****************************************************************************/
int                 /*                                                      */
is_of_type(         /*                                                      */
char *ext,          /* Extension to check to see if it is in the list       */
char *extlist ) {   /* Extension list, separator can be [,;:|]              */
/****************************************************************************/
	char	*pos;
	int		index;
	int		i;
	char	char_after,char_before;	/* char after/before the matching chain */
	char	flag_after,flag_before;	/* the char after/before is a separator */
	char	separator[]={',' , ';' , ':' , '|'};	/* Allowed separators   */

	/* We have to be carreful. Example: if the user gives the extension list
	 * ACC;acc;ACX;acx, strstr("AC",extlist) will return OK whereas the extension
	 * AC is not legal ! That's why is strstr says "OK" we still have to
	 * check if the caracter after and before (if possible) is a separator
	 */
	
	pos = strstr( extlist, ext );
	if( pos ) {
		index = (int)(pos-extlist);
		flag_after = ( (index+strlen(ext))==strlen(extlist) );
		flag_before= (index);

		if( !flag_after ) {
			char_after = extlist[index+strlen(ext)];
			for( i=0 ; i<=3 ; i++)
				if( char_after==separator[i] ) {
					flag_after = 1;
					break;
				}
		}
	
		if( !flag_before ) {
			char_before= extlist[index-1];
			for( i=0 ; i<=3 ; i++)
				if( char_before==separator[i] ) {
					flag_before = 1;
					break;
				}
		}

		return (flag_after && flag_before);

	} else return 0;
}

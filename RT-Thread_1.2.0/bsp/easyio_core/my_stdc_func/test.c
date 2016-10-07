#include "my_stdc_func.c"
int main(int argc , char **argv)
{
	char test_str[] = "AAABBB,CCC";
	printf("### count %d\n",get_parma_count(test_str));
	//printf("PARMA %s\n",get_parma(2,test_str));

	
	return 0;
	//
}

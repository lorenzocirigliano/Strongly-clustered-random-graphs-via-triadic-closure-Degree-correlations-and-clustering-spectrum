#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define err_max 1.E-8

int main(int argc, char *argv[]){

  if(argc!=1) {
    fprintf(stderr, "\nInvalid number of arguments in %s.\nSintax must be:\n\t1) program_name\t2) number_of_nodes\t3) gamma\t4) k_min\n\n", argv[0]);
    exit(1);
  }

	char file_name_1[50];
  unsigned int i;
  double x, x_temp, y, err;
  FILE *f1;

  sprintf(file_name_1, "ER_asympt.dat");
  if((f1 = fopen(file_name_1, "w")) == NULL) {
    fprintf(stderr, "Could not open a file. Process interrupted.\n");
    exit(1);
  }

for(y=1; y<1.E60; y*=exp(1.E-2)){
  x=0.1E1;  
  err = 0.1;
  while(err > err_max){
    x_temp = x;
    x = log(y/(x+pow(x,2)));
    err = sqrt(pow(x-x_temp,2));
  }
  fprintf(f1, "%.20lf\t%.20lf\n", y, x);
  }
  fclose(f1);
  return 0;
}

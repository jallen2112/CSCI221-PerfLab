#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <stdint.h>
#include <fstream>
#include "Filter.h"
#include <omp.h>

using namespace std;

#include "rtdsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output, string filtername);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output, filtername);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  }
}

#if defined(__arm__)
static inline unsigned int get_cyclecount (void)
{
 unsigned int value;
 // Read CCNT Register
 asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(value)); 
 return value;
}

static inline void init_perfcounters (int32_t do_reset, int32_t enable_divider)
{
 // in general enable all counters (including cycle counter)
 int32_t value = 1;

 // peform reset: 
 if (do_reset)
 {
   value |= 2;     // reset all counters to zero.
   value |= 4;     // reset cycle counter to zero.
 }

 if (enable_divider)
   value |= 8;     // enable "by 64" divider for CCNT.

 value |= 16;

 // program the performance-counter control-register:
 asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value)); 

 // enable all counters: 
 asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f)); 

 // clear overflows:
 asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
}



#endif



double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output, string filtername)
{
  #if defined(__arm__)
  init_perfcounters (1, 1);
  #endif

  long long cycStart, cycStop;
  double start,stop;
  #if defined(__arm__)
  cycStart = get_cyclecount();
  #else
  cycStart = rdtscll();
  #endif
  output -> width = input -> width;
  output -> height = input -> height;

  int inputWidth = (input -> width) - 1;
  int inputHeight = (input -> height) - 1;
  int t = 3;//filter -> getSize();
  int filterDivisor = filter->getDivisor();

  int filterMatrix[t][t];
  for(int i = 0; i < t; i++)
    for(int j = 0; j < t; j++)
      filterMatrix[i][j] = filter -> get(i, j);

  #pragma omp parallel for
  for(int col = 1; col < inputWidth; ++col) {
    for(int row = 1; row < inputHeight; ++row) {
      for(int plane = 0; plane < 3; ++plane) {

//	int t = 0;

//	output -> color[plane][row][col] = 0;
  int total = 0;
	//for (int j = 0; j < t; ++j) {
   // int COL = col + j - 1;
   /*
  if(filtername == "emboss.filter"){
  int COL = col - 1;
  int a = row-1;
  int b = row+1;
  total = input -> color[plane][a][COL];
  total += input -> color[plane][row][COL];
  total += input -> color[plane][b][COL];
  COL++;
  total += input -> color[plane][a][COL];
  total += input -> color[plane][row][COL];
  total -= input -> color[plane][b][COL];
  COL++;
  total += input -> color[plane][a][COL] * filterMatrix[0][2];
  total += input -> color[plane][row][COL] * filterMatrix[1][2];
  total += input -> color[plane][b][COL] * filterMatrix[2][2];
  }
  */
 // else{
  int COL = col - 1;
  int a = row-1;
  int b = row+1;
  total += input -> color[plane][a][COL] * filterMatrix[0][0];
  total += input -> color[plane][row][COL] * filterMatrix[1][0];
  total += input -> color[plane][b][COL] * filterMatrix[2][0];
  COL++;
  total += input -> color[plane][a][COL] * filterMatrix[0][1];
  total += input -> color[plane][row][COL] * filterMatrix[1][1];
  total += input -> color[plane][b][COL] * filterMatrix[2][1];
  COL++;
  total += input -> color[plane][a][COL] * filterMatrix[0][2];
  total += input -> color[plane][row][COL] * filterMatrix[1][2];
  total += input -> color[plane][b][COL] * filterMatrix[2][2];
//  }
    /*
	  for (int i = 0; i < t; ++i) {	
	    output -> color[plane][row][col]
	      += (input -> color[plane][row + i - 1][COL]  //Could be problem changed to +=
		  * filter -> get(i, j) );
	  }
    */
//	}
	
  output->color[plane][row][col] = total;

  if(filterDivisor != 1){
    output -> color[plane][row][col] =
      output -> color[plane][row][col] / filterDivisor;
  }

  if ( output -> color[plane][row][col]  < 0 ) {
    output -> color[plane][row][col] = 0;
  }

  else if ( output -> color[plane][row][col]  > 255 ) {
    output -> color[plane][row][col] = 255;
  }
      }
    }
  }
  #if defined(__arm__)
  cycStop = get_cyclecount();
  #else
  cycStop = rdtscll();
  #endif

  double diff = cycStop-cycStart;
  #if defined(__arm__)
  diff = diff * 64;
  #endif
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}

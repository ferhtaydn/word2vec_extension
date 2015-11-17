//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

const long long max_size = 2000;         // max length of strings
const long long N = 100;                  // number of closest words that will be shown
const long long max_w = 100;              // max length of vocabulary entries
//const long long N = 40;                  // number of closest words that will be shown
//const long long max_w = 50;              // max length of vocabulary entries

int main(int argc, char **argv) {
  FILE *f, *outFile;
  char st1[max_size];
  char dir_name[max_size], dir_under[max_size];
  char *bestw[N];
  char file_name[max_size], st[100][max_size];
  float dist, len, bestd[N], vec[max_size];
  long long words, size, a, b, c, d, cn, bi[100], arg;
  float *M;
  char *vocab;
  struct stat sta = {0};
  if (argc < 3) {
    printf("Usage: ./distance <FILE> <TARGET> \nwhere FILE contains word projections in the BINARY FORMAT\n    TARGET is the target word");
    return 0;
  }
  
  strcpy(file_name, argv[1]);
  f = fopen(file_name, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return 0;
  }
  fscanf(f, "%lld", &words);
  fscanf(f, "%lld", &size);

  vocab = (char *)malloc((long long)words * max_w * sizeof(char));
  for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
    return 0;
  }

  for (b = 0; b < words; b++) {
    a = 0;

    //init vocab with the next word
    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    //init vocab with the next word - 0: end of string.

    //construct the M
    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);

    //normalize M  
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;

  }

  fclose(f);

  strcpy(dir_name, argv[2]);

  if (stat(dir_name, &sta) == -1) {
	  mkdir(dir_name, 0700);
  }

  for (arg = 2; arg < argc; arg++) {

    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    a = 0;

    strcpy(st1, "");
    //take the entered word(s)
    strcpy(st1, argv[arg]);
    //take the entered word(s)

    strcpy(dir_under, dir_name);
    strcat(dir_under, "/");

    cn = 0;
    b = 0;
    c = 0;

    //split the entered sentence or word
    while (1) {
      st[cn][b] = st1[c];
      b++;
      c++;
      st[cn][b] = 0;
      if (st1[c] == 0) break;
      if (st1[c] == ' ') {
        cn++;
        b = 0;
        c++;
      }
    }
    //split the entered sentence or word

    cn++; //for the loop convention, not necessay. for (a = 0; a <=cn; a++) could be used
    
    for (a = 0; a < cn; a++) {

      //finding the index of input word in vocab
      for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
      if (b == words) b = -1;
      //finding the index of input word in vocab

      bi[a] = b;
      if (b == -1) {
        printf("Out of dictionary word: %s \n", st[0]);
        return 0;
      }
    }

    //if one of the input words is out of dict, take new input!

    for (a = 0; a < size; a++) vec[a] = 0;

    // take the weights vec of words and take sum in vec.
    // input: the of
    // vec[a] is total of the[a] and of[a]
    for (b = 0; b < cn; b++) {
      if (bi[b] == -1) continue;
      for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
    }

    //normalization
    len = 0;
    for (a = 0; a < size; a++) len += vec[a] * vec[a];
    len = sqrt(len);
    for (a = 0; a < size; a++) vec[a] /= len;
    //normalization

    for (a = 0; a < N; a++) bestd[a] = -1;
    for (a = 0; a < N; a++) bestw[a][0] = 0;

    for (c = 0; c < words; c++) {
      a = 0;

      // do not take the word itself
      for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
      if (a == 1) continue;
      // do not take the word itself

      dist = 0;

      //dot product (input o eachWordInDictionary)
      for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];

      //get the max N dist  
      for (a = 0; a < N; a++) {
        if (dist > bestd[a]) {
          for (d = N - 1; d > a; d--) {
            bestd[d] = bestd[d - 1];
            strcpy(bestw[d], bestw[d - 1]);
          }
          bestd[a] = dist;
          strcpy(bestw[a], &vocab[c * max_w]);
          break;
        }
      }
    }

    outFile = fopen(strcat(dir_under, strcat(st1, ".txt")), "w");
    for (a = 0; a < N; a++) fprintf(outFile, "%s,%f\n", bestw[a], bestd[a]);
  }
  return 0;
}

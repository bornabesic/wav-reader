#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <malloc.h>
#include <string.h>

typedef uint8_t byte;
typedef uint16_t byte2;
typedef uint32_t byte4;

typedef struct{
  byte ID[4];
  byte4 size;
} chunk_header;

typedef struct{
  byte format[4];
} RIFF_descriptor;

typedef struct{
  byte2 audio_format;
  byte2 num_channels;
  byte4 sample_rate;
  byte4 byte_rate;
  byte2 block_align;
  byte2 bits_per_sample;
} fmt_chunk;

int main(int argc, char const *argv[]) {
  if(argc<2) return 1;

  const char *file_path = argv[1];
  FILE *file = fopen(file_path, "rb");
  if (file==NULL){
    printf("Cannot open file.\n");
    return 1;
  }

  RIFF_descriptor rd;
  fmt_chunk fmt;
  chunk_header data_header;

  chunk_header header;
  char ID[32+1];
  while(fread(&header, sizeof(chunk_header), 1, file)==1){
    strcpy(ID, header.ID);
    ID[4]=0;

    if(!strcmp("RIFF", ID)){
      fread(&rd, sizeof(RIFF_descriptor), 1, file);
    }
    else if(!strcmp("fmt ", ID)){
      fread(&fmt, sizeof(fmt_chunk), 1, file);

      if(fmt.audio_format!=1){ // non-PCM
        printf("Unsuppored audio format.\n");
        fclose(file);
        return 1;
      }
    }
    else if(!strcmp("data", ID)){
      data_header=header;
      break;
    }
    else{
      fseek(file, header.size, SEEK_CUR);
    }
  }

  if(feof(file)){
    printf("EOF reached. 'data' chunk could not be found.\n");
    fclose(file);
    return 1;
  }

  printf("Format: %.*s\n", 4, rd.format);
  printf("Audio format: %" PRIu16 "\n", fmt.audio_format);
  printf("Number of channels: %" PRIu16 "\n", fmt.num_channels);
  printf("Sample rate: %" PRIu32 "\n", fmt.sample_rate);
  printf("Byte rate: %" PRIu32 "\n", fmt.byte_rate);
  printf("Block align: %" PRIu16 "\n", fmt.block_align);
  printf("Bits per sample: %" PRIu16 "\n", fmt.bits_per_sample);

  int bytes_per_sample = fmt.bits_per_sample / 8;

  void *frame=NULL;
  frame=malloc(bytes_per_sample*fmt.num_channels);

  if(frame==NULL){
    printf("Unable to allocate memory for the frame buffer.\n");
    fclose(file);
    return 1;
  }

  while(fread(frame, bytes_per_sample, fmt.num_channels, file)==fmt.num_channels){

    if(bytes_per_sample==1){ // unsigned integer
      uint8_t *samples = (uint8_t *)frame;
      for(int i=0; i<fmt.num_channels; i++){
        printf("%10" PRIu8, samples[i]);
      }
    }
    else if(bytes_per_sample==2){ // signed integer
      int16_t *samples = (int16_t *)frame;
      for(int i=0; i<fmt.num_channels; i++){
        printf("%10" PRId16, samples[i]);
      }
    }
    // TODO support other sample sizes
    printf("\n");
  }

  free(frame);
  fclose(file);
  return 0;
}

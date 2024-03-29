 
// tutorial01.c
// Code based on a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1

// A small sample program that shows how to use libavformat and libavcodec to
// read video from a file.
//
// Use
//
// gcc -o tutorial01 tutorial01.c -lavformat -lavcodec -lz
//
// to build (assuming libavformat and libavcodec are correctly installed
// your system).
//
// Run using
//
// tutorial01 myvideofile.mpg
//
// to write the first five frames from "myvideofile.mpg" to disk in PPM
// format.

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <stdio.h>

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

int main(int argc, char *argv[]) {
  AVFormatContext *pFormatCtx = NULL;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx = NULL;
  AVCodec         *pCodec = NULL;
  AVFrame         *pFrame = NULL; 
  AVFrame         *pFrameRGB = NULL;
  AVPacket        packet;
  int             frameFinished;
  int             numBytes;
  uint8_t         *buffer= NULL;
  int ret, got_frame;
  
  if(argc < 2) {
    printf("Please provide a movie file\n");
    return -1;
  }
  // Register all formats and codecs
  //  avcodec_register_all();
  av_register_all();
  

  // Open video file
  if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) < 0 ) {
    av_log(NULL, AV_LOG_ERROR,"파일을 열 수 없습니다.\n");
    return -1;
  }

  
    
  // Retrieve stream information
  if((ret = avformat_find_stream_info(pFormatCtx,NULL)) < 0 ) {
    av_log(NULL, AV_LOG_ERROR,"stream 정보을 찾을 수 없습니다.\n");
    return ret; // Couldn't find stream information
  }
  
  
  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, argv[1], 0);
  
  // Find the first video stream
  videoStream = av_find_best_stream(pFormatCtx,AVMEDIA_TYPE_VIDEO,-1,-1,&pCodec,0);

  
  if(videoStream < 0 ) {
    av_log(NULL,AV_LOG_ERROR,"Cannot find a video stream in the input file\n");
    return videoStream;
  }

  
  // Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
  // Find the decoder for the video stream
  /* pCodec=avcodec_find_decoder(pCodecCtx->codec_id); */
  /* if(pCodec==NULL) { */
  /*   av_log(NULL, AV_LOG_ERROR,"지원되지 않는 코덱입니다.\n"); */
  /*   return -1; // Codec not found */
  /* } */
  // Open codec


  if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
    return -1; // Could not open codec
  
  // Allocate video frame
  pFrame=avcodec_alloc_frame();
  
  // Allocate an AVFrame structure
  pFrameRGB=avcodec_alloc_frame();
  if(pFrameRGB==NULL)
    return -1;

  
  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, 
         		      pCodecCtx->height); 
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t)); 
  
  /* // Assign appropriate parts of buffer to image planes in pFrameRGB */
  /* // Note that pFrameRGB is an AVFrame, but AVFrame is a superset */
  /* // of AVPicture */
   avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, 
         	 pCodecCtx->width, pCodecCtx->height); 


  av_init_packet(&packet);
  packet.data = NULL;
  packet.size = 0;

  // Read frames and save first five frames to disk
  i=0;

  
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
      
      // Did we get a video frame?
      if(frameFinished) {
	// Convert the image from its native format to RGB
	av_picture_crop((AVPicture *)pFrameRGB, (AVPicture*)pFrame, 
                        PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	
	// Save the frame to disk
	if(++i<=100)
	  SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, 
		    i);
        if( i >100 )
          break;
      }
    }
    
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);

    
  }
  
  // Free the RGB image
  av_free(buffer);
  
  printf(" av_free ");
  av_free(pFrameRGB);
  
  // Free the YUV frame
  av_free(pFrame);
  
  // Close the codec
  avcodec_close(pCodecCtx);
  
  // Close the video file
  avformat_close_input(&pFormatCtx);
  
  return 0;
}

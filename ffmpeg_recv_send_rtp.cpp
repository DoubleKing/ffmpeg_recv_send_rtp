#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>  
#include <libavutil/mathematics.h>  
#include <libavutil/time.h> 
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/random_seed.h>
#include <libavutil/imgutils.h>
#include "libswscale/swscale.h"
#ifdef __cplusplus
}  /* end extern "C" */
#endif

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include <unistd.h>
#include <iostream>

using namespace cv;
cv::Mat signImage;
cv::Mat backgroundImage;
cv::VideoCapture capture;
//AVFrame 转 cv::mat  
cv::Mat avframeToCvmat(const AVFrame * frame)  
{  
	static int i = 0;
	char filename[128]="";
	int width = frame->width;  
	int height = frame->height;  
	cv::Mat image(height, width, CV_8UC3);  
	int cvLinesizes[1];  
	cvLinesizes[0] = image.step1();  
	SwsContext* conversion = sws_getContext(width, height, (AVPixelFormat) frame->format, width, height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	//SwsContext* conversion = sws_getContext(width, height, (AVPixelFormat) frame->format, width, height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
	sws_scale(conversion, frame->data, frame->linesize, 0, height, &image.data, cvLinesizes);  
	sws_freeContext(conversion);
	
	
	cv::Mat mp4Frame;
	bool ret = capture.read(mp4Frame);
	
	if(ret)
	{
		//绿色背景替换
		cv::Rect rect(620,0,640,1080);
		cv::Mat image_ori = image(rect);
		
		cv::Mat HSV, mask;
		cv::Mat resizeMask,resizeImageOri;
		//绿色HSV颜色分量范围
		int hmin = 35, smin = 43, vmin = 46; 
		int hmax = 77, smax = 255, vmax = 255;
		cvtColor(image_ori, HSV, COLOR_BGR2HSV);   //将原图转换成HSV色彩空间

		//经过inRange API处理，输出一张二值图像，即将前景从绿幕中扣出来啦
		inRange(HSV, Scalar(hmin, smin, vmin), Scalar(hmax, smax, vmax), mask);
		bitwise_not(mask,mask);
		//cv::imwrite("mask.jpg", mask);
		resize(mask, resizeMask, cv::Size(160, 270));
		resize(image_ori, resizeImageOri, cv::Size(160, 270));
		
		cv::Rect roi_rect = cv::Rect(0, 270, resizeImageOri.cols, resizeImageOri.rows);
		resizeImageOri.copyTo(mp4Frame(roi_rect), resizeMask);
		image = mp4Frame;
	}
	else{
		
		cv::Rect rect(640,0,480,1040);
		cv::Mat image_ori = image(rect);
		return image_ori.clone();
		//cv::imwrite("image_ori.jpg", image_ori);
		//return image_ori;
	}

/*	//视频 圆形右下角
	cv::Mat mp4Frame;
	bool ret = capture.read(mp4Frame);
	
	if(ret)
	{
		cv::Mat roi;
		cv::Mat resized_srcImg, resizeRoi;
		cv::Mat resized_mp4Img;

		roi = Mat::zeros(image.size(), CV_8UC1);

		resize(mp4Frame, resized_mp4Img, cv::Size(width, height));
		resize(image, resized_srcImg, cv::Size(180, 320));
		circle(roi, Point(320, 640), 320, CV_RGB(255, 255, 255), -1);//mask建立
		resize(roi, resizeRoi, cv::Size(180, 320));

		cv::Rect roi_rect = cv::Rect(resized_mp4Img.cols-180, resized_mp4Img.rows-320, resizeRoi.cols, resizeRoi.rows);
		resized_srcImg.copyTo(resized_mp4Img(roi_rect), resizeRoi);
		image = resized_mp4Img;
	}
*/	
/*  //长方形左下角
	if(ret)
	{
		//printf("capture.read sucess!!!!!!\n");
		
		
		cv::Mat resized_srcImg;
		cv::Mat resized_mp4Img;
		resize(mp4Frame, resized_mp4Img, cv::Size(width, height));
		resize(image, resized_srcImg, cv::Size(90, 160));
		cv::Rect roi_rect = cv::Rect(resized_mp4Img.cols-90, resized_mp4Img.rows-160, resized_srcImg.cols, resized_srcImg.rows);
		resized_srcImg.copyTo(resized_mp4Img(roi_rect));
		image = resized_mp4Img;
	}
*/	

/*
	if(i < 1200)
	{
		//画线
		//cv::line(image, Point(1, 1), Point(250, 250), Scalar(0, 0, 0), 5, cv::INTER_LINEAR);
		//画矩形框
		cv::rectangle(image, cv::Point(0,840), cv::Point(480,1040), Scalar(180, 255, 255), -1);
		//写文字
		cv::putText(image, "Hello World", cv::Point(100,940), cv::FONT_HERSHEY_SIMPLEX, 2, (0,0,255), 1);
		//图片覆盖
		//cv::Mat roi = cv::imread("./sign.jpg", cv::IMREAD_COLOR);
		//cv::Mat roiB;
		//resize(roi, roiB, cv::Size(480, 200));
		cv::Rect roi_rect = cv::Rect(0, 680, signImage.cols, signImage.rows);
		signImage.copyTo(image(roi_rect));
	}else
	{
		cv::Mat result;
		cv::Mat resized_srcImg;
		resize(backgroundImage, result, cv::Size(width, height));
		resize(image, resized_srcImg, cv::Size(50, 100));
		cv::Rect roi_rect = cv::Rect(width-50, height-100, resized_srcImg.cols, resized_srcImg.rows);
		resized_srcImg.copyTo(backgroundImage(roi_rect));
		image = backgroundImage;
	}
*/

	i++;
	//sprintf(filename,"cv%d.jpg", i++);
	//cv::imwrite(filename, image_ori);
	
    return image;  
}


//cv::Mat 转 AVFrame 
//不能在原视频帧avframe上直接修改，av_frame_is_writable会返回0，表示不可写。
/* AVFrame* cvmatToAvframe(cv::Mat* image, AVFrame * frame){  
	int width = image->cols;  
	int height = image->rows;  
	int cvLinesizes[4];
	memset(cvLinesizes,0,sizeof(cvLinesizes));
	cvLinesizes[0] = image->step1();
	printf("av_frame_is_writable : %d\n",av_frame_is_writable(frame));
	if (frame == NULL){  
		frame = av_frame_alloc();  
		av_image_alloc(frame->data, frame->linesize, width, height, AVPixelFormat::AV_PIX_FMT_YUV420P, 1); 
		frame->format = AVPixelFormat::AV_PIX_FMT_YUV420P;
	}
	
	SwsContext* conversion = sws_getContext(width, height, AVPixelFormat::AV_PIX_FMT_BGR24, width, height, (AVPixelFormat) frame->format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	//SwsContext* conversion = sws_getContext(width, height, AVPixelFormat::AV_PIX_FMT_BGR24, width, height, (AVPixelFormat) frame->format, SWS_BICUBIC, NULL, NULL, NULL);
	sws_scale(conversion, &image->data, cvLinesizes , 0, height, frame->data, frame->linesize);  
	sws_freeContext(conversion);
	return  frame;  
} */

//使用av_image_alloc申请空间，需要用av_freep释放，不能直接用av_frame_free直接释放avframe，否则内存泄漏。
AVFrame* cvmat_to_avframe(const cv::Mat& src, int to_pix_fmt)
{
	uint8_t *src_data[4]/*, *dst_data[4]*/;
	int src_linesize[4]/*, dst_linesize[4]*/;
	int src_w = 320, src_h = 240, dst_w, dst_h;

	enum AVPixelFormat src_pix_fmt = AV_PIX_FMT_BGR24;
	enum AVPixelFormat dst_pix_fmt = (enum AVPixelFormat)to_pix_fmt;
	struct SwsContext *convert_ctx = NULL;

	src_w = dst_w = src.cols;
	src_h = dst_h = src.rows;

	convert_ctx = sws_getContext(src_w, src_h, src_pix_fmt, dst_w, dst_h, (enum AVPixelFormat)dst_pix_fmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	//convert_ctx = sws_getContext(src_w, src_h, src_pix_fmt, dst_w, dst_h, (enum AVPixelFormat)dst_pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
	if (nullptr == convert_ctx) {
		printf("sws_getContext failed!");
		return nullptr;
	}

	av_image_fill_arrays(src_data, src_linesize, src.data, src_pix_fmt, src_w, src_h, 1);
	//cv::imwrite("image_ori.jpg", src);

	AVFrame *dst = av_frame_alloc();
	av_image_alloc(dst->data, dst->linesize, dst_w, dst_h, (enum AVPixelFormat) dst_pix_fmt, 1);

	dst->format = dst_pix_fmt;
	dst->width = dst_w;
	dst->height = dst_h;

	int ret = sws_scale(convert_ctx, src_data, src_linesize, 0, dst_h,
		dst->data, dst->linesize);
	if (ret < 0) {
		printf("sws_scale err\n");
	}

    sws_freeContext(convert_ctx);

    return dst;
}
/* 
int saveAsJPEG(AVFrame* pFrame, int width, int height, int index)
{
	
	char out_file[256] = {0};
	snprintf(out_file, sizeof(out_file), "%s%d.jpg", "ff", index);
	AVFormatContext* pFormatCtx = avformat_alloc_context();
	pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
	if( avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0)
	{
	
		printf("Couldn't open output file.");
		return -1;
	}
	AVStream* pAVStream = avformat_new_stream(pFormatCtx, 0);
	if( pAVStream == NULL )
	{
	
		return -1;
	}
	AVCodecContext* pCodecCtx = pAVStream->codec;
	pCodecCtx->codec_id   = pFormatCtx->oformat->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt    = AV_PIX_FMT_YUVJ420P;
	pCodecCtx->width      = pFrame->width;
	pCodecCtx->height     = pFrame->height;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;
	//打印输出相关信息
	av_dump_format(pFormatCtx, 0, out_file, 1);
	//================================== 查找编码器 ==================================//
	AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if( !pCodec )
	{
	
		printf("Codec not found.");
		return -1;
	}
	if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 )
	{
	
		printf("Could not open codec.");
		return -1;
	}
	//================================Write Header ===============================//
	avformat_write_header(pFormatCtx, NULL);
	int y_size = pCodecCtx->width * pCodecCtx->height;
	AVPacket pkt;
	av_new_packet(&pkt, y_size * 3);

	//
	int got_picture = 0;
	int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
	if( ret < 0 )
	{
	
		printf("Encode Error.\n");
		return -1;
	}
	if( got_picture == 1 )
	{
	
		pkt.stream_index = pAVStream->index;
		ret = av_write_frame(pFormatCtx, &pkt);
	}
	av_free_packet(&pkt);
	av_write_trailer(pFormatCtx);
	if( pAVStream )
	{
	
		avcodec_close(pAVStream->codec);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);
	return 0;
}
 */
int main(int argc, char * argv[])
{
	
	cv::Mat srcSignImage = cv::imread("./sign.jpg", cv::IMREAD_COLOR);
	resize(srcSignImage, signImage, cv::Size(480, 200));
	backgroundImage = cv::imread("./background.jpg", cv::IMREAD_COLOR);
	capture.open("1.mp4");
	if (!capture.isOpened()) {
		printf("cv:open could not open file...\n");
	}

	
	
	
	char					buffer[1280] = {0};
	char					port_v[10] = {0};
	char					port_a[10] = {0};
	char					sdp_file_name[128] = {0};
	char					sdp_file_name_a[128] = {0};
	FILE   				*fp = NULL;
	int ret;
	int i;
	
	AVPacket packet;
	packet.data = NULL;
	packet.size = 0;
	int got_frame;
 
	//初始化输入输出上下文 
	AVFormatContext     *ofmt_ctx_v=NULL;
	AVStream *out_stream = NULL;
	AVCodec  *enc = NULL;
	AVCodecContext *enc_ctx=NULL;
 
	AVFormatContext 	*p_ifmt_ctx_v = avformat_alloc_context();
	AVInputFormat 		*p_ifmt_v = NULL;
	AVCodecContext      *dec_ctx;
	
	p_ifmt_ctx_v ->flags |= AVFMT_NOFILE;
	//添加白名单，这里很重要，如果不申请内存，在avformat_close_input中会宕
	p_ifmt_ctx_v ->protocol_whitelist = (char*)av_malloc(sizeof("file,udp,rtp"));
	memcpy(p_ifmt_ctx_v ->protocol_whitelist,"file,udp,rtp",sizeof("file,udp,rtp"));
 
	//输入的文件格式为sdp
	p_ifmt_v = av_find_input_format("sdp");
 
	//av_register_all();
	//avfilter_register_all();
	avformat_network_init();

	strcpy(sdp_file_name,"video.sdp");
 
	//video.sdp  文件内容
	/*
	v=0
	o=- 0 0 IN IP4 127.0.0.1
	s=No Name
	c=IN IP4 192.168.141.133
	t=0 0
	a=tool:libavformat 58.45.100
	m=video 5004 RTP/AVP 96
	b=AS:768
	a=rtpmap:96 H264/90000
	a=fmtp:96 packetization-mode=1
	*/
 
	if ((ret = avformat_open_input(&p_ifmt_ctx_v , sdp_file_name, p_ifmt_v , NULL)) != 0) 
	{
		printf( "Cannot open p_ifmt_ctx_v file \n");
		return -1;
	}
	if ((ret = avformat_find_stream_info(p_ifmt_ctx_v, NULL)) < 0) 
	{
		printf( "Cannot find ifmt_ctx_v stream information\n");
		
		return -1;
	} 
	for (i = 0; i < p_ifmt_ctx_v->nb_streams; i++) {
		AVStream *stream = p_ifmt_ctx_v->streams[i];
		if(stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
		{
			continue;
		}
		AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
		if (!dec) {
			printf("avcodec_find_decoder error; Failed to find decoder for stream #%u!",i);
			return -1;
		}
		dec_ctx = avcodec_alloc_context3(dec);
		if (!dec_ctx) {
			printf("avcodec_alloc_context3 error; Failed to allocate the decoder context for stream #%u!",i);
			return -1;
		}

		ret = avcodec_parameters_to_context(dec_ctx, stream->codecpar);
		if (ret < 0) {
			printf("avcodec_parameters_to_context error; Failed to copy decoder parameters to input decoder context!");
			return -1;
		}
		dec_ctx->framerate = av_guess_frame_rate(p_ifmt_ctx_v, stream, NULL);

		ret = avcodec_open2(dec_ctx, dec, NULL);
		if (ret < 0) {
			printf("avcodec_open2 error; Failed to open decoder for stream #%u!",i);
			return -1;
		}
	}	
	printf("===========Input Information================\n");
	av_dump_format(p_ifmt_ctx_v, 0, sdp_file_name, 0);	
	printf("============================================\n");
	
	
	avformat_alloc_output_context2(&ofmt_ctx_v, NULL, "rtp", "rtp://192.168.101.144:5004");
	if(!ofmt_ctx_v)
	{
		printf("avformat_alloc_output_context2 error\n");
		return -1;
	}
	out_stream = avformat_new_stream(ofmt_ctx_v, NULL);
	if(!out_stream)
	{
		printf("avformat_new_stream error\n");
		return -1;
	}
	enc = avcodec_find_encoder(AV_CODEC_ID_H264);
	if(!enc)
	{
		printf("avcodec_find_encoder error\n");
		return -1;
	}
	enc_ctx = avcodec_alloc_context3(enc);
	if(!enc_ctx)
	{
		printf("avcodec_alloc_context3 error\n");
		return -1;
	}
	//enc_ctx->height = dec_ctx->height;
	//enc_ctx->width = dec_ctx->width;
	
	enc_ctx->height = 1040;
	enc_ctx->width = 480;
	
	enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
	
	if (enc->pix_fmts)
		enc_ctx->pix_fmt = enc->pix_fmts[0];
	else
		enc_ctx->pix_fmt = dec_ctx->pix_fmt;
	
	enc_ctx->time_base = dec_ctx->time_base;
	enc_ctx->framerate = dec_ctx->framerate;
	
	if (ofmt_ctx_v->oformat->flags & AVFMT_GLOBALHEADER)
		enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	av_opt_set(enc_ctx->priv_data, "slice-max-size", "1450", 0);
	
	enc_ctx->keyint_min = 25;
    enc_ctx->gop_size = 25;
	
	av_opt_set(enc_ctx->priv_data, "x264-params", "cabac=0:ref=1:subme=8:mixed_ref=0:me_range=4:chroma_me=0:trellis=0:threads=2:lookahead_threads=1:sliced_threads=0:slices=1:nr=0:decimate=1:interlaced=0:bluray_compat=0:constrained_intra=0:bframes=0:weightp=1:keyint=25:keyint_min=25:scenecut=0:intra_refresh=0:rc_lookahead=0:rc=cbr:mbtree=0:bitrate=768:ratetol=1.0:qcomp=0.50:qpmin=24:qpmax=42:qpstep=3:vbv_maxrate=768:ratetol=1.0:qcomp=0.50:qpmin=24:qpmax=42:qpstep=3:vbv_maxrate=768:nal_hrd=cbr:filler=1:ip_ratio=1.40", 0);
	//av_opt_set(enc_ctx->priv_data, "x264-params", "bframes=0:bitrate=768:ratetol=1.0:qcomp=0.50:qpmin=24:qpmax=42:qpstep=3:vbv_maxrate=768:ratetol=1.0:qcomp=0.50:qpmin=24:qpmax=42:qpstep=3:vbv_maxrate=768:nal_hrd=cbr:filler=1:ip_ratio=1.40", 0);

	ret = avcodec_open2(enc_ctx, enc, NULL);
	if (ret < 0) {
		printf("avcodec_open2 \n");
		return -1;
	}
	ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
	if (ret < 0) {
		printf("avcodec_parameters_from_context \n");
		return -1;
	}
	out_stream->avg_frame_rate = p_ifmt_ctx_v->streams[0]->avg_frame_rate;
	out_stream->r_frame_rate = p_ifmt_ctx_v->streams[0]->r_frame_rate;
	
	//printf("!!!!!!!!!!!!!!!!!!!enc_ctx framerate = %d\n",channel->enc_ctx->framerate);
	//printf("!!!!!!!!!!!!!!!!!!!enc_ctx time_base = %d/%d\n",channel->enc_ctx->time_base.num,channel->enc_ctx->time_base.den);
	//printf("!!!!!!!!!!!!!!!!!!!dec_ctx time_base = %d/%d\n",channel->dec_ctx->time_base.num,channel->dec_ctx->time_base.den);
	
	if (!(ofmt_ctx_v->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx_v->pb, "rtp://192.168.101.155:5004", AVIO_FLAG_WRITE);
		if (ret < 0) {
			printf("avio_open error \n");
			return -1;
		}
	}
	ret = avformat_write_header(ofmt_ctx_v, NULL);
	if (ret < 0) {
		printf("avformat_write_header error \n");
		return -1;
	}
	
	char sdp[16384];
	av_sdp_create(&ofmt_ctx_v, 1, sdp, sizeof(sdp));
	printf("sdp:\n%s",sdp);
	av_dump_format(ofmt_ctx_v, 0, "rtp://192.168.101.155:5004", 1);
	
	AVPacket enc_pkt;
	
	char error[1024];
	while(1)
	{
		if ((ret = av_read_frame(p_ifmt_ctx_v, &packet)) < 0)
		{
			av_strerror(ret,error,sizeof(error));
			printf("av_read_frame ret=%d  %s\n ",ret,error);
			break;
		}
		//printf("packet \n ");
		AVFrame *frame = av_frame_alloc();
		if (!frame) {
			printf("av_frame_alloc \n ");
			break;
		}
		//av_frame_make_writable(frame);
		ret = avcodec_decode_video2(dec_ctx, frame, &got_frame, &packet);
		if(got_frame)
		{
			static int count = 0;
			//saveAsJPEG(frame, frame->width, frame->height, count);
			//opencv处理后的Mat
			cv::Mat prossed = avframeToCvmat(frame);
			//cvmatToAvframe(&prossed,frame);
			//AVFrame *frame1 = cvmatToAvframe(&prossed,NULL);
			//printf("format=%d\n",frame->format);
			AVFrame *frame1 = cvmat_to_avframe(prossed,frame->format);
			//saveAsJPEG(frame1, frame1->width, frame1->height, 1000+count++);
			frame1->pts = frame->pts;

			ret = avcodec_send_frame(enc_ctx,frame1);
	
			av_freep(&frame1->data[0]);
			av_frame_free(&frame1);

			if (ret < 0)
			{
				printf("Error during avcodec_send_frame ret=%d\n",ret);
				return -1;
			}
			enc_pkt.data = NULL;
			enc_pkt.size = 0;
			av_init_packet(&enc_pkt);
			while (ret >= 0) {
				ret = avcodec_receive_packet(enc_ctx, &enc_pkt);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				{
					break;
				}
				else if (ret < 0) {
					printf("Error during encoding\n");
					break;
				}
				enc_pkt.stream_index = 0;
				//av_packet_rescale_ts(&enc_pkt,enc_ctx->time_base,ofmt_ctx_v->streams[0]->time_base);
				ret = av_interleaved_write_frame(ofmt_ctx_v, &enc_pkt);
				av_packet_unref(&enc_pkt);
			}
			
		}
		
		av_frame_free(&frame);
		av_packet_unref(&packet);
	}
	//TODO 释放资源
	return 0;
}
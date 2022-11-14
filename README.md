# ffmpeg_recv_send_rtp
基于ffmpeg、opencv编辑rtp视频流，示例代码。

# 说明
ffmpeg_recv_send_rtp：作为中间层接收编辑RTP视频流并推流。【接收RTP视频流->解码->编辑(opencv)->编码->发送 RTP视频流】


rtp发送端->ffmpeg_recv_send_rtp(IP地址192.168.141.133)->rtp播放端(IP地址192.168.101.*)


# 发送端命令

ffmpeg     -re     -fflags +genpts     -i 111.mp4     -an     -c:v libx264     -f rtp     -sdp_file video.sdp     "rtp://192.168.141.133:5004"



# 播放端命令

ffplay.exe -protocol_whitelist file,rtp,udp   -i video.sdp

也可以用vlc等工具播放。





# 注意点

1.ffmpeg 输入上下文需要 把rtp、udp添加到白名单，就像ffplay播放rtp视频流命令需要如此写：ffplay.exe -protocol_whitelist file,rtp,udp -i video.sdp 。

2.ffmpeg avframe 与 cv::Mat  相互转换的实现，ffmpeg视频一般使用yuv420p像素格式，opencv使用BGR像素格式，使用ffmpeg sws_scale转换颜色空间。

3.在cv::Mat 转为avframe 时，不能直接使用原avframe，需重新alloc。因为使用av_frame_is_writable返回1时才可编辑帧，否则不可编辑。

4.使用av_image_alloc申请空间，需要用av_freep释放，不能直接用av_frame_free直接释放avframe，否则内存泄漏。

5.cv::Mat 转为avframe 时，注意cv::Mat深浅拷贝。例如：

cv::Rect rect(640,0,480,1040);
cv::Mat image_ori = image(rect);
return image_ori.clone();

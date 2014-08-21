to compile sdk, open "compile_line_sdk.txt" and run the command line script

to compile test utility, open "compile_line_test.txt" and run the command line script

-----------------------------------------------------------------------------------------------------
after compiling test.cpp

run ./test -md find

to find camera ip

For single sensor camera, run ./test -md img -ip camera_ip -o filename.jpg

FOr 8360/8180, run ./test -md imgdef -ip camera_ip

For H.264 models, run ./test -md h264 -ip camera_ip

please read test.cpp for implementation details.


-----------------------------------------------------------------------------------------------------
in case of questions, please create a ticket at http://support.arecontvision.com 
our support team will respond to your questions in a timely manner


-----------------------------------------------------------------------------------------------------
information about I-frame, P-frame settings for H.264 cameras

  int Iframe = 0;

  //Iframe set to 1 will force the camera to return an Intra frame with a corresponding SPS and PPS 
  //as an IDR slice ¨C so that the stream is decodable from this point. 
  //When opening a new stream (for example when changing the image size and/or frame rate) 
  //the Intra frame will be sent  automatically regardless of the input value of Iframe.  
  //To reduce the stream size, reduce the frequency of Iframe = 1 in the requests.  
  //The default number of P-frames for any of the streams sent by the camera is set using 
  //SetAV2000Register with register=21, value=number of P-frames  (but also see intra_period below).  
  //The camera will return an Intra frame even if Iframe in the request is set to 0 
  //when the on-camera counter of P-frames underflows.  
  //To find out whether an Intra frame was received, check the value of Iframe after call getImage... function

-----------------------------------------------------------------------------------------------------

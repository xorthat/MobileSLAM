//
//  ViewController.m
//  MobileLSD
//
//  Created by Guanhang Wu on 11/24/16.
//  Copyright © 2016 Guanhang Wu. All rights reserved.
//

#import "ViewController.h"
#include <boost/thread.hpp>
#include "util/settings.h"
#include "util/globalFuncs.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include "SlamSystem.h"


boost::mutex mutex;

int count = 0;
@interface ViewController (){
    UIImageView *imageView_; // Setup the image view
	UITextView *fpsView_; // Display the current FPS
    UITextView *avgfpsView_; // Display the current FPS
    UITextView *translation_; // Display the current FPS
    int64 curr_time_; // Store the current time
    lsd_slam::SlamSystem* system_;
	float max_fps_;
	float avg_fps_;
    int cam_width;
    int cam_height;
    int runningIdx_;
    int count;
    cv::Mat depthMap;
    cv::Mat displayImage;
    cv::Mat colorImage;
    UIButton *resetButton_;
}
@end

@implementation ViewController

@synthesize videoCamera;

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    cam_width = 352;
    cam_height = 288;
    
    // Take into account size of camera input
    int view_width = self.view.frame.size.width;
    int view_height = (int)(cam_height*self.view.frame.size.width/cam_width)/2;
    int offset = (self.view.frame.size.height - view_height)/2;
    
    imageView_ = [[UIImageView alloc] initWithFrame:CGRectMake(0.0, offset, view_width, view_height)];
    
    //[imageView_ setContentMode:UIViewContentModeScaleAspectFill]; (does not work)
    [self.view addSubview:imageView_]; // Add the view
    
    // Initialize the video camera
    self.videoCamera = [[CvVideoCamera alloc] initWithParentView:imageView_];
    self.videoCamera.delegate = self;
    self.videoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionBack;
    self.videoCamera.defaultAVCaptureVideoOrientation = AVCaptureVideoOrientationLandscapeRight;
    self.videoCamera.defaultFPS = 60; // Set the frame rate
    self.videoCamera.grayscaleMode = YES; // Get grayscale
    self.videoCamera.rotateVideo = YES; // Rotate video so everything looks correct
    
    // Choose these depending on the camera input chosen
        self.videoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset352x288;
//    self.videoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset640x480;
    
    // Finally add the FPS text to the view
    fpsView_ = [[UITextView alloc] initWithFrame:CGRectMake(0,15,view_width,std::max(offset,35))];
    [fpsView_ setOpaque:false]; // Set to be Opaque
    [fpsView_ setBackgroundColor:[UIColor clearColor]]; // Set background color to be clear
    [fpsView_ setTextColor:[UIColor redColor]]; // Set text to be RED
    [fpsView_ setFont:[UIFont systemFontOfSize:18]]; // Set the Font size
    [self.view addSubview:fpsView_];

    avgfpsView_ = [[UITextView alloc] initWithFrame:CGRectMake(165,15,view_width,std::max(offset,35))];
    [avgfpsView_ setOpaque:false]; // Set to be Opaque
    [avgfpsView_ setBackgroundColor:[UIColor clearColor]]; // Set background color to be clear
    [avgfpsView_ setTextColor:[UIColor redColor]]; // Set text to be RED
    [avgfpsView_ setFont:[UIFont systemFontOfSize:18]]; // Set the Font size
    [self.view addSubview:avgfpsView_];

    translation_ = [[UITextView alloc] initWithFrame:CGRectMake(0,60,view_width,std::max(offset,35))];
    [translation_ setOpaque:false]; // Set to be Opaque
    [translation_ setBackgroundColor:[UIColor clearColor]]; // Set background color to be clear
    [translation_ setTextColor:[UIColor redColor]]; // Set text to be RED
    [translation_ setFont:[UIFont systemFontOfSize:18]]; // Set the Font size
    [self.view addSubview:translation_];
    
    resetButton_ = [self simpleButton:@"Reset" buttonColor:[UIColor blackColor]];
    // Important part that connects the action to the member function buttonWasPressed
    [resetButton_ addTarget:self action:@selector(restartWasPressed) forControlEvents:UIControlEventTouchUpInside];

    
    runningIdx_ = 0;
    float fx = 1.1816992757731507e+03;
    float fy = 3.3214250594664935e+02;
    float cx = 0;
    float cy = 0;
    bool doSlam = false;
    Sophus::Matrix3f K;
    K << fx, 0.0, cx, 0.0, fy, cy, 0.0, 0.0, 1.0;
    system_ = new lsd_slam::SlamSystem(cam_width, cam_height, K, doSlam);
    count = 0;
    displayImage = cv::Mat(cam_height, cam_width*2, CV_8UC3);
    depthMap = cv::Mat(cam_height, cam_width, CV_8UC3);
    colorImage = cv::Mat(cam_height, cam_width, CV_8UC3);
    // Finally show the output
    // Do any additional setup after loading the view, typically from a nib.
    
    
    UIView *scaleView = [[UIView alloc] initWithFrame:CGRectMake(self.view.frame.size.width - 320 , 20, 300, 50)];
    
    scaleView.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"scalebar.png"]];
    
    [self.view addSubview:scaleView];
	curr_time_ = cv::getTickCount();
	max_fps_ = 0;
	avg_fps_ = 0;
    [videoCamera start];

}

void increment_count()
{
    boost::unique_lock<boost::mutex> lock(mutex);
    std::cout << "count = " << ++count << std::endl;
}


- (void)restartWasPressed {
    //[videoCamera stop];
    count = 0;
    runningIdx_ = 0;
    //[videoCamera start];
}



- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (UIButton *) simpleButton:(NSString *)buttonName buttonColor:(UIColor *)color
{
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom]; // Initialize the button
    // Bit of a hack, but just positions the button at the bottom of the screen
    int button_width = 200; int button_height = 50; // Set the button height and width (heuristic)
    // Botton position is adaptive as this could run on a different device (iPAD, iPhone, etc.)
    int button_x = (self.view.frame.size.width - button_width)/2; // Position of top-left of button
    int button_y = self.view.frame.size.height - 80; // Position of top-left of button
    button.frame = CGRectMake(button_x, button_y, button_width, button_height); // Position the button
    [button setTitle:buttonName forState:UIControlStateNormal]; // Set the title for the button
    [button setTitleColor:color forState:UIControlStateNormal]; // Set the color for the title
    
    [self.view addSubview:button]; // Important: add the button as a subview
    //[button setEnabled:bflag]; [button setHidden:(!bflag)]; // Set visibility of the button
    return button; // Return the button pointer
}

- (void) processImage:(cv:: Mat &)image
{
    if (count < 20){
        count++;
        return;
    }

    //cv::resize(image, image, cv::Size(cam_width,cam_height));
    //if(runningIdx == 0)
    //{
    //    std::cout<<"index is "<<runningIdx<<std::endl;
    //    system->randomInit(image.data, curr_time_, runningIdx);
    //}
    //else{
    //    try
    //    {
    //        system->trackFrame(image.data, runningIdx , 0, curr_time_);
    //        std::cout<<"index is "<<runningIdx<<std::endl;
    //    }
    //    catch(Sophus::SophusException e){
    //        std::cout << e.what() << std::endl;
    //        return;
    //    }
    //}
    //if (!system->displayMatQueue.empty()){
    //    std::cout<<"queue size is "<<system->displayMatQueue.size()<<std::endl;
    //    int size = system->displayMatQueue.size();
    //    for(int i = 0; i < size; ++i){
    //        depthMap = system->displayMatQueue.front();
    //        system->displayMatQueue.pop();
    //    }
    //}

    if(runningIdx_ == 0 || !system_->trackingIsGood){
        if (runningIdx_ != 0) system_->reinit();
        system_->randomInit(image.data, curr_time_, runningIdx_);
    }
    else{
        std::cout<<"tracking"<<std::endl;
        system_->trackFrame(image.data, runningIdx_,true,curr_time_);
    }

	system_->displayDepImageMutex.lock();	
	depthMap = system_->displayDepImage;
	system_->displayDepImageMutex.unlock();
	auto sim3mat = system_->getSim3Mat();
	auto transmat = sim3mat.translation();
	

    cv::cvtColor(image, colorImage, CV_GRAY2RGB);
    colorImage.copyTo(displayImage(cv::Rect(0, 0, cam_width, cam_height)));
    depthMap.copyTo(displayImage(cv::Rect(cam_width, 0, cam_width, cam_height)));
    image = displayImage;
    runningIdx_++;

    // Finally estimate the frames per second (FPS)
    int64 next_time = cv::getTickCount(); // Get the next time stamp
    float fps = (float)cv::getTickFrequency()/(next_time - curr_time_); // Estimate the fps
	max_fps_ = std::min(59.99f, std::max(fps, max_fps_));
	
	avg_fps_ = avg_fps_ == 0 ? fps : (avg_fps_ * runningIdx_ + fps)/(runningIdx_ + 1);


    curr_time_ = next_time; // Update the time
    NSString *fps_NSStr = [NSString stringWithFormat:@"FPS = %2.2f", fps];
	
    // Have to do this so as to communicate with the main thread
    // to update the text display
    dispatch_sync(dispatch_get_main_queue(), ^{
        fpsView_.text = fps_NSStr;
    });

    fps_NSStr = [NSString stringWithFormat:@"AVG FPS = %2.2f", avg_fps_];
	
    // Have to do this so as to communicate with the main thread
    // to update the text display
    dispatch_sync(dispatch_get_main_queue(), ^{
        avgfpsView_.text = fps_NSStr;
    });

    fps_NSStr = [NSString stringWithFormat:@"Camera Center X:%2.2f  Y:%2.2f  Z:%2.2f",transmat(0),transmat(1),transmat(2)];
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        translation_.text = fps_NSStr;
    });
}

@end

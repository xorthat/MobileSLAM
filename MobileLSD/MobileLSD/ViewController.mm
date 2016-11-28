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
    int64 curr_time_; // Store the current time
    lsd_slam::SlamSystem* system;
    int cam_width;
    int cam_height;
    int runningIdx;
    int count;
}
@end

@implementation ViewController

@synthesize videoCamera;

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    cam_width = 352/2;
    cam_height = 288/2;
    
    // Take into account size of camera input
    int view_width = self.view.frame.size.width;
    int view_height = (int)(cam_height*self.view.frame.size.width/cam_width);
    int offset = (self.view.frame.size.height - view_height)/2;
    
    imageView_ = [[UIImageView alloc] initWithFrame:CGRectMake(0.0, offset, view_width, view_height)];
    
    //[imageView_ setContentMode:UIViewContentModeScaleAspectFill]; (does not work)
    [self.view addSubview:imageView_]; // Add the view
    
    // Initialize the video camera
    self.videoCamera = [[CvVideoCamera alloc] initWithParentView:imageView_];
    self.videoCamera.delegate = self;
    self.videoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionBack;
    self.videoCamera.defaultAVCaptureVideoOrientation = AVCaptureVideoOrientationPortrait;
    self.videoCamera.defaultFPS = 30; // Set the frame rate
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
    
    runningIdx = 0;
    float fx = 3.3;
    float fy = 3.3;
    float cx = 0;
    float cy = 0;
    bool doSlam = false;
    Sophus::Matrix3f K;
    K << fx, 0.0, cx, 0.0, fy, cy, 0.0, 0.0, 1.0;
    system = new lsd_slam::SlamSystem(cam_width, cam_height, K, doSlam);
    count = 0;
    // Finally show the output
    [videoCamera start];
    // Do any additional setup after loading the view, typically from a nib.

}

void increment_count()
{
    boost::unique_lock<boost::mutex> lock(mutex);
    std::cout << "count = " << ++count << std::endl;
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) processImage:(cv:: Mat &)image
{
    if (count < 10){
        count++;
        return;
    }

    cv::resize(image, image, cv::Size(cam_width,cam_height));
    if(runningIdx == 0)
    {
        std::cout<<"index is "<<runningIdx<<std::endl;
        system->randomInit(image.data, curr_time_, runningIdx);
    }
    else{
        system->trackFrame(image.data, runningIdx , 0, curr_time_);
        std::cout<<"index is "<<runningIdx<<std::endl;
    }
    if (!system->displayMatQueue.empty()){
        std::cout<<"queue size is "<<system->displayMatQueue.size()<<std::endl;
        int size = system->displayMatQueue.size();
        for(int i = 0; i < size; ++i){
            image = system->displayMatQueue.front();
            system->displayMatQueue.pop();
        }
    }
    runningIdx++;
    // Finally estimate the frames per second (FPS)
    int64 next_time = cv::getTickCount(); // Get the next time stamp
    float fps = (float)cv::getTickFrequency()/(next_time - curr_time_); // Estimate the fps
    curr_time_ = next_time; // Update the time
    NSString *fps_NSStr = [NSString stringWithFormat:@"FPS = %2.2f",fps];
    
    // Have to do this so as to communicate with the main thread
    // to update the text display
    dispatch_sync(dispatch_get_main_queue(), ^{
        fpsView_.text = fps_NSStr;
    });
    
}

@end

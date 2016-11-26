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
@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    //boost::thread t1;
    //boost::thread_group threads;
    //for (int i = 0; i < 3; ++i)
    //    threads.create_thread(&increment_count);
	float fx = 905;
	float fy = 2144;
	float cx = 540;
	float cy = 960;
	Sophus::Matrix3f K;
	K << fx, 0.0, cx, 0.0, fy, cy, 0.0, 0.0, 1.0;
	float w = 480;
	float h = 640;
	bool doSlam = false;
	lsd_slam::SlamSystem* system = new lsd_slam::SlamSystem(w, h, K, doSlam);
	delete system;

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

@end

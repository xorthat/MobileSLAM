//
//  ViewController.m
//  MobileLSD
//
//  Created by Guanhang Wu on 11/24/16.
//  Copyright © 2016 Guanhang Wu. All rights reserved.
//

#import "ViewController.h"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <iostream>
boost::mutex mutex;
int count = 0;
@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    boost::thread t1;
    boost::thread_group threads;
    for (int i = 0; i < 3; ++i)
        threads.create_thread(&increment_count);
    

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

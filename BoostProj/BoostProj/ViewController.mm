//
//  ViewController.m
//  BoostProj
//
//  Created by Jennifer Lake on 11/23/16.
//  Copyright © 2016 Jennifer Lake. All rights reserved.
//

#import "ViewController.h"
#ifdef __cplusplus
#include <string>
#include <deque>
#include <iostream>
#include <boost/foreach.hpp>
#endif

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    std::deque<int> deque_int;
    deque_int.push_back(1);
    deque_int.push_back(2);
    deque_int.push_back(3);
    
    int i = 0;
    BOOST_FOREACH( i, deque_int )
    {
        std::cout << std::stoi(i) << std::endl);
    }
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end

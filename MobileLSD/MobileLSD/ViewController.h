//
//  ViewController.h
//  MobileLSD
//
//  Created by Guanhang Wu on 11/24/16.
//  Copyright © 2016 Guanhang Wu. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <opencv2/highgui/ios.h>

@interface ViewController : UIViewController<CvVideoCameraDelegate>
{
    CvVideoCamera *videoCamers;
}

@property (nonatomic, retain) CvVideoCamera *videoCamera;


@end


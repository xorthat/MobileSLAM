# MobileSLAM

Title: Direct Visual Odometry on Mobile Device

Team: Jennifer Lake & Guanhang Wu

Summary: Summarize your project in no more than 2-3 sentences. Describe what you plan to do and
what mobile vision problem you will be working on. See 16623.courses.cs.cmu.edu/ideas for
example project ideas.

Background: If your project involves taking advantage of computational speedups available on your iOS
device - such as box filters, inverse composition in the LK algorithm, NEON intrinsics, OpenGL
ES, Accelerate Framework, binary descriptors such as FAST and BRIEF, etc. - describe their
application and why they are necessary in more detail. If your project involves something around
using your device in a mobile fashion - for example virtually rendering an object in your room - then
describe what components of your solution are unique to a mobile device (the high-speed camera,
GPS, IMU, Gyro?).

The Challenge: Describe in a few sentences why the problem is challenging. Could you solve your
problem using just a few pre-existing functions in OpenCV? Try to state explicitly what you are
hoping to learn by doing this project and how is it related to mobile computer vision? A flow chart
or visual depiction of what you are trying to do would be good here. Some questions to consider:
• Does the challenge lie in making an existing vision algorithm more efficient in iOS?
• Are you trying to solve a real-world problem that can only be solved through a combination
of computer vision and mobile computing?

Goals & Deliverables: The goal for this project is to take an existing SLAM implementation and port it over to ios.  
It is not feasible to make this realtime in the time alloted, but rather, we will explore where the algorithm creates bottlenecks
and try to mitigate some of these.  By the end of this project, we should be able to show which parts of the algorithm are creating
bottlenecks, show some performance improvement, and be able to explain how in theory we could make this algorithm realtime.

Schedule: 
Week 1 (November 6th- 12th): Port existing C++ code in xcode project
Week 2 (November 13th - 20th): Performance test existing code, identify bottlenecks and possible solutions (Project checkpoint due November 16th)
Week 3 (November 21st - 27th): Implement optimizations
Week 4 (Novemeber 28th - December 3rd): Performance test optimized code, identify further refinements, and bottlenecks
Week 5 (December 4th - December 10th): Create Presentation and Write Report (Presentation due November 8th, Report due November 11th)

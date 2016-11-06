# MobileSLAM

Title: Direct Visual Odometry on Mobile Device

Team: Jennifer Lake & Guanhang Wu

Summary: For this project, we want to implement direct method visual odemetry algorithm for ios device. Specifically, estimating a Semi\-Dense Depth Map of the scene and reconstructure a 3D Mesh.

Background: Our project will utilize the computation power of the modern CPU on the mobile device. Direct method known for its computational intensive. Paper[1] discribed a way to estimate semi-dense map, which reduced the computation dramatically. As the computation power grows, the visual odometry algorithm could be running in real-time in a modern mobile device. 

The Challenge: There is no direct method implemented for ios device at this moment. We need to implement the tracking, depth map estimation algorithm by ourself. In addition to that, real-time visual odometry needs to be running in a multi-thread environment.

Goals & Deliverables: The goal for this project is to implement the direct visual odometry algorithm on the ios device.
We will try to make the algorithm running in the real-time. If not successed, we will explore where the algorithm creates bottlenecks
and try to mitigate some of these.  By the end of this project, we should be able to show which parts of the algorithm are creating
bottlenecks, show some performance improvement, and be able to explain how in theory we could make this algorithm realtime.
If time allowed, we want to adding virtual object on the 3D surface.

Schedule: 
Week 1 (November 6th- 12th): Port existing C++ code in xcode project
Week 2 (November 13th - 20th): Performance test existing code, identify bottlenecks and possible solutions (Project checkpoint due November 16th)
Week 3 (November 21st - 27th): Implement optimizations
Week 4 (Novemeber 28th - December 3rd): Performance test optimized code, identify further refinements, and bottlenecks
Week 5 (December 4th - December 10th): Create Presentation and Write Report (Presentation due December 8th, Report due December 11th)


[1] T. Schöps, J. Engel, D. Cremers, Semi-Dense Visual Odometry for AR on a Smartphone, In International Symposium on Mixed and Augmented Reality, 2014.

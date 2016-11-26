/**
* This file is part of LSD-SLAM.
*
* Copyright 2013 Jakob Engel <engelj at in dot tum dot de> (Technical University of Munich)
* For more information see <http://vision.in.tum.de/lsdslam> 
*
* LSD-SLAM is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* LSD-SLAM is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with LSD-SLAM. If not, see <http://www.gnu.org/licenses/>.
*/

#include "SlamSystem.h"

#include "DataStructures/Frame.h"
#include "Tracking/SE3Tracker.h"
#include "Tracking/Sim3Tracker.h"
#include "DepthEstimation/DepthMap.h"
#include "Tracking/TrackingReference.h"
//#include "LiveSLAMWrapper.h"
#include "util/globalFuncs.h"
//#include "GlobalMapping/KeyFrameGraph.h"
//#include "GlobalMapping/TrackableKeyFrameSearch.h"
//#include "GlobalMapping/g2oTypeSim3Sophus.h"
//#include "IOWrapper/ImageDisplay.h"
//#include "IOWrapper/Output3DWrapper.h"
//#include <g2o/core/robust_kernel_impl.h>
#include "DataStructures/FrameMemory.h"
#include "deque"

// for mkdir
#include <sys/types.h>
#include <sys/stat.h>

#ifdef ANDROID
#include <android/log.h>
#endif

#include "opencv2/opencv.hpp"

using namespace lsd_slam;


SlamSystem::SlamSystem(int w, int h, Eigen::Matrix3f K, bool enableSLAM)
: SLAMEnabled(enableSLAM), relocalizer(w,h,K)
{
	if(w%16 != 0 || h%16!=0)
	{
		printf("image dimensions must be multiples of 16! Please crop your images / video accordingly.\n");
		assert(false);
	}

	this->width = w;
	this->height = h;
	this->K = K;
	trackingIsGood = true;


	currentKeyFrame =  nullptr;
	trackingReferenceFrameSharedPT = nullptr;
	//keyFrameGraph = new KeyFrameGraph();
	keyFrame_replaceg = nullptr;
	createNewKeyFrame = false;

	map =  new DepthMap(w,h,K);
	
	newConstraintAdded = false;


	tracker = new SE3Tracker(w,h,K);
	// Do not use more than 4 levels for odometry tracking
	for (int level = 4; level < PYRAMID_LEVELS; ++level)
		tracker->settings.maxItsPerLvl[level] = 0;
	trackingReference = new TrackingReference();
	mappingTrackingReference = new TrackingReference();


	if(SLAMEnabled)
	{
		//trackableKeyFrameSearch = new TrackableKeyFrameSearch(keyFrameGraph,w,h,K);
		//constraintTracker = new Sim3Tracker(w,h,K);
		//constraintSE3Tracker = new SE3Tracker(w,h,K);
		//newKFTrackingReference = new TrackingReference();
		//candidateTrackingReference = new TrackingReference();
		constraintSE3Tracker = 0;
		constraintTracker = 0;
		newKFTrackingReference = 0;
		candidateTrackingReference = 0;
	}
	else
	{
		constraintSE3Tracker = 0;
		constraintTracker = 0;
		newKFTrackingReference = 0;
		candidateTrackingReference = 0;
	}



	keepRunning = true;
	doFinalOptimization = false;
	depthMapScreenshotFlag = false;
	lastTrackingClosenessScore = 0;

	thread_mapping = boost::thread(&SlamSystem::mappingThreadLoop, this);

	if(SLAMEnabled)
	{
	}



	msTrackFrame = msOptimizationIteration = msFindConstraintsItaration = msFindReferences = 0;
	nTrackFrame = nOptimizationIteration = nFindConstraintsItaration = nFindReferences = 0;
	nAvgTrackFrame = nAvgOptimizationIteration = nAvgFindConstraintsItaration = nAvgFindReferences = 0;
	gettimeofday(&lastHzUpdate, NULL);

}

SlamSystem::~SlamSystem()
{
	keepRunning = false;

	// make sure none is waiting for something.
	printf("... waiting for SlamSystem's threads to exit\n");
	newFrameMappedSignal.notify_all();
	unmappedTrackedFramesSignal.notify_all();
	newKeyFrameCreatedSignal.notify_all();
	newConstraintCreatedSignal.notify_all();

	thread_mapping.join();
	//thread_constraint_search.join();
	//thread_optimization.join();
	printf("DONE waiting for SlamSystem's threads to exit\n");

	if(constraintTracker != 0) delete constraintTracker;
	if(constraintSE3Tracker != 0) delete constraintSE3Tracker;
	if(newKFTrackingReference != 0) delete newKFTrackingReference;
	if(candidateTrackingReference != 0) delete candidateTrackingReference;

	delete mappingTrackingReference;
	delete map;
	delete trackingReference;
	delete tracker;

	// make shure to reset all shared pointers to all frames before deleting the keyframegraph!
	unmappedTrackedFrames.clear();
	latestFrameTriedForReloc.reset();
	latestTrackedFrame.reset();
	currentKeyFrame.reset();
	trackingReferenceFrameSharedPT.reset();

	// delte keyframe graph
	//delete keyFrameGraph;

	FrameMemory::getInstance().releaseBuffes();


	//Util::closeAllWindows();
}



void SlamSystem::mappingThreadLoop()
{
	printf("Started mapping thread!\n");
	while(keepRunning)
	{
		if (!doMappingIteration())
		{
			boost::unique_lock<boost::mutex> lock(unmappedTrackedFramesMutex);
			unmappedTrackedFramesSignal.timed_wait(lock,boost::posix_time::milliseconds(200));	// slight chance of deadlock otherwise
			lock.unlock();
		}

		newFrameMappedMutex.lock();
		newFrameMappedSignal.notify_all();
		newFrameMappedMutex.unlock();
	}
	printf("Exited mapping thread \n");
}

//void SlamSystem::finalize()
//{
//	return;
//	printf("Finalizing Graph... finding final constraints!!\n");
//
//	lastNumConstraintsAddedOnFullRetrack = 1;
//	while(lastNumConstraintsAddedOnFullRetrack != 0)
//	{
//		doFullReConstraintTrack = true;
//		usleep(200000);
//	}
//
//
//	printf("Finalizing Graph... optimizing!!\n");
//	doFinalOptimization = true;
//	newConstraintMutex.lock();
//	newConstraintAdded = true;
//	newConstraintCreatedSignal.notify_all();
//	newConstraintMutex.unlock();
//	while(doFinalOptimization)
//	{
//		usleep(200000);
//	}
//
//
//	printf("Finalizing Graph... publishing!!\n");
//	unmappedTrackedFramesMutex.lock();
//	unmappedTrackedFramesSignal.notify_one();
//	unmappedTrackedFramesMutex.unlock();
//	while(doFinalOptimization)
//	{
//		usleep(200000);
//	}
//	boost::unique_lock<boost::mutex> lock(newFrameMappedMutex);
//	newFrameMappedSignal.wait(lock);
//	newFrameMappedSignal.wait(lock);
//
//	usleep(200000);
//	printf("Done Finalizing Graph.!!\n");
//}





void SlamSystem::requestDepthMapScreenshot(const std::string& filename)
{
	depthMapScreenshotFilename = filename;
	depthMapScreenshotFlag = true;
}



void SlamSystem::createNewCurrentKeyframe(std::shared_ptr<Frame> newKeyframeCandidate)
{
	if(enablePrintDebugInfo && printThreadingInfo)
		printf("CREATE NEW KF %d from %d\n", newKeyframeCandidate->id(), currentKeyFrame->id());


	if(SLAMEnabled)
	{
		// add NEW keyframe to id-lookup
	}

	// propagate & make new.
	map->createKeyFrame(newKeyframeCandidate.get());

	if(printPropagationStatistics)
	{

		Eigen::Matrix<float, 20, 1> data;
		data.setZero();
		data[0] = runningStats.num_prop_attempts / ((float)width*height);
		data[1] = (runningStats.num_prop_created + runningStats.num_prop_merged) / (float)runningStats.num_prop_attempts;
		data[2] = runningStats.num_prop_removed_colorDiff / (float)runningStats.num_prop_attempts;

	}

	currentKeyFrameMutex.lock();
	currentKeyFrame = newKeyframeCandidate;
	currentKeyFrameMutex.unlock();
}
void SlamSystem::loadNewCurrentKeyframe(Frame* keyframeToLoad)
{
}

void SlamSystem::changeKeyframe(bool noCreate, bool force, float maxScore)
{
	Frame* newReferenceKF=0;
	std::cout<<"change key frame" <<std::endl;
	std::shared_ptr<Frame> newKeyframeCandidate = latestTrackedFrame;

	if(newReferenceKF != 0)
		loadNewCurrentKeyframe(newReferenceKF);
	else
	{
		if(force)
		{
			if(noCreate)
			{
				trackingIsGood = false;
				nextRelocIdx = -1;
				printf("mapping is disabled & moved outside of known map. Starting Relocalizer!\n");
			}
			else
				createNewCurrentKeyframe(newKeyframeCandidate);
		}
	}


	createNewKeyFrame = false;
}

bool SlamSystem::updateKeyframe()
{
	std::shared_ptr<Frame> reference = nullptr;
	std::deque< std::shared_ptr<Frame> > references;

	unmappedTrackedFramesMutex.lock();

	// remove frames that have a different tracking parent.
	while(unmappedTrackedFrames.size() > 0 &&
			(!unmappedTrackedFrames.front()->hasTrackingParent() ||
					unmappedTrackedFrames.front()->getTrackingParent() != currentKeyFrame.get()))
	{
		unmappedTrackedFrames.front()->clear_refPixelWasGood();
		unmappedTrackedFrames.pop_front();
	}

	// clone list
	if(unmappedTrackedFrames.size() > 0)
	{
		for(unsigned int i=0;i<unmappedTrackedFrames.size(); i++)
			references.push_back(unmappedTrackedFrames[i]);

		std::shared_ptr<Frame> popped = unmappedTrackedFrames.front();
		unmappedTrackedFrames.pop_front();
		unmappedTrackedFramesMutex.unlock();

		if(enablePrintDebugInfo && printThreadingInfo)
			printf("MAPPING %d on %d to %d (%d frames)\n", currentKeyFrame->id(), references.front()->id(), references.back()->id(), (int)references.size());

		map->updateKeyframe(references);

		popped->clear_refPixelWasGood();
		references.clear();
	}
	else
	{
		unmappedTrackedFramesMutex.unlock();
		return false;
	}


	if(enablePrintDebugInfo && printRegularizeStatistics)
	{
		Eigen::Matrix<float, 20, 1> data;
		data.setZero();
		data[0] = runningStats.num_reg_created;
		data[2] = runningStats.num_reg_smeared;
		data[3] = runningStats.num_reg_deleted_secondary;
		data[4] = runningStats.num_reg_deleted_occluded;
		data[5] = runningStats.num_reg_blacklisted;

		data[6] = runningStats.num_observe_created;
		data[7] = runningStats.num_observe_create_attempted;
		data[8] = runningStats.num_observe_updated;
		data[9] = runningStats.num_observe_update_attempted;


		data[10] = runningStats.num_observe_good;
		data[11] = runningStats.num_observe_inconsistent;
		data[12] = runningStats.num_observe_notfound;
		data[13] = runningStats.num_observe_skip_oob;
		data[14] = runningStats.num_observe_skip_fail;

	}

	return true;
}




void SlamSystem::debugDisplayDepthMap()
{

	map->debugPlotDepthMap();
	double scale = 1;
	if(currentKeyFrame != 0 && currentKeyFrame != 0)
		scale = currentKeyFrame->getScaledCamToWorld().scale();
	// debug plot depthmap
	char buf1[200];
	char buf2[200];


	snprintf(buf1,200,"Map: Upd %3.0fms (%2.0fHz); Trk %3.0fms (%2.0fHz); %d / %d / %d",
			map->msUpdate, map->nAvgUpdate,
			msTrackFrame, nAvgTrackFrame,
			currentKeyFrame->numFramesTrackedOnThis, currentKeyFrame->numMappedOnThis, (int)unmappedTrackedFrames.size());

	//snprintf(buf2,200,"dens %2.0f%%; good %2.0f%%; scale %2.2f; res %2.1f/; usg %2.0f%%; Map: %d F, %d KF, %d E, %.1fm Pts",
	//		100*currentKeyFrame->numPoints/(float)(width*height),
	//		100*tracking_lastGoodPerBad,
	//		scale,
	//		tracking_lastResidual,
	//		100*tracking_lastUsage,
	//		(int)keyFrameGraph->allFramePoses.size(),
	//		keyFrameGraph->totalVertices,
	//		(int)keyFrameGraph->edgesAll.size(),
	//		1e-6 * (float)keyFrameGraph->totalPoints);


	if(onSceenInfoDisplay)
		printMessageOnCVImage(map->debugImageDepth, buf1, buf2);

	displayMatQueue.push(map->debugImageDepth);
	if (displayDepthMap);
		//Util::displayImage( "DebugWindow DEPTH", map->debugImageDepth, false );

}



bool SlamSystem::doMappingIteration()
{
	if(currentKeyFrame == 0)
		return false;

	if(!doMapping && currentKeyFrame->idxInKeyframes < 0)
	{
		std::cout<<"doMappingIteration"<<std::endl;
		if(currentKeyFrame->numMappedOnThisTotal >= MIN_NUM_MAPPED);
		else;

		map->invalidate();
		printf("Finished KF %d as Mapping got disabled!\n",currentKeyFrame->id());

		changeKeyframe(true, true, 1.0f);
	}


	if(dumpMap)
	{
		dumpMap = false;
	}


	// set mappingFrame
	if(trackingIsGood)
	{
		if(!doMapping)
		{
			//printf("tryToChange refframe, lastScore %f!\n", lastTrackingClosenessScore);
			if(lastTrackingClosenessScore > 1)
				changeKeyframe(true, false, lastTrackingClosenessScore * 0.75);

			if (displayDepthMap || depthMapScreenshotFlag)
				debugDisplayDepthMap();

			return false;
		}


		if (createNewKeyFrame)
		{
			changeKeyframe(false, true, 1.0f);


			if (displayDepthMap || depthMapScreenshotFlag)
				debugDisplayDepthMap();
		}
		else
		{
			bool didSomething = updateKeyframe();

			if (displayDepthMap || depthMapScreenshotFlag)
				debugDisplayDepthMap();
			if(!didSomething)
				return false;
		}

		return true;
	}
	else
	{
		// invalidate map if it was valid.
		if(map->isValid())
		{
			if(currentKeyFrame->numMappedOnThisTotal >= MIN_NUM_MAPPED);
			else;

			map->invalidate();
		}

		// start relocalizer if it isnt running already
		if(!relocalizer.isRunning);

		// did we find a frame to relocalize with?
		if(relocalizer.waitResult(50));


		return true;
	}
}


void SlamSystem::gtDepthInit(uchar* image, float* depth, double timeStamp, int id)
{
	printf("Doing GT initialization!\n");

	currentKeyFrameMutex.lock();

	currentKeyFrame.reset(new Frame(id, width, height, K, timeStamp, image));
	currentKeyFrame->setDepthFromGroundTruth(depth);

	map->initializeFromGTDepth(currentKeyFrame.get());
	keyFrame_replaceg = currentKeyFrame.get();

	currentKeyFrameMutex.unlock();

	printf("Done GT initialization!\n");
}


void SlamSystem::randomInit(uchar* image, double timeStamp, int id)
{
	printf("Doing Random initialization!\n");

	if(!doMapping)
		printf("WARNING: mapping is disabled, but we just initialized... THIS WILL NOT WORK! Set doMapping to true.\n");


	currentKeyFrameMutex.lock();

	currentKeyFrame.reset(new Frame(id, width, height, K, timeStamp, image));
	map->initializeRandomly(currentKeyFrame.get());
	keyFrame_replaceg = currentKeyFrame.get();

	currentKeyFrameMutex.unlock();

	if (displayDepthMap || depthMapScreenshotFlag)
		debugDisplayDepthMap();


	printf("Done Random initialization!\n");

}

void SlamSystem::trackFrame(uchar* image, unsigned int frameID, bool blockUntilMapped, double timestamp)
{
	// Create new frame
	std::shared_ptr<Frame> trackingNewFrame(new Frame(frameID, width, height, K, timestamp, image));

	if(!trackingIsGood)
	{
		relocalizer.updateCurrentFrame(trackingNewFrame);

		unmappedTrackedFramesMutex.lock();
		unmappedTrackedFramesSignal.notify_one();
		unmappedTrackedFramesMutex.unlock();
		return;
	}

	currentKeyFrameMutex.lock();
	bool my_createNewKeyframe = createNewKeyFrame;	// pre-save here, to make decision afterwards.
	if(trackingReference->keyframe != currentKeyFrame.get() || currentKeyFrame->depthHasBeenUpdatedFlag)
	{
		trackingReference->importFrame(currentKeyFrame.get());
		currentKeyFrame->depthHasBeenUpdatedFlag = false;
		trackingReferenceFrameSharedPT = currentKeyFrame;
	}

	FramePoseStruct* trackingReferencePose = trackingReference->keyframe->pose;
	currentKeyFrameMutex.unlock();

	// DO TRACKING & Show tracking result.
	if(enablePrintDebugInfo && printThreadingInfo)
		printf("TRACKING %d on %d\n", trackingNewFrame->id(), trackingReferencePose->frameID);


	poseConsistencyMutex.lock_shared();
	SE3 frameToReference_initialEstimate = se3FromSim3(
			trackingReferencePose->getCamToWorld().inverse() * keyFrame_replaceg->pose ->getCamToWorld());
	poseConsistencyMutex.unlock_shared();



	struct timeval tv_start, tv_end;
	gettimeofday(&tv_start, NULL);

	SE3 newRefToFrame_poseUpdate = tracker->trackFrame(
			trackingReference,
			trackingNewFrame.get(),
			frameToReference_initialEstimate);


	gettimeofday(&tv_end, NULL);
	msTrackFrame = 0.9*msTrackFrame + 0.1*((tv_end.tv_sec-tv_start.tv_sec)*1000.0f + (tv_end.tv_usec-tv_start.tv_usec)/1000.0f);
	nTrackFrame++;

	tracking_lastResidual = tracker->lastResidual;
	tracking_lastUsage = tracker->pointUsage;
	tracking_lastGoodPerBad = tracker->lastGoodCount / (tracker->lastGoodCount + tracker->lastBadCount);
	tracking_lastGoodPerTotal = tracker->lastGoodCount / (trackingNewFrame->width(SE3TRACKING_MIN_LEVEL)*trackingNewFrame->height(SE3TRACKING_MIN_LEVEL));


	if(manualTrackingLossIndicated || tracker->diverged)
	{
		printf("TRACKING LOST for frame %d (%1.2f%% good Points, which is %1.2f%% of available points, %s)!\n",
				trackingNewFrame->id(),
				100*tracking_lastGoodPerTotal,
				100*tracking_lastGoodPerBad,
				tracker->diverged ? "DIVERGED" : "NOT DIVERGED");

		trackingReference->invalidate();

		trackingIsGood = false;
		nextRelocIdx = -1;

		unmappedTrackedFramesMutex.lock();
		unmappedTrackedFramesSignal.notify_one();
		unmappedTrackedFramesMutex.unlock();

		manualTrackingLossIndicated = false;
		return;
	}



	if(plotTracking)
	{
		Eigen::Matrix<float, 20, 1> data;
		data.setZero();
		data[0] = tracker->lastResidual;

		data[3] = tracker->lastGoodCount / (tracker->lastGoodCount + tracker->lastBadCount);
		data[4] = 4*tracker->lastGoodCount / (width*height);
		data[5] = tracker->pointUsage;

		data[6] = tracker->affineEstimation_a;
		data[7] = tracker->affineEstimation_b;
	}

	keyFrame_replaceg = trackingNewFrame.get();




	// Keyframe selection
	latestTrackedFrame = trackingNewFrame;
	if (!my_createNewKeyframe && currentKeyFrame->numMappedOnThisTotal > MIN_NUM_MAPPED)
	{
		Sophus::Vector3d dist = newRefToFrame_poseUpdate.translation() * currentKeyFrame->meanIdepth;
		float  minVal = 0.1f;

		lastTrackingClosenessScore = getRefFrameScore(dist.dot(dist), tracker->pointUsage);

		if (lastTrackingClosenessScore > minVal)
		{
			createNewKeyFrame = true;

		}
		else
		{

		}
	}


	unmappedTrackedFramesMutex.lock();
	if(unmappedTrackedFrames.size() < 50 || (unmappedTrackedFrames.size() < 100 && trackingNewFrame->getTrackingParent()->numMappedOnThisTotal < 10))
		unmappedTrackedFrames.push_back(trackingNewFrame);
	unmappedTrackedFramesSignal.notify_one();
	unmappedTrackedFramesMutex.unlock();

	// implement blocking
	if(blockUntilMapped && trackingIsGood)
	{
		boost::unique_lock<boost::mutex> lock(newFrameMappedMutex);
		while(unmappedTrackedFrames.size() > 0)
		{
			newFrameMappedSignal.wait(lock);
		}
		lock.unlock();
	}
}


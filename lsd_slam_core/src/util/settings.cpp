doSlam = true;
bool doKFReActivation = true;
bool doMapping = true;

int maxLoopClosureCandidates = 10;
int maxOptimizationIterations = 100;
int propagateKeyFrameDepthCount = 0;
float loopclosureStrictness = 1.5;
float relocalizationTH = 0.7;


bool saveKeyframes =  false;
bool saveAllTracked =  false;
bool saveLoopClosureImages =  false;
bool saveAllTrackingStages = false;
bool saveAllTrackingStagesInternal = false;

bool continuousPCOutput = false;


bool fullResetRequested = false;
bool manualTrackingLossIndicated = false;


std::string packagePath = "";


void handleKey(char k)
{
	char kkk = k;
	switch(kkk)
	{
	case 'a': case 'A':
//		autoRun = !autoRun;		// disabled... only use for debugging & if you really, really know what you are doing
		break;
	case 's': case 'S':
//		autoRunWithinFrame = !autoRunWithinFrame; 	// disabled... only use for debugging & if you really, really know what you are doing
		break;
	case 'd': case 'D':
		debugDisplay = (debugDisplay+1)%6;
		printf("debugDisplay is now: %d\n", debugDisplay);
		break;
	case 'e': case 'E':
		debugDisplay = (debugDisplay-1+6)%6;
		printf("debugDisplay is now: %d\n", debugDisplay);
		break;
	case 'o': case 'O':
		onSceenInfoDisplay = !onSceenInfoDisplay;
		break;
	case 'r': case 'R':
		printf("requested full reset!\n");
		fullResetRequested = true;
		break;
	case 'm': case 'M':
		printf("Dumping Map!\n");
		dumpMap = true;
		break;
	case 'p': case 'P':
		printf("Tracking all Map-Frames again!\n");
		doFullReConstraintTrack = true;
		break;
	case 'l': case 'L':
		printf("Manual Tracking Loss Indicated!\n");
		manualTrackingLossIndicated = true;
		break;
	}

}

}
